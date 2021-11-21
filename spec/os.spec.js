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


    describe('os.getEnv', () => {
        it('returns an environment variable value', async () => {
            runner.run(`
                let value = await Neutralino.os.getEnv('PATH');
                await __close(value);
            `);
            assert.ok(runner.getOutput().length > 0);
        });

        it('throws an error if the key doesn\'t exist', async () => {
            runner.run(`
                try {
                    await Neutralino.os.getEnv('test_env_key');
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_OS_ENVNOEX');
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
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.os.open('http://neutralino.js.org');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
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
