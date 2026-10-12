const assert = require('assert');

const runner = require('./runner');

describe('config.spec: App configuration tests', () => {

    it('extends the default user agent', async () => {
        runner.run(`
            await __close(navigator.userAgent);
        `, {args: '--window-extend-user-agent-with="TestUserAgentValue"'});

        assert.ok(runner.getOutput().includes('TestUserAgentValue'));
    });

    describe('exportAuthInfo permissions (CWE-732 / GHSA-jrpq-x4rc-3w47)', () => {
        const fs = require('fs');
        const path = require('path');
        const { spawn } = require('child_process');

        const tmpDir = path.resolve(__dirname, '../bin/.tmp');
        const authFile = path.join(tmpDir, 'auth_info.json');
        const binPath = path.resolve(__dirname, '../bin/neutralino-' +
            (process.platform === 'darwin' ? 'mac_' + process.arch :
             process.platform === 'linux' ? 'linux_' + process.arch : 'win_x64.exe'));

        afterEach(() => {
            try { fs.rmSync(authFile, { force: true }); } catch (e) {}
        });

        it('does not export auth_info.json by default', async function() {
            this.timeout(10000);
            try { fs.rmSync(authFile, { force: true }); } catch (e) {}

            const child = spawn(binPath, ['--load-dir-res', '--window-enable-inspector=false'], {
                cwd: path.resolve(__dirname, '../bin')
            });

            await new Promise(r => setTimeout(r, 1000));
            child.kill();

            assert.strictEqual(fs.existsSync(authFile), false, 'auth_info.json should not exist by default');
        });

        it('exports auth_info.json with owner-only permissions (0600 file, 0700 dir) when enabled', async function() {
            this.timeout(10000);
            try { fs.rmSync(tmpDir, { recursive: true, force: true }); } catch (e) {}

            const child = spawn(binPath, ['--load-dir-res', '--window-enable-inspector=false', '--export-auth-info'], {
                cwd: path.resolve(__dirname, '../bin')
            });

            for (let i = 0; i < 30; i++) {
                await new Promise(r => setTimeout(r, 100));
                if (fs.existsSync(authFile)) break;
            }
            child.kill();

            assert.ok(fs.existsSync(authFile), 'auth_info.json should be exported');

            const content = JSON.parse(fs.readFileSync(authFile, 'utf8'));
            assert.ok(content.nlPort, 'nlPort should be present');
            assert.ok(content.nlToken, 'nlToken should be present');
            assert.ok(content.nlConnectToken, 'nlConnectToken should be present');

            if (process.platform !== 'win32') {
                const fileStat = fs.statSync(authFile);
                const fileMode = (fileStat.mode & 0o777).toString(8);
                assert.strictEqual(fileMode, '600', `Expected auth_info.json mode 0600, got ${fileMode}`);

                const dirStat = fs.statSync(tmpDir);
                const dirMode = (dirStat.mode & 0o777).toString(8);
                assert.strictEqual(dirMode, '700', `Expected .tmp dir mode 0700, got ${dirMode}`);
            }
        });

        it('tightens pre-existing loose permissions on .tmp and auth_info.json', async function() {
            this.timeout(10000);
            fs.mkdirSync(tmpDir, { recursive: true });
            fs.writeFileSync(authFile, '{}');

            if (process.platform !== 'win32') {
                fs.chmodSync(tmpDir, 0o755);
                fs.chmodSync(authFile, 0o644);
            }

            const child = spawn(binPath, ['--load-dir-res', '--window-enable-inspector=false', '--export-auth-info'], {
                cwd: path.resolve(__dirname, '../bin')
            });

            for (let i = 0; i < 30; i++) {
                await new Promise(r => setTimeout(r, 100));
                try {
                    const data = JSON.parse(fs.readFileSync(authFile, 'utf8'));
                    if (data.nlToken) break;
                } catch (e) {}
            }
            child.kill();

            if (process.platform !== 'win32') {
                const fileStat = fs.statSync(authFile);
                const fileMode = (fileStat.mode & 0o777).toString(8);
                assert.strictEqual(fileMode, '600', `Expected auth_info.json mode tightened to 0600, got ${fileMode}`);

                const dirStat = fs.statSync(tmpDir);
                const dirMode = (dirStat.mode & 0o777).toString(8);
                assert.strictEqual(dirMode, '700', `Expected .tmp dir mode tightened to 0700, got ${dirMode}`);
            }
        });
    });

});
