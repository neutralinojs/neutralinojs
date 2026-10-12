const net = require('net');

function rawGet(port, path) {
    return new Promise((resolve, reject) => {
        const client = net.createConnection({ port, host: '127.0.0.1' }, () => {
            const req = 'GET ' + path + ' HTTP/1.1\r\nHost: 127.0.0.1:' + port + '\r\nConnection: close\r\n\r\n';
            client.write(req);
        });
        let data = '';
        client.on('data', (chunk) => { data += chunk.toString(); });
        client.on('end', () => {
            const [head, ...bodyParts] = data.split('\r\n\r\n');
            const statusLine = head.split('\r\n')[0] || '';
            const statusCode = parseInt(statusLine.split(' ')[1], 10);
            resolve({ statusCode, body: bodyParts.join('\r\n\r\n') });
        });
        client.on('error', reject);
    });
}

module.exports = {
    rawGet
};
