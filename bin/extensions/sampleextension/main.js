const fs = require('fs');
const process = require('process');
const argv = require('minimist')(process.argv.slice(2));
const WS = require('websocket').w3cwebsocket;
const { v4: uuidv4 } = require('uuid');

// Obtain required params to start a WS connection from stdIn.
const processInput = JSON.parse(fs.readFileSync(process.stdin.fd, 'utf-8'));
const NL_PORT = processInput.nlPort;
const NL_TOKEN = processInput.nlToken;
const NL_CTOKEN = processInput.nlConnectToken;
const NL_EXTID = processInput.nlExtensionId;

let client = new WS(`ws://127.0.0.1:${NL_PORT}?extensionId=${NL_EXTID}&connectToken=${NL_CTOKEN}`);

client.onerror = function () {
    // Always good to log some useful things from extension
    // You also can write to neutralinojs.log by calling debug.log
    // But, don't try to manipulate the log file directly via the extension process.
    console.log('Connection error!', 'ERROR');
};

client.onopen = function() {
    console.log('Connected');
};

client.onclose = function() {
    console.log('Connection closed');
    // Make sure to exit the extension process when WS extension is closed (when Neutralino app exits)
    process.exit();
};

client.onmessage = function(e) {
    if(typeof e.data === 'string') {
        let message = JSON.parse(e.data);

        // Use extensions.dispatch or extensions.broadcast from the app,
        // to send an event here
        switch(message.event) {
            case 'eventToExtension':
                console.log("Logging from extension: " + message.data);
                // Use Neutralinojs server's messaging protocol to trigger native API functions
                // Use app.broadcast method to send an event to all app instances
                client.send(JSON.stringify({
                    id: uuidv4(),
                    method: 'app.broadcast',
                    accessToken: NL_TOKEN,
                    data: {
                        event: 'eventFromExtension',
                        data: 'Hello app!'
                    }
                }));
                break;
        }
        //throw new Error("Test error");
        //console.error("Test error");
    }
};
