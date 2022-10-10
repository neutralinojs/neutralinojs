const assert = require('assert');

const runner = require('./runner');

describe('custom.spec: custom namespace tests', () => {

    describe('custom.getMethods', () => {
        it('returns the custom methods array', async () => {
            runner.run(`
                let methods = await Neutralino.custom.getMethods();
                await __close(JSON.stringify(methods));
            `);

            let methods = JSON.parse(runner.getOutput());
            assert.ok(Array.isArray(methods));
        });
    });

});
