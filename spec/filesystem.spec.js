const assert = require('assert');

const runner = require('./runner');

describe('filesystem.spec: filesystem namespace tests', () => {

    describe('filesystem.createDirectory', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/abc');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('filesystem.removeDirectory', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/abcd');
                await Neutralino.filesystem.removeDirectory(NL_PATH + '/.tmp/abcd');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('filesystem.writeFile', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('filesystem.appendFile', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.appendFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await Neutralino.filesystem.appendFile(NL_PATH + '/.tmp/test.txt', 'World');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('appends content to files', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await Neutralino.filesystem.appendFile(NL_PATH + '/.tmp/test.txt', ' World');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test.txt');
                await __close(content);
            `);
            assert.equal(runner.getOutput(), 'Hello World');
        });
    });

    describe('filesystem.readFile', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test.txt');
                await __close(content);
            `);
            assert.equal(runner.getOutput(), 'Hello');
        });

        it('allows changing file cursor', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'World');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test.txt', { pos: 2 });
                await __close(content);
            `);
            assert.equal(runner.getOutput(), 'rld');
        });

        it('allows changing file reader buffer size', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Neutralinojs');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test.txt', { size: 3 });
                await __close(content);
            `);
            assert.equal(runner.getOutput(), 'Neu');
        });

        it('works properly with both pos and size options', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Neutralinojs');
                let content = await Neutralino.filesystem
                                    .readFile(NL_PATH + '/.tmp/test.txt', { pos: 6, size: 4 });
                await __close(content);
            `);
            assert.equal(runner.getOutput(), 'lino');
        });
    });

    describe('filesystem.writeBinaryFile', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                let rawBin = new ArrayBuffer(1);
                let view = new Uint8Array(rawBin);
                view[0] = 64; // Saves ASCII '@' to the binary file
                await Neutralino.filesystem.writeBinaryFile(NL_PATH + '/.tmp/test.bin', rawBin);
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('filesystem.appendBinaryFile', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                let rawBin = new ArrayBuffer(1);
                let view = new Uint8Array(rawBin);
                view[0] = 64; // Saves ASCII '@' to the binary file
                await Neutralino.filesystem.appendBinaryFile(NL_PATH + '/.tmp/test.bin', rawBin);
                await Neutralino.filesystem.appendBinaryFile(NL_PATH + '/.tmp/test.bin', rawBin);
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('appends binary content to files', async () => {
            runner.run(`
                let rawBin = new ArrayBuffer(1);
                let view = new Uint8Array(rawBin);
                view[0] = 64; // Saves ASCII '@' to the binary file
                await Neutralino.filesystem.appendBinaryFile(NL_PATH + '/.tmp/test.bin', rawBin);
                await Neutralino.filesystem.appendBinaryFile(NL_PATH + '/.tmp/test.bin', rawBin);
                let arrayBuffer = await Neutralino.filesystem.readBinaryFile(NL_PATH + '/.tmp/test.bin');
                await __close(new Uint8Array(arrayBuffer).toString());
            `);
            assert.equal(runner.getOutput(), '64,64');
        });
    });

    describe('filesystem.readBinaryFile', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                let view = new Uint8Array(1);
                view[0] = 64; // Saves ASCII '@' to the binary file
                await Neutralino.filesystem.writeBinaryFile(NL_PATH + '/.tmp/test.bin', view.buffer);

                let buffer = await Neutralino.filesystem.readBinaryFile(NL_PATH + '/.tmp/test.bin');
                view = new Uint8Array(buffer);
                await __close(view[0].toString());
            `);
            assert.equal(runner.getOutput(), '64');
        });

        it('allows changing file cursor', async () => {
            runner.run(`
                let view = new Uint8Array([64, 65, 66, 67]);
                await Neutralino.filesystem.writeBinaryFile(NL_PATH + '/.tmp/test.bin', view.buffer);

                let buffer = await Neutralino.filesystem
                                    .readBinaryFile(NL_PATH + '/.tmp/test.bin', { pos: 2 });
                view = new Uint8Array(buffer);
                await __close(JSON.stringify(Array.from(view)));
            `);
            let data = JSON.parse(runner.getOutput());
            assert.ok(typeof data == 'object');
            assert.equal(data.length, 2);
            assert.equal(data[0], 66);
            assert.equal(data[1], 67);
        });

        it('allows changing file reader buffer size', async () => {
            runner.run(`
                let view = new Uint8Array([164, 165, 166, 167]);
                await Neutralino.filesystem.writeBinaryFile(NL_PATH + '/.tmp/test.bin', view.buffer);

                let buffer = await Neutralino.filesystem
                                    .readBinaryFile(NL_PATH + '/.tmp/test.bin', { size: 1 });
                view = new Uint8Array(buffer);
                await __close(JSON.stringify(Array.from(view)));
            `);
            let data = JSON.parse(runner.getOutput());
            assert.ok(typeof data == 'object');
            assert.equal(data.length, 1);
            assert.equal(data[0], 164);
        });

        it('works properly with both pos and size options', async () => {
            runner.run(`
                let view = new Uint8Array([1, 2, 3, 4]);
                await Neutralino.filesystem.writeBinaryFile(NL_PATH + '/.tmp/test.bin', view.buffer);

                let buffer = await Neutralino.filesystem
                                    .readBinaryFile(NL_PATH + '/.tmp/test.bin', { pos: 1, size: 3 });
                view = new Uint8Array(buffer);
                await __close(JSON.stringify(Array.from(view)));
            `);
            let data = JSON.parse(runner.getOutput());
            assert.ok(typeof data == 'object');
            assert.equal(data.length, 3);
            assert.equal(data[0], 2);
            assert.equal(data[1], 3);
            assert.equal(data[2], 4);
        });
    });

    describe('filesystem.removeFile', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await Neutralino.filesystem.removeFile(NL_PATH + '/.tmp/test.txt');
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
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await Neutralino.filesystem.copyFile(NL_PATH + '/.tmp/test.txt', NL_PATH + '/.tmp/test_new.txt');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('filesystem.moveFile', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test_new.txt', 'Hello');
                await Neutralino.filesystem.copyFile(NL_PATH + '/.tmp/test_new.txt', NL_PATH + '/.tmp/test.txt');
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
            assert.ok(typeof stats.createdAt == 'number');
            assert.ok(typeof stats.modifiedAt == 'number');
        });
    });
});
