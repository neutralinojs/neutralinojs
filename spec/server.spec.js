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
    });

    describe('static HTTP server security (CWE-22 / GHSA-645w-p5gr-j5xm)', () => {
        const fs = require('fs');
        const path = require('path');
        const { spawn } = require('child_process');
        const { rawGet } = require('./raw_http_helper');

        let serverProcess = null;
        let serverPort = null;
        const testOutsideDir = path.resolve(__dirname, '../bin/test-outside');
        const markerFile = path.join(testOutsideDir, 'secret.txt');
        const markerContent = 'SECRET_MARKER_' + Date.now();

        before(async function() {
            this.timeout(20000);
            fs.mkdirSync(testOutsideDir, { recursive: true });
            fs.writeFileSync(markerFile, markerContent);

            const binPath = path.resolve(__dirname, '../bin/neutralino-' +
                (process.platform === 'darwin' ? 'mac_' + process.arch :
                 process.platform === 'linux' ? 'linux_' + process.arch : 'win_x64.exe'));

            serverProcess = spawn(binPath, ['--load-dir-res', '--window-enable-inspector=false'], {
                cwd: path.resolve(__dirname, '../bin')
            });

            const { execSync } = require('child_process');
            for (let i = 0; i < 30; i++) {
                try {
                    const lsofOut = execSync(`lsof -Pan -p ${serverProcess.pid} -i`).toString();
                    const match = lsofOut.match(/:(\d{4,5})\s+\(LISTEN\)/);
                    if (match) {
                        serverPort = parseInt(match[1], 10);
                        break;
                    }
                } catch (e) {}
                await new Promise(r => setTimeout(r, 200));
            }

            assert.ok(serverPort, 'Failed to obtain Neutralino server port');
        });

        after(() => {
            if (serverProcess) {
                serverProcess.kill();
            }
            try {
                fs.rmSync(testOutsideDir, { recursive: true, force: true });
            } catch (e) {}
        });

        it('serves legitimate files from documentRoot with 200 OK', async () => {
            const res = await rawGet(serverPort, '/icons/appIcon.png');
            assert.equal(res.statusCode, 200);
        });

        it('blocks raw path traversal attempts (/../) outside documentRoot with 403 Forbidden', async () => {
            const res = await rawGet(serverPort, '/../test-outside/secret.txt');
            assert.equal(res.statusCode, 403);
            assert.ok(!res.body.includes(markerContent), 'Secret marker must not be leaked');
        });

        it('blocks URL-encoded path traversal attempts (/%2e%2e/) outside documentRoot with 403 Forbidden', async () => {
            const res = await rawGet(serverPort, '/%2e%2e/test-outside/secret.txt');
            assert.equal(res.statusCode, 403);
            assert.ok(!res.body.includes(markerContent), 'Secret marker must not be leaked');
        });
    });

});

