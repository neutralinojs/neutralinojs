#include <string>

#if defined(__APPLE__)
#include <Security/Security.h>
#elif defined(_WIN32)
#include <windows.h>
#include <wincred.h>
#elif defined(__linux__) || defined(__FreeBSD__)
#include <libsecret/secret.h>
#endif

#include "lib/json/json.hpp"
#include "helpers.h"
#include "errors.h"
#include "settings.h"
#include "api/secureStorage/secureStorage.h"

using namespace std;
using json = nlohmann::json;

namespace secureStorage {

void init() {
}

bool setSecureCredential(const string& key, const string& secret) {
#if defined(__APPLE__)
    CFStringRef serviceRef = CFStringCreateWithCString(NULL, "neutralinojs", kCFStringEncodingUTF8);
    CFStringRef keyRef = CFStringCreateWithCString(NULL, key.c_str(), kCFStringEncodingUTF8);
    CFMutableDictionaryRef query = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    CFDictionaryAddValue(query, kSecClass, kSecClassGenericPassword);
    CFDictionaryAddValue(query, kSecAttrService, serviceRef);
    CFDictionaryAddValue(query, kSecAttrAccount, keyRef);
    SecItemDelete(query); // Clear existing item before writing
    
    CFDataRef dataRef = CFDataCreate(NULL, (const UInt8*)secret.c_str(), secret.length());
    CFDictionaryAddValue(query, kSecValueData, dataRef);
    
    OSStatus status = SecItemAdd(query, NULL);
    
    CFRelease(query);
    CFRelease(keyRef);
    CFRelease(serviceRef);
    CFRelease(dataRef);
    
    return status == errSecSuccess;
#elif defined(_WIN32)
    CREDENTIALA cred = {0};
    cred.Type = CRED_TYPE_GENERIC;
    cred.TargetName = (LPSTR)key.c_str();
    cred.CredentialBlobSize = (DWORD)secret.size();
    cred.CredentialBlob = (LPBYTE)secret.c_str();
    cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
    return CredWriteA(&cred, 0) == TRUE;
#elif defined(__linux__) || defined(__FreeBSD__)
    GError *error = NULL;
    static const SecretSchema schema = {
        "org.neutralino.secureStorage", SECRET_SCHEMA_NONE,
        {
            { "key", SECRET_SCHEMA_ATTRIBUTE_STRING },
            { NULL, SECRET_SCHEMA_ATTRIBUTE_STRING },
        }
    };
    
    gboolean success = secret_password_store_sync(&schema, SECRET_COLLECTION_DEFAULT,
                               key.c_str(), secret.c_str(), NULL, &error,
                               "key", key.c_str(), NULL);
    if (error != NULL) {
        g_error_free(error);
        return false;
    }
    return success;
#else
    return false;
#endif
}

string getSecureCredential(const string& key) {
#if defined(__APPLE__)
    CFStringRef serviceRef = CFStringCreateWithCString(NULL, "neutralinojs", kCFStringEncodingUTF8);
    CFStringRef keyRef = CFStringCreateWithCString(NULL, key.c_str(), kCFStringEncodingUTF8);
    
    CFMutableDictionaryRef query = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionaryAddValue(query, kSecClass, kSecClassGenericPassword);
    CFDictionaryAddValue(query, kSecAttrService, serviceRef);
    CFDictionaryAddValue(query, kSecAttrAccount, keyRef);
    CFDictionaryAddValue(query, kSecReturnData, kCFBooleanTrue);
    CFDictionaryAddValue(query, kSecMatchLimit, kSecMatchLimitOne);
    
    CFTypeRef dataTypeRef = NULL;
    OSStatus status = SecItemCopyMatching(query, &dataTypeRef);
    
    string secret = "";
    if (status == errSecSuccess && dataTypeRef != NULL) {
        CFDataRef dataRef = (CFDataRef)dataTypeRef;
        secret = string((const char*)CFDataGetBytePtr(dataRef), CFDataGetLength(dataRef));
        CFRelease(dataTypeRef);
    }
    
    CFRelease(query);
    CFRelease(keyRef);
    CFRelease(serviceRef);
    return secret;
#elif defined(_WIN32)
    PCREDENTIALA pCred;
    if (CredReadA(key.c_str(), CRED_TYPE_GENERIC, 0, &pCred)) {
        string secret((char*)pCred->CredentialBlob, pCred->CredentialBlobSize);
        CredFree(pCred);
        return secret;
    }
    return "";
#elif defined(__linux__) || defined(__FreeBSD__)
    GError *error = NULL;
    static const SecretSchema schema = {
        "org.neutralino.secureStorage", SECRET_SCHEMA_NONE,
        {
            { "key", SECRET_SCHEMA_ATTRIBUTE_STRING },
            { NULL, SECRET_SCHEMA_ATTRIBUTE_STRING },
        }
    };
    
    gchar *password = secret_password_lookup_sync(&schema, NULL, &error,
                                                  "key", key.c_str(), NULL);
    
    if (error != NULL) {
        g_error_free(error);
        return "";
    }
    
    if (password != NULL) {
        string secret(password);
        secret_password_free(password);
        return secret;
    }
    return "";
#else
    return "";
#endif
}

namespace controllers {

json setData(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"key", "data"})) {
        output["error"] = errors::makeMissingArgErrorPayload("key or data");
        return output;
    }
    
    string key = input["key"].get<string>();
    string data = input["data"].get<string>();
    
    string appId = settings::getConfig()["applicationId"].get<string>();
    string secureKey = appId + "_" + key;

    if (setSecureCredential(secureKey, data)) {
        output["success"] = true;
        output["message"] = "Data saved to secure storage";
    } else {
        output["error"] = errors::makeErrorPayload(errors::NE_ST_STKEYWE, "Failed to write secure data");
    }
    return output;
}

json getData(const json &input) {
    json output;
    if(!helpers::hasRequiredFields(input, {"key"})) {
        output["error"] = errors::makeMissingArgErrorPayload("key");
        return output;
    }
    
    string key = input["key"].get<string>();
    string appId = settings::getConfig()["applicationId"].get<string>();
    string secureKey = appId + "_" + key;

    string data = getSecureCredential(secureKey);
    if (!data.empty()) {
        output["success"] = true;
        output["returnValue"] = data;
    } else {
        output["error"] = errors::makeErrorPayload(errors::NE_ST_NOSTKEX, key);
    }
    return output;
}

} // namespace controllers
} // namespace secureStorage
