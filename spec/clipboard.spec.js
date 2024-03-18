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
        it('throws an error if the parameter is not a string', async () => {
            let exitCode = runner.run(`
                try {
                    await Neutralino.clipboard.writeText(123); // Passing a number instead of a string
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });
        
        it('successfully writes special characters to the clipboard', async () => {
            let exitCode = runner.run(`
                await Neutralino.clipboard.writeText('Special characters: @#$%^&*');
                await __close('ok');
            `);
            assert.equal(runner.getOutput(), 'ok');
        });
        
        it('successfully writes a large amount of text to the clipboard', async () => {
            let largeText = 'A'.repeat(10000000);
            let exitCode = runner.run(`
                await Neutralino.clipboard.writeText('${largeText}');
                await __close('ok');
            `);
            assert.equal(runner.getOutput(), 'ok');
        });
        it('throws an error if the parameter is null', async () => {
            let exitCode = runner.run(`
                try {
                    await Neutralino.clipboard.writeText(null);
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });
        it('throws an error if the parameter is undefined', async () => {
            let exitCode = runner.run(`
                try {
                    await Neutralino.clipboard.writeText(undefined);
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });
        it('throws an error if the parameter is an object', async () => {
            let exitCode = runner.run(`
                try {
                    await Neutralino.clipboard.writeText({ key: 'value' });
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });
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
        it('returns an empty string if the clipboard is empty', async () => {
            let exitCode = runner.run(`
                await Neutralino.clipboard.clear();
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), '');
        });
        it('returns the same text for multiple consecutive reads', async () => {
            let exitCode = runner.run(`
                await Neutralino.clipboard.writeText('Test value');
                let clipboardText1 = await Neutralino.clipboard.readText();
                let clipboardText2 = await Neutralino.clipboard.readText();
                await __close(clipboardText1 === clipboardText2 ? 'ok' : 'not ok');
            `);
            assert.equal(runner.getOutput(), 'ok');
        });
        it('returns text with leading and trailing whitespaces intact', async () => {
            let exitCode = runner.run(`
                await Neutralino.clipboard.writeText('  Test value  ');
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), '  Test value  ');
        });
        it('returns text with special characters intact', async () => {
            let exitCode = runner.run(`
                await Neutralino.clipboard.writeText('@#$%^&*');
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), '@#$%^&*');
        });
    });

    describe('clipboard.clear', () => {
        it('clears the clipboard', async () => {
            let exitCode = runner.run(`
                await Neutralino.clipboard.writeText('Test value');
                await Neutralino.clipboard.clear();
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), '');
        });
        it('clears the clipboard even when it contains special characters', async () => {
            let exitCode = runner.run(`
                await Neutralino.clipboard.writeText('@#$%^&*');
                await Neutralino.clipboard.clear();
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), '');
        });
        it('clears the clipboard even when it contains a large amount of text', async () => {
            let exitCode = runner.run(`
                let largeText = 'A'.repeat(1000000);
                await Neutralino.clipboard.writeText(largeText);
                await Neutralino.clipboard.clear();
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), '');
        });
    });
