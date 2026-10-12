/**
 * raw_http_helper.js
 *
 * Utility for sending raw HTTP requests over raw TCP sockets.
 *
 * Why this is needed for security regression testing:
 * Standard HTTP client implementations (such as fetch, axios, or curl without --path-as-is)
 * automatically normalize and collapse relative path segments (e.g. converting "/../secret.txt"
 * or "/%2e%2e/secret.txt" into "/secret.txt" or rejecting them locally before sending).
 *
 * Because the browser/client intercepts and sanitizes the path client-side, tests using standard
 * clients produce false-negative results: an unpatched server appears to return 404 (because
 * /secret.txt does not exist) even though a malicious raw socket client can easily send raw ".."
 * and traverse directories.
 *
 * By writing directly to a net.Socket, this helper guarantees that the exact request bytes
 * (including raw `..` and encoded `%2e%2e`) reach the Neutralino C++ HTTP server unaltered,
 * allowing us to reliably verify that server-side traversal protection is effective.
 */
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
