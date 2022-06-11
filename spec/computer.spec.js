const assert = require('assert');

const runner = require('./runner');

describe('computer.spec: computer namespace tests', () => {

    describe('app.getMemoryInfo', () => {
        it('returns physical memory info', async () => {
            runner.run(`
                let memoryInfo = await Neutralino.computer.getMemoryInfo();
                await __close(JSON.stringify(memoryInfo));
            `);
            let memoryInfo = JSON.parse(runner.getOutput());
            assert.ok(typeof memoryInfo == 'object');
            assert.ok(typeof memoryInfo.physical == 'object');
            assert.ok(typeof memoryInfo.virtual == 'object');
            assert.ok(typeof memoryInfo.physical.total == 'number');
            assert.ok(typeof memoryInfo.physical.available == 'number');
            assert.ok(typeof memoryInfo.virtual.total == 'number');
            assert.ok(typeof memoryInfo.virtual.available == 'number');
        });
    });
    
    describe('app.getArch', () => {
        it('returns the computer architecture', async () => {
            runner.run(`
                let arch = await Neutralino.computer.getArch();
                await __close(arch);
            `);
            let arch = runner.getOutput();
            assert.ok(typeof arch == 'string');
        });
    });
});
