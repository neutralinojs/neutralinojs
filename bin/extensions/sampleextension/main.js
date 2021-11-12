const argv = require('minimist')(process.argv.slice(2));
const WS = require('websocket').w3cwebsocket;
const { v4: uuidv4 } = require('uuid');
const chalk = require('chalk');

const NL_PORT = argv['nl-port'];
const NL_TOKEN = argv['nl-token'];
const NL_EXTID = argv['nl-extension-id'];


let client = new WS(`ws://localhost:${NL_PORT}?extensionId=${NL_EXTID}`);

client.onerror = function() {
    log('Connection error!', 'ERROR');
};

client.onopen = function() {
    log('Connected');

    setTimeout(() => {
        client.send(JSON.stringify({
            id: uuidv4(),
            method: 'app.broadcast',
            accessToken: NL_TOKEN,
            data: {
                event: 'eventFromExtension',
                data: 'Hello app!'
            }
        }));
    }, 5000);
};

client.onclose = function() {
    log('Connection closed');
    process.exit();
};

client.onmessage = function(e) {
    if(typeof e.data === 'string') {
        let message = JSON.parse(e.data);

        switch(message.event) {
            case 'eventToExtension':
                log(message.data);
                break;
        }
    }
};


function log(message, type = 'INFO') {
    let logLine = `[${NL_EXTID}]: `;
    switch(type) {
        case 'INFO':
            logLine += chalk.green(type);
            logLine += ' ' + message;
            console.log(logLine);
            break;
        case 'ERROR':
            logLine += chalk.red(type);
            logLine += ' ' + message;
            console.error(logLine);
            break;
    }
}
