const assert = require('assert');

const runner = require('./runner');

describe('storage.spec: storage namespace tests', () => {

    describe('storage.setData', () => {

        it('sets data without throwing errors', async () => {
            let exitCode = runner.run(`
                await Neutralino.storage.setData('container', 'value');
                await __close('done');
            `);
            assert.ok(runner.getOutput() == 'done');
        });

        it('throws an error for invalid keys', async () => {
            let exitCode = runner.run(`
                try {
                    await Neutralino.storage.setData('/home/', 'value');
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.ok(runner.getOutput() == 'NE_ST_INVSTKY');
        });


        it('removes storage record when data arg is not provided', async () => {
            let exitCode = runner.run(`
                try {
                    await Neutralino.storage.setData('container', 'value');
                    await Neutralino.storage.setData('container');
                    await Neutralino.storage.getData('container');
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.ok(runner.getOutput() == 'NE_ST_NOSTKEX');
        });
    });

    describe('storage.getData', () => {
        it('gets saved data without throwing errors', async () => {
            let exitCode = runner.run(`
                await Neutralino.storage.setData('container', 'value');
                let value = await Neutralino.storage.getData('container');
                await __close(value);
            `);
            assert.ok(runner.getOutput() == 'value');
        });

        it('throws an error for invalid keys', async () => {
            let exitCode = runner.run(`
                try {
                    await Neutralino.storage.getData('./test*', 'value');
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.ok(runner.getOutput() == 'NE_ST_INVSTKY');
        });

        it('throws an error for keys that don\'t exist', async () => {
            let exitCode = runner.run(`
                try {
                    await Neutralino.storage.getData('test_key', 'value');
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.ok(runner.getOutput() == 'NE_ST_NOSTKEX');
        });
    });

    describe('storage.getKeys', () => {
        it('returns a list of storage keys', async () => {
            let exitCode = runner.run(`
                await Neutralino.storage.setData('test_key_test', 'data');
                let keys = await Neutralino.storage.getKeys();
                await __close(JSON.stringify(keys));
            `);
            let keys = JSON.parse(runner.getOutput());
            assert.ok(Array.isArray(keys));
            assert.ok(keys.indexOf('test_key_test') != -1);
        });
    });
});
