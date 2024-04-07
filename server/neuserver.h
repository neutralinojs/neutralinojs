/**
 * @file neuserver.h
 * @brief Header file containing declarations for the neuserver namespace.
 * 
 * This header file declares various functions and methods for managing a WebSocket server 
 * using the WebSocket++ library, and provides a namespace `neuserver` to encapsulate these 
 * functionalities.
 */

#ifndef NEU_SERVER_H
#define NEU_SERVER_H

#include <string>
#include <vector>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace neuserver {

/**
 * @brief Initializes the WebSocket server.
 * 
 * @return A string indicating the initialization status.
 */
string init();

/**
 * @brief Checks if the WebSocket server is initialized.
 * 
 * @return True if the server is initialized, otherwise false.
 */
bool isInitialized();

/**
 * @brief Starts the WebSocket server asynchronously.
 */
void startAsync();

/**
 * @brief Stops the WebSocket server.
 */
void stop();

/**
 * @brief Handles incoming WebSocket messages.
 * 
 * @param handler The handler of the WebSocket connection.
 * @param msg Pointer to the received message.
 */
void handleMessage(websocketpp::connection_hdl handler, websocketpp::server<websocketpp::config::asio>::message_ptr msg);

/**
 * @brief Handles incoming HTTP requests.
 * 
 * @param handler The handler of the WebSocket connection.
 */
void handleHTTP(websocketpp::connection_hdl handler);

/**
 * @brief Handles the connection event of WebSocket clients.
 * 
 * @param handler The handler of the WebSocket connection.
 */
void handleConnect(websocketpp::connection_hdl handler);

/**
 * @brief Handles the disconnection event of WebSocket clients.
 * 
 * @param handler The handler of the WebSocket connection.
 */
void handleDisconnect(websocketpp::connection_hdl handler);

/**
 * @brief Validates WebSocket connections.
 * 
 * @param handler The handler of the WebSocket connection.
 * @return True if the connection is valid, otherwise false.
 */
bool handleValidate(websocketpp::connection_hdl handler);

/**
 * @brief Broadcasts a JSON message to all connected WebSocket clients.
 * 
 * @param message The JSON message to broadcast.
 */
void broadcast(const json &message);

/**
 * @brief Broadcasts a JSON message to all connected WebSocket extensions.
 * 
 * @param message The JSON message to broadcast.
 */
void broadcastToAllExtensions(const json &message);

/**
 * @brief Broadcasts a JSON message to all connected WebSocket apps.
 * 
 * @param message The JSON message to broadcast.
 */
void broadcastToAllApps(const json &message);

/**
 * @brief Sends a JSON message to a specific WebSocket extension.
 * 
 * @param extensionId The ID of the extension.
 * @param message The JSON message to send.
 * @return True if the message is successfully sent, otherwise false.
 */
bool sendToExtension(const string &extensionId, const json &message);

/**
 * @brief Gets a list of IDs of connected WebSocket extensions.
 * 
 * @return A vector of strings containing the IDs of connected extensions.
 */
vector<string> getConnectedExtensions();

} // namespace neuserver

#endif // NEU_SERVER_H
