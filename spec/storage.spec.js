const assert = require('assert');

const runner = require('./runner');

describe('storage.spec: storage namespace tests', () => {

    describe('storage.setData', () => {

        it('sets data without throwing errors', async () => {
            runner.run(`
                await Neutralino.storage.setData('container', 'value');
                await __close('done');
            `);
            assert.ok(runner.getOutput() == 'done');
        });

        it('throws an error for invalid keys', async () => {
            runner.run(`
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
            runner.run(`
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

        it('throws an error for invalid datatypes', async () => {
            runner.run(`
                try {
                    await Neutralino.storage.setData('container', 123);
                } catch(error) {
                    await __close(error.code);
                }
            `);
            assert.ok(runner.getOutput() == 'NE_RT_NATRTER');
        });

        it('persists data that is set', async () => {
            runner.run(`
                await Neutralino.storage.setData('container', 'value');
                let value = await Neutralino.storage.getData('container');
                await __close(value);
            `);
            assert.ok(runner.getOutput() == 'value');
        });

        it('updates existing data without throwing errors', async () => {
            runner.run(`
                await Neutralino.storage.setData('container', 'initial_value');
                await Neutralino.storage.setData('container', 'updated_value');
                let value = await Neutralino.storage.getData('container');
                await __close(value);
            `);
            assert.ok(runner.getOutput() == 'updated_value');
        });

        it('throws an error for empty string key', async () => {
            runner.run(`
                try {
                    await Neutralino.storage.setData('', 'value');
                }
                catch(error) {
                    await __close(error.code);
                }
            `);
            assert.ok(runner.getOutput() == 'NE_ST_INVSTKY');
        });

        it('sets large data values without throwing errors', async () => {
            runner.run(`
                let largeValue = 'N'.repeat(10000); 
                await Neutralino.storage.setData('large_value_key', largeValue);
                let value = await Neutralino.storage.getData('large_value_key');
                await __close(value);
            `);
            assert.equal(runner.getOutput().length, 10000);
        });
        
        it('handles concurrent access without errors', async () => {
            runner.run(`
                await Promise.all([
                    Neutralino.storage.setData('concurrent_key_1', 'value_1'),
                    Neutralino.storage.setData('concurrent_key_2', 'value_2')
                ]);
                let value_1 = await Neutralino.storage.getData('concurrent_key_1');
                let value_2 = await Neutralino.storage.getData('concurrent_key_2');
                await __close(JSON.stringify({value_1, value_2}));
            `);
            const output = JSON.parse(runner.getOutput());
            assert.ok(output.value_1 == 'value_1' && output.value_2 == 'value_2');
        });
    });

    describe('storage.getData', () => {
        it('gets saved data without throwing errors', async () => {
            runner.run(`
                await Neutralino.storage.setData('container', 'value');
                let value = await Neutralino.storage.getData('container');
                await __close(value);
            `);
            assert.ok(runner.getOutput() == 'value');
        });

        it('throws an error for invalid keys', async () => {
            runner.run(`
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
            runner.run(`
                try {
                    await Neutralino.storage.getData('test_key', 'value');
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.ok(runner.getOutput() == 'NE_ST_NOSTKEX');
        });
        
        it('handles concurrent access without errors', async () => {
            runner.run(`
                await Neutralino.storage.setData('concurrent_key', 'value');
                let values = await Promise.all([
                    Neutralino.storage.getData('concurrent_key'),
                    Neutralino.storage.getData('concurrent_key')
                ]);
                await __close(JSON.stringify(values));
            `);
            const values = JSON.parse(runner.getOutput());
            assert.deepStrictEqual(values, ['value', 'value']);
        });

        it('gets JSON data without throwing errors', async () => {
            runner.run(`
                let jsonData = { name: "test", value: 123 };
                await Neutralino.storage.setData('json_key', JSON.stringify(jsonData));
                let value = await Neutralino.storage.getData('json_key');
                await __close(value);
            `);
            assert.deepStrictEqual(JSON.parse(runner.getOutput()), { name: "test", value: 123 });
        });
    });

    describe('storage.getKeys', () => {
        it('returns a list of storage keys', async () => {
            runner.run(`
                await Neutralino.storage.setData('test_key_test', 'data');
                let keys = await Neutralino.storage.getKeys();
                await __close(JSON.stringify(keys));
            `);
            let keys = JSON.parse(runner.getOutput());
            assert.ok(Array.isArray(keys));
            assert.ok(keys.indexOf('test_key_test') != -1);
        });

        it('returns all stored keys', async () => {
            runner.run(`
                await Neutralino.storage.setData('key_1', 'value_1');
                await Neutralino.storage.setData('key_2', 'value_2');
                let keys = await Neutralino.storage.getKeys();
                await __close(JSON.stringify(keys));
            `);
            let keys = JSON.parse(runner.getOutput());
            assert.ok(Array.isArray(keys));
            assert.strictEqual(keys.length, 9);
            assert.ok(keys.includes('key_1'));
            assert.ok(keys.includes('key_2'));
        });     
    });
});
