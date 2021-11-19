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
            assert.ok(exitCode);
        });

        it('works with parameters', async () => {
            let exitCode = runner.run(`
                setTimeout(() => {
                    Neutralino.app.exit(1);
                }, 2000);
            `);
            assert.ok(exitCode);
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
            let exitCode = runner.run(`
                let config = await Neutralino.app.getConfig();
                await __close(JSON.stringify(config));
            `);
            assert.ok(typeof JSON.parse(runner.getOutput()) == 'object');
        });
    });
});
