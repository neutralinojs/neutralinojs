const assert = require('assert');
const runner = require('./runner');

describe('net.spec: network namespace tests', () => {

    const BASE_URL = 'https://httpbin.io';

    describe('net.get', () => {
        it('makes a GET request without throwing errors', async () => {
            runner.run(`
                const response = await Neutralino.net.get('${BASE_URL}/get');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('returns correct status code for GET request', async () => {
            runner.run(`
                const response = await Neutralino.net.get('${BASE_URL}/get');
                await __close(JSON.stringify(response.statusCode));
            `);
            assert.equal(JSON.parse(runner.getOutput()), 200);
        });

        it('handles query parameters correctly', async () => {
            runner.run(`
                const options = {
                    params: {
                        name: '张三',
                        age: '25',
                        city: '上海'
                    }
                };
                const response = await Neutralino.net.get('${BASE_URL}/get', options);
                const data = JSON.parse(response.text);
                await __close(JSON.stringify(data.args.name));
            `);
            const result = JSON.parse(runner.getOutput());
            const name = Array.isArray(result) ? result[0] : result;
            assert.strictEqual(name, '张三');
        });

        it('handles custom headers', async () => {
            runner.run(`
                const options = {
                    headers: {
                        'X-Custom-Header': 'TestValue',
                        'X-Request-ID': '12345'
                    }
                };
                const response = await Neutralino.net.get('${BASE_URL}/headers', options);
                const data = JSON.parse(response.text);
                await __close(JSON.stringify(data.headers['X-Custom-Header']));
            `);
            const result = JSON.parse(runner.getOutput());
            const headerValue = Array.isArray(result) ? result[0] : result;
            assert.strictEqual(headerValue, 'TestValue');
        });

        it('handles timeout', async () => {
            runner.run(`
                const options = {
                    timeout: 1000
                };
                try {
                    await Neutralino.net.get('${BASE_URL}/delay/3', options);
                } catch (err) {
                    await __close(err.code);
                }
            `);
            assert.strictEqual(runner.getOutput(), 'NE_NW_HTTPERR');
        });
    });

    describe('net.post', () => {
        it('makes a POST request without throwing errors', async () => {
            runner.run(`
                const options = {
                    body: JSON.stringify({ test: 'data' })
                };
                const response = await Neutralino.net.post('${BASE_URL}/post', options);
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('returns correct status code for POST request', async () => {
            runner.run(`
                const options = {
                    body: JSON.stringify({ test: 'data' })
                };
                const response = await Neutralino.net.post('${BASE_URL}/post', options);
                await __close(JSON.stringify(response.statusCode));
            `);
            assert.equal(JSON.parse(runner.getOutput()), 200);
        });

        it('sends JSON data correctly', async () => {
            runner.run(`
                const testData = {
                    username: 'testuser',
                    email: 'test@example.com',
                    age: 28,
                    hobbies: ['coding', 'reading']
                };
                const options = {
                    headers: {
                        'Content-Type': 'application/json'
                    },
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
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded'
                    },
                    body: 'username=john_doe&password=pass123&remember=true'
                };
                const response = await Neutralino.net.post('${BASE_URL}/post', options);
                const data = JSON.parse(response.text);
                await __close(JSON.stringify(data.form.username));
            `);
            const result = JSON.parse(runner.getOutput());
            const username = Array.isArray(result) ? result[0] : result;
            assert.strictEqual(username, 'john_doe');
        });

        it('handles empty body', async () => {
            runner.run(`
                const response = await Neutralino.net.post('${BASE_URL}/post');
                await __close(JSON.stringify(response.statusCode));
            `);
            assert.equal(JSON.parse(runner.getOutput()), 200);
        });
    });

    describe('net.put', () => {
        it('makes a PUT request without throwing errors', async () => {
            runner.run(`
                const options = {
                    body: JSON.stringify({ id: 123, status: 'updated' })
                };
                const response = await Neutralino.net.put('${BASE_URL}/put', options);
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('returns correct status code for PUT request', async () => {
            runner.run(`
                const options = {
                    body: JSON.stringify({ id: 123, status: 'updated' })
                };
                const response = await Neutralino.net.put('${BASE_URL}/put', options);
                await __close(JSON.stringify(response.statusCode));
            `);
            assert.equal(JSON.parse(runner.getOutput()), 200);
        });

        it('sends PUT data correctly', async () => {
            runner.run(`
                const testData = {
                    id: 123,
                    username: 'updateduser',
                    email: 'updated@example.com',
                    age: 29,
                    status: 'active'
                };
                const options = {
                    headers: {
                        'Content-Type': 'application/json'
                    },
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
        it('makes a DELETE request without throwing errors', async () => {
            runner.run(`
                const options = {
                    body: JSON.stringify({ id: 123, reason: 'test deletion' })
                };
                const response = await Neutralino.net.del('${BASE_URL}/delete', options);
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('returns correct status code for DELETE request', async () => {
            runner.run(`
                const options = {
                    body: JSON.stringify({ id: 123 })
                };
                const response = await Neutralino.net.del('${BASE_URL}/delete', options);
                await __close(JSON.stringify(response.statusCode));
            `);
            assert.equal(JSON.parse(runner.getOutput()), 200);
        });

        it('sends DELETE data correctly', async () => {
            runner.run(`
                const testData = {
                    id: 123,
                    reason: '测试删除'
                };
                const options = {
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify(testData)
                };
                const response = await Neutralino.net.del('${BASE_URL}/delete', options);
                const data = JSON.parse(response.text);
                await __close(JSON.stringify(data.json));
            `);
            const json = JSON.parse(runner.getOutput());
            assert.strictEqual(json.id, 123);
            assert.strictEqual(json.reason, '测试删除');
        });
    });

    describe('net.patch', () => {
        it('makes a PATCH request without throwing errors', async () => {
            runner.run(`
                const options = {
                    body: JSON.stringify({ age: 30 })
                };
                const response = await Neutralino.net.patch('${BASE_URL}/patch', options);
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('returns correct status code for PATCH request', async () => {
            runner.run(`
                const options = {
                    body: JSON.stringify({ age: 30 })
                };
                const response = await Neutralino.net.patch('${BASE_URL}/patch', options);
                await __close(JSON.stringify(response.statusCode));
            `);
            assert.equal(JSON.parse(runner.getOutput()), 200);
        });

        it('sends PATCH data correctly', async () => {
            runner.run(`
                const testData = {
                    age: 30,
                    status: 'updated'
                };
                const options = {
                    headers: {
                        'Content-Type': 'application/json'
                    },
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
        it('makes a HEAD request without throwing errors', async () => {
            runner.run(`
                const response = await Neutralino.net.head('${BASE_URL}/get');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('returns correct status code for HEAD request', async () => {
            runner.run(`
                const response = await Neutralino.net.head('${BASE_URL}/get');
                await __close(JSON.stringify(response.statusCode));
            `);
            assert.equal(JSON.parse(runner.getOutput()), 200);
        });

        it('returns headers but no body for HEAD request', async () => {
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
        it('makes an OPTIONS request without throwing errors', async () => {
            runner.run(`
                const response = await Neutralino.net.options('${BASE_URL}/');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('returns correct status code for OPTIONS request', async () => {
            runner.run(`
                const response = await Neutralino.net.options('${BASE_URL}/');
                await __close(JSON.stringify(response.statusCode));
            `);
            assert.equal(JSON.parse(runner.getOutput()), 200);
        });

        it('returns response headers for OPTIONS request', async () => {
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

        it('handles network errors gracefully', async () => {
            runner.run(`
                try {
                    await Neutralino.net.get('https://invalid-domain-that-does-not-exist.xyz');
                } catch (err) {
                    await __close(err.code);
                }
            `);
            assert.ok(runner.getOutput().startsWith('NE_NW_'));
        });
    });

    describe('net advanced options', () => {
        it('supports custom timeout', async () => {
            runner.run(`
                const options = {
                    timeout: 5000
                };
                const response = await Neutralino.net.get('${BASE_URL}/get', options);
                await __close(JSON.stringify(response.statusCode));
            `);
            assert.equal(JSON.parse(runner.getOutput()), 200);
        });

        it('supports authentication', async () => {
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

        it('handles failed authentication', async () => {
            const authUser = 'testuser';
            const authPass = 'testpass';
            runner.run(`
                const options = {
                    auth: {
                        username: 'wrong',
                        password: 'wrong'
                    }
                };
                try {
                    const response = await Neutralino.net.get('${BASE_URL}/basic-auth/${authUser}/${authPass}', options);
                    await __close(JSON.stringify(response.statusCode));
                } catch (err) {
                    await __close(JSON.stringify(err.statusCode || err.code || '401'));
                }
            `);
            const output = runner.getOutput();
            assert.ok(output === '401' || output === 'NE_NW_HTTPERR');
        });
    });

    describe('net response structure', () => {
        it('returns correct response structure', async () => {
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