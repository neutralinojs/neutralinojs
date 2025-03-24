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

        it('works for nested directories', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/nested/abc');
                let entries = await Neutralino.filesystem.readDirectory(NL_PATH + '/.tmp/');
                let nested_entries = await Neutralino.filesystem.readDirectory(NL_PATH + '/.tmp/nested');
                entries.push(nested_entries[0]);
                await __close(JSON.stringify(entries));
            `);
            const entries = JSON.parse(runner.getOutput());
            assert.ok((entries.find((entry) => entry.type == 'DIRECTORY' && entry.entry == 'nested')) && 
            (entries.find((entry) => entry.type == 'DIRECTORY' && entry.entry == 'abc')));
        });

        it('throws error for a duplicate directory name', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/abc');
                    await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/abc');
                } catch (error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_FS_DIRCRER');
        });

        it('throws an error for invalid path names', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/\0invalidpath');
                } catch (error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_FS_DIRCRER');
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

        it('throws an error if the directory does not exist', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.remove(NL_PATH + '/.tmp/abcd');
                } catch (error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_FS_REMVERR');
        });

        it('removing a file instead of directory works as expected', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await Neutralino.filesystem.remove(NL_PATH + '/.tmp/test.txt');
                let entries = await Neutralino.filesystem.readDirectory(NL_PATH + '/.tmp');
                await __close(JSON.stringify(entries));
            `);
            const entries = JSON.parse(runner.getOutput())
            assert.ok(Array.isArray(entries));
            assert.equal(entries.length, 0);
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

        it('works with special characters', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Special characters: @#$%^&*☁☀☊☄');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test.txt');
                await __close(content);
            `);
            assert.equal(runner.getOutput(), 'Special characters: @#$%^&*☁☀☊☄');
        });

        it('overwrites existing files', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Updated Hello');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test.txt');
                await __close(content)
            `);
            assert.equal(runner.getOutput(), 'Updated Hello');
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

        it('appends to an empty file', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', '');
                await Neutralino.filesystem.appendFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test.txt');
                await __close(content);
            `);
            assert.equal(runner.getOutput(), 'Hello');
        });

        it('appends an empty string to a file', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await Neutralino.filesystem.appendFile(NL_PATH + '/.tmp/test.txt', '');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test.txt');
                await __close(content);
            `);
            assert.equal(runner.getOutput(), 'Hello');
        });

        it('appends special characters to a file', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await Neutralino.filesystem.appendFile(NL_PATH + '/.tmp/test.txt', '@#$%^&*☁☀☊☄');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test.txt');
                await __close(content);
            `);
            assert.equal(runner.getOutput(), 'Hello@#$%^&*☁☀☊☄');
        });

        it('creates and appends to a non-existent file', async () => {
            runner.run(`
                await Neutralino.filesystem.appendFile(NL_PATH + '/.tmp/newfile.txt', 'Hello World');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/newfile.txt');
                await __close(content);
            `);
            assert.equal(runner.getOutput(), 'Hello World');
        });

        it('appends large content to a file', async () => {
            let largeContent = 'N'.repeat(10000); 
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Start');
                await Neutralino.filesystem.appendFile(NL_PATH + '/.tmp/test.txt', '${largeContent}');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test.txt');
                await __close(content);
            `);
            assert.equal(runner.getOutput(), 'Start' + largeContent);
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

        it('reads an empty file', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', '');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test.txt');
                await __close(content);
            `);
            assert.equal(runner.getOutput(), '');
        });

        it('throws an error if the file does not exist', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test.txt');
                } catch (error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_FS_FILRDER');
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

        it('writes a large binary file without throwing errors', async () => {
            runner.run(`
                let size = 5 * 1024 * 1024;
                let rawBin = new ArrayBuffer(size);
                let view = new Uint8Array(rawBin);

                for (let i = 0; i < size; i++) {
                    view[i] = i % 2 === 0 ? 0 : 255;
                }
        
                await Neutralino.filesystem.writeBinaryFile(NL_PATH + '/.tmp/large_test.bin', rawBin);
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

        it('appends to a non-existent file', async () => {
            runner.run(`
                let rawBin = new ArrayBuffer(1);
                let view = new Uint8Array(rawBin);
                view[0] = 64; // Saves ASCII '@' to the binary file
                await Neutralino.filesystem.appendBinaryFile(NL_PATH + '/.tmp/nonexistent.bin', rawBin);
                let arrayBuffer = await Neutralino.filesystem.readBinaryFile(NL_PATH + '/.tmp/nonexistent.bin');
                await __close(new Uint8Array(arrayBuffer).toString());
            `);
            assert.equal(runner.getOutput(), '64');
        });

        it('handles concurrent appends', async () => {
            runner.run(`
                let rawBin1 = new ArrayBuffer(1);
                let view1 = new Uint8Array(rawBin1);
                view1[0] = 65; 
    
                let rawBin2 = new ArrayBuffer(1);
                let view2 = new Uint8Array(rawBin2);
                view2[0] = 66; 
    
                await Promise.all([
                    Neutralino.filesystem.appendBinaryFile(NL_PATH + '/.tmp/concurrent.bin', rawBin1),
                    Neutralino.filesystem.appendBinaryFile(NL_PATH + '/.tmp/concurrent.bin', rawBin2)
                ]);
                let arrayBuffer = await Neutralino.filesystem.readBinaryFile(NL_PATH + '/.tmp/concurrent.bin');
                await __close(new Uint8Array(arrayBuffer).toString());
            `);
            assert(runner.getOutput() == '65,66' || runner.getOutput() == '66,65'); 
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

        it('reads a partially filled file', async () => {
            runner.run(`
                let view = new Uint8Array([1, 2, 3]);
                await Neutralino.filesystem.writeBinaryFile(NL_PATH + '/.tmp/test.bin', view.buffer);
    
                let buffer = await Neutralino.filesystem.readBinaryFile(NL_PATH + '/.tmp/test.bin');
                view = new Uint8Array(buffer);
                await __close(JSON.stringify(Array.from(view)));
            `);
            let data = JSON.parse(runner.getOutput());
            assert.ok(typeof data == 'object');
            assert.equal(data.length, 3);
            assert.equal(data[0], 1);
            assert.equal(data[1], 2);
            assert.equal(data[2], 3);
        });

        it('reads large files without throwing errors', async () => {
            runner.run(`
                let size = 10 * 1024 * 1024;
                let largeBin = new ArrayBuffer(size);
                let view = new Uint8Array(largeBin);
                for(let i = 0; i < view.length; i++) {
                    view[i] = i % 256;
                }
                await Neutralino.filesystem.writeBinaryFile(NL_PATH + '/.tmp/largefile.bin', largeBin);
    
                let buffer = await Neutralino.filesystem.readBinaryFile(NL_PATH + '/.tmp/largefile.bin');
                view = new Uint8Array(buffer);
                await __close(view.length.toString());
            `);
            assert.equal(runner.getOutput(), '10485760'); 
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

        it('throws an error when opening a non-existent file', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.openFile(NL_PATH + '/.tmp/nonexistent.txt');
                } catch (error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_FS_FILOPER');
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

        it('throws an error if an invalid action is provided', async () => {
            runner.run(`
                try {
                    let fileId = await Neutralino.filesystem.openFile(NL_PATH + '/.tmp/test.txt');
                    await Neutralino.filesystem.updateOpenedFile(fileId, 'invalid', 3);
                } catch (error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_FS_FILOPER');
        });

        it('handles large data reading in chunks', async () => {
            runner.run(`
                let largeText = 'a'.repeat(1024 * 1024);
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/large.txt', largeText);
    
                let fileId = await Neutralino.filesystem.openFile(NL_PATH + '/.tmp/large.txt');
                let content = '';
                Neutralino.events.on('openedFile', async (evt) => {
                  if(evt.detail.id == fileId) {
                    switch(evt.detail.action) {
                      case 'data':
                        content += evt.detail.data;
                        break;
                      case 'end':
                        await __close(content.length.toString());
                        break;
                    }
                  }
                });

                await Neutralino.filesystem.updateOpenedFile(fileId, 'readAll', 64 * 1024);
            `);
            assert.equal(runner.getOutput(), '1048576'); 
        });

        it('throws an error if updating an unopened file', async () => {
            runner.run(`    
                try {
                    await Neutralino.filesystem.updateOpenedFile(123, 'read', 3);
                } catch (error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_FS_UNLTOUP');
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
        
        it('returns updated pos after reading bytes', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                let fileId = await Neutralino.filesystem.openFile(NL_PATH + '/.tmp/test.txt');
                await Neutralino.filesystem.updateOpenedFile(fileId, 'read', 2);
                let info = await Neutralino.filesystem.getOpenedFileInfo(fileId);
                await __close(JSON.stringify(info));
            `);
            let info = JSON.parse(runner.getOutput());
            assert.equal(info.pos, 2);
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

        it('returns empty array for an empty directory', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/emptyDir');
                let entries = await Neutralino.filesystem.readDirectory(NL_PATH + '/.tmp/emptyDir');
                await __close(JSON.stringify(entries));
            `);
            let entries = JSON.parse(runner.getOutput());
            assert.ok(Array.isArray(entries));
            assert.equal(entries.length, 0);
        });

        it('returns entries with different file types', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/mixedDir');
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/mixedDir/test.txt', 'Hello');
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/mixedDir/subDir');
                let entries = await Neutralino.filesystem.readDirectory(NL_PATH + '/.tmp/mixedDir');
                await __close(JSON.stringify(entries));
            `);
            let entries = JSON.parse(runner.getOutput());
            assert.ok(Array.isArray(entries));
            assert.ok(entries.find((entry) => entry.type == 'FILE' && entry.entry == 'test.txt'));
            assert.ok(entries.find((entry) => entry.type == 'DIRECTORY' && entry.entry == 'subDir'));
        });

        it('returns entries for nested directories', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/parentDir');
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/parentDir/childDir');
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/parentDir/childDir/test.txt', 'Nested file');
                let entries = await Neutralino.filesystem.readDirectory(NL_PATH + '/.tmp/parentDir');
                let childEntries = await Neutralino.filesystem.readDirectory(NL_PATH + '/.tmp/parentDir/childDir');
                await __close(JSON.stringify({ entries, childEntries }));
            `);
            let { entries, childEntries } = JSON.parse(runner.getOutput());
            assert.ok(Array.isArray(entries));
            assert.ok(entries.find((entry) => entry.type == 'DIRECTORY' && entry.entry == 'childDir'));
            assert.ok(Array.isArray(childEntries));
            assert.ok(childEntries.find((entry) => entry.type == 'FILE' && entry.entry == 'test.txt'));
        });

        it('throws an error for non-existent directory', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.readDirectory(NL_PATH + '/.tmp/nonExistentDir');
                } catch (error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_FS_NOPATHE')
        });    

        it('works with Unicode characters', async () => {
             runner.run(`
                 await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/ɦ');
                 let entries = await Neutralino.filesystem.readDirectory(NL_PATH + '/.tmp');
                 await __close(JSON.stringify(entries));
             `);
             const entries = JSON.parse(runner.getOutput());
             assert.ok(entries.find((entry) => entry.type == 'DIRECTORY' && entry.entry == 'ɦ'));
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
        it('works when the destination file already exists', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test_new.txt', 'Hello');
                await Neutralino.filesystem.copy(NL_PATH + '/.tmp/test.txt', NL_PATH + '/.tmp/test_new.txt');
                await __close('done');
            `);
            assert.equal(runner.getOutput(), 'done');
        });

        it('copies file content correctly', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await Neutralino.filesystem.copy(NL_PATH + '/.tmp/test.txt', NL_PATH + '/.tmp/test_new.txt');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test_new.txt');
                await __close(content);
            `);
            let content = runner.getOutput();
            assert.equal(content, 'Hello');
        });

        it('copies a file into a different directory', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/newDir');
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await Neutralino.filesystem.copy(NL_PATH + '/.tmp/test.txt', NL_PATH + '/.tmp/newDir/test.txt');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/newDir/test.txt');
                await __close(content);
            `);
            let content = runner.getOutput();
            assert.equal(content, 'Hello');
        });

        it('copies a large file correctly', async () => {
            let largeContent = 'N'.repeat(10 * 1024 * 1024); 
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/large_test.txt', '${largeContent}');
                await Neutralino.filesystem.copy(NL_PATH + '/.tmp/large_test.txt', NL_PATH + '/.tmp/large_test_new.txt');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/large_test_new.txt');
                await __close(content);
            `);
            let content = runner.getOutput();
            assert.equal(content, largeContent);
        });
        
        it('overwrites the existing file with overwrite option', async () => {
            runner.run(`
            await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test_new.txt', 'Old Content');
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await Neutralino.filesystem.copy(NL_PATH + '/.tmp/test.txt', NL_PATH + '/.tmp/test_new.txt', { overwrite: true });
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test_new.txt');
                await __close(content);
            `);
            let content = runner.getOutput();
            assert.equal(content, 'Hello');
        });

        it('skips copying if the destination file exists with skip option', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Hello');
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test_new.txt', 'Old Content');
                await Neutralino.filesystem.copy(NL_PATH + '/.tmp/test.txt', NL_PATH + '/.tmp/test_new.txt', { skip: true });
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test_new.txt');
                await __close(content);
            `);
            let content = runner.getOutput();
            assert.equal(content, 'Old Content');
        });
        
        it('copies directories recursively with recursive option', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/sourceDir');
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/sourceDir/test.txt', 'Hello');
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/sourceDir/nestedDir');
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/sourceDir/nestedDir/nested.txt', 'Nested Hello');
                await Neutralino.filesystem.copy(NL_PATH + '/.tmp/sourceDir', NL_PATH + '/.tmp/destDir', { recursive: true });
                let sourceEntries = await Neutralino.filesystem.readDirectory(NL_PATH + '/.tmp/sourceDir');
                let destEntries = await Neutralino.filesystem.readDirectory(NL_PATH + '/.tmp/destDir');
                let nestedContent = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/destDir/nestedDir/nested.txt');
                await __close(JSON.stringify({ sourceEntries, destEntries, nestedContent }));
            `);
            let { sourceEntries, destEntries, nestedContent } = JSON.parse(runner.getOutput());
            assert.ok(destEntries.find((entry) => entry.entry === 'test.txt'));
            assert.ok(destEntries.find((entry) => entry.entry === 'nestedDir'));
            assert.equal(nestedContent, 'Nested Hello');
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

        it('moves a file and retains content', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test_new.txt', 'Hello Content');
                await Neutralino.filesystem.move(NL_PATH + '/.tmp/test_new.txt', NL_PATH + '/.tmp/test.txt');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test.txt');
                await __close(content);
            `);
            assert.equal(runner.getOutput(), 'Hello Content');
        });

        it('throws an error when moving to a non-existent directory', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test_new.txt', 'Hello');
                try {
                    await Neutralino.filesystem.move(NL_PATH + '/.tmp/test_new.txt', NL_PATH + '/.tmp/nonexistent/test.txt');
                } catch (error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_FS_MOVEERR');
        });

        it('overwrites an existing file', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Old Content');
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test_new.txt', 'New Content');
                await Neutralino.filesystem.move(NL_PATH + '/.tmp/test_new.txt', NL_PATH + '/.tmp/test.txt');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test.txt');
                await __close(content);
            `);
            assert.equal(runner.getOutput(), 'New Content');
        });

        it('moves a file to the same location without error', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test_same.txt', 'Hello Same');
                await Neutralino.filesystem.move(NL_PATH + '/.tmp/test_same.txt', NL_PATH + '/.tmp/test_same.txt');
                let content = await Neutralino.filesystem.readFile(NL_PATH + '/.tmp/test_same.txt');
                await __close(content)
            `);
            assert.equal(runner.getOutput(), 'Hello Same');
        });
    });

    describe('filesystem.getStats', () => {
        it('returns file stats', async () => {
            runner.run(`
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

        it('throws an error for non-existent paths', async () => {
            runner.run(`
                try {
                    let stats = await Neutralino.filesystem.getStats(NL_PATH + '/invalid-path');
                }
                catch(error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_FS_NOPATHE');
        });

        it('returns updated modifiedAt after file modification', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test-file.txt', 'Hello World');
                let statsBefore = await Neutralino.filesystem.getStats(NL_PATH + '/.tmp/test-file.txt');
                await new Promise(resolve => setTimeout(resolve, 1000));  // Ensure the timestamp changes
                await Neutralino.filesystem.appendFile(NL_PATH + '/.tmp/test-file.txt', '!');
                let statsAfter = await Neutralino.filesystem.getStats(NL_PATH + '/.tmp/test-file.txt');
                await __close(JSON.stringify({ before: statsBefore, after: statsAfter }));
            `);
            let { before, after } = JSON.parse(runner.getOutput());
            assert.ok(after.modifiedAt > before.modifiedAt);
        });

        it('returns stats for hidden files', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/.hidden-file', 'Hidden Content');
                let stats = await Neutralino.filesystem.getStats(NL_PATH + '/.tmp/.hidden-file');
                await __close(JSON.stringify(stats));
            `);
            let stats = JSON.parse(runner.getOutput());
            assert.ok(typeof stats == 'object');
            assert.ok(stats.isFile);
            assert.ok(!stats.isDirectory);
        });

        it('returns stats for empty files', async () => {
            runner.run(`
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/empty-file.txt', '');
                let stats = await Neutralino.filesystem.getStats(NL_PATH + '/.tmp/empty-file.txt');
                await __close(JSON.stringify(stats));
            `);
            let stats = JSON.parse(runner.getOutput());
            assert.ok(typeof stats == 'object');
            assert.ok(stats.isFile);
            assert.ok(!stats.isDirectory);
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
    
    describe('filesystem.getRelativePath', () => {
        it('returns an error if the path field is missing', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.getRelativePath();
                } catch (error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });

        it('calculates relative path with a custom base path', async () => {
            runner.run(`
                let base = NL_PATH + '/.tmp';
                await Neutralino.filesystem.writeFile(NL_PATH + '/.tmp/test.txt', 'Dummy content');
                let response = await Neutralino.filesystem.getRelativePath(NL_PATH + '/.tmp/test.txt', base);
                await __close(JSON.stringify(response));
            `);
            const output = JSON.parse(runner.getOutput());
            assert.equal(output, 'test.txt');
        });
    
        it('handles paths that are equal to the base path', async () => {
            runner.run(`
                let base = NL_PATH + '/folder';
                await Neutralino.filesystem.writeFile(NL_PATH + '/folder', 'Dummy content');
                let response = await Neutralino.filesystem.getRelativePath(NL_PATH + '/folder', NL_PATH + '/folder');
                await __close(JSON.stringify(response));
            `);
            const output = JSON.parse(runner.getOutput());
            assert.equal(output, '.');
        });
    });

    describe('filesystem.getPathParts', () => {
        it('returns path parts for a valid file path with no extension', async () => {
            runner.run(`
                let path = NL_PATH + '/folder/file';
                let response = await Neutralino.filesystem.getPathParts(path);
                await __close(JSON.stringify(response));
            `);
            const output = JSON.parse(runner.getOutput());
            assert.deepEqual(output, {
                extension: '',
                filename: 'file',
                parentPath: '../bin/folder',
                relativePath: '../bin/folder/file',
                rootDirectory: '',
                rootName: '',
                rootPath: '',
                stem: 'file'
              });
        });
    
        it('returns path parts for a file path with a single dot extension', async () => {
            runner.run(`
                let path = NL_PATH + '/folder/.hiddenfile';
                let response = await Neutralino.filesystem.getPathParts(path);
                await __close(JSON.stringify(response));
            `);
            const output = JSON.parse(runner.getOutput());
            assert.deepEqual(output, {
                extension: '',
                filename: '.hiddenfile',
                parentPath: '../bin/folder',
                relativePath: '../bin/folder/.hiddenfile',
                rootDirectory: '',
                rootName: '',
                rootPath: '',
                stem: '.hiddenfile'
              });
        });
    
        it('returns path parts for a path with special characters', async () => {
            runner.run(`
                let path = NL_PATH + '/folder/special@file#name.txt';
                let response = await Neutralino.filesystem.getPathParts(path);
                await __close(JSON.stringify(response));
            `);
            const output = JSON.parse(runner.getOutput());
            assert.deepEqual(output, {
                extension: '.txt',
                filename: 'special@file#name.txt',
                parentPath: '../bin/folder',
                relativePath: '../bin/folder/special@file#name.txt',
                rootDirectory: '',
                rootName: '',
                rootPath: '',
                stem: 'special@file#name'
              });
        });
    
        it('returns path parts for a root path', async () => {
            runner.run(`
                let path = NL_PATH;
                let response = await Neutralino.filesystem.getPathParts(path);
                await __close(JSON.stringify(response));
            `);
            const output = JSON.parse(runner.getOutput());
            assert.deepEqual(output, {
                extension: '',
                filename: 'bin',
                parentPath: '..',
                relativePath: '../bin',
                rootDirectory: '',
                rootName: '',
                rootPath: '',
                stem: 'bin'
              });
        });
    
        it('returns path parts for a path with a root directory', async () => {
            runner.run(`
                let path = NL_PATH + '/root/directory/file.txt';
                let response = await Neutralino.filesystem.getPathParts(path);
                await __close(JSON.stringify(response));
            `);
            const output = JSON.parse(runner.getOutput());
            assert.deepEqual(output, {
                extension: '.txt',
                filename: 'file.txt',
                parentPath: '../bin/root/directory',
                relativePath: '../bin/root/directory/file.txt',
                rootDirectory: '',
                rootName: '',
                rootPath: '',
                stem: 'file'
              });
        });
    
        it('returns path parts for a path with multiple nested directories', async () => {
            runner.run(`
                let path = NL_PATH + '/nested/dir/structure/file.ext';
                let response = await Neutralino.filesystem.getPathParts(path);
                await __close(JSON.stringify(response));
            `);
            const output = JSON.parse(runner.getOutput());
            assert.deepEqual(output, {
                extension: '.ext',
                filename: 'file.ext',
                parentPath: '../bin/nested/dir/structure',
                relativePath: '../bin/nested/dir/structure/file.ext',
                rootDirectory: '',
                rootName: '',
                rootPath: '',
                stem: 'file'
              });
        });
    
        it('returns path parts for a path with a filename that starts with a dot', async () => {
            runner.run(`
                let path = NL_PATH + '/folder/.config/file.txt';
                let response = await Neutralino.filesystem.getPathParts(path);
                await __close(JSON.stringify(response));
            `);
            const output = JSON.parse(runner.getOutput());
            assert.deepEqual(output, {
                extension: '.txt',
                filename: 'file.txt',
                parentPath: '../bin/folder/.config',
                relativePath: '../bin/folder/.config/file.txt',
                rootDirectory: '',
                rootName: '',
                rootPath: '',
                stem: 'file'
              });
        });
    
        it('returns path parts for a path with absolute path separators', async () => {
            runner.run(`
                let path = '/absolute/path/to/file.txt';
                let response = await Neutralino.filesystem.getPathParts(path);
                await __close(JSON.stringify(response));
            `);
            const output = JSON.parse(runner.getOutput());
            assert.deepEqual(output, {
                extension: '.txt',
                filename: 'file.txt',
                parentPath: '/absolute/path/to',
                relativePath: 'absolute/path/to/file.txt',
                rootDirectory: '/',
                rootName: '',
                rootPath: '/',
                stem: 'file'
              });
        });
    });
    describe('filesystem.getPermissions', () => {
        it('throws an error if the path field is missing', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.getPermissions();
                } catch (error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });

        it('throws an error if the path doesn\'t exist', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.getPermissions(NL_PATH + '/.tmp/test-dir');
                } catch (error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_FS_NOPATHE');
        });
        
        it('returns file permissions', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/test-dir')
                await Neutralino.filesystem.setPermissions(NL_PATH + '/.tmp/test-dir', {
                    ownerRead: true,
                    groupRead: true,
                    othersRead: true,
                });
                const permissions = await Neutralino.filesystem.getPermissions(NL_PATH + '/.tmp/test-dir');
                await __close(JSON.stringify(permissions));
            `);
            const permissions = JSON.parse(runner.getOutput());
            
            assert.ok(typeof permissions == 'object');
            assert.equal(permissions.ownerRead, true);
            assert.equal(permissions.groupRead, true);
            assert.equal(permissions.othersRead, true);
        });

        it('updates file permissions when all parameters are used', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/test-dir')
                const permissions = await Neutralino.filesystem.getPermissions(NL_PATH + '/.tmp/test-dir');
                await __close(JSON.stringify(permissions));
            `);
            const permissions = JSON.parse(runner.getOutput());
            
            assert.ok(typeof permissions == 'object');
            assert.ok(typeof permissions.all == 'boolean');
            assert.ok(typeof permissions.ownerAll == 'boolean');
            assert.ok(typeof permissions.groupAll == 'boolean');
            assert.ok(typeof permissions.othersAll == 'boolean');
            assert.ok(typeof permissions.ownerRead == 'boolean');
            assert.ok(typeof permissions.ownerWrite == 'boolean');
            assert.ok(typeof permissions.ownerExec == 'boolean');
            assert.ok(typeof permissions.groupRead == 'boolean');
            assert.ok(typeof permissions.groupWrite == 'boolean');
            assert.ok(typeof permissions.groupExec == 'boolean');
            assert.ok(typeof permissions.othersRead == 'boolean');
            assert.ok(typeof permissions.othersWrite == 'boolean');
            assert.ok(typeof permissions.othersExec == 'boolean');
        });
    });

    describe('filesystem.setPermissions', () => {
        it('throws an error if the path field is missing', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.setPermissions();
                } catch (error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_RT_NATRTER');
        });

        it('throws an error if the path doesn\'t exist', async () => {
            runner.run(`
                try {
                    await Neutralino.filesystem.setPermissions(NL_PATH + '/.tmp/test-dir', {ownerRead: true});
                } catch (error) {
                    await __close(error.code);
                }
            `);
            assert.equal(runner.getOutput(), 'NE_FS_UNLSTPR');
        });
        
        it('updates file permissions when two parameters are used', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/test-dir')
                await Neutralino.filesystem.setPermissions(NL_PATH + '/.tmp/test-dir', {
                    ownerRead: true,
                    groupRead: true,
                    othersRead: true,
                });
                const permissions = await Neutralino.filesystem.getPermissions(NL_PATH + '/.tmp/test-dir');
                await __close(JSON.stringify(permissions));
            `);
            const permissions = JSON.parse(runner.getOutput());
            
            assert.ok(typeof permissions == 'object');
            assert.equal(permissions.ownerRead, true);
            assert.equal(permissions.groupRead, true);
            assert.equal(permissions.othersRead, true);
        });

        it('updates file permissions when all parameters are used', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/test-dir')
                await Neutralino.filesystem.setPermissions(NL_PATH + '/.tmp/test-dir', {
                    ownerRead: true,
                    groupRead: true,
                    othersRead: true,
                }, 'REPLACE');
                const permissions = await Neutralino.filesystem.getPermissions(NL_PATH + '/.tmp/test-dir');
                await __close(JSON.stringify(permissions));
            `);
            const permissions = JSON.parse(runner.getOutput());
            
            assert.ok(typeof permissions == 'object');
            assert.equal(permissions.ownerRead, true);
            assert.equal(permissions.groupRead, true);
            assert.equal(permissions.othersRead, true);
        });

        it('adds file permissions', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/test-dir')
                await Neutralino.filesystem.setPermissions(NL_PATH + '/.tmp/test-dir', {
                    othersAll: true 
                }, 'ADD');
                const permissions = await Neutralino.filesystem.getPermissions(NL_PATH + '/.tmp/test-dir');
                await __close(JSON.stringify(permissions));
            `);
            const permissions = JSON.parse(runner.getOutput());
            
            assert.ok(typeof permissions == 'object');
            assert.equal(permissions.othersAll, true);
        });

        it('removes file permissions', async () => {
            runner.run(`
                await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp/test-dir')
                await Neutralino.filesystem.setPermissions(NL_PATH + '/.tmp/test-dir', {
                    all: true 
                }, 'REMOVE');
                const permissions = await Neutralino.filesystem.getPermissions(NL_PATH + '/.tmp/test-dir');
                await __close(JSON.stringify(permissions));
            `);
            const permissions = JSON.parse(runner.getOutput());
            
            assert.ok(typeof permissions == 'object');
            assert.equal(permissions.all, false);
        });
    });
});
