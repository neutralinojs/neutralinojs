const assert = require('assert');

const runner = require('./runner');

describe('computer.spec: computer namespace tests', () => {

    describe('app.getMemoryInfo', () => {
        it('returns physical memory info', async () => {
            let exitCode = runner.run(`
                let config = await Neutralino.computer.getMemoryInfo();
                await __close(JSON.stringify(config));
            `);
            let memoryInfo = JSON.parse(runner.getOutput());
            assert.ok(typeof memoryInfo == 'object');
            assert.ok(typeof memoryInfo.total == 'number');
            assert.ok(typeof memoryInfo.available == 'number');
        });
    });
});
