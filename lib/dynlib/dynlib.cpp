#include "dynlib.h"
#include <iostream>
#include <mutex>

namespace dynlib {

// Thread-safe storage for loaded libraries
static std::unordered_map<std::string, LoadedLibrary> loadedLibraries;
static std::mutex librariesMutex;
static std::string lastError;

// Platform-specific library loading implementations

#if defined(_WIN32)

LibraryHandle platformLoadLibrary(const std::string& path) {
    return LoadLibraryA(path.c_str());
}

void* platformGetFunction(LibraryHandle handle, const std::string& name) {
    return (void*)GetProcAddress(handle, name.c_str());
}

void platformUnloadLibrary(LibraryHandle handle) {
    FreeLibrary(handle);
}

std::string platformGetError() {
    DWORD errorCode = GetLastError();
    if (errorCode == 0) {
        return "";
    }
    
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
        (LPSTR)&messageBuffer, 0, NULL);
    
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

#else // POSIX (Linux, macOS)

LibraryHandle platformLoadLibrary(const std::string& path) {
    return dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
}

void* platformGetFunction(LibraryHandle handle, const std::string& name) {
    return dlsym(handle, name.c_str());
}

void platformUnloadLibrary(LibraryHandle handle) {
    dlclose(handle);
}

std::string platformGetError() {
    const char* error = dlerror();
    return error ? std::string(error) : "";
}

#endif

bool loadLibrary(const std::string& path, const std::string& extensionId, const std::string& config) {
    std::lock_guard<std::mutex> lock(librariesMutex);
    
    // Check if already loaded
    if (loadedLibraries.find(extensionId) != loadedLibraries.end()) {
        lastError = "Extension '" + extensionId + "' is already loaded";
        return false;
    }
    
    // Load the library
    LibraryHandle handle = platformLoadLibrary(path);
    if (!handle) {
        lastError = "Failed to load library '" + path + "': " + platformGetError();
        return false;
    }
    
    // Create library entry
    LoadedLibrary library;
    library.handle = handle;
    library.path = path;
    library.extensionId = extensionId;
    
    // Try to load init function (optional but recommended)
    library.initFunc = (ExtensionInitFunction)platformGetFunction(handle, "neutralino_extension_init");
    
    // Try to load cleanup function (optional)
    library.cleanupFunc = (ExtensionCleanupFunction)platformGetFunction(handle, "neutralino_extension_cleanup");
    
    // Call init function if available
    if (library.initFunc) {
        if (!library.initFunc(extensionId, config)) {
            lastError = "Extension initialization failed for '" + extensionId + "'";
            platformUnloadLibrary(handle);
            return false;
        }
    }
    
    // Store the library
    loadedLibraries[extensionId] = library;
    
    std::cout << "[Neutralino] Dynamic library extension loaded: " << extensionId << " (" << path << ")" << std::endl;
    return true;
}

bool unloadLibrary(const std::string& extensionId) {
    std::lock_guard<std::mutex> lock(librariesMutex);
    
    auto it = loadedLibraries.find(extensionId);
    if (it == loadedLibraries.end()) {
        lastError = "Extension '" + extensionId + "' is not loaded";
        return false;
    }
    
    LoadedLibrary& library = it->second;
    
    // Call cleanup function if available
    if (library.cleanupFunc) {
        library.cleanupFunc();
    }
    
    // Unload the library
    platformUnloadLibrary(library.handle);
    
    // Remove from map
    loadedLibraries.erase(it);
    
    std::cout << "[Neutralino] Dynamic library extension unloaded: " << extensionId << std::endl;
    return true;
}

bool callFunction(const std::string& extensionId, const std::string& functionName, 
                  const std::string& input, std::string& output) {
    std::lock_guard<std::mutex> lock(librariesMutex);
    
    auto it = loadedLibraries.find(extensionId);
    if (it == loadedLibraries.end()) {
        lastError = "Extension '" + extensionId + "' is not loaded";
        return false;
    }
    
    LoadedLibrary& library = it->second;
    
    // Check if function is cached
    auto funcIt = library.functions.find(functionName);
    ExtensionFunction func = nullptr;
    
    if (funcIt != library.functions.end()) {
        func = funcIt->second;
    } else {
        // Try to load the function
        func = (ExtensionFunction)platformGetFunction(library.handle, functionName.c_str());
        if (!func) {
            lastError = "Function '" + functionName + "' not found in extension '" + extensionId + "'";
            return false;
        }
        // Cache it
        library.functions[functionName] = func;
    }
    
    // Call the function
    try {
        output = func(input);
        return true;
    } catch (const std::exception& e) {
        lastError = "Exception in extension function: " + std::string(e.what());
        return false;
    } catch (...) {
        lastError = "Unknown exception in extension function";
        return false;
    }
}

bool isLibraryLoaded(const std::string& extensionId) {
    std::lock_guard<std::mutex> lock(librariesMutex);
    return loadedLibraries.find(extensionId) != loadedLibraries.end();
}

std::vector<std::string> getLoadedLibraries() {
    std::lock_guard<std::mutex> lock(librariesMutex);
    std::vector<std::string> result;
    result.reserve(loadedLibraries.size());
    
    for (const auto& pair : loadedLibraries) {
        result.push_back(pair.first);
    }
    
    return result;
}

std::string getLastError() {
    return lastError;
}

void unloadAll() {
    std::lock_guard<std::mutex> lock(librariesMutex);
    
    for (auto& pair : loadedLibraries) {
        LoadedLibrary& library = pair.second;
        
        // Call cleanup function if available
        if (library.cleanupFunc) {
            library.cleanupFunc();
        }
        
        // Unload the library
        platformUnloadLibrary(library.handle);
    }
    
    loadedLibraries.clear();
    std::cout << "[Neutralino] All dynamic library extensions unloaded" << std::endl;
}

} // namespace dynlib
