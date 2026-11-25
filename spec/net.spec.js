const assert = require('assert');
const runner = require('./runner');
const { startFixtureServer } = require('./servers');

describe('net.spec: network namespace tests', () => {

    const BASE_URL = 'https://httpbin.io';

    let server;

    before(async () => {
        server = await startFixtureServer();
    });

    after(() => {
        server.stop();
    });

    describe('net.get', () => {
        it('makes a GET request with query parameters', async () => {
            runner.run(`
                const options = {
                    params: {
                        name: 'Zhou',
                        age: '25',
                        city: 'Shanghai'
                    }
                };
                const response = await Neutralino.net.get('${BASE_URL}/get', options);
                const data = JSON.parse(response.text);
                await __close(JSON.stringify(data.args.name));
            `);
            const result = JSON.parse(runner.getOutput());
            const name = Array.isArray(result) ? result[0] : result;
            assert.strictEqual(name, 'Zhou');
        });
    });

    describe('net.post', () => {
        it('sends JSON data correctly', async () => {
            runner.run(`
                const testData = {
                    username: 'testuser',
                    email: 'test@example.com',
                    age: 28,
                    hobbies: ['coding', 'reading']
                };
                const options = {
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(testData)
                };
                const response = await Neutralino.net.post('${BASE_URL}/post', options);
                const data = JSON.parse(response.text);
                await __close(JSON.stringify(data.json));
            `);
            const json = JSON.parse(runner.getOutput());
            assert.strictEqual(json.username, 'testuser');
            assert.strictEqual(json.email, 'test@example.com');
            assert.strictEqual(json.age, 28);
            assert.deepStrictEqual(json.hobbies, ['coding', 'reading']);
        });

        it('sends form data correctly', async () => {
            runner.run(`
                const options = {
                    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                    body: 'username=john_doe&password=pass123'
                };
                const response = await Neutralino.net.post('${BASE_URL}/post', options);
                const data = JSON.parse(response.text);
                await __close(JSON.stringify(data.form.username));
            `);
            const result = JSON.parse(runner.getOutput());
            const username = Array.isArray(result) ? result[0] : result;
            assert.strictEqual(username, 'john_doe');
        });
    });

    describe('net.put', () => {
        it('sends PUT data correctly', async () => {
            runner.run(`
                const testData = {
                    id: 123,
                    username: 'updateduser',
                    status: 'active'
                };
                const options = {
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(testData)
                };
                const response = await Neutralino.net.put('${BASE_URL}/put', options);
                const data = JSON.parse(response.text);
                await __close(JSON.stringify(data.json));
            `);
            const json = JSON.parse(runner.getOutput());
            assert.strictEqual(json.id, 123);
            assert.strictEqual(json.username, 'updateduser');
            assert.strictEqual(json.status, 'active');
        });
    });

    describe('net.del', () => {
        it('sends DELETE data correctly', async () => {
            runner.run(`
                const testData = { id: 123, reason: 'For testing' };
                const options = {
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(testData)
                };
                const response = await Neutralino.net.del('${BASE_URL}/delete', options);
                const data = JSON.parse(response.text);
                await __close(JSON.stringify(data.json));
            `);
            const json = JSON.parse(runner.getOutput());
            assert.strictEqual(json.id, 123);
            assert.strictEqual(json.reason, 'For testing');
        });
    });

    describe('net.patch', () => {
        it('sends PATCH data correctly', async () => {
            runner.run(`
                const testData = { age: 30, status: 'updated' };
                const options = {
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify(testData)
                };
                const response = await Neutralino.net.patch('${BASE_URL}/patch', options);
                const data = JSON.parse(response.text);
                await __close(JSON.stringify(data.json));
            `);
            const json = JSON.parse(runner.getOutput());
            assert.strictEqual(json.age, 30);
            assert.strictEqual(json.status, 'updated');
        });
    });

    describe('net.head', () => {
        it('returns headers but no body', async () => {
            runner.run(`
                const response = await Neutralino.net.head('${BASE_URL}/get');
                const hasHeaders = response.headers && Object.keys(response.headers).length > 0;
                const bodyIsEmpty = !response.text || response.text === '';
                await __close(JSON.stringify({ hasHeaders, bodyIsEmpty }));
            `);
            const result = JSON.parse(runner.getOutput());
            assert.strictEqual(result.hasHeaders, true);
            assert.strictEqual(result.bodyIsEmpty, true);
        });
    });

    describe('net.options', () => {
        it('returns response headers', async () => {
            runner.run(`
                const response = await Neutralino.net.options('${BASE_URL}/');
                const hasHeaders = Object.keys(response.headers).length > 0;
                await __close(JSON.stringify(hasHeaders));
            `);
            assert.strictEqual(JSON.parse(runner.getOutput()), true);
        });
    });

    describe('net error handling', () => {
        it('handles 404 errors correctly', async () => {
            runner.run(`
                try {
                    const response = await Neutralino.net.get('${BASE_URL}/status/404');
                    await __close(JSON.stringify(response.statusCode));
                } catch (err) {
                    await __close(JSON.stringify(err.statusCode || err.code || '404'));
                }
            `);
            const output = runner.getOutput();
            assert.ok(output === '404' || output === 'NE_NW_HTTPERR');
        });
    });

    describe('net advanced options', () => {
        it('supports basic authentication', async () => {
            const authUser = 'testuser';
            const authPass = 'testpass';
            runner.run(`
                const options = {
                    auth: {
                        username: '${authUser}',
                        password: '${authPass}'
                    }
                };
                const response = await Neutralino.net.get('${BASE_URL}/basic-auth/${authUser}/${authPass}', options);
                await __close(JSON.stringify(response.statusCode));
            `);
            assert.equal(JSON.parse(runner.getOutput()), 200);
        });

        it('handles timeout', async () => {
            runner.run(`
                const options = { timeout: 1000 };
                try {
                    await Neutralino.net.get('${BASE_URL}/delay/3', options);
                } catch (err) {
                    await __close(err.code);
                }
            `);
            assert.strictEqual(runner.getOutput(), 'NE_NW_HTTPERR');
        });
    });

    describe('net response structure', () => {
        it('returns correct response fields', async () => {
            runner.run(`
                const response = await Neutralino.net.get('${BASE_URL}/get');
                const hasAllFields = response.statusCode !== undefined &&
                                    response.text !== undefined &&
                                    response.reason !== undefined &&
                                    response.headers !== undefined &&
                                    response.cookies !== undefined &&
                                    response.version !== undefined;
                await __close(JSON.stringify(hasAllFields));
            `);
            assert.strictEqual(JSON.parse(runner.getOutput()), true);
        });
    });

    describe('net binary responses', () => {
        it('delivers raw bytes for non-UTF-8 bodies', async () => {
            runner.run(`
                const res = await Neutralino.net.get('http://127.0.0.1:${server.port}/windows1250',
                    { responseType: 'binary' });
                await __close(JSON.stringify({
                    bytes: Array.from(res.bodyBytes),
                    hasTextBody: res.body !== undefined,
                    status: res.status,
                    contentType: res.contentType
                }));
            `);
            const out = JSON.parse(runner.getOutput());

            assert.deepStrictEqual(out.bytes, [60, 97, 62, 236, 154, 232, 248, 60, 47, 97, 62]);
            assert.strictEqual(out.hasTextBody, false);
            assert.strictEqual(out.status, 200);
            assert.match(out.contentType, /windows-1250/);
        });

        it('keeps all 256 byte values intact', async () => {
            runner.run(`
                const res = await Neutralino.net.get('http://127.0.0.1:${server.port}/all-bytes',
                    { responseType: 'binary' });
                await __close(JSON.stringify(Array.from(res.bodyBytes)));
            `);

            assert.deepStrictEqual(JSON.parse(runner.getOutput()),
                Array.from({ length: 256 }, (_, i) => i));
        });

        it('handles empty and bodyless responses', async () => {
            runner.run(`
                const empty = await Neutralino.net.get('http://127.0.0.1:${server.port}/empty',
                    { responseType: 'binary' });
                const none = await Neutralino.net.get('http://127.0.0.1:${server.port}/no-content',
                    { responseType: 'binary' });
                await __close(JSON.stringify({
                    empty: Array.from(empty.bodyBytes),
                    noneStatus: none.status,
                    none: Array.from(none.bodyBytes)
                }));
            `);
            const out = JSON.parse(runner.getOutput());

            assert.deepStrictEqual(out.empty, []);
            assert.strictEqual(out.noneStatus, 204);
            assert.deepStrictEqual(out.none, []);
        });

        it('handles a body larger than the read buffer', async () => {
            const size = 1024 * 1024;
            const fill = 0x80 + (size % 96);

            runner.run(`
                const res = await Neutralino.net.get('http://127.0.0.1:${server.port}/large?size=${size}',
                    { responseType: 'binary' });
                await __close(JSON.stringify({
                    length: res.bodyBytes.length,
                    first: res.bodyBytes[0],
                    last: res.bodyBytes[res.bodyBytes.length - 1]
                }));
            `);
            const out = JSON.parse(runner.getOutput());

            assert.strictEqual(out.length, size);
            assert.strictEqual(out.first, fill);
            assert.strictEqual(out.last, fill);
        });

        it('keeps the text body when responseType is not set', async () => {
            runner.run(`
                const res = await Neutralino.net.get('http://127.0.0.1:${server.port}/utf8');
                await __close(JSON.stringify({
                    text: res.body,
                    hasBytes: res.bodyBytes !== undefined
                }));
            `);
            const out = JSON.parse(runner.getOutput());

            assert.strictEqual(out.text, '<a>ěščř</a>');
            assert.strictEqual(out.hasBytes, false);
        });
    });

    describe('net.fetch', () => {
        it('returns a fetch-like response', async () => {
            runner.run(`
                const res = await Neutralino.net.fetch('http://127.0.0.1:${server.port}/windows1250');
                const clone = res.clone();
                await __close(JSON.stringify({
                    ok: res.ok,
                    status: res.status,
                    type: res.type,
                    url: res.url,
                    headers: res.headers.get('content-type'),
                    bytes: Array.from(new Uint8Array(await res.arrayBuffer())),
                    bodyUsed: res.bodyUsed,
                    cloneText: await clone.text()
                }));
            `);
            const out = JSON.parse(runner.getOutput());

            assert.strictEqual(out.ok, true);
            assert.strictEqual(out.status, 200);
            assert.strictEqual(out.type, 'basic');
            assert.match(out.url, /windows1250$/);
            assert.match(out.headers, /windows-1250/);
            assert.deepStrictEqual(out.bytes, [60, 97, 62, 236, 154, 232, 248, 60, 47, 97, 62]);
            assert.strictEqual(out.bodyUsed, true);
            // text() decodes as UTF-8, so these bytes become replacement characters
            assert.match(out.cloneText, /^<a>\uFFFD+<\/a>$/);
        });

        it('decodes UTF-8 text and JSON bodies', async () => {
            runner.run(`
                const base = 'http://127.0.0.1:${server.port}';
                await __close(JSON.stringify({
                    text: await (await Neutralino.net.fetch(base + '/utf8')).text(),
                    json: await (await Neutralino.net.fetch(base + '/json')).json()
                }));
            `);
            const out = JSON.parse(runner.getOutput());

            assert.strictEqual(out.text, '<a>ěščř</a>');
            assert.deepStrictEqual(out.json, { ok: true, name: 'ěščř' });
        });

        it('handles bodyless responses', async () => {
            runner.run(`
                const res = await Neutralino.net.fetch('http://127.0.0.1:${server.port}/no-content');
                await __close(JSON.stringify({
                    status: res.status,
                    text: await res.text()
                }));
            `);
            const out = JSON.parse(runner.getOutput());

            assert.strictEqual(out.status, 204);
            assert.strictEqual(out.text, '');
        });

        it('follows redirects and keeps non-ok statuses', async () => {
            runner.run(`
                const base = 'http://127.0.0.1:${server.port}';
                await __close(JSON.stringify({
                    redirect: (await Neutralino.net.fetch(base + '/redirect')).status,
                    error: (await Neutralino.net.fetch(base + '/status/401')).status
                }));
            `);
            const out = JSON.parse(runner.getOutput());

            assert.strictEqual(out.redirect, 200);
            assert.strictEqual(out.error, 401);
        });

        it('sends the request init', async () => {
            runner.run(`
                const echo = await (await Neutralino.net.fetch('http://127.0.0.1:${server.port}/echo', {
                    method: 'POST',
                    headers: { 'X-Test': 'yes' },
                    body: new URLSearchParams({ a: '1' })
                })).json();
                await __close(JSON.stringify({
                    method: echo.method,
                    header: echo.headers['x-test'],
                    contentType: echo.headers['content-type'],
                    body: echo.body
                }));
            `);
            const out = JSON.parse(runner.getOutput());

            assert.strictEqual(out.method, 'POST');
            assert.strictEqual(out.header, 'yes');
            assert.strictEqual(out.contentType, 'application/x-www-form-urlencoded;charset=UTF-8');
            assert.strictEqual(out.body, 'a=1');
        });

        it('applies the fetch Content-Type for string bodies', async () => {
            runner.run(`
                const echo = await (await Neutralino.net.fetch('http://127.0.0.1:${server.port}/echo', {
                    method: 'POST',
                    body: 'hello'
                })).json();
                await __close(JSON.stringify({
                    contentType: echo.headers['content-type'],
                    body: echo.body
                }));
            `);
            const out = JSON.parse(runner.getOutput());

            assert.strictEqual(out.contentType, 'text/plain;charset=UTF-8');
            assert.strictEqual(out.body, 'hello');
        });

        it('rejects unsupported request bodies', async () => {
            runner.run(`
                try {
                    await Neutralino.net.fetch('http://127.0.0.1:${server.port}/echo', {
                        method: 'POST',
                        body: new Blob(['x'])
                    });
                    await __close('resolved');
                }
                catch(err) {
                    await __close(err.name);
                }
            `);

            assert.strictEqual(runner.getOutput(), 'TypeError');
        });

        it('rejects when redirect mode is "error" and the server redirects', async () => {
            runner.run(`
                try {
                    await Neutralino.net.fetch('http://127.0.0.1:${server.port}/redirect', { redirect: 'error' });
                    await __close('resolved');
                }
                catch(err) {
                    await __close(err.name);
                }
            `);

            assert.strictEqual(runner.getOutput(), 'TypeError');
        });
    });
});
