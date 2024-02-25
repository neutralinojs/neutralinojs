const assert = require('assert');

const runner = require('./runner');

describe('clipboard.spec: clipboard namespace tests', () => {

    describe('clipboard.writeText', () => {
        it('throws an error if the parameter is missing', async () => {
            let exitCode = runner.run(`
                try {
                    await Neutralino.clipboard.writeText();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });
        it('works without throwing errors if parameter is provided', async () => {
            let exitCode = runner.run(`
                await Neutralino.clipboard.writeText('Test');
                await __close('ok');
            `);
            assert.equal(runner.getOutput(), 'ok');
        });
    });

    describe('clipboard.readText', () => {
        it('returns the previously stored text', async () => {
            let exitCode = runner.run(`
                await Neutralino.clipboard.writeText('Test value');
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), 'Test value');
        });
    });
});
