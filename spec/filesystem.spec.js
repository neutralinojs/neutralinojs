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

    describe('filesystem.remove', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/abcd');
                await Neutralino.filesystem.remove(NL_PATH + '/.tmp/abcd');
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

    describe('filesystem.openFile', () => {
        it('returns file identifiers without throwing errors', async () => {
            runner.run(`
                let fileIds = [];
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test2.txt', 'Hello');

                let fileId = await Neutralino.filesystem.openFile(NL_PATH + '/.tmp/test.txt');
                fileIds.push(fileId);
                fileId = await Neutralino.filesystem.openFile(NL_PATH + '/.tmp/test2.txt');
                fileIds.push(fileId);

                await __close(JSON.stringify(fileIds));
            `);
            let fileIds = JSON.parse(runner.getOutput());
            assert.ok(Array.isArray(fileIds));
            assert.equal(fileIds[0], 0);
            assert.equal(fileIds[1], 1);
        });

        it('throws an error for missing args', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.openFile();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });
    });

    describe('filesystem.updateOpenedFile', () => {
        it('triggers the data event', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Neutralinojs');

                let fileId = await Neutralino.filesystem.openFile(NL_PATH + '/.tmp/test.txt');
                Neutralino.events.on('openedFile', async (evt) => {
                  if(evt.detail.id == fileId && evt.detail.action == 'data') {
                      await __close(evt.detail.data);
                  }
                });
                await Neutralino.filesystem.updateOpenedFile(fileId, 'read', 3); // Reads the first 3 bytes
            `);
            assert.equal(runner.getOutput(), 'Neu');
        });

        it('triggers the end event', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Neutralinojs');

                let fileId = await Neutralino.filesystem.openFile(NL_PATH + '/.tmp/test.txt');
                Neutralino.events.on('openedFile', async (evt) => {
                  if(evt.detail.id == fileId && evt.detail.action == 'end') {
                      await __close('done');
                  }
                });
                // Reads the first 3 bytes
                await Neutralino.filesystem.updateOpenedFile(fileId, 'read', 3);
                // Reads the next 10 bytes (reaches EOF)
                await Neutralino.filesystem.updateOpenedFile(fileId, 'read', 10);
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('reads the entire file with readAll', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Neutralinojs');

                let fileId = await Neutralino.filesystem.openFile(NL_PATH + '/.tmp/test.txt');
                let content = '';
                Neutralino.events.on('openedFile', async (evt) => {
                  if(evt.detail.id == fileId) {
                    switch(evt.detail.action) {
                      case 'data':
                        content += evt.detail.data;
                        break;
                      case 'end':
                        await __close(content);
                        break;
                    }
                  }
                });
                // Reads all bytes with a 2-bytes-sized buffer
                await Neutralino.filesystem.updateOpenedFile(fileId, 'readAll', 2);
            `);
            assert.equal(runner.getOutput(), 'Neutralinojs');
        });

        it('changes the file cursor', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Neutralinojs');

                let fileId = await Neutralino.filesystem.openFile(NL_PATH + '/.tmp/test.txt');
                let content = '';
                Neutralino.events.on('openedFile', async (evt) => {
                  if(evt.detail.id == fileId) {
                    switch(evt.detail.action) {
                      case 'data':
                        content += evt.detail.data;
                        break;
                      case 'end':
                        await __close(content);
                        break;
                    }
                  }
                });
                // Sets the file cursor to 10th byte
                await Neutralino.filesystem.updateOpenedFile(fileId, 'seek', 10);
                // Reads 2 bytes from the cursor position
                await Neutralino.filesystem.updateOpenedFile(fileId, 'read', 2);
                // Reads the next 1 byte (reaches EOF)
                await Neutralino.filesystem.updateOpenedFile(fileId, 'read', 1);
            `);
            assert.equal(runner.getOutput(), 'js');
        });
    });


    describe('filesystem.getOpenedFileInfo', () => {
        it('returns opened file information', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                let fileId = await Neutralino.filesystem.openFile(NL_PATH + '/.tmp/test.txt');
                let info = await Neutralino.filesystem.getOpenedFileInfo(fileId);
                await __close(JSON.stringify(info));
            `);
            let info = JSON.parse(runner.getOutput());
            assert.ok(typeof info == 'object');
            assert.ok(typeof info.id == 'number');
            assert.ok(typeof info.eof == 'boolean');
            assert.ok(typeof info.pos == 'number');
            assert.ok(typeof info.lastRead == 'number');
        });


        it('returns the file identifier properly', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                let fileId = await Neutralino.filesystem.openFile(NL_PATH + '/.tmp/test.txt');
                let info = await Neutralino.filesystem.getOpenedFileInfo(fileId);
                await __close(JSON.stringify(info));
            `);
            let info = JSON.parse(runner.getOutput());
            assert.equal(info.id, 0);
        });

        it('returns updated eof properly', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                let fileId = await Neutralino.filesystem.openFile(NL_PATH + '/.tmp/test.txt');
                await Neutralino.filesystem.updateOpenedFile(fileId, 'readAll');
                let info = await Neutralino.filesystem.getOpenedFileInfo(fileId);
                await __close(JSON.stringify(info));
            `);
            let info = JSON.parse(runner.getOutput());
            assert.equal(info.eof, true);
        });

        it('returns returns updated pos properly', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                let fileId = await Neutralino.filesystem.openFile(NL_PATH + '/.tmp/test.txt');
                await Neutralino.filesystem.updateOpenedFile(fileId, 'seek', 3);
                let info = await Neutralino.filesystem.getOpenedFileInfo(fileId);
                await __close(JSON.stringify(info));
            `);
            let info = JSON.parse(runner.getOutput());
            assert.equal(info.pos, 3);
        });

        it('returns returns updated lastRead properly', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                let fileId = await Neutralino.filesystem.openFile(NL_PATH + '/.tmp/test.txt');
                await Neutralino.filesystem.updateOpenedFile(fileId, 'read', 2);
                let info = await Neutralino.filesystem.getOpenedFileInfo(fileId);
                await __close(JSON.stringify(info));
            `);
            let info = JSON.parse(runner.getOutput());
            assert.equal(info.lastRead, 2);
        });
    });

    describe('filesystem.readDirectory', () => {
        it('returns directory entries', async () => {
            runner.run(`
                let entries = await Neutralino.filesystem.readDirectory(NL_PATH);
                await __close(JSON.stringify(entries));
            `);
            let entries = JSON.parse(runner.getOutput());
            assert.ok(Array.isArray(entries));
            assert.ok(entries.length > 0);
            assert.ok(entries.find((entry) => entry.type == 'DIRECTORY' && entry.entry == 'resources'))
        });
    });

    describe('filesystem.copy', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await Neutralino.filesystem.copy(NL_PATH + '/.tmp/test.txt', NL_PATH + '/.tmp/test_new.txt');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });
    });

    describe('filesystem.move', () => {
        it('works without throwing errors', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test_new.txt', 'Hello');
                await Neutralino.filesystem.move(NL_PATH + '/.tmp/test_new.txt', NL_PATH + '/.tmp/test.txt');
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

    describe('filesystem.createWatcher', () => {
        it('returns watcher identifiers without throwing errors', async () => {
            runner.run(`
                let watcherIds = [];

                let watcherId = await Neutralino.filesystem.createWatcher(NL_PATH);
                watcherIds.push(watcherId);
                watcherId = await Neutralino.filesystem.createWatcher(NL_PATH + '/.tmp');
                watcherIds.push(watcherId);

                await __close(JSON.stringify(watcherIds));
            `);
            let watcherIds = JSON.parse(runner.getOutput());
            assert.ok(Array.isArray(watcherIds));
            assert.ok(watcherIds[0] > 0);
            assert.ok(watcherIds[1] > 0);
        });

        it('dispatches events for internal watcher events', async () => {
            runner.run(`
                let watcherId = await Neutralino.filesystem.createWatcher(NL_PATH + '/.tmp');
                await Neutralino.events.on('watchFile', async (evt) => {
                    if(evt.detail.id == watcherId) {
                        await __close(JSON.stringify(evt.detail));
                    }
                });
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/new-dir');
            `);
            let data = JSON.parse(runner.getOutput());
            assert.ok(typeof data == 'object');
            assert.ok(typeof data.id == 'number');
            assert.ok(typeof data.action == 'string');
            assert.ok(typeof data.dir == 'string');
            assert.ok(typeof data.filename == 'string');
        });

        it('throws an error for missing args', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.createWatcher();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });

        it('throws an error for non-existent paths', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.createWatcher(NL_PATH + '/invalid-path');
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_FS_UNLCWAT');
        });
    });

    describe('filesystem.removeWatcher', () => {
        it('removes a watcher without throwing errors', async () => {
            runner.run(`
                let watcherId = await Neutralino.filesystem.createWatcher(NL_PATH);
                await Neutralino.filesystem.removeWatcher(watcherId);
                await __close(watcherId.toString());
            `);
            let watcherId = parseInt(runner.getOutput());
            assert.ok(watcherId > 0);
        });

        it('throws an error for missing args', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.removeWatcher();
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });

        it('throws an error for non-existent watcher identifiers', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.removeWatcher(1000);
                }
                catch(err) {
                    await __close(err.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_FS_NOWATID');
        });
    });



    describe('filesystem.getWatchers', () => {
        it('returns current watchers', async () => {
            runner.run(`
                let watcherId = await Neutralino.filesystem.createWatcher(NL_PATH);
                let watchers = await Neutralino.filesystem.getWatchers();
                await Neutralino.filesystem.removeWatcher(watcherId);
                await __close(JSON.stringify(watchers));
            `);
            let watchers = JSON.parse(runner.getOutput());
            assert.ok(typeof watchers == 'object');
            assert.equal(watchers.length, 1);
            assert.ok(typeof watchers[0] == 'object');
            assert.ok(typeof watchers[0].id == 'number');
            assert.ok(typeof watchers[0].path == 'string');
        });
    });
});
