const assert = require('assert');

const runner = require('./runner');

describe('storage.spec: storage namespace tests', () => {

    describe('storage.setData, storage.getData', () => {
        it('sets and sets data', async () => {
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
                    await Neutralino.storage.setData('/home/', 'value');
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
});
