#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <vector>

#if defined(_WIN32)
#define _WINSOCKAPI_
#include <windows.h>
#elif defined(__APPLE__)
#include <Carbon/Carbon.h>
#elif defined(__linux__) || defined(__FreeBSD__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif

#include "api/os/os.h"
#include "api/events/events.h"
#include "errors.h"
#include "helpers.h"

using namespace std;
using json = nlohmann::json;

namespace os {

namespace {

struct ParsedHotkey {
    string normalized;
    string key;
    bool ctrl = false;
    bool shift = false;
    bool alt = false;
    bool meta = false;
};

struct RegisteredHotkey {
    int id = 0;
    ParsedHotkey parsed;
    #if defined(__APPLE__)
    EventHotKeyRef macRef = nullptr;
    #endif
};

mutex hotkeysLock;
map<string, RegisteredHotkey> registeredHotkeys;
map<int, string> hotkeysById;
atomic<int> nextHotkeyId(1);

string trim(const string &value) {
    size_t start = 0;
    size_t end = value.size();

    while(start < end && isspace(static_cast<unsigned char>(value[start]))) {
        start++;
    }
    while(end > start && isspace(static_cast<unsigned char>(value[end - 1]))) {
        end--;
    }
    return value.substr(start, end - start);
}

string toUpper(string value) {
    transform(value.begin(), value.end(), value.begin(),
        [](unsigned char ch) { return static_cast<char>(toupper(ch)); });
    return value;
}

string normalizeKeyToken(const string &rawToken) {
    string token = trim(rawToken);
    string upperToken = toUpper(token);

    map<string, string> aliases = {
        {"ESC", "Escape"},
        {"ESCAPE", "Escape"},
        {"RETURN", "Enter"},
        {"ENTER", "Enter"},
        {"TAB", "Tab"},
        {"SPACE", "Space"},
        {"BACKSPACE", "Backspace"},
        {"DELETE", "Delete"},
        {"DEL", "Delete"},
        {"INSERT", "Insert"},
        {"INS", "Insert"},
        {"HOME", "Home"},
        {"END", "End"},
        {"PAGEUP", "PageUp"},
        {"PAGE_UP", "PageUp"},
        {"PAGEDOWN", "PageDown"},
        {"PAGE_DOWN", "PageDown"},
        {"UP", "Up"},
        {"DOWN", "Down"},
        {"LEFT", "Left"},
        {"RIGHT", "Right"},
        {"PRINTSCREEN", "PrintScreen"},
        {"PRINT_SCREEN", "PrintScreen"}
    };

    if(aliases.find(upperToken) != aliases.end()) {
        return aliases[upperToken];
    }

    if(upperToken.size() >= 2 && upperToken[0] == 'F') {
        string number = upperToken.substr(1);
        if(!number.empty() && all_of(number.begin(), number.end(),
                                     [](unsigned char ch) { return isdigit(ch); })) {
            int functionKey = 0;
            for(char digit: number) {
                functionKey = functionKey * 10 + (digit - '0');
                if(functionKey > 24) {
                    break;
                }
            }
            if(functionKey >= 1 && functionKey <= 24) {
                return upperToken;
            }
        }
    }

    if(token.size() == 1 && isalnum(static_cast<unsigned char>(token[0]))) {
        return string(1, static_cast<char>(toupper(static_cast<unsigned char>(token[0]))));
    }

    return "";
}

optional<ParsedHotkey> parseHotkey(const string &hotkey) {
    ParsedHotkey parsed;
    set<string> seenModifiers;
    vector<string> tokens = helpers::split(hotkey, '+');

    if(tokens.size() < 2) {
        return {};
    }

    for(string token: tokens) {
        token = trim(token);
        string upperToken = toUpper(token);
        if(token.empty()) {
            return {};
        }

        string modifier;
        if(upperToken == "CTRL" || upperToken == "CONTROL") {
            modifier = "Ctrl";
            parsed.ctrl = true;
        }
        else if(upperToken == "SHIFT") {
            modifier = "Shift";
            parsed.shift = true;
        }
        else if(upperToken == "ALT" || upperToken == "OPTION") {
            modifier = "Alt";
            parsed.alt = true;
        }
        else if(upperToken == "META" || upperToken == "SUPER" ||
                upperToken == "WIN" || upperToken == "WINDOWS" ||
                upperToken == "CMD" || upperToken == "COMMAND") {
            modifier = "Meta";
            parsed.meta = true;
        }

        if(!modifier.empty()) {
            if(seenModifiers.find(modifier) != seenModifiers.end()) {
                return {};
            }
            seenModifiers.insert(modifier);
            continue;
        }

        if(!parsed.key.empty()) {
            return {};
        }
        parsed.key = normalizeKeyToken(token);
        if(parsed.key.empty()) {
            return {};
        }
    }

    if(parsed.key.empty() || (!parsed.ctrl && !parsed.shift && !parsed.alt && !parsed.meta)) {
        return {};
    }

    vector<string> normalizedParts;
    if(parsed.ctrl) normalizedParts.push_back("Ctrl");
    if(parsed.shift) normalizedParts.push_back("Shift");
    if(parsed.alt) normalizedParts.push_back("Alt");
    if(parsed.meta) normalizedParts.push_back("Meta");
    normalizedParts.push_back(parsed.key);

    for(size_t i = 0; i < normalizedParts.size(); i++) {
        if(i > 0) {
            parsed.normalized += "+";
        }
        parsed.normalized += normalizedParts[i];
    }

    return parsed;
}

void dispatchGlobalHotkeyFired(int id) {
    string hotkey;
    {
        lock_guard<mutex> guard(hotkeysLock);
        if(hotkeysById.find(id) == hotkeysById.end()) {
            return;
        }
        hotkey = hotkeysById[id];
    }

    json eventData;
    eventData["hotkey"] = hotkey;
    events::dispatch("globalHotkeyFired", eventData);
}

int makeHotkeyId() {
    int id = nextHotkeyId++;
    if(id == INT_MAX) {
        nextHotkeyId = 1;
    }
    return id;
}

#if defined(_WIN32)

constexpr wchar_t HOTKEY_WINDOW_CLASS[] = L"Neutralinojs_global_hotkeys";
constexpr UINT WM_NEU_REGISTER_HOTKEY = WM_APP + 101;
constexpr UINT WM_NEU_UNREGISTER_HOTKEY = WM_APP + 102;
constexpr UINT WM_NEU_CLEANUP_HOTKEYS = WM_APP + 103;

struct WinHotkeyRequest {
    int id = 0;
    UINT modifiers = 0;
    UINT key = 0;
    bool result = false;
    bool done = false;
    mutex lock;
    condition_variable cv;
};

thread winHotkeyThread;
mutex winHotkeyWindowLock;
condition_variable winHotkeyWindowCv;
HWND winHotkeyWindow = nullptr;
bool winHotkeyWindowReady = false;
bool winHotkeyThreadStopping = false;

LRESULT CALLBACK hotkeyWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch(msg) {
        case WM_NEU_REGISTER_HOTKEY: {
            auto request = reinterpret_cast<WinHotkeyRequest*>(lp);
            bool result = RegisterHotKey(hwnd, request->id, request->modifiers, request->key) != 0;
            {
                lock_guard<mutex> guard(request->lock);
                request->result = result;
                request->done = true;
            }
            request->cv.notify_one();
            return 0;
        }
        case WM_NEU_UNREGISTER_HOTKEY: {
            auto request = reinterpret_cast<WinHotkeyRequest*>(lp);
            bool result = UnregisterHotKey(hwnd, request->id) != 0;
            {
                lock_guard<mutex> guard(request->lock);
                request->result = result;
                request->done = true;
            }
            request->cv.notify_one();
            return 0;
        }
        case WM_HOTKEY:
            dispatchGlobalHotkeyFired(static_cast<int>(wp));
            return 0;
        case WM_NEU_CLEANUP_HOTKEYS:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY: {
            lock_guard<mutex> guard(hotkeysLock);
            for(const auto &[id, hotkey]: hotkeysById) {
                UnregisterHotKey(hwnd, id);
            }
            hotkeysById.clear();
            registeredHotkeys.clear();
            PostQuitMessage(0);
            return 0;
        }
        default:
            return DefWindowProc(hwnd, msg, wp, lp);
    }
}

void runWinHotkeyThread() {
    HINSTANCE instance = GetModuleHandle(nullptr);

    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = hotkeyWindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = HOTKEY_WINDOW_CLASS;
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowEx(0, HOTKEY_WINDOW_CLASS, L"", 0, 0, 0, 0, 0,
                               HWND_MESSAGE, nullptr, instance, nullptr);
    {
        lock_guard<mutex> guard(winHotkeyWindowLock);
        winHotkeyWindow = hwnd;
        winHotkeyWindowReady = true;
    }
    winHotkeyWindowCv.notify_all();

    if(!hwnd) {
        return;
    }

    MSG msg;
    while(GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    {
        lock_guard<mutex> guard(winHotkeyWindowLock);
        winHotkeyWindow = nullptr;
        winHotkeyWindowReady = false;
        winHotkeyThreadStopping = false;
    }
}

bool ensureWinHotkeyThread() {
    unique_lock<mutex> lock(winHotkeyWindowLock);
    if(winHotkeyWindow != nullptr) {
        return true;
    }
    if(!winHotkeyThread.joinable()) {
        winHotkeyWindowReady = false;
        winHotkeyThreadStopping = false;
        winHotkeyThread = thread(runWinHotkeyThread);
    }
    winHotkeyWindowCv.wait(lock, [] { return winHotkeyWindowReady; });
    return winHotkeyWindow != nullptr;
}

optional<UINT> getWinKeyCode(const string &key) {
    if(key.size() == 1 && isalnum(static_cast<unsigned char>(key[0]))) {
        return static_cast<UINT>(key[0]);
    }

    map<string, UINT> keys = {
        {"F1", VK_F1}, {"F2", VK_F2}, {"F3", VK_F3}, {"F4", VK_F4},
        {"F5", VK_F5}, {"F6", VK_F6}, {"F7", VK_F7}, {"F8", VK_F8},
        {"F9", VK_F9}, {"F10", VK_F10}, {"F11", VK_F11}, {"F12", VK_F12},
        {"F13", VK_F13}, {"F14", VK_F14}, {"F15", VK_F15}, {"F16", VK_F16},
        {"F17", VK_F17}, {"F18", VK_F18}, {"F19", VK_F19}, {"F20", VK_F20},
        {"F21", VK_F21}, {"F22", VK_F22}, {"F23", VK_F23}, {"F24", VK_F24},
        {"Escape", VK_ESCAPE}, {"Enter", VK_RETURN}, {"Tab", VK_TAB},
        {"Space", VK_SPACE}, {"Backspace", VK_BACK}, {"Delete", VK_DELETE},
        {"Insert", VK_INSERT}, {"Home", VK_HOME}, {"End", VK_END},
        {"PageUp", VK_PRIOR}, {"PageDown", VK_NEXT}, {"Up", VK_UP},
        {"Down", VK_DOWN}, {"Left", VK_LEFT}, {"Right", VK_RIGHT},
        {"PrintScreen", VK_SNAPSHOT}
    };

    if(keys.find(key) == keys.end()) {
        return {};
    }
    return keys[key];
}

bool sendWinHotkeyRequest(UINT message, WinHotkeyRequest &request) {
    HWND hwnd = nullptr;
    {
        lock_guard<mutex> guard(winHotkeyWindowLock);
        hwnd = winHotkeyWindow;
    }
    if(!hwnd || !PostMessage(hwnd, message, 0,
                             reinterpret_cast<LPARAM>(&request))) {
        return false;
    }

    unique_lock<mutex> requestLock(request.lock);
    request.cv.wait(requestLock, [&request] { return request.done; });
    return request.result;
}

bool registerPlatformHotkey(const RegisteredHotkey &hotkey) {
    optional<UINT> key = getWinKeyCode(hotkey.parsed.key);
    if(!key || !ensureWinHotkeyThread()) {
        return false;
    }

    UINT modifiers = MOD_NOREPEAT;
    if(hotkey.parsed.ctrl) modifiers |= MOD_CONTROL;
    if(hotkey.parsed.shift) modifiers |= MOD_SHIFT;
    if(hotkey.parsed.alt) modifiers |= MOD_ALT;
    if(hotkey.parsed.meta) modifiers |= MOD_WIN;

    WinHotkeyRequest request;
    request.id = hotkey.id;
    request.modifiers = modifiers;
    request.key = key.value();

    return sendWinHotkeyRequest(WM_NEU_REGISTER_HOTKEY, request);
}

bool unregisterPlatformHotkey(const RegisteredHotkey &hotkey) {
    if(!ensureWinHotkeyThread()) {
        return false;
    }

    WinHotkeyRequest request;
    request.id = hotkey.id;
    return sendWinHotkeyRequest(WM_NEU_UNREGISTER_HOTKEY, request);
}

void cleanupPlatformHotkeys() {
    HWND hwnd = nullptr;
    {
        lock_guard<mutex> guard(winHotkeyWindowLock);
        if(winHotkeyThreadStopping) {
            return;
        }
        hwnd = winHotkeyWindow;
        winHotkeyThreadStopping = true;
    }

    if(hwnd) {
        PostMessage(hwnd, WM_NEU_CLEANUP_HOTKEYS, 0, 0);
    }
    if(winHotkeyThread.joinable()) {
        winHotkeyThread.join();
    }
}

#elif defined(__APPLE__)

constexpr OSType MAC_HOTKEY_SIGNATURE = 'NLGH';
EventHandlerRef macHotkeyHandler = nullptr;

OSStatus macHotkeyEventHandler(EventHandlerCallRef nextHandler, EventRef event, void *userData) {
    (void)nextHandler;
    (void)userData;

    EventHotKeyID hotkeyId;
    OSStatus status = GetEventParameter(event, kEventParamDirectObject,
                                        typeEventHotKeyID, nullptr,
                                        sizeof(hotkeyId), nullptr, &hotkeyId);
    if(status == noErr && hotkeyId.signature == MAC_HOTKEY_SIGNATURE) {
        dispatchGlobalHotkeyFired(static_cast<int>(hotkeyId.id));
    }
    return noErr;
}

bool ensureMacHotkeyHandler() {
    if(macHotkeyHandler != nullptr) {
        return true;
    }

    EventTypeSpec eventType;
    eventType.eventClass = kEventClassKeyboard;
    eventType.eventKind = kEventHotKeyPressed;

    return InstallApplicationEventHandler(&macHotkeyEventHandler, 1, &eventType,
                                          nullptr, &macHotkeyHandler) == noErr;
}

optional<UInt32> getMacKeyCode(const string &key) {
    map<string, UInt32> keys = {
        {"A", 0x00}, {"S", 0x01}, {"D", 0x02}, {"F", 0x03}, {"H", 0x04},
        {"G", 0x05}, {"Z", 0x06}, {"X", 0x07}, {"C", 0x08}, {"V", 0x09},
        {"B", 0x0B}, {"Q", 0x0C}, {"W", 0x0D}, {"E", 0x0E}, {"R", 0x0F},
        {"Y", 0x10}, {"T", 0x11}, {"1", 0x12}, {"2", 0x13}, {"3", 0x14},
        {"4", 0x15}, {"6", 0x16}, {"5", 0x17}, {"=", 0x18}, {"9", 0x19},
        {"7", 0x1A}, {"-", 0x1B}, {"8", 0x1C}, {"0", 0x1D}, {"]", 0x1E},
        {"O", 0x1F}, {"U", 0x20}, {"[", 0x21}, {"I", 0x22}, {"P", 0x23},
        {"L", 0x25}, {"J", 0x26}, {"'", 0x27}, {"K", 0x28}, {";", 0x29},
        {"\\", 0x2A}, {",", 0x2B}, {"/", 0x2C}, {"N", 0x2D}, {"M", 0x2E},
        {".", 0x2F}, {"`", 0x32}, {"Space", 0x31}, {"Tab", 0x30},
        {"Enter", 0x24}, {"Backspace", 0x33}, {"Escape", 0x35},
        {"F1", 0x7A}, {"F2", 0x78}, {"F3", 0x63}, {"F4", 0x76},
        {"F5", 0x60}, {"F6", 0x61}, {"F7", 0x62}, {"F8", 0x64},
        {"F9", 0x65}, {"F10", 0x6D}, {"F11", 0x67}, {"F12", 0x6F},
        {"F13", 0x69}, {"F14", 0x6B}, {"F15", 0x71}, {"F16", 0x6A},
        {"F17", 0x40}, {"F18", 0x4F}, {"F19", 0x50}, {"F20", 0x5A},
        {"Home", 0x73}, {"End", 0x77}, {"PageUp", 0x74},
        {"PageDown", 0x79}, {"Delete", 0x75}, {"Left", 0x7B},
        {"Right", 0x7C}, {"Down", 0x7D}, {"Up", 0x7E}
    };

    if(keys.find(key) == keys.end()) {
        return {};
    }
    return keys[key];
}

bool registerPlatformHotkey(RegisteredHotkey &hotkey) {
    optional<UInt32> key = getMacKeyCode(hotkey.parsed.key);
    if(!key || !ensureMacHotkeyHandler()) {
        return false;
    }

    UInt32 modifiers = 0;
    if(hotkey.parsed.ctrl) modifiers |= controlKey;
    if(hotkey.parsed.shift) modifiers |= shiftKey;
    if(hotkey.parsed.alt) modifiers |= optionKey;
    if(hotkey.parsed.meta) modifiers |= cmdKey;

    EventHotKeyID hotkeyId;
    hotkeyId.signature = MAC_HOTKEY_SIGNATURE;
    hotkeyId.id = static_cast<UInt32>(hotkey.id);

    return RegisterEventHotKey(key.value(), modifiers, hotkeyId,
                               GetApplicationEventTarget(), 0,
                               &hotkey.macRef) == noErr;
}

bool unregisterPlatformHotkey(const RegisteredHotkey &hotkey) {
    if(hotkey.macRef == nullptr) {
        return false;
    }
    return UnregisterEventHotKey(hotkey.macRef) == noErr;
}

void cleanupPlatformHotkeys() {
    for(const auto &[hotkey, entry]: registeredHotkeys) {
        (void)hotkey;
        if(entry.macRef != nullptr) {
            UnregisterEventHotKey(entry.macRef);
        }
    }
    if(macHotkeyHandler != nullptr) {
        RemoveEventHandler(macHotkeyHandler);
        macHotkeyHandler = nullptr;
    }
}

#elif defined(__linux__) || defined(__FreeBSD__)

thread xHotkeyThread;
atomic<bool> xHotkeyThreadRunning(false);
Display *xDisplay = nullptr;
Window xRootWindow = 0;
mutex xDisplayLock;
atomic<bool> xLastGrabError(false);

int xGrabErrorHandler(Display *display, XErrorEvent *event) {
    (void)display;
    if(event->error_code == BadAccess || event->error_code == BadValue ||
       event->error_code == BadWindow) {
        xLastGrabError = true;
    }
    return 0;
}

optional<KeySym> getXKeySym(const string &key) {
    if(key.size() == 1 && isalnum(static_cast<unsigned char>(key[0]))) {
        return XStringToKeysym(key.c_str());
    }

    map<string, KeySym> keys = {
        {"Escape", XK_Escape}, {"Enter", XK_Return}, {"Tab", XK_Tab},
        {"Space", XK_space}, {"Backspace", XK_BackSpace},
        {"Delete", XK_Delete}, {"Insert", XK_Insert}, {"Home", XK_Home},
        {"End", XK_End}, {"PageUp", XK_Page_Up}, {"PageDown", XK_Page_Down},
        {"Up", XK_Up}, {"Down", XK_Down}, {"Left", XK_Left},
        {"Right", XK_Right}, {"PrintScreen", XK_Print},
        {"F1", XK_F1}, {"F2", XK_F2}, {"F3", XK_F3}, {"F4", XK_F4},
        {"F5", XK_F5}, {"F6", XK_F6}, {"F7", XK_F7}, {"F8", XK_F8},
        {"F9", XK_F9}, {"F10", XK_F10}, {"F11", XK_F11}, {"F12", XK_F12},
        {"F13", XK_F13}, {"F14", XK_F14}, {"F15", XK_F15}, {"F16", XK_F16},
        {"F17", XK_F17}, {"F18", XK_F18}, {"F19", XK_F19}, {"F20", XK_F20},
        {"F21", XK_F21}, {"F22", XK_F22}, {"F23", XK_F23}, {"F24", XK_F24}
    };

    if(keys.find(key) == keys.end()) {
        return {};
    }
    return keys[key];
}

unsigned int getXModifiers(const ParsedHotkey &hotkey) {
    unsigned int modifiers = 0;
    if(hotkey.ctrl) modifiers |= ControlMask;
    if(hotkey.shift) modifiers |= ShiftMask;
    if(hotkey.alt) modifiers |= Mod1Mask;
    if(hotkey.meta) modifiers |= Mod4Mask;
    return modifiers;
}

bool ensureXDisplay() {
    lock_guard<mutex> guard(xDisplayLock);
    if(xDisplay != nullptr) {
        return true;
    }

    const char *sessionType = getenv("XDG_SESSION_TYPE");
    if(sessionType != nullptr && string(sessionType) == "wayland") {
        return false;
    }

    XInitThreads();
    xDisplay = XOpenDisplay(nullptr);
    if(xDisplay == nullptr) {
        return false;
    }
    xRootWindow = DefaultRootWindow(xDisplay);
    return true;
}

void runXHotkeyThread() {
    while(xHotkeyThreadRunning) {
        Display *display = nullptr;
        {
            lock_guard<mutex> guard(xDisplayLock);
            display = xDisplay;
        }

        if(display == nullptr) {
            this_thread::sleep_for(chrono::milliseconds(50));
            continue;
        }

        while(xHotkeyThreadRunning && XPending(display) > 0) {
            XEvent event;
            XNextEvent(display, &event);
            if(event.type != KeyPress) {
                continue;
            }

            int firedId = -1;
            {
                lock_guard<mutex> guard(hotkeysLock);
                for(const auto &[id, hotkey]: hotkeysById) {
                    const RegisteredHotkey &entry = registeredHotkeys[hotkey];
                    optional<KeySym> keySym = getXKeySym(entry.parsed.key);
                    if(!keySym) {
                        continue;
                    }
                    KeyCode keyCode = XKeysymToKeycode(display, keySym.value());
                    unsigned int cleanState = event.xkey.state & ~(LockMask | Mod2Mask);
                    if(event.xkey.keycode == keyCode &&
                       cleanState == getXModifiers(entry.parsed)) {
                        firedId = id;
                        break;
                    }
                }
            }

            if(firedId != -1) {
                dispatchGlobalHotkeyFired(firedId);
            }
        }

        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

bool ensureXHotkeyThread() {
    if(xHotkeyThreadRunning) {
        return true;
    }
    xHotkeyThreadRunning = true;
    xHotkeyThread = thread(runXHotkeyThread);
    return true;
}

bool registerPlatformHotkey(const RegisteredHotkey &hotkey) {
    if(!ensureXDisplay()) {
        return false;
    }

    optional<KeySym> keySym = getXKeySym(hotkey.parsed.key);
    if(!keySym) {
        return false;
    }

    lock_guard<mutex> guard(xDisplayLock);
    KeyCode keyCode = XKeysymToKeycode(xDisplay, keySym.value());
    if(keyCode == 0) {
        return false;
    }

    unsigned int modifiers = getXModifiers(hotkey.parsed);
    vector<unsigned int> lockMasks = {0, LockMask, Mod2Mask, LockMask | Mod2Mask};
    int (*previousHandler)(Display*, XErrorEvent*) = XSetErrorHandler(xGrabErrorHandler);

    for(unsigned int lockMask: lockMasks) {
        xLastGrabError = false;
        XGrabKey(xDisplay, keyCode, modifiers | lockMask, xRootWindow, True,
                 GrabModeAsync, GrabModeAsync);
        XSync(xDisplay, False);
        if(xLastGrabError) {
            for(unsigned int rollbackMask: lockMasks) {
                XUngrabKey(xDisplay, keyCode, modifiers | rollbackMask, xRootWindow);
            }
            XSetErrorHandler(previousHandler);
            return false;
        }
    }

    XSetErrorHandler(previousHandler);
    XSelectInput(xDisplay, xRootWindow, KeyPressMask);
    XFlush(xDisplay);
    ensureXHotkeyThread();
    return true;
}

bool unregisterPlatformHotkey(const RegisteredHotkey &hotkey) {
    if(!ensureXDisplay()) {
        return false;
    }

    optional<KeySym> keySym = getXKeySym(hotkey.parsed.key);
    if(!keySym) {
        return false;
    }

    lock_guard<mutex> guard(xDisplayLock);
    KeyCode keyCode = XKeysymToKeycode(xDisplay, keySym.value());
    if(keyCode == 0) {
        return false;
    }

    unsigned int modifiers = getXModifiers(hotkey.parsed);
    vector<unsigned int> lockMasks = {0, LockMask, Mod2Mask, LockMask | Mod2Mask};
    for(unsigned int lockMask: lockMasks) {
        XUngrabKey(xDisplay, keyCode, modifiers | lockMask, xRootWindow);
    }
    XFlush(xDisplay);
    return true;
}

void cleanupPlatformHotkeys() {
    xHotkeyThreadRunning = false;
    if(xHotkeyThread.joinable()) {
        xHotkeyThread.join();
    }

    lock_guard<mutex> guard(xDisplayLock);
    if(xDisplay != nullptr) {
        for(const auto &[hotkey, entry]: registeredHotkeys) {
            optional<KeySym> keySym = getXKeySym(entry.parsed.key);
            if(!keySym) {
                continue;
            }
            KeyCode keyCode = XKeysymToKeycode(xDisplay, keySym.value());
            unsigned int modifiers = getXModifiers(entry.parsed);
            vector<unsigned int> lockMasks = {0, LockMask, Mod2Mask, LockMask | Mod2Mask};
            for(unsigned int lockMask: lockMasks) {
                XUngrabKey(xDisplay, keyCode, modifiers | lockMask, xRootWindow);
            }
        }
        XCloseDisplay(xDisplay);
        xDisplay = nullptr;
        xRootWindow = 0;
    }
}

#else

bool registerPlatformHotkey(const RegisteredHotkey &hotkey) {
    (void)hotkey;
    return false;
}

bool unregisterPlatformHotkey(const RegisteredHotkey &hotkey) {
    (void)hotkey;
    return false;
}

void cleanupPlatformHotkeys() {
}

#endif

} // namespace

os::GlobalHotkeyResult registerGlobalHotkey(const string &hotkey) {
    optional<ParsedHotkey> parsed = parseHotkey(hotkey);
    if(!parsed) {
        return { false, errors::NE_OS_INVHOTKY };
    }

    {
        lock_guard<mutex> guard(hotkeysLock);
        if(registeredHotkeys.find(parsed->normalized) != registeredHotkeys.end()) {
            return { true, errors::NE_ST_OK };
        }
    }

    RegisteredHotkey entry;
    entry.id = makeHotkeyId();
    entry.parsed = parsed.value();

    bool registered = registerPlatformHotkey(entry);
    if(!registered) {
        return { false, errors::NE_OS_UNLTOHK };
    }

    lock_guard<mutex> guard(hotkeysLock);
    registeredHotkeys[entry.parsed.normalized] = entry;
    hotkeysById[entry.id] = entry.parsed.normalized;
    return { true, errors::NE_ST_OK };
}

os::GlobalHotkeyResult unregisterGlobalHotkey(const string &hotkey) {
    optional<ParsedHotkey> parsed = parseHotkey(hotkey);
    if(!parsed) {
        return { false, errors::NE_OS_INVHOTKY };
    }

    RegisteredHotkey entry;
    {
        lock_guard<mutex> guard(hotkeysLock);
        auto registeredHotkey = registeredHotkeys.find(parsed->normalized);
        if(registeredHotkey == registeredHotkeys.end()) {
            return { true, errors::NE_ST_OK };
        }
        entry = registeredHotkey->second;
    }

    if(!unregisterPlatformHotkey(entry)) {
        return { false, errors::NE_OS_UNLTOUHK };
    }

    lock_guard<mutex> guard(hotkeysLock);
    auto registeredHotkey = registeredHotkeys.find(parsed->normalized);
    if(registeredHotkey != registeredHotkeys.end() &&
       registeredHotkey->second.id == entry.id) {
        registeredHotkeys.erase(registeredHotkey);
        hotkeysById.erase(entry.id);
    }
    return { true, errors::NE_ST_OK };
}

vector<string> getRegisteredHotkeys() {
    vector<string> hotkeys;
    lock_guard<mutex> guard(hotkeysLock);
    for(const auto &[hotkey, entry]: registeredHotkeys) {
        (void)entry;
        hotkeys.push_back(hotkey);
    }
    return hotkeys;
}

void cleanupGlobalHotkeys() {
    cleanupPlatformHotkeys();
    lock_guard<mutex> guard(hotkeysLock);
    registeredHotkeys.clear();
    hotkeysById.clear();
}

namespace controllers {

json registerGlobalHotkey(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"hotkey"})) {
        output["error"] = errors::makeMissingArgErrorPayload("hotkey");
        return output;
    }

    string hotkey = input["hotkey"].get<string>();
    os::GlobalHotkeyResult result = os::registerGlobalHotkey(hotkey);
    if(result.success) {
        output["returnValue"] = true;
        output["success"] = true;
    }
    else {
        output["error"] = errors::makeErrorPayload(result.status, hotkey);
    }
    return output;
}

json unregisterGlobalHotkey(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"hotkey"})) {
        output["error"] = errors::makeMissingArgErrorPayload("hotkey");
        return output;
    }

    string hotkey = input["hotkey"].get<string>();
    os::GlobalHotkeyResult result = os::unregisterGlobalHotkey(hotkey);
    if(result.success) {
        output["returnValue"] = true;
        output["success"] = true;
    }
    else {
        output["error"] = errors::makeErrorPayload(result.status, hotkey);
    }
    return output;
}

json getRegisteredHotkeys(const json &input) {
    (void)input;
    json output;
    output["returnValue"] = os::getRegisteredHotkeys();
    output["success"] = true;
    return output;
}

} // namespace controllers
} // namespace os
