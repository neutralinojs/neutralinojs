const assert = require('assert');
const runner = require('./runner');
const path = require('path');

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

        it('executes command in the background', async () => {
            runner.run(`
                let info = await Neutralino.os.execCommand('node', { background: true });
                await __close(JSON.stringify(info));
            `);
            const info = JSON.parse(runner.getOutput());
            assert.ok(typeof info == 'object');
            assert.ok(typeof info.pid == 'number');
            assert.ok(info.exitCode == -1); 
        });

        it('executes a command in a specific directory', async () => {
            runner.run(`
                let info = await Neutralino.os.execCommand('node --version', { cwd: './' });
                await __close(JSON.stringify(info));
            `);
            const info = JSON.parse(runner.getOutput());
            assert.ok(typeof info == 'object');
            assert.ok(typeof info.pid == 'number');
            assert.ok(typeof info.stdOut == 'string');
            assert.ok(typeof info.stdErr == 'string');
            assert.ok(info.exitCode == 0);
            assert.ok(info.stdOut.includes('v'));
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

        it('assigns a unique virtual pid to each process', async () => {
            runner.run(`
                let ids = [];
                for (let i = 0; i < 10; i++) {
                    let proc = await Neutralino.os.spawnProcess('node -e "setTimeout(() => {}, 5000);"');
                    ids.push(proc.id);
                    if (i % 2 === 0) {
                        await Neutralino.os.updateSpawnedProcess(proc.id, 'exit');
                    }
                }
                await __close(JSON.stringify(ids));
            `);

            const ids = JSON.parse(runner.getOutput());
            const hasDuplicates = ids.some((id, index) => ids.indexOf(id) !== index);
            assert.ok(!hasDuplicates);
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
                let proc = await Neutralino
                            .os.spawnProcess('node -e "setTimeout(() => console.log(\\\\"done\\\\"), 1000);"');
                Neutralino.events.on('spawnedProcess', async (evt) => {
                    if(evt.detail.id == proc.id && evt.detail.action == 'stdOut') {
                        await __close(evt.detail.data.trim());
                    }
                });
            `);
            assert.equal(runner.getOutput(), 'done');
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

        it('handles long-running processes', async () => {
            runner.run(`
                let proc = await Neutralino.os.spawnProcess('node -e "setInterval(() => console.log(\\\\"running\\\\"), 1000);"');
                let receivedData = '';
                Neutralino.events.on('spawnedProcess', async (evt) => {
                    if(evt.detail.id == proc.id && evt.detail.action == 'stdOut') {
                        receivedData += evt.detail.data;
                        if (receivedData.includes('running')) {
                            await __close('done');
                        }
                    }
                });
            `);
            assert.equal(runner.getOutput(), 'done');
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

        it('returns empty array when no processes are running', async () => {
            runner.run(`
                let processes = await Neutralino.os.getSpawnedProcesses();
                await __close(JSON.stringify(processes));
            `);
            const processes = JSON.parse(runner.getOutput());
            assert.equal(processes.length, 0);
        });

        it('returns multiple spawned processes', async () => {
            runner.run(`
                await Neutralino.os.spawnProcess('node -e "setTimeout(() => {}, 5000);"');
                await Neutralino.os.spawnProcess('node -e "setTimeout(() => {}, 5000);"');
                let processes = await Neutralino.os.getSpawnedProcesses();
                await __close(JSON.stringify(processes));
            `);
            const processes = JSON.parse(runner.getOutput());
            assert.ok(processes.length >= 2);
            processes.forEach(proc => {
                assert.ok(typeof proc.id == 'number');
                assert.ok(typeof proc.pid == 'number');
                assert.ok(proc.pid > 0);
            });
        });

        it('ensures processes have valid ids and pids', async () => {
            runner.run(`
                await Neutralino.os.spawnProcess('node -e "setTimeout(() => {}, 5000);"');
                let processes = await Neutralino.os.getSpawnedProcesses();
                await __close(JSON.stringify(processes));
            `);
            const processes = JSON.parse(runner.getOutput());
            processes.forEach(proc => {
                assert.ok(typeof proc.id === 'number');
                assert.ok(typeof proc.pid === 'number');
                assert.ok(proc.id >= 0);
                assert.ok(proc.pid > 0);
            });
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
                    if(evt.detail.id == proc.id && evt.detail.action == 'exit') {
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

        it('captures stdErr output', async () => {
            runner.run(`
                let proc = await Neutralino.os.spawnProcess('node');
                Neutralino.events.on('spawnedProcess', async (evt) => {
                    if(evt.detail.id == proc.id && evt.detail.action == 'stdErr') {
                        await __close(evt.detail.data);
                    }
                });
                await Neutralino.os.updateSpawnedProcess(proc.id, 'stdIn', 'console.error("error");');
                await Neutralino.os.updateSpawnedProcess(proc.id, 'stdInEnd');
            `);
            assert.equal(runner.getOutput().trim(), 'error');
        });

        it('exits with code zero on success', async () => {
            runner.run(`
                let proc = await Neutralino.os.spawnProcess('node');
                Neutralino.events.on('spawnedProcess', async (evt) => {
                    if(evt.detail.id == proc.id && evt.detail.action == 'exit') {
                        await __close(evt.detail.data.toString());
                    }
                });
                await Neutralino.os.updateSpawnedProcess(proc.id, 'stdIn', 'console.log("done");');
                await Neutralino.os.updateSpawnedProcess(proc.id, 'stdInEnd');
            `);
            let exitCode = parseInt(runner.getOutput().trim());
            assert.equal(exitCode, 0);
        });

        it('handles multiple stdIn commands', async () => {
            runner.run(`
                let proc = await Neutralino.os.spawnProcess('node');
                let output = '';
                Neutralino.events.on('spawnedProcess', async (evt) => {
                    if(evt.detail.id == proc.id && evt.detail.action == 'stdOut') {
                        output += evt.detail.data;
                    }
                    if(evt.detail.id == proc.id && evt.detail.action == 'exit') {
                        await __close(output);
                    }
                });
                await Neutralino.os.updateSpawnedProcess(proc.id, 'stdIn', 'console.log(2 + 2);');
                await Neutralino.os.updateSpawnedProcess(proc.id, 'stdIn', 'console.log(3 + 3);');
                await Neutralino.os.updateSpawnedProcess(proc.id, 'stdInEnd');
            `);
            let output = runner.getOutput().trim().split('\n');
            assert.equal(parseInt(output[0]), 4);
            assert.equal(parseInt(output[1]), 6);
        });

        it('handles stdInEnd without stdIn', async () => {
            runner.run(`
                let proc = await Neutralino.os.spawnProcess('node');
                Neutralino.events.on('spawnedProcess', async (evt) => {
                    if(evt.detail.id == proc.id && evt.detail.action == 'exit') {
                        await __close(evt.detail.data.toString());
                    }
                });
                await Neutralino.os.updateSpawnedProcess(proc.id, 'stdInEnd');
            `);
            let exitCode = parseInt(runner.getOutput().trim());
            assert.equal(exitCode, 0);
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

        it('handles case-sensitive environment variables', async () => {
            process.env.TEST_VAR = 'testValue';
            runner.run(`
                let value = await Neutralino.os.getEnv('TEST_VAR');
                await __close(value);
            `);
            assert.equal(runner.getOutput(), 'testValue');
        });
    
        it('retrieves value with leading or trailing spaces', async () => {
            process.env.SPACE_VAR = '  spacedValue  ';
            runner.run(`
                let value = await Neutralino.os.getEnv('SPACE_VAR');
                await __close(value);
            `);
            assert.equal(runner.getOutput(), '  spacedValue  ');
        });
    
        it('retrieves value with special characters', async () => {
            process.env.SPECIAL_VAR = '@#$%^&*☊☄';
            runner.run(`
                let value = await Neutralino.os.getEnv('SPECIAL_VAR');
                await __close(value);
            `);
            assert.equal(runner.getOutput(), '@#$%^&*☊☄');
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

        it('contains npm_command environment variable', async () => {
            runner.run(`
                let envs = await Neutralino.os.getEnvs();
                await __close(JSON.stringify(envs));
            `);
            let envs = JSON.parse(runner.getOutput());
            assert.ok(envs['npm_command']);
        });

        it('checks case sensitivity of environment variable keys', async () => {
            runner.run(`
                let envs = await Neutralino.os.getEnvs();
                await __close(JSON.stringify(envs));
            `);
            let envs = JSON.parse(runner.getOutput());
            if (envs['PATH']) {
                assert.ok(!envs['Path']);
            } else if (envs['Path']) {
                assert.ok(!envs['PATH']);
            }
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
        
        it('displays notification with longer content', async () => {
            runner.run(`
                await Neutralino.os.showNotification('Details', 'This is a notification with a longer message content to test the display capabilities.');
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

        it('works when icon path is missing', async () => {
            runner.run(`
                await Neutralino.os.setTray({
                    menuItems: [
                        {id: 'id1', text: 'ID1', checked: true, disabled: false},
                        {id: 'id2', text: 'ID2'}
                    ]
                });
                await __close('done');
            `);

            assert.equal(runner.getOutput(), 'done');
        });

        it('works with empty menu items array', async () => {
            runner.run(`
                await Neutralino.os.setTray({
                    icon: '/resources/icons/appIcon.png',
                    menuItems: []
                });
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('sets a disabled and checked menu item correctly', async () => {
            runner.run(`
                await Neutralino.os.setTray({
                    icon: '/resources/icons/appIcon.png',
                    menuItems: [
                        {id: 'id1', text: 'ID1', checked: true, disabled: true}
                    ]
                });
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('works with a separator in the menu items', async () => {
            runner.run(`
                await Neutralino.os.setTray({
                    icon: '/resources/icons/appIcon.png',
                    menuItems: [
                        {id: 'id1', text: 'ID1'},
                        {id: 'separator', text: '-'},
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

        it('throws an error for non-string input', async () => {
            runner.run(`
                try {
                    await Neutralino.os.open(12345);
                } catch (error) {
                    __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
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

        it('returns a valid path', async () => {
            runner.run(`
                let documentsPath;
                documentsPath = await Neutralino.os.getPath('documents');
                await __close(documentsPath);
            `);
            let output = runner.getOutput().trim();
            let normalizedOutput = path.normalize(output);
            let isValidPath = path.isAbsolute(normalizedOutput);
            assert.ok(isValidPath);
        });

        it('throws an error for non-existent directories', async () => {
            runner.run(`
                try {
                    await Neutralino.os.getPath('nonExistentDir');
                }
                catch(error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_OS_INVKNPT');
        });
    });
});
