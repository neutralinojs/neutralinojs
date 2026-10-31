#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "single_instance.h"
#include "server/neuserver.h"
#include "settings.h"
#include "helpers.h"
#include "api/app/app.h"
#include "api/events/events.h"
#include "api/fs/fs.h"
#include "api/window/window.h"
#include "mbedtls/sha256.h"

#if defined(_WIN32)
#include <sddl.h>
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#endif

using namespace std;
using json = nlohmann::json;

namespace single_instance {

namespace {

constexpr uint32_t PROTOCOL_VERSION = 1;
constexpr uint32_t MAX_MESSAGE_SIZE = 1024 * 1024;
constexpr int FORWARD_TIMEOUT_MS = 5000;
constexpr int RETRY_INTERVAL_MS = 50;
constexpr int IO_TIMEOUT_MS = 1000;
constexpr size_t MAX_PENDING_EVENTS = 128;

atomic<bool> running(false);
atomic<bool> appClientConnected(false);
atomic<bool> windowReady(false);
atomic<bool> activationPending(false);
mutex pendingMutex;
vector<json> pendingEvents;
deque<string> recentRequestIds;
unordered_set<string> recentRequestIdSet;
thread listenerThread;

#if defined(_WIN32)
HANDLE instanceMutex = nullptr;
HANDLE pipeHandle = INVALID_HANDLE_VALUE;
wstring pipeName;
#else
int lockFd = -1;
int serverFd = -1;
string socketPath;
#endif

string makeHash(const string &value) {
    unsigned char digest[32];
    mbedtls_sha256(reinterpret_cast<const unsigned char*>(value.data()), value.size(), digest, 0);

    stringstream output;
    output << hex << setfill('0');
    for(size_t i = 0; i < 16; i++) {
        output << setw(2) << static_cast<unsigned int>(digest[i]);
    }
    return output.str();
}

string makeRequestId() {
    auto timestamp = chrono::duration_cast<chrono::microseconds>(
        chrono::system_clock::now().time_since_epoch()).count();
    return to_string(app::getProcessId()) + "-" + to_string(timestamp);
}

json makeRequest(const json &args) {
    return {
        {"version", PROTOCOL_VERSION},
        {"requestId", makeRequestId()},
        {"args", args},
        {"cwd", fs::getCurrentDirectory()}
    };
}

bool validateRequest(const json &request) {
    if(!(request.is_object() &&
        request.value("version", 0u) == PROTOCOL_VERSION &&
        request.contains("requestId") && request["requestId"].is_string() &&
        request.contains("args") && request["args"].is_array() &&
        request.contains("cwd") && request["cwd"].is_string())) {
        return false;
    }
    if(request["requestId"].get_ref<const string&>().size() > 128 ||
            request["cwd"].get_ref<const string&>().size() > 32768 ||
            request["args"].size() > 1024) {
        return false;
    }
    for(const auto &arg: request["args"]) {
        if(!arg.is_string()) {
            return false;
        }
    }
    return true;
}

void activateWindow() {
    if(settings::getMode() != settings::AppModeWindow) {
        return;
    }
    if(!windowReady.load()) {
        activationPending.store(true);
        return;
    }
    window::activate();
}

bool acceptRequest(const json &request) {
    bool scheduleDispatch = false;
    json eventData = {
        {"requestId", request["requestId"]},
        {"args", request["args"]},
        {"cwd", request["cwd"]}
    };
    {
        lock_guard<mutex> guard(pendingMutex);
        string requestId = request["requestId"].get<string>();
        if(recentRequestIdSet.find(requestId) != recentRequestIdSet.end()) {
            return true;
        }
        if(pendingEvents.size() >= MAX_PENDING_EVENTS) {
            return false;
        }
        recentRequestIds.push_back(requestId);
        recentRequestIdSet.insert(requestId);
        if(recentRequestIds.size() > 1024) {
            recentRequestIdSet.erase(recentRequestIds.front());
            recentRequestIds.pop_front();
        }
        pendingEvents.push_back(eventData);
        scheduleDispatch = appClientConnected.load();
    }

    if(scheduleDispatch) {
        neuserver::runOnServerThread([]() {
            single_instance::flushPendingEvents();
        });
    }
    activateWindow();
    return true;
}

#if defined(_WIN32)

bool waitForOverlapped(HANDLE handle, OVERLAPPED &operation, DWORD timeout, DWORD &transferred) {
    DWORD waitResult = WaitForSingleObject(operation.hEvent, timeout);
    if(waitResult != WAIT_OBJECT_0) {
        CancelIoEx(handle, &operation);
        WaitForSingleObject(operation.hEvent, INFINITE);
        return false;
    }
    return GetOverlappedResult(handle, &operation, &transferred, FALSE) == TRUE;
}

bool writeExact(HANDLE handle, const void *buffer, uint32_t size) {
    char *data = const_cast<char*>(static_cast<const char*>(buffer));
    uint32_t writtenTotal = 0;
    while(writtenTotal < size) {
        uint32_t chunkSize = size - writtenTotal;
        OVERLAPPED operation{};
        operation.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if(!operation.hEvent) {
            return false;
        }
        BOOL started = WriteFile(handle, data + writtenTotal, chunkSize, nullptr, &operation);
        DWORD error = started ? ERROR_SUCCESS : GetLastError();
        DWORD written = 0;
        bool success = started
            ? GetOverlappedResult(handle, &operation, &written, FALSE) == TRUE
            : error == ERROR_IO_PENDING &&
                waitForOverlapped(handle, operation, IO_TIMEOUT_MS, written);
        CloseHandle(operation.hEvent);
        if(!success || written == 0) {
            return false;
        }
        writtenTotal += static_cast<uint32_t>(written);
    }
    return true;
}

bool readExact(HANDLE handle, void *buffer, uint32_t size) {
    char *data = static_cast<char*>(buffer);
    uint32_t readTotal = 0;
    while(readTotal < size) {
        uint32_t chunkSize = size - readTotal;
        OVERLAPPED operation{};
        operation.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if(!operation.hEvent) {
            return false;
        }
        BOOL started = ReadFile(handle, data + readTotal, chunkSize, nullptr, &operation);
        DWORD error = started ? ERROR_SUCCESS : GetLastError();
        DWORD count = 0;
        bool success = started
            ? GetOverlappedResult(handle, &operation, &count, FALSE) == TRUE
            : error == ERROR_IO_PENDING &&
                waitForOverlapped(handle, operation, IO_TIMEOUT_MS, count);
        CloseHandle(operation.hEvent);
        if(!success || count == 0) {
            return false;
        }
        readTotal += static_cast<uint32_t>(count);
    }
    return true;
}

bool sendMessage(HANDLE handle, const json &message) {
    string data = helpers::jsonToString(message);
    if(data.size() > MAX_MESSAGE_SIZE) {
        return false;
    }
    uint32_t size = static_cast<uint32_t>(data.size());
    return writeExact(handle, &size, sizeof(size)) && writeExact(handle, data.data(), size);
}

bool receiveMessage(HANDLE handle, json &message) {
    uint32_t size = 0;
    if(!readExact(handle, &size, sizeof(size)) || size == 0 || size > MAX_MESSAGE_SIZE) {
        return false;
    }
    string data(size, '\0');
    if(!readExact(handle, data.data(), size)) {
        return false;
    }
    try {
        message = json::parse(data);
        return true;
    }
    catch(const exception&) {
        return false;
    }
}

bool createSecurityAttributes(SECURITY_ATTRIBUTES &attributes, PSECURITY_DESCRIPTOR &descriptor) {
    HANDLE token = nullptr;
    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        return false;
    }

    DWORD requiredSize = 0;
    GetTokenInformation(token, TokenUser, nullptr, 0, &requiredSize);
    vector<unsigned char> tokenData(requiredSize);
    if(!GetTokenInformation(token, TokenUser, tokenData.data(), requiredSize, &requiredSize)) {
        CloseHandle(token);
        return false;
    }
    CloseHandle(token);

    LPWSTR sidString = nullptr;
    TOKEN_USER *tokenUser = reinterpret_cast<TOKEN_USER*>(tokenData.data());
    if(!ConvertSidToStringSidW(tokenUser->User.Sid, &sidString)) {
        return false;
    }

    wstring sddl = L"D:P(A;;GA;;;SY)(A;;GA;;;" + wstring(sidString) + L")";
    LocalFree(sidString);
    if(!ConvertStringSecurityDescriptorToSecurityDescriptorW(
            sddl.c_str(), SDDL_REVISION_1, &descriptor, nullptr)) {
        return false;
    }

    attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    attributes.lpSecurityDescriptor = descriptor;
    attributes.bInheritHandle = FALSE;
    return true;
}

bool getWindowsIdentity(string &identity) {
    HANDLE token = nullptr;
    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        return false;
    }
    DWORD requiredSize = 0;
    GetTokenInformation(token, TokenUser, nullptr, 0, &requiredSize);
    vector<unsigned char> tokenData(requiredSize);
    if(!GetTokenInformation(token, TokenUser, tokenData.data(), requiredSize, &requiredSize)) {
        CloseHandle(token);
        return false;
    }
    CloseHandle(token);

    LPSTR sidString = nullptr;
    TOKEN_USER *tokenUser = reinterpret_cast<TOKEN_USER*>(tokenData.data());
    if(!ConvertSidToStringSidA(tokenUser->User.Sid, &sidString)) {
        return false;
    }
    identity = sidString;
    LocalFree(sidString);
    return true;
}

void listenLoop() {
    while(running.load()) {
        OVERLAPPED operation{};
        operation.hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if(!operation.hEvent) {
            break;
        }
        BOOL connected = ConnectNamedPipe(pipeHandle, &operation);
        DWORD connectError = connected ? ERROR_SUCCESS : GetLastError();
        if(!connected && connectError == ERROR_IO_PENDING) {
            DWORD transferred = 0;
            connected = waitForOverlapped(pipeHandle, operation, INFINITE, transferred);
        }
        else if(!connected && connectError == ERROR_PIPE_CONNECTED) {
            connected = TRUE;
        }
        CloseHandle(operation.hEvent);
        if(!connected) {
            if(running.load()) {
                this_thread::sleep_for(chrono::milliseconds(RETRY_INTERVAL_MS));
            }
            continue;
        }

        json request;
        json response = {{"version", PROTOCOL_VERSION}, {"accepted", false}};
        if(receiveMessage(pipeHandle, request) && validateRequest(request)) {
            if(acceptRequest(request)) {
                response["accepted"] = true;
                response["requestId"] = request["requestId"];
            }
        }
        sendMessage(pipeHandle, response);
        uint8_t confirmation = 0;
        readExact(pipeHandle, &confirmation, sizeof(confirmation));
        DisconnectNamedPipe(pipeHandle);
    }
}

bool createServerPipe(SECURITY_ATTRIBUTES *attributes, string &error) {
    pipeHandle = CreateNamedPipeW(
        pipeName.c_str(),
        PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
        1,
        MAX_MESSAGE_SIZE,
        MAX_MESSAGE_SIZE,
        0,
        attributes);
    if(pipeHandle == INVALID_HANDLE_VALUE) {
        error = "Unable to create the single-instance named pipe (Windows error " +
            to_string(GetLastError()) + ")";
        return false;
    }
    running.store(true);
    listenerThread = thread(listenLoop);
    return true;
}

StartResult forwardToPrimary(const json &request, string &error) {
    auto deadline = chrono::steady_clock::now() + chrono::milliseconds(FORWARD_TIMEOUT_MS);
    while(chrono::steady_clock::now() < deadline) {
        DWORD waitResult = WaitForSingleObject(instanceMutex, 0);
        if(waitResult == WAIT_OBJECT_0 || waitResult == WAIT_ABANDONED) {
            return StartResult::Primary;
        }

        HANDLE client = CreateFileW(pipeName.c_str(), GENERIC_READ | GENERIC_WRITE,
            0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
        if(client != INVALID_HANDLE_VALUE) {
            ULONG serverPid = 0;
            if(GetNamedPipeServerProcessId(client, &serverPid)) {
                AllowSetForegroundWindow(serverPid);
            }

            json response;
            bool exchanged = sendMessage(client, request) && receiveMessage(client, response);
            if(exchanged) {
                uint8_t confirmation = 1;
                writeExact(client, &confirmation, sizeof(confirmation));
            }
            CloseHandle(client);
            if(exchanged) {
                if(response.value("version", 0u) == PROTOCOL_VERSION &&
                        response.value("accepted", false) &&
                        response.value("requestId", "") == request.value("requestId", "")) {
                    return StartResult::Forwarded;
                }
                error = "The primary instance rejected the launch request";
                return StartResult::Error;
            }
            this_thread::sleep_for(chrono::milliseconds(RETRY_INTERVAL_MS));
            continue;
        }

        DWORD lastError = GetLastError();
        if(lastError != ERROR_PIPE_BUSY && lastError != ERROR_FILE_NOT_FOUND) {
            error = "Unable to connect to the primary instance (Windows error " +
                to_string(lastError) + ")";
            return StartResult::Error;
        }
        WaitNamedPipeW(pipeName.c_str(), RETRY_INTERVAL_MS);
    }

    error = "Timed out while forwarding the launch request to the primary instance";
    return StartResult::Error;
}

#else

uint32_t hostToNetwork(uint32_t value) {
    return htonl(value);
}

uint32_t networkToHost(uint32_t value) {
    return ntohl(value);
}

void configureSocket(int fd) {
    timeval timeout{};
    timeout.tv_sec = IO_TIMEOUT_MS / 1000;
    timeout.tv_usec = (IO_TIMEOUT_MS % 1000) * 1000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    #if defined(SO_NOSIGPIPE)
    int noSigPipe = 1;
    setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &noSigPipe, sizeof(noSigPipe));
    #endif
}

bool ensureRuntimeDirectory(const string &path, string &error) {
    struct stat info{};
    if(lstat(path.c_str(), &info) != 0) {
        if(errno != ENOENT) {
            error = "Unable to inspect the single-instance runtime directory";
            return false;
        }
        if(mkdir(path.c_str(), S_IRWXU) != 0 && errno != EEXIST) {
            error = "Unable to create the single-instance runtime directory";
            return false;
        }
        if(lstat(path.c_str(), &info) != 0) {
            error = "Unable to inspect the single-instance runtime directory";
            return false;
        }
    }
    if(!S_ISDIR(info.st_mode) || info.st_uid != getuid()) {
        error = "The single-instance runtime path is not a directory owned by the current user";
        return false;
    }
    if(chmod(path.c_str(), S_IRWXU) != 0) {
        error = "Unable to secure the single-instance runtime directory";
        return false;
    }
    return true;
}

bool writeExact(int fd, const void *buffer, uint32_t size) {
    const char *data = static_cast<const char*>(buffer);
    uint32_t writtenTotal = 0;
    while(writtenTotal < size) {
        #if defined(MSG_NOSIGNAL)
        ssize_t written = send(fd, data + writtenTotal, size - writtenTotal, MSG_NOSIGNAL);
        #else
        ssize_t written = send(fd, data + writtenTotal, size - writtenTotal, 0);
        #endif
        if(written < 0 && errno == EINTR) {
            continue;
        }
        if(written <= 0) {
            return false;
        }
        writtenTotal += static_cast<uint32_t>(written);
    }
    return true;
}

bool readExact(int fd, void *buffer, uint32_t size) {
    char *data = static_cast<char*>(buffer);
    uint32_t readTotal = 0;
    while(readTotal < size) {
        ssize_t count = recv(fd, data + readTotal, size - readTotal, 0);
        if(count < 0 && errno == EINTR) {
            continue;
        }
        if(count <= 0) {
            return false;
        }
        readTotal += static_cast<uint32_t>(count);
    }
    return true;
}

bool sendMessage(int fd, const json &message) {
    string data = helpers::jsonToString(message);
    if(data.size() > MAX_MESSAGE_SIZE) {
        return false;
    }
    uint32_t size = hostToNetwork(static_cast<uint32_t>(data.size()));
    return writeExact(fd, &size, sizeof(size)) &&
        writeExact(fd, data.data(), static_cast<uint32_t>(data.size()));
}

bool receiveMessage(int fd, json &message) {
    uint32_t encodedSize = 0;
    if(!readExact(fd, &encodedSize, sizeof(encodedSize))) {
        return false;
    }
    uint32_t size = networkToHost(encodedSize);
    if(size == 0 || size > MAX_MESSAGE_SIZE) {
        return false;
    }
    string data(size, '\0');
    if(!readExact(fd, data.data(), size)) {
        return false;
    }
    try {
        message = json::parse(data);
        return true;
    }
    catch(const exception&) {
        return false;
    }
}

void listenLoop() {
    while(running.load()) {
        int client = accept(serverFd, nullptr, nullptr);
        if(client < 0) {
            if(running.load() && errno == EINTR) {
                continue;
            }
            break;
        }
        configureSocket(client);

        json request;
        json response = {{"version", PROTOCOL_VERSION}, {"accepted", false}};
        if(receiveMessage(client, request) && validateRequest(request)) {
            if(acceptRequest(request)) {
                response["accepted"] = true;
                response["requestId"] = request["requestId"];
            }
        }
        sendMessage(client, response);
        close(client);
    }
}

bool createServerSocket(string &error) {
    serverFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(serverFd < 0) {
        error = "Unable to create the single-instance Unix socket";
        return false;
    }

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    if(socketPath.size() >= sizeof(address.sun_path)) {
        error = "The single-instance Unix socket path is too long";
        close(serverFd);
        serverFd = -1;
        return false;
    }
    strncpy(address.sun_path, socketPath.c_str(), sizeof(address.sun_path) - 1);
    unlink(socketPath.c_str());
    if(::bind(serverFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0 ||
            listen(serverFd, 16) != 0) {
        error = "Unable to bind the single-instance Unix socket";
        close(serverFd);
        serverFd = -1;
        return false;
    }
    if(chmod(socketPath.c_str(), S_IRUSR | S_IWUSR) != 0) {
        error = "Unable to secure the single-instance Unix socket";
        close(serverFd);
        serverFd = -1;
        unlink(socketPath.c_str());
        return false;
    }
    running.store(true);
    listenerThread = thread(listenLoop);
    return true;
}

StartResult forwardToPrimary(const json &request, string &error) {
    auto deadline = chrono::steady_clock::now() + chrono::milliseconds(FORWARD_TIMEOUT_MS);
    while(chrono::steady_clock::now() < deadline) {
        if(flock(lockFd, LOCK_EX | LOCK_NB) == 0) {
            return StartResult::Primary;
        }

        int client = socket(AF_UNIX, SOCK_STREAM, 0);
        if(client < 0) {
            error = "Unable to create a client socket for the primary instance";
            return StartResult::Error;
        }
        configureSocket(client);
        sockaddr_un address{};
        address.sun_family = AF_UNIX;
        strncpy(address.sun_path, socketPath.c_str(), sizeof(address.sun_path) - 1);
        if(connect(client, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0) {
            json response;
            bool exchanged = sendMessage(client, request) && receiveMessage(client, response);
            close(client);
            if(exchanged) {
                if(response.value("version", 0u) == PROTOCOL_VERSION &&
                        response.value("accepted", false) &&
                        response.value("requestId", "") == request.value("requestId", "")) {
                    return StartResult::Forwarded;
                }
                error = "The primary instance rejected the launch request";
                return StartResult::Error;
            }
            this_thread::sleep_for(chrono::milliseconds(RETRY_INTERVAL_MS));
            continue;
        }
        close(client);
        this_thread::sleep_for(chrono::milliseconds(RETRY_INTERVAL_MS));
    }

    error = "Timed out while forwarding the launch request to the primary instance";
    return StartResult::Error;
}

#endif

} // namespace

StartResult start(const json &args, string &error) {
    json option = settings::getOptionForCurrentMode("singleInstance");
    if(option.is_null() || (option.is_boolean() && !option.get<bool>())) {
        return StartResult::Disabled;
    }
    if(!option.is_boolean()) {
        error = "The singleInstance configuration option must be a boolean";
        return StartResult::Error;
    }
    json config = settings::getConfig();
    if(!config.contains("applicationId") || !config["applicationId"].is_string() ||
            config["applicationId"].get<string>().empty()) {
        error = "The singleInstance configuration option requires a non-empty applicationId";
        return StartResult::Error;
    }
    string applicationIdentity = config["applicationId"].get<string>();

#if defined(_WIN32)
    string windowsIdentity;
    if(!getWindowsIdentity(windowsIdentity)) {
        error = "Unable to resolve the current Windows user identity";
        return StartResult::Error;
    }
    DWORD sessionId = 0;
    if(!ProcessIdToSessionId(GetCurrentProcessId(), &sessionId)) {
        error = "Unable to resolve the current Windows session";
        return StartResult::Error;
    }
    string key = makeHash(applicationIdentity + "|" + windowsIdentity +
        "|" + to_string(sessionId));
    wstring mutexName = L"Local\\neutralinojs-single-instance-" + helpers::str2wstr(key);
    pipeName = L"\\\\.\\pipe\\neutralinojs-single-instance-" + helpers::str2wstr(key);

    SECURITY_ATTRIBUTES attributes{};
    PSECURITY_DESCRIPTOR descriptor = nullptr;
    if(!createSecurityAttributes(attributes, descriptor)) {
        error = "Unable to create the single-instance security descriptor";
        return StartResult::Error;
    }
    SECURITY_ATTRIBUTES *attributesPtr = &attributes;
    instanceMutex = CreateMutexW(attributesPtr, TRUE, mutexName.c_str());
    DWORD createError = GetLastError();
    if(!instanceMutex) {
        if(descriptor) {
            LocalFree(descriptor);
        }
        error = "Unable to create the single-instance mutex (Windows error " +
            to_string(createError) + ")";
        return StartResult::Error;
    }

    if(createError == ERROR_ALREADY_EXISTS) {
        StartResult result = forwardToPrimary(makeRequest(args), error);
        if(result == StartResult::Primary) {
            if(!createServerPipe(attributesPtr, error)) {
                if(descriptor) {
                    LocalFree(descriptor);
                }
                return StartResult::Error;
            }
        }
        if(descriptor) {
            LocalFree(descriptor);
        }
        return result;
    }
    if(!createServerPipe(attributesPtr, error)) {
        if(descriptor) {
            LocalFree(descriptor);
        }
        return StartResult::Error;
    }
    if(descriptor) {
        LocalFree(descriptor);
    }
    return StartResult::Primary;
#else
    string runtimeDirectory;
    const char *xdgRuntimeDirectory = getenv("XDG_RUNTIME_DIR");
    if(xdgRuntimeDirectory && xdgRuntimeDirectory[0] != '\0') {
        runtimeDirectory = string(xdgRuntimeDirectory) + "/neutralinojs";
    }
    else {
        runtimeDirectory = "/tmp/neutralinojs-" + to_string(getuid());
    }
    if(!ensureRuntimeDirectory(runtimeDirectory, error)) {
        return StartResult::Error;
    }

    string key = makeHash(applicationIdentity);
    string lockPath = runtimeDirectory + "/" + key + ".lock";
    socketPath = runtimeDirectory + "/" + key + ".sock";
    if(socketPath.size() >= sizeof(sockaddr_un::sun_path)) {
        error = "The single-instance Unix socket path is too long";
        return StartResult::Error;
    }
    lockFd = open(lockPath.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if(lockFd < 0) {
        error = "Unable to open the single-instance lock file";
        return StartResult::Error;
    }
    if(flock(lockFd, LOCK_EX | LOCK_NB) != 0) {
        StartResult result = forwardToPrimary(makeRequest(args), error);
        if(result == StartResult::Primary && !createServerSocket(error)) {
            return StartResult::Error;
        }
        return result;
    }
    if(!createServerSocket(error)) {
        return StartResult::Error;
    }
    return StartResult::Primary;
#endif
}

void shutdown() {
    if(!running.exchange(false)) {
        return;
    }
#if defined(_WIN32)
    if(pipeHandle != INVALID_HANDLE_VALUE) {
        CancelIoEx(pipeHandle, nullptr);
    }
    if(listenerThread.joinable()) {
        listenerThread.join();
    }
    if(pipeHandle != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(pipeHandle);
        CloseHandle(pipeHandle);
    }
    pipeHandle = INVALID_HANDLE_VALUE;
    if(instanceMutex) {
        ReleaseMutex(instanceMutex);
        CloseHandle(instanceMutex);
        instanceMutex = nullptr;
    }
#else
    if(serverFd >= 0) {
        ::shutdown(serverFd, SHUT_RDWR);
        close(serverFd);
    }
    if(listenerThread.joinable()) {
        listenerThread.join();
    }
    serverFd = -1;
    if(!socketPath.empty()) {
        unlink(socketPath.c_str());
    }
    if(lockFd >= 0) {
        flock(lockFd, LOCK_UN);
        close(lockFd);
        lockFd = -1;
    }
#endif
}

void onAppClientConnect() {
    appClientConnected.store(true);
    flushPendingEvents();
}

void flushPendingEvents() {
    vector<json> eventsToDispatch;
    {
        lock_guard<mutex> guard(pendingMutex);
        if(!appClientConnected.load()) {
            return;
        }
        eventsToDispatch.swap(pendingEvents);
    }
    for(const auto &eventData: eventsToDispatch) {
        events::dispatchToAllApps("secondInstance", eventData);
    }
}

void onAppClientDisconnect() {
    appClientConnected.store(false);
}

void onWindowReady() {
    windowReady.store(true);
    if(activationPending.exchange(false)) {
        window::activate();
    }
}

} // namespace single_instance
