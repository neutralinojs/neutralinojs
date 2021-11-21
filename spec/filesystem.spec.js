const assert = require('assert');

const runner = require('./runner');

describe('filesystem.spec: filesystem namespace tests', () => {

    describe('filesystem.createDirectory', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/abc');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('filesystem.removeDirectory', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.removeDirectory(NL_PATH + '/abc');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('filesystem.writeFile', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/test.txt', 'Hello');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('filesystem.readFile', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/test.txt');
                await __close(content);
            `);
            assert.equal(runner.getOutput(), 'Hello');
        });
    });

    describe('filesystem.writeBinaryFile', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                let rawBin = new ArrayBuffer(1);
                let view = new Uint8Array(rawBin);
                view[0] = 64; // Saves ASCII '@' to the binary file
                await Neutralino.filesystem.writeBinaryFile(NL_PATH + '/test.bin', rawBin);
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('filesystem.readBinaryFile', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                let buffer = await Neutralino.filesystem.readBinaryFile(NL_PATH + '/test.bin');
                let view = new Uint8Array(buffer);
                await __close(view[0].toString());
            `);
            assert.equal(runner.getOutput(), '64');
        });
    });

    describe('filesystem.removeFile', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.removeFile(NL_PATH + '/test.txt');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('filesystem.readDirectory', () => {
        it('returns directory entries', async () => {
            runner.run(`
                let entries = await Neutralino.filesystem.readDirectory(NL_PATH);
                await __close(JSON.stringify(entries));
            `);
            let entries = JSON.parse(runner.getOutput());
            assert.ok(typeof entries == 'object');
            assert.ok(entries.length > 0);
            assert.ok(entries.find((entry) => entry.type == 'DIRECTORY' && entry.entry == 'resources'))
        });
    });

    describe('filesystem.copyFile', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.copyFile(NL_PATH + '/test.txt', NL_PATH + '/test_new.txt');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('filesystem.moveFile', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.copyFile(NL_PATH + '/test_new.txt', NL_PATH + '/test.txt');
                await Neutralino.filesystem.removeFile(NL_PATH + '/test.txt'); // cleanup
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('filesystem.getStats', () => {
        it('returns file stats', async () => {
            let exitCode = runner.run(`
                let stats = await Neutralino.filesystem.getStats(NL_PATH);
                await __close(JSON.stringify(stats));
            `);
            let stats = JSON.parse(runner.getOutput());
            assert.ok(typeof stats == 'object');
            assert.ok(typeof stats.isDirectory == 'boolean');
            assert.ok(stats.isDirectory === true);
            assert.ok(typeof stats.isFile == 'boolean');
            assert.ok(stats.isFile === false);
        });
    });
});
