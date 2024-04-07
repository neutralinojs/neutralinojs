/**
 * @file neu_router.h
 * @brief Header file containing declarations for the router namespace.
 * 
 * This header file declares various structures and functions for routing and handling 
 * HTTP requests within a WebSocket server application. It provides a namespace `router` 
 * to encapsulate these functionalities.
 */

#ifndef NEU_ROUTER_H
#define NEU_ROUTER_H

#include <string>
#include <map>

#include <websocketpp/server.hpp>

#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace router {

/**
 * @brief Alias for native methods used in routing.
 */
typedef json (*NativeMethod)(const json &);

/**
 * @brief Structure representing an HTTP response.
 */
struct Response {
    websocketpp::http::status_code::value status = websocketpp::http::status_code::ok; ///< HTTP status code.
    string contentType = "application/octet-stream"; ///< Content type of the response.
    string data; ///< Data to be sent in the response.
};

/**
 * @brief Structure representing a native message.
 */
struct NativeMessage {
    string id; ///< Identifier of the message.
    string method; ///< Method to be executed.
    string accessToken; ///< Access token for authentication.
    json data; ///< JSON data associated with the message.
};

/**
 * @brief Serves an HTTP request based on the provided path.
 * 
 * @param path The path of the HTTP request.
 * @return A Response object representing the HTTP response.
 */
Response serve(string path);

/**
 * @brief Executes a native method based on the provided request.
 * 
 * @param request The NativeMessage containing the request details.
 * @return A Response object representing the result of executing the native method.
 */
NativeMessage executeNativeMethod(const NativeMessage &request);

/**
 * @brief Retrieves an asset based on the provided path.
 * 
 * @param path The path of the asset.
 * @param prependData Data to be prepended to the asset content.
 * @return A Response object representing the retrieved asset.
 */
Response getAsset(string path, const string &prependData = "");

/**
 * @brief Retrieves the method map containing mappings of method names to native methods.
 * 
 * @return A map containing mappings of method names to native methods.
 */
map<string, NativeMethod> getMethodMap();

} // namespace router

#endif // NEU_ROUTER_H
