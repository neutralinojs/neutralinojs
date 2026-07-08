const assert = require('assert');

const runner = require('./runner');

describe('net.spec: net namespace tests', () => {
    describe('net.request', () => {
        it('send a simple HTTPS GET request', async () => {
            runner.run(`
                try {
                    let res = await Neutralino.net.get('https://httpbin.io/get');
                    await __close(res.reason)
                } catch {
                    await __close("error");
                }
            `);
            assert.ok(runner.getOutput().includes('OK'));
        });
    });
});