const assert = require('assert');

const runner = require('./runner');

describe('computer.spec: computer namespace tests', () => {

    describe('computer.getMemoryInfo', () => {
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

    describe('computer.getArch', () => {
        it('returns the computer architecture', async () => {
            runner.run(`
                let arch = await Neutralino.computer.getArch();
                await __close(arch);
            `);
            let arch = runner.getOutput();
            assert.ok(typeof arch == 'string');
        });
    });

    describe('computer.getKernelInfo', () => {
        it('returns kernel details', async () => {
            runner.run(`
                let kernelInfo = await Neutralino.computer.getKernelInfo();
                await __close(JSON.stringify(kernelInfo));
            `);
            let kernelInfo = JSON.parse(runner.getOutput());
            assert.ok(typeof kernelInfo.variant == 'string');
            assert.ok(typeof kernelInfo.version == 'string');
        });
    });

    describe('computer.getOSInfo', () => {
        it('returns OS details', async () => {
            runner.run(`
                let osInfo = await Neutralino.computer.getOSInfo();
                await __close(JSON.stringify(osInfo));
            `);
            let osInfo = JSON.parse(runner.getOutput());
            assert.ok(typeof osInfo.name == 'string');
            assert.ok(typeof osInfo.description == 'string');
            assert.ok(typeof osInfo.version == 'string');
        });
    });

    describe('computer.getCPUInfo', () => {
        it('returns CPU details', async () => {
            runner.run(`
                let cpuInfo = await Neutralino.computer.getCPUInfo();
                await __close(JSON.stringify(cpuInfo));
            `);
            let cpuInfo = JSON.parse(runner.getOutput());
            assert.ok(typeof cpuInfo.vendor == 'string');
            assert.ok(typeof cpuInfo.model == 'string');
            assert.ok(typeof cpuInfo.frequency == 'number');
            assert.ok(typeof cpuInfo.architecture == 'string');
            assert.ok(typeof cpuInfo.logicalThreads == 'number');
            assert.ok(typeof cpuInfo.physicalCores == 'number');
            assert.ok(typeof cpuInfo.physicalUnits == 'number');
        });
    });

    describe('computer.getDisplays', () => {
        it('returns available displays', async () => {
            runner.run(`
                let displays = await Neutralino.computer.getDisplays();
                await __close(JSON.stringify(displays));
            `);
            let displays = JSON.parse(runner.getOutput());
            assert.ok(typeof displays == 'object');

            if(displays.length > 0) {
                let display = displays[0];
                assert.ok(typeof display == 'object');
                assert.ok(typeof display.id == 'number');
                assert.ok(typeof display.resolution == 'object');
                assert.ok(typeof display.resolution.width == 'number');
                assert.ok(typeof display.resolution.height == 'number');
                assert.ok(typeof display.dpi == 'number');
                assert.ok(typeof display.bpp == 'number');
                assert.ok(typeof display.refreshRate == 'number');
            }
            else {
                // No displays in the machine
            }
        });
    });
});
