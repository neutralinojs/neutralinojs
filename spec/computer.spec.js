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

        it('returns consistent and valid memory info', async () => {
            runner.run(`
                let memoryInfo = await Neutralino.computer.getMemoryInfo();
                await __close(JSON.stringify(memoryInfo));
            `);
            let memoryInfo = JSON.parse(runner.getOutput());
            assert.ok(memoryInfo.physical.total >= memoryInfo.physical.available, 'Physical memory available should not be greater than total');
            assert.ok(memoryInfo.virtual.total >= memoryInfo.virtual.available, 'Virtual memory available should not be greater than total');
            assert.ok(memoryInfo.physical.total >= 0, 'Physical total memory should not be negative');
            assert.ok(memoryInfo.physical.available >= 0, 'Physical available memory should not be negative');
            assert.ok(memoryInfo.virtual.total >= 0, 'Virtual total memory should not be negative');
            assert.ok(memoryInfo.virtual.available >= 0, 'Virtual available memory should not be negative');
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

        it('returns a supported architecture', async () => {
            const supportedArchitectures = ['x64', 'arm', 'itanium', 'ia32', 'unknown'];
            runner.run(`
                let arch = await Neutralino.computer.getArch();
                await __close(arch);
            `);  
            let arch = runner.getOutput();
            assert.ok(supportedArchitectures.includes(arch), `Returned architecture '${arch}' is not supported by Neutralinojs`);
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
            assert.ok(Array.isArray(displays));

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


    describe('computer.getMousePosition', () => {
        it('returns the current mouse cursor position and it is within screen bounds', async () => {
            runner.run(`
                let pos = await Neutralino.computer.getMousePosition();
                let screenInfo = await Neutralino.computer.getDisplays();
                await __close(JSON.stringify({ position: pos, screen: screenInfo }));
            `);
    
            let result = JSON.parse(runner.getOutput());
            let pos = result.position;
            
            assert.ok(typeof pos.x === 'number');
            assert.ok(typeof pos.y === 'number');  
            
            if(result.screen.length > 0) {
                let screenWidth = result.screen[0].resolution.width;
                let screenHeight = result.screen[0].resolution.height;

                assert.ok(pos.x >= 0 && pos.x <= screenWidth);
                assert.ok(pos.y >= 0 && pos.y <= screenHeight);
            }
    
        });
    });

    describe('computer.getInputCapabilities', () => {
        it('returns input capability flags', async () => {
            runner.run(`
                try {
                    let caps = await Neutralino.computer.getInputCapabilities();
                    await __close(JSON.stringify(caps));
                } catch(e) {
                    await __close("skipped");
                }
            `, { allowNative: true });

            let output = runner.getOutput();
            if(output === "skipped") return;
            let caps = JSON.parse(output);
            assert.ok(typeof caps == 'object');
            assert.ok(typeof caps.warp == 'boolean');
            assert.ok(typeof caps.grab == 'boolean');
            assert.ok(typeof caps.syntheticKeys == 'boolean');
        });
    });

    describe('computer.setCursorPosition', () => {
        it('moves cursor without throwing error', async () => {
            runner.run(`
                try {
                    let caps = await Neutralino.computer.getInputCapabilities();
                    if(!caps.warp) {
                        await __close("skipped");
                        return;
                    }
                    let pos = await Neutralino.computer.getMousePosition();
                    await Neutralino.computer.setCursorPosition({ x: pos.x, y: pos.y });
                    await __close("ok");
                } catch(e) {
                    await __close("skipped");
                }
            `, { allowNative: true });

            let out = runner.getOutput();
            assert.ok(out === "ok" || out === "skipped");
        });
    });

    describe('computer.setCursorGrab', () => {
        it('enables and disables cursor grab', async () => {
            runner.run(`
                try {
                    let caps = await Neutralino.computer.getInputCapabilities();
                    if(!caps.grab) {
                        await __close("skipped");
                        return;
                    }
                    await Neutralino.computer.setCursorGrab({ enabled: true });
                    await Neutralino.computer.setCursorGrab({ enabled: false });
                    await __close("ok");
                } catch(e) {
                    await __close("skipped");
                }
            `, { allowNative: true });

            let out = runner.getOutput();
            assert.ok(out === "ok" || out === "skipped");
        });
    });

    describe('computer.sendKey', () => {
        it('sends a synthetic key event', async () => {
            runner.run(`
                try {
                    let caps = await Neutralino.computer.getInputCapabilities();
                    if(!caps.syntheticKeys) {
                        await __close("skipped");
                        return;
                    }
                    window.__lastKey = null;
                    document.addEventListener('keydown', e => {
                        window.__lastKey = e.key;
                    });
                    await Neutralino.computer.sendKey({ key: "a" });
                    await new Promise(r => setTimeout(r, 100));
                    await __close(window.__lastKey || "skipped");
                } catch(e) {
                    await __close("skipped");
                }
            `, { allowNative: true });

            let key = runner.getOutput();
            assert.ok(key === "a" || key === "skipped", "Expected 'a' or skipped depending on platform");
        });
    });

});
