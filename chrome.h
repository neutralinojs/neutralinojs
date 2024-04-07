/**
 * @file chrome.h
 * @brief Header file containing declarations for the chrome namespace.
 * 
 * This header file declares a function for initializing Chrome-related functionality
 * within a WebSocket server application. It provides a namespace `chrome` to encapsulate 
 * these functionalities.
 */

#ifndef NEU_CHROME_H
#define NEU_CHROME_H

#include <string>
#include <vector>

#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace chrome {

/**
 * @brief Initializes Chrome-related functionality with the provided input.
 * 
 * This function is responsible for initializing Chrome-related functionality
 * within the WebSocket server application based on the provided input parameters.
 * 
 * @param input A JSON object containing input parameters for initialization.
 */
void init(const json &input);

} // namespace chrome

#endif // NEU_CHROME_H
