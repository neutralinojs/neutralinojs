#include <string>
#include <sstream>
#include <ctime>
#include <iomanip>

#if defined(_WIN32)
    #define EXPORT __declspec(dllexport)
    #include <windows.h>
#else
    #define EXPORT __attribute__((visibility("default")))
    #include <unistd.h>
    #include <sys/utsname.h>
#endif

// Static storage for returned strings (must persist after function returns)
static std::string g_lastResult;
static std::string g_extensionId;
static std::string g_config;

extern "C" {

/**
 * Extension initialization function
 * Called when the extension is loaded by Neutralinojs
 */
EXPORT bool neutralino_extension_init(const char* extensionId, const char* configJson) {
    g_extensionId = extensionId ? extensionId : "";
    g_config = configJson ? configJson : "{}";
    
    // You can perform initialization here
    // Parse config, allocate resources, etc.
    
    return true; // Return false if initialization fails
}

/**
 * Extension cleanup function
 * Called when the extension is unloaded
 */
EXPORT void neutralino_extension_cleanup() {
    // Clean up any resources
    g_lastResult.clear();
    g_extensionId.clear();
    g_config.clear();
}

/**
 * Get system information
 * Example function that returns OS details
 */
EXPORT const char* get_system_info(const char* inputJson) {
    std::ostringstream oss;
    oss << R"({"success": true, "data": {)";
    
#if defined(_WIN32)
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    
    oss << R"("platform": "Windows",)";
    oss << R"("type": "Native DLL")";
#elif defined(__APPLE__)
    struct utsname unameData;
    uname(&unameData);
    oss << R"("platform": "macOS",)";
    oss << R"("type": "Native DYLIB",)";
    oss << R"("kernel": ")" << unameData.sysname << "\",";
    oss << R"("version": ")" << unameData.release << "\"";
#else
    struct utsname unameData;
    uname(&unameData);
    oss << R"("platform": "Linux",)";
    oss << R"("type": "Native SO",)";
    oss << R"("kernel": ")" << unameData.sysname << "\",";
    oss << R"("version": ")" << unameData.release << "\"";
#endif
    
    oss << "}}";
    g_lastResult = oss.str();
    return g_lastResult.c_str();
}

/**
 * Get current timestamp
 * Example function that returns current time
 */
EXPORT const char* get_timestamp(const char* inputJson) {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    std::ostringstream oss;
    oss << R"({"success": true, "timestamp": ")" 
        << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") 
        << R"(", "unix": )" << now << "}";
    
    g_lastResult = oss.str();
    return g_lastResult.c_str();
}

/**
 * Echo function for testing
 * Returns the input back with metadata
 */
EXPORT const char* echo(const char* inputJson) {
    std::string input = inputJson ? inputJson : "null";
    
    std::ostringstream oss;
    oss << R"({"success": true, "echo": )" << input 
        << R"(, "extensionId": ")" << g_extensionId << "\"}";
    
    g_lastResult = oss.str();
    return g_lastResult.c_str();
}

/**
 * Get extension configuration
 * Returns the config passed during initialization
 */
EXPORT const char* get_config(const char* inputJson) {
    std::ostringstream oss;
    oss << R"({"success": true, "config": )" << g_config << "}";
    
    g_lastResult = oss.str();
    return g_lastResult.c_str();
}

} // extern "C"
