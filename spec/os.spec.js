const assert = require('assert');

const runner = require('./runner');

describe('os.spec: os namespace tests', () => {

    describe('os.execCommand', () => {
        it('executes a command and returns result', async () => {
            runner.run(`
                let info = await Neutralino.os.execCommand('node --version');
                await __close(JSON.stringify(info));
            `);
            let info = JSON.parse(runner.getOutput());
            assert.ok(typeof info == 'object');
            assert.ok(typeof info.pid == 'number');
            assert.ok(typeof info.stdErr == 'string');
            assert.ok(typeof info.stdOut == 'string');
            assert.ok(typeof info.exitCode == 'number');

            assert.ok(info.stdOut.charAt(0) == 'v');
        });

        it('accepts stdIn', async () => {
            runner.run(`
                let info = await Neutralino.os.execCommand('node', {stdIn: 'console.log("N");'});
                await __close(JSON.stringify(info));
            `);
            let info = JSON.parse(runner.getOutput());

            assert.ok(info.stdOut.charAt(0) == 'N');
        });
    });


     describe('os.spawnProcess', () => {
        it('spawns a processes and returns the virtual pid and pid', async () => {
            runner.run(`
                let vids = [];
                let vid = await Neutralino.os.spawnProcess('node'); // This is non-blocking (multi-threaded)
                vids.push(vid);
                vid = await Neutralino.os.spawnProcess('node');
                vids.push(vid);
                await __close(JSON.stringify(vids));
            `);
            let vids = JSON.parse(runner.getOutput());
            assert.equal(vids[0].id, 0);
            assert.equal(vids[1].id, 1);
            assert.ok(vids[0].pid > 0);
            assert.ok(vids[1].pid > 0);
        });


        it('sends the exit code with the exit action via the spawnProcess event', async () => {
            runner.run(`

                let proc = await Neutralino.os.spawnProcess('node --version');
                // Immediate command, so we get exit code instantly
                Neutralino.events.on('spawnedProcess', async (evt) => {
                    if(evt.detail.id == proc.id && evt.detail.action == 'exit') {
                        await __close(evt.detail.data.toString());
                    }
                });
            `);
            let vid = parseInt(runner.getOutput());
            assert.equal(vid, 0);
        });

        it('sends stdOut with the stdOut action via the spawnProcess event', async () => {
            runner.run(`
                let proc = Neutralino.os.spawnProcess('node --version');
                Neutralino.events.on('spawnedProcess', async (evt) => {
                    if(evt.detail.action == 'stdOut') {
                        await __close(evt.detail.data);
                    }
                });
            `);
            let output = runner.getOutput();
            assert.ok(output.charAt(0) == 'v');
        });

        it('sends stdErr with the stdErr action via the spawnProcess event', async () => {
            runner.run(`
                let proc = await Neutralino.os.spawnProcess('node --unknown-option');
                Neutralino.events.on('spawnedProcess', async (evt) => {
                    if(evt.detail.id == proc.id && evt.detail.action == 'stdErr') {
                        await __close(evt.detail.data);
                    }
                });
            `);
            let output = runner.getOutput();
            assert.ok(output.length > 0);
        });
    });


    describe('os.getSpawnedProcesses', () => {
        it('returns spawned processes', async () => {
            runner.run(`
                let vid = await Neutralino.os.spawnProcess('node');
                let processes = await Neutralino.os.getSpawnedProcesses();
                await __close(JSON.stringify(processes));
            `);
            let processes = JSON.parse(runner.getOutput());
            assert.ok(processes.length > 0);
            assert.ok(typeof processes[0].id == 'number');
            assert.ok(typeof processes[0].pid == 'number');
            assert.equal(processes[0].id, 0);
            assert.ok(processes[0].pid > 0);
        });
    });

    describe('os.updateSpawnedProcess', () => {
        it('accepts stdIn and stdInEnd actions', async () => {
            runner.run(`
                let proc = await Neutralino.os.spawnProcess('node');
                Neutralino.events.on('spawnedProcess', async (evt) => {
                    if(evt.detail.id == proc.id && evt.detail.action == 'stdOut') {
                        await __close(evt.detail.data);
                    }
                });
                await Neutralino.os.updateSpawnedProcess(proc.id, 'stdIn', 'console.log(5+5);');
                await Neutralino.os.updateSpawnedProcess(proc.id, 'stdInEnd');
            `);
            let output = parseInt(runner.getOutput().trim());
            assert.equal(output, 10);
        });

        it('accepts the exit action', async () => {
            runner.run(`
                let proc = await Neutralino.os.spawnProcess('node');
                Neutralino.events.on('spawnedProcess', async (evt) => {
                    if(evt.detail.action == 'exit') {
                        await __close(evt.detail.data.toString());
                    }
                });
                setTimeout(async () => {
                    await Neutralino.os.updateSpawnedProcess(proc.id, 'exit');
                }, 1000);
            `);
            let exitCode = parseInt(runner.getOutput().trim());
            assert.ok(exitCode > 0);
        });
    });

    describe('os.getEnv', () => {
        it('returns an environment variable value', async () => {
            runner.run(`
                let value = await Neutralino.os.getEnv('PATH');
                await __close(value);
            `);
            assert.ok(runner.getOutput().length > 0);
        });

        it('returns an empty string if the key doesn\'t exist', async () => {
            runner.run(`
                let value = await Neutralino.os.getEnv('test_env_key');
                await __close(value);
            `);
            assert.equal(runner.getOutput(), '');
        });

        it('throws an error for missing args', async () => {
            runner.run(`
                try {
                    await Neutralino.os.getEnv();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });
    });

    describe('os.getEnvs', () => {
        it('returns all environment variables', async () => {
            runner.run(`
                let envs = await Neutralino.os.getEnvs();
                await __close(JSON.stringify(envs));
            `);
            let envs = JSON.parse(runner.getOutput());
            assert.ok(typeof envs == 'object');
            assert.ok(envs['PATH'] || envs['Path']);
        });
    });

    describe('os.showOpenDialog', () => {
        it('exports the function to the app', async () => {
            runner.run(`
                await __close(typeof Neutralino.os.showOpenDialog);
            `);
            assert.equal(runner.getOutput(), 'function');
        });
    });

    describe('os.showFolderDialog', () => {
        it('exports the function to the app', async () => {
            runner.run(`
                await __close(typeof Neutralino.os.showFolderDialog);
            `);
            assert.equal(runner.getOutput(), 'function');
        });
    });

    describe('os.showSaveDialog', () => {
        it('exports the function to the app', async () => {
            runner.run(`
                await __close(typeof Neutralino.os.showSaveDialog);
            `);
            assert.equal(runner.getOutput(), 'function');
        });
    });

    describe('os.showNotification', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.os.showNotification('Hello', 'Neutralinojs');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('os.showMessageBox', () => {
        it('exports the function to the app', async () => {
            runner.run(`
                await __close(typeof Neutralino.os.showMessageBox);
            `);
            assert.equal(runner.getOutput(), 'function');
        });
    });

    describe('os.setTray', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.os.setTray({
                    icon: '/resources/icons/appIcon.png',
                    menuItems: [
                        {id: 'id1', text: 'ID1', checked: true, disabled: false},
                        {id: 'id2', text: 'ID2'}
                    ]
                });
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('os.open', () => {
        it('exports the function to the app', async () => {
            runner.run(`
                await __close(typeof Neutralino.os.open);
            `);
            assert.equal(runner.getOutput(), 'function');
        });
    });

    describe('os.getPath', () => {
        it('returns a known directory', async () => {
            runner.run(`
                let downloadsPath = await Neutralino.os.getPath('downloads');
                await __close(downloadsPath);
            `);
            assert.ok(typeof runner.getOutput() == 'string');
        });
    });
});
