const argv = require('minimist')(process.argv.slice(2));
const WS = require('websocket').w3cwebsocket;
const { v4: uuidv4 } = require('uuid');

const NL_PORT = argv['nl-port'];
const NL_TOKEN = argv['nl-token'];
const NL_EXTID = argv['nl-extension-id'];


let client = new WS(`ws://localhost:${NL_PORT}?extensionId=${NL_EXTID}`);

client.onerror = function() {
    console.log('Connection error!');
};

client.onopen = function() {
    console.log('Connected');

    setTimeout(() => {
        client.send(JSON.stringify({
            id: uuidv4(),
            method: 'app.broadcast',
            accessToken: NL_TOKEN,
            data: {
                event: 'eventFromExtension',
                data: 'Hello'
            }
        }));
    }, 5000);
};

client.onclose = function() {
    console.log('Connection closed');
    process.exit();
};

client.onmessage = function(e) {
    if(typeof e.data === 'string') {
        console.log('Data:', e.data);
    }
};
