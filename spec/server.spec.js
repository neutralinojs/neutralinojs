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
        it('serves index.html with correct MIME type when requesting a mounted directory', async () => {
            runner.run(`
                const response = {};
                const targetPath = NL_PATH + '/.tmp/test-mount-index';
                await Neutralino.filesystem.createDirectory(targetPath);
                await Neutralino.filesystem.writeFile(
                    targetPath + '/index.html',
                    '<!doctype html><html><body>OK</body></html>'
                );

                await Neutralino.server.mount('/test', targetPath);

                const fetch1 = await fetch('/test');
                response.status = fetch1.status;
                response.contentType = fetch1.headers.get('content-type');
                response.body = await fetch1.text();

                await __close(JSON.stringify(response));
            `);

            const output = JSON.parse(runner.getOutput());
            assert.strictEqual(output.status, 200, 'Expected directory request to succeed');
            assert.ok(
                output.contentType && output.contentType.startsWith('text/html'),
                'Expected content-type to be text/html'
            );
            assert.ok(output.body.includes('<html>'), 'Expected HTML body to be returned');
        });
        it('serves index.html with correct MIME type when requesting a mounted directory with trailing slash', async () => {
            runner.run(`
                const response = {};
                const targetPath = NL_PATH + '/.tmp/test-mount-index-slash';
                await Neutralino.filesystem.createDirectory(targetPath);
                await Neutralino.filesystem.writeFile(
                    targetPath + '/index.html',
                    '<html><body>Slash OK</body></html>'
                );

                await Neutralino.server.mount('/test', targetPath);

                const fetch1 = await fetch('/test/');
                response.status = fetch1.status;
                response.contentType = fetch1.headers.get('content-type');

                await __close(JSON.stringify(response));
            `);

            const output = JSON.parse(runner.getOutput());
            assert.strictEqual(output.status, 200);
            assert.ok(
                output.contentType && output.contentType.startsWith('text/html'),
                'Expected text/html for index.html via trailing slash'
            );
        });
        it('serves index.html from resources with correct MIME type when requesting a directory', async () => {
            runner.run(`
                const response = {};

                // index_spec.html lives in resources
                const fetch1 = await fetch('/');
                response.status = fetch1.status;
                response.contentType = fetch1.headers.get('content-type');

                await __close(JSON.stringify(response));
            `);

            const output = JSON.parse(runner.getOutput());
            assert.strictEqual(output.status, 200);
            assert.ok(
                output.contentType && output.contentType.startsWith('text/html'),
                'Expected resource-served index.html to have text/html MIME'
            );
        });
        it('detects text/plain for extensionless text files (non-Windows)', async () => {
            runner.run(`
                const response = {};
                const targetPath = NL_PATH + '/.tmp/test-no-ext-text';
                await Neutralino.filesystem.createDirectory(targetPath);
                await Neutralino.filesystem.writeFile(
                    targetPath + '/README',
                    'this is clearly plain text'
                );

                await Neutralino.server.mount('/test', targetPath);

                const res = await fetch('/test/README');
                response.status = res.status;
                response.contentType = res.headers.get('content-type');

                await __close(JSON.stringify(response));
            `);

            const output = JSON.parse(runner.getOutput());
            assert.strictEqual(output.status, 200);

            if (process.platform === 'win32') {
                assert.ok(output.contentType.startsWith('application/octet-stream'));
            } else {
                assert.ok(output.contentType.startsWith('text/plain'));
            }
        });
        it('detects PNG via libmagic without extension (non-Windows)', async () => {
            runner.run(`
                const response = {};
                const targetPath = NL_PATH + '/.tmp/test-no-ext-png';
                await Neutralino.filesystem.createDirectory(targetPath);

                const pngMagic = new Uint8Array([
                    0x89, 0x50, 0x4E, 0x47,
                    0x0D, 0x0A, 0x1A, 0x0A
                ]);

                await Neutralino.filesystem.writeBinaryFile(
                    targetPath + '/image',
                    pngMagic
                );

                await Neutralino.server.mount('/test', targetPath);

                const res = await fetch('/test/image');
                response.status = res.status;
                response.contentType = res.headers.get('content-type');

                await __close(JSON.stringify(response));
            `);

            const output = JSON.parse(runner.getOutput());
            assert.strictEqual(output.status, 200);

            if (process.platform === 'win32') {
                assert.ok(output.contentType.startsWith('application/octet-stream'));
            } else {
                assert.ok(output.contentType.startsWith('image/png'));
            }
        });
        it('falls back to application/octet-stream for unrecognizable files without extension', async () => {
            runner.run(`
                const response = {};
                const targetPath = NL_PATH + '/.tmp/test-no-ext-binary';
                await Neutralino.filesystem.createDirectory(targetPath);

                const binaryData = new Uint8Array([0]);
                await Neutralino.filesystem.writeBinaryFile(
                    targetPath + '/blob',
                    binaryData
                );

                await Neutralino.server.mount('/test', targetPath);

                const fetch1 = await fetch('/test/blob');
                response.status = fetch1.status;
                response.contentType = fetch1.headers.get('content-type');

                await __close(JSON.stringify(response));
            `);

            const output = JSON.parse(runner.getOutput());
            assert.strictEqual(output.status, 200);
            assert.ok(
                output.contentType.startsWith('application/octet-stream'),
                'Expected fallback MIME type for unrecognized binary file'
            );
        });
        it('treats a directory with an extension-like name as a directory and serves index.html', async () => {
            runner.run(`
                const response = {};
                const targetPath = NL_PATH + '/.tmp/test-dir-with-dot';
                const dirWithDot = targetPath + '/app.v1';

                await Neutralino.filesystem.createDirectory(dirWithDot);
                await Neutralino.filesystem.writeFile(
                    dirWithDot + '/index.html',
                    '<html><body>dir with dot</body></html>'
                );

                await Neutralino.server.mount('/test', targetPath);

                const fetch1 = await fetch('/test/app.v1');
                response.status = fetch1.status;
                response.contentType = fetch1.headers.get('content-type');
                response.body = await fetch1.text();

                await __close(JSON.stringify(response));
            `);

            const output = JSON.parse(runner.getOutput());
            assert.strictEqual(output.status, 200, 'Expected directory with dot in name to resolve');
            assert.ok(
                output.contentType && output.contentType.startsWith('text/html'),
                'Expected text/html MIME for directory index.html'
            );
            assert.ok(output.body.includes('dir with dot'));
        });
    });

});
