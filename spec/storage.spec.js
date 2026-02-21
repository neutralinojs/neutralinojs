const assert = require('assert');
const runner = require('./runner');

describe('storage.spec: storage namespace tests', () => {

    beforeEach(() => {
        runner.run(`
            await Neutralino.storage.clear();
            await __close('done');
        `);
        assert.strictEqual(runner.getOutput(), 'done');
    });

    describe('storage.setData', () => {

        it('sets data without throwing errors', () => {
            runner.run(`
                await Neutralino.storage.setData('container', 'value');
                await __close('done');
            `);
            assert.strictEqual(runner.getOutput(), 'done');
        });

        it('throws an error for invalid keys', () => {
            runner.run(`
                try {
                    await Neutralino.storage.setData('/home/', 'value');
                } catch(err) {
                    await __close(err.code);
                }
            `);
            assert.strictEqual(runner.getOutput(), 'NE_ST_INVSTKY');
        });

        it('removes storage record when data arg is not provided', () => {
            runner.run(`
                try {
                    await Neutralino.storage.setData('container', 'value');
                    await Neutralino.storage.setData('container');
                    await Neutralino.storage.getData('container');
                } catch(err) {
                    await __close(err.code);
                }
            `);
            assert.strictEqual(runner.getOutput(), 'NE_ST_NOSTKEX');
        });

        it('throws an error for invalid datatypes', () => {
            runner.run(`
                try {
                    await Neutralino.storage.setData('container', 123);
                } catch(error) {
                    await __close(error.code);
                }
            `);
            assert.strictEqual(runner.getOutput(), 'NE_RT_NATRTER');
        });

        it('persists data that is set', () => {
            runner.run(`
                await Neutralino.storage.setData('container', 'value');
                let value = await Neutralino.storage.getData('container');
                await __close(value);
            `);
            assert.strictEqual(runner.getOutput(), 'value');
        });

        it('updates existing data without throwing errors', () => {
            runner.run(`
                await Neutralino.storage.setData('container', 'initial_value');
                await Neutralino.storage.setData('container', 'updated_value');
                let value = await Neutralino.storage.getData('container');
                await __close(value);
            `);
            assert.strictEqual(runner.getOutput(), 'updated_value');
        });

        it('throws an error for empty string key', () => {
            runner.run(`
                try {
                    await Neutralino.storage.setData('', 'value');
                } catch(error) {
                    await __close(error.code);
                }
            `);
            assert.strictEqual(runner.getOutput(), 'NE_ST_INVSTKY');
        });

        it('sets large data values without throwing errors', () => {
            runner.run(`
                let largeValue = 'N'.repeat(10000); 
                await Neutralino.storage.setData('large_value_key', largeValue);
                let value = await Neutralino.storage.getData('large_value_key');
                await __close(value);
            `);
            assert.strictEqual(runner.getOutput().length, 10000);
        });

        it('handles concurrent access without errors', () => {
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
            assert.strictEqual(output.value_1, 'value_1');
            assert.strictEqual(output.value_2, 'value_2');
        });
    });

    describe('storage.getData', () => {

        it('gets saved data without throwing errors', () => {
            runner.run(`
                await Neutralino.storage.setData('container', 'value');
                let value = await Neutralino.storage.getData('container');
                await __close(value);
            `);
            assert.strictEqual(runner.getOutput(), 'value');
        });

        it('throws an error for invalid keys', () => {
            runner.run(`
                try {
                    await Neutralino.storage.getData('./test*');
                } catch(err) {
                    await __close(err.code);
                }
            `);
            assert.strictEqual(runner.getOutput(), 'NE_ST_INVSTKY');
        });

        it('throws an error for keys that don\'t exist', () => {
            runner.run(`
                try {
                    await Neutralino.storage.getData('test_key');
                } catch(err) {
                    await __close(err.code);
                }
            `);
            assert.strictEqual(runner.getOutput(), 'NE_ST_NOSTKEX');
        });

        it('handles concurrent access without errors', () => {
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

        it('gets JSON data without throwing errors', () => {
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

        it('returns a list of storage keys', () => {
            runner.run(`
                await Neutralino.storage.setData('test_key_test', 'data');
                let keys = await Neutralino.storage.getKeys();
                await __close(JSON.stringify(keys));
            `);
            const keys = JSON.parse(runner.getOutput());
            assert.ok(Array.isArray(keys));
            assert.ok(keys.includes('test_key_test'));
        });

        it('returns all stored keys created in this test only', () => {
            runner.run(`
                await Neutralino.storage.setData('key_1', 'value_1');
                await Neutralino.storage.setData('key_2', 'value_2');
                let keys = await Neutralino.storage.getKeys();
                await __close(JSON.stringify(keys));
            `);
            const keys = JSON.parse(runner.getOutput());
            assert.ok(Array.isArray(keys));
            assert.strictEqual(keys.length, 2);
            assert.ok(keys.includes('key_1'));
            assert.ok(keys.includes('key_2'));
        });
    });
});