const assert = require('assert');

const runner = require('./runner');

describe('clipboard.spec: clipboard namespace tests', () => {

    describe('clipboard.writeText', () => {
        it('throws an error if the parameter is missing', async () => {
            runner.run(`
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
            runner.run(`
                await Neutralino.clipboard.writeText('Test');
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), 'Test');
        });
        it('throws an error if the parameter is not a string', async () => {
            runner.run(`
                try {
                    await Neutralino.clipboard.writeText(123);
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
            runner.run(`
                try {
                    await Neutralino.clipboard.writeText(null);
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
            runner.run(`
                try {
                    await Neutralino.clipboard.writeText(undefined);
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
            runner.run(`
                try {
                    await Neutralino.clipboard.writeText({ key: 'value' });
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });
        
        it('successfully writes special characters to the clipboard', async () => {
            runner.run(`
                await Neutralino.clipboard.writeText('Special characters: @#$%^&*☁☀☊☄');
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), 'Special characters: @#$%^&*☁☀☊☄');
        });
        
        it('successfully writes a large amount of text to the clipboard', async () => {
            let largeText = 'A'.repeat(10000000);
            runner.run(`
                await Neutralino.clipboard.writeText('${largeText}');
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), largeText);
        });

        it('successfully writes an empty string to the clipboard', async () => {
            runner.run(`
                await Neutralino.clipboard.writeText('');
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), '');
        });

        it('handles concurrent writes to the clipboard', async () => {
            const text1 = 'Hello';
            const text2 = 'World';
            runner.run(`
                await Promise.all([
                    Neutralino.clipboard.writeText('${text1}'),
                    Neutralino.clipboard.writeText('${text2}')
                ]);
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            const clipboardText = runner.getOutput();
            assert(
                clipboardText === text1 || clipboardText === text2,
                `Expected clipboard text to be either '${text1}' or '${text2}', but got '${clipboardText}'`
            );
        });
    });

    describe('clipboard.readText', () => {
        it('returns the previously stored text', async () => {
            runner.run(`
                await Neutralino.clipboard.writeText('Test value');
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), 'Test value');
        });

        it('returns an empty string if the clipboard is empty', async () => {
            runner.run(`
                await Neutralino.clipboard.clear();
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), '');
        });

        it('returns the same text for multiple consecutive reads', async () => {
            runner.run(`
                await Neutralino.clipboard.writeText('Test value');
                let clipboardText1 = await Neutralino.clipboard.readText();
                let clipboardText2 = await Neutralino.clipboard.readText();
                let result = {clipboardText1, clipboardText2};
                await __close(JSON.stringify(result));
            `);
            let result = await runner.getOutput();
            let resultJson = JSON.parse(result);
            assert.equal(resultJson.clipboardText1, resultJson.clipboardText2);
        });

        it('returns text with leading and trailing whitespaces intact', async () => {
            runner.run(`
                await Neutralino.clipboard.writeText('  Test value  ');
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), '  Test value  ');
        });

        it('returns text with special characters intact', async () => {
            runner.run(`
                await Neutralino.clipboard.writeText('@#$%^&*☁☀☊☄');
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), '@#$%^&*☁☀☊☄');
        });

        it('returns text with newline characters intact', async () => {
            runner.run(`
                await Neutralino.clipboard.writeText('Line1\\nLine2\\nLine3');
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), 'Line1\nLine2\nLine3');
        });
    });

    describe('clipboard.writeImage', () => {
        it('throws an error when the image data is invalid', async () => {
            runner.run(`
                try {
                    await Neutralino.clipboard.writeImage('Invalid Image Data');
                    let clipboardImage = await Neutralino.clipboard.readImage();
                    await __close(JSON.stringify(clipboardImage));
                } catch (error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        })
    })

    describe('clipboard.readImage', () => {
        it('returns null when the clipboard is empty', async () => {
            runner.run(`
                await Neutralino.clipboard.clear();
                let clipboardImage = await Neutralino.clipboard.readImage();
                await __close(JSON.stringify(clipboardImage));
            `);
            assert.equal(runner.getOutput(), 'null');
        });

        it('throws an error when the data is invalid', async () => {
            runner.run(`
                try {
                    await Neutralino.clipboard.writeImage('Invalid image data');
                    let clipboardImage = await Neutralino.clipboard.readImage();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });

        it('extracts image contents and forms a JSON object', async () => {
            
            runner.run(`
                let imageData = {
                    width: 400,
                    height: 400,
                    bpp: 32,
                    bpr: 400,
                    redMask: 0xff0000,
                    greenMask: 0x00ff00,
                    blueMask: 0x0000ff,
                    redShift: 16,
                    greenShift: 8,
                    blueShift: 0,
                    alphaMask: 0,
                    alphaShift: 0,
                    data: new ArrayBuffer(40000)
                };
                await Neutralino.clipboard.writeImage(imageData);
                let clipboardImage = await Neutralino.clipboard.readImage();
                await __close(JSON.stringify(clipboardImage));
            `);

            console.log(runner.getOutput());
        });
    })

    describe('clipboard.clear', () => {
        it('clears the clipboard', async () => {
            runner.run(`
                await Neutralino.clipboard.writeText('Test value');
                await Neutralino.clipboard.clear();
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), '');
        });
        it('clears the clipboard even when it contains special characters', async () => {
            runner.run(`
                await Neutralino.clipboard.writeText('@#$%^&*☁☀☊☄');
                await Neutralino.clipboard.clear();
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), '');
        });
        it('clears the clipboard even when it contains a large amount of text', async () => {
            runner.run(`
                let largeText = 'A'.repeat(1000000);
                await Neutralino.clipboard.writeText(largeText);
                await Neutralino.clipboard.clear();
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), '');
        });

        it('does nothing if the clipboard is already empty', async () => {
            runner.run(`
                await Neutralino.clipboard.clear();
                let clipboardText = await Neutralino.clipboard.readText();
                await __close(clipboardText);
            `);
            assert.equal(runner.getOutput(), '');
        });
    });

    describe('clipboard.getFormat', () => {
        it('returns the correct format when the format is text', async () => {
            runner.run(`
                await Neutralino.clipboard.writeText('Test value');
                let format = await Neutralino.clipboard.getFormat();
                await __close(format);
            `);
            assert.equal(runner.getOutput(), 'text');
        });

        it('returns unknown when the format is unknown', async () => {
            runner.run(`
                await Neutralino.clipboard.clear();
                let format = await Neutralino.clipboard.getFormat();
                await __close(format);
            `);
            assert.equal(runner.getOutput(), 'unknown');
        });
    })
});
