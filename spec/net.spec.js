const assert = require('assert');
const runner = require('./runner');

describe('net.spec: network namespace tests', () => {

    const BASE_URL = 'https://httpbin.io';

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
});