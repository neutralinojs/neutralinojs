#ifndef NEU_DYNLIB_H
#define NEU_DYNLIB_H

#include <string>
#include <functional>
#include <unordered_map>

#if defined(_WIN32)
    #include <windows.h>
    typedef HMODULE LibraryHandle;
#else
    #include <dlfcn.h>
    typedef void* LibraryHandle;
#endif

namespace dynlib {

/**
 * Extension function pointer type
 * Takes JSON string input and returns JSON string output
 */
typedef std::string (*ExtensionFunction)(const std::string&);

/**
 * Extension initialization function pointer
 * Called when the extension is loaded
 */
typedef bool (*ExtensionInitFunction)(const std::string& extensionId, const std::string& config);

/**
 * Extension cleanup function pointer
 * Called when the extension is unloaded
 */
typedef void (*ExtensionCleanupFunction)();

/**
 * Structure to hold loaded library information
 */
struct LoadedLibrary {
    LibraryHandle handle;
    std::string path;
    std::string extensionId;
    ExtensionInitFunction initFunc;
    ExtensionCleanupFunction cleanupFunc;
    std::unordered_map<std::string, ExtensionFunction> functions;
};

/**
 * Load a dynamic library from the specified path
 * @param path Path to the dynamic library file
 * @param extensionId Unique identifier for this extension
 * @param config Configuration JSON string to pass to the extension
 * @return true if loaded successfully, false otherwise
 */
bool loadLibrary(const std::string& path, const std::string& extensionId, const std::string& config = "{}");

/**
 * Unload a previously loaded dynamic library
 * @param extensionId Identifier of the extension to unload
 * @return true if unloaded successfully, false otherwise
 */
bool unloadLibrary(const std::string& extensionId);

/**
 * Call a function from a loaded extension
 * @param extensionId Identifier of the extension
 * @param functionName Name of the function to call
 * @param input JSON string input parameter
 * @param output JSON string output result
 * @return true if call succeeded, false otherwise
 */
bool callFunction(const std::string& extensionId, const std::string& functionName, 
                  const std::string& input, std::string& output);

/**
 * Check if a library is loaded
 * @param extensionId Identifier of the extension
 * @return true if loaded, false otherwise
 */
bool isLibraryLoaded(const std::string& extensionId);

/**
 * Get list of all loaded library extension IDs
 * @return Vector of extension IDs
 */
std::vector<std::string> getLoadedLibraries();

/**
 * Get the last error message from library operations
 * @return Error message string
 */
std::string getLastError();

/**
 * Unload all loaded dynamic libraries
 */
void unloadAll();

} // namespace dynlib

#endif // NEU_DYNLIB_H
