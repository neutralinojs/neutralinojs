const assert = require('assert');

const runner = require('./runner');

describe('server.spec: server namespace tests', () => {

    describe('server.mount', () => {
        it('mounts and serves a directory successfully', async () => {
            runner.run(`
                const response = {};
                const targetPath = NL_PATH + '/.tmp/test-mount';
                await Neutralino.filesystem.createDirectory(targetPath);
                await Neutralino.filesystem.writeFile(targetPath + '/test.txt', 'Hello');

                const fetch1 = await fetch('/test/test.txt');
                response.fetch1 = fetch1.status;

                await Neutralino.server.mount('/test', targetPath);

                const fetch2 = await fetch('/test/test.txt');
                response.fetch2 = fetch2.status;

                await __close(JSON.stringify(response));
            `);
            const output = JSON.parse(runner.getOutput());
            assert.ok(typeof output === 'object', 'Expected output is an object');
            assert.ok(output.fetch1 === 404, 'Expected a request to a file in a yet not mounted directory to fail');
            assert.ok(output.fetch2 === 200, 'Expected a request to a file in a mounted directory to succeed');
        });
        it('unmounts a directory successfully', async () => {
            runner.run(`
                const response = {};
                const targetPath = NL_PATH + '/.tmp/test-mount';
                await Neutralino.filesystem.createDirectory(targetPath);
                await Neutralino.filesystem.writeFile(targetPath + '/test.txt', 'Hello');

                await Neutralino.server.mount('/test', targetPath);

                const fetch1 = await fetch('/test/test.txt');
                response.fetch1 = fetch1.status;

                await Neutralino.server.unmount('/test');

                const fetch2 = await fetch('/test/test.txt');
                response.fetch2 = fetch2.status;

                await __close(JSON.stringify(response));
            `);
            const output = JSON.parse(runner.getOutput());
            assert.ok(typeof output === 'object', 'Expected output is an object');
            assert.ok(output.fetch1 === 200, 'Expected a file request to a mounted directory before unmounting it to succeed');
            assert.ok(output.fetch2 === 404, 'Expected a file request to an unmounted directory to fail');
        });
        it('mounts and reads from a directory that has non-latin characters', async () => {
            runner.run(`
                const response = {};
                const targetPath = NL_PATH + '/.tmp/test-mount-сосисочка';
                await Neutralino.filesystem.createDirectory(targetPath);
                await Neutralino.filesystem.writeFile(targetPath + '/test.txt', 'Hello');

                await Neutralino.server.mount('/test', targetPath);

                const fetch1 = await fetch('/test/test.txt');
                response.fetch1 = fetch1.status;

                await Neutralino.server.unmount('/test');

                await __close(JSON.stringify(response));
            `);
            const output = JSON.parse(runner.getOutput());
            assert.ok(typeof output === 'object', 'Expected output is an object');
            assert.ok(output.fetch1 === 200, 'The file request to a mounted directory succeeds');
        });

        it('handles similar prefix mount paths correctly', async () => {
            runner.run(`
                const response = {};
                const testPath = NL_PATH + '/.tmp/test';
                const testingPath = NL_PATH + '/.tmp/testing';
                
                await Neutralino.filesystem.createDirectory(testPath);
                await Neutralino.filesystem.createDirectory(testingPath);
                await Neutralino.filesystem.writeFile(testPath + '/file.txt', 'From /test');
                await Neutralino.filesystem.writeFile(testingPath + '/file.txt', 'From /testing');
                
                await Neutralino.server.mount('/test', testPath);
                await Neutralino.server.mount('/testing', testingPath);
                
                const fetch1 = await fetch('/test/file.txt');
                const fetch2 = await fetch('/testing/file.txt');
                
                response.testContent = await fetch1.text();
                response.testingContent = await fetch2.text();
                
                await __close(JSON.stringify(response));
            `);
            const output = JSON.parse(runner.getOutput());
            assert.ok(output.testContent === 'From /test', 'Expected /test to serve from correct directory');
            assert.ok(output.testingContent === 'From /testing', 'Expected /testing to serve from correct directory (not /test)');
        });
    });

});
