// Fixture HTTP server for the net specs.
// It runs in a separate process, because spec.runner.run() blocks the event loop.
const http = require('http');

const utf8 = Buffer.from('<a>ěščř</a>', 'utf8');
const windows1250 = Buffer.from([0x3c, 0x61, 0x3e, 0xec, 0x9a, 0xe8, 0xf8, 0x3c, 0x2f, 0x61, 0x3e]);
const json = Buffer.from(JSON.stringify({ ok: true, name: 'ěščř' }), 'utf8');

const send = (res, status, body, contentType) => {
    res.writeHead(status, {
        'Content-Type': contentType,
        'Content-Length': body.length
    });
    res.end(body);
};

const server = http.createServer((req, res) => {
    const url = new URL(req.url, 'http://127.0.0.1');

    switch(url.pathname) {
        case '/utf8':
            return send(res, 200, utf8, 'text/plain; charset=utf-8');
        case '/windows1250':
            return send(res, 200, windows1250, 'text/plain; charset=windows-1250');
        case '/all-bytes':
            return send(res, 200,
                Buffer.from(Array.from({ length: 256 }, (_, i) => i)), 'application/octet-stream');
        case '/json':
            return send(res, 200, json, 'application/json; charset=utf-8');
        case '/empty':
            return send(res, 200, Buffer.alloc(0), 'text/plain');
        case '/no-content':
            res.writeHead(204);
            return res.end();
        case '/redirect':
            res.writeHead(302, { Location: '/utf8' });
            return res.end();
        case '/status/401':
            return send(res, 401, Buffer.from('nope', 'utf8'), 'text/plain');
        case '/large': {
            const size = Number(url.searchParams.get('size') || 1024);
            return send(res, 200, Buffer.alloc(size, 0x80 + (size % 96)), 'application/octet-stream');
        }
        case '/echo': {
            let body = '';
            req.on('data', (chunk) => body += chunk);
            req.on('end', () => send(res, 200,
                Buffer.from(JSON.stringify({ method: req.method, headers: req.headers, body }), 'utf8'),
                'application/json'));
            return;
        }
        default:
            return send(res, 404, Buffer.alloc(0), 'text/plain');
    }
});

server.listen(0, '127.0.0.1', () => {
    process.stdout.write(`PORT:${server.address().port}\n`);
});
