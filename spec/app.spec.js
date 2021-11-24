const assert = require('assert');

const runner = require('./runner');

describe('app.spec: app namespace tests', () => {

    describe('app.exit', () => {
        it('works without parameters', async () => {
            let exitCode = runner.run(`
                setTimeout(() => {
                    Neutralino.app.exit();
                }, 2000);
            `);
            assert.ok(typeof exitCode != undefined);
        });

        it('works with parameters', async () => {
            let exitCode = runner.run(`
                setTimeout(() => {
                    Neutralino.app.exit(1);
                }, 2000);
            `);
            assert.ok(typeof exitCode != undefined);
        });
    });

    describe('app.killProcess', () => {
        it('closes the app immediately', async () => {
            let exitCode = runner.run(`
                setTimeout(() => {
                    Neutralino.app.killProcess();
                }, 2000);
            `);
            assert.ok(exitCode != 0);
        });
    });

    describe('app.getConfig', () => {
        it('returns the config as a JSON object', async () => {
            runner.run(`
                let config = await Neutralino.app.getConfig();
                await __close(JSON.stringify(config));
            `);
            assert.ok(typeof JSON.parse(runner.getOutput()) == 'object');
        });
    });

    describe('app.broadcast', () => {
        it('triggers the registered event callback', async () => {
            let exitCode = runner.run(`
                function onTestEvent(evt) {
                    __close('done');
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.app.broadcast('testEvent');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('triggers the registered event callback with data', async () => {
            let exitCode = runner.run(`
                function onTestEvent(evt) {
                    __close(evt.detail);
                }
                await Neutralino.events.on('testEvent', onTestEvent);
                await Neutralino.app.broadcast('testEvent', 'data');
            `);
            assert.equal(runner.getOutput(), 'data');
        });

        it('throws an error for missing params', async () => {
            let exitCode = runner.run(`
                try {
                    await Neutralino.app.broadcast();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });
    });

});
