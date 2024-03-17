const fs = require("fs");
const { v4: uuidv4 } = require("uuid");
const chalk = require("chalk");
const { w3cwebsocket: WS } = require("websocket");

function setupWebSocketClient() {
  const processInput = JSON.parse(fs.readFileSync(process.stdin.fd, "utf-8"));
  const NL_PORT = processInput.nlPort;
  const NL_TOKEN = processInput.nlToken;
  const NL_CTOKEN = processInput.nlConnectToken;
  const NL_EXTID = processInput.nlExtensionId;

  const client = new WS(
    `ws://localhost:${NL_PORT}?extensionId=${NL_EXTID}&connectToken=${NL_CTOKEN}`
  );

  client.onerror = function () {
    log("Connection error!", "ERROR");
  };

  client.onopen = function () {
    log("Connected");
  };

  client.onclose = function () {
    log("Connection closed");
    process.exit();
  };

  client.onmessage = handleMessage;

  return client;
}

function handleMessage(event) {
  if (typeof event.data === "string") {
    const message = JSON.parse(event.data);

    switch (message.event) {
      case "eventToExtension":
        log(message.data);
        sendMessageToApp();
        break;
    }
  }
}

function sendMessageToApp() {
  client.send(
    JSON.stringify({
      id: uuidv4(),
      method: "app.broadcast",
      accessToken: NL_TOKEN,
      data: {
        event: "eventFromExtension",
        data: "Hello app!",
      },
    })
  );
}

function log(message, type = "INFO") {
  let logLine = `[${NL_EXTID}]: `;
  switch (type) {
    case "INFO":
      logLine += chalk.green(type);
      logLine += ` ${message}`;
      console.log(logLine);
      break;
    case "ERROR":
      logLine += chalk.red(type);
      logLine += ` ${message}`;
      console.error(logLine);
      break;
  }
}

// Start the WebSocket client
const client = setupWebSocketClient();
