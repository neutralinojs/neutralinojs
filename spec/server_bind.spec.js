const assert = require('assert');
const { spawn, execSync } = require('child_process');
const path = require('path');

describe('server binding address security (CWE-1327 / GHSA-fp96-mrqq-p5rq)', () => {
    const binPath = path.resolve(__dirname, '../bin/neutralino-' +
        (process.platform === 'darwin' ? 'mac_' + process.arch :
         process.platform === 'linux' ? 'linux_' + process.arch : 'win_x64.exe'));

    async function getListenAddress(child) {
        if (process.platform === 'win32') {
            const out = execSync(`netstat -ano | findstr ${child.pid}`).toString();
            // Windows netstat output line: TCP 127.0.0.1:port 0.0.0.0:0 LISTENING pid
            const match = out.match(/TCP\s+([^\s:]+):(\d+)\s+.*LISTENING/);
            return match ? { ip: match[1], port: parseInt(match[2], 10) } : null;
        } else {
            // Linux and macOS
            const lsofOut = execSync(`lsof -Pan -p ${child.pid} -i -sTCP:LISTEN`).toString();
            // macOS / Linux line: neutralin pid user fd IPv4 ... TCP 127.0.0.1:port (LISTEN) or *:port (LISTEN)
            const match = lsofOut.match(/TCP\s+([^\s:]+):(\d+)\s+\(LISTEN\)/);
            return match ? { ip: match[1], port: parseInt(match[2], 10) } : null;
        }
    }

    async function waitForListen(child) {
        for (let i = 0; i < 30; i++) {
            await new Promise(r => setTimeout(r, 100));
            try {
                const info = await getListenAddress(child);
                if (info) return info;
            } catch (e) {}
        }
        throw new Error('Server process failed to bind port');
    }

    it('binds to 0.0.0.0 by default in cloud mode and logs a security warning', async function() {
        this.timeout(10000);
        let capturedLogs = '';
        const child = spawn(binPath, ['--mode=cloud', '--window-enable-inspector=false'], {
            cwd: path.resolve(__dirname, '../bin')
        });

        child.stdout.on('data', d => { capturedLogs += d.toString(); });
        child.stderr.on('data', d => { capturedLogs += d.toString(); });

        try {
            const listenInfo = await waitForListen(child);
            // On macOS/Linux lsof, 0.0.0.0 is displayed as '*' or '0.0.0.0'
            assert.ok(listenInfo.ip === '*' || listenInfo.ip === '0.0.0.0', 'Cloud mode must bind 0.0.0.0 by default');

            await new Promise(r => setTimeout(r, 200));
            assert.ok(capturedLogs.includes('SECURITY WARNING'), 'Cloud mode must log security warning when binding 0.0.0.0');
        } finally {
            child.kill();
        }
    });

    it('allows restricting cloud mode to loopback 127.0.0.1 via --host=127.0.0.1', async function() {
        this.timeout(10000);
        let capturedLogs = '';
        const child = spawn(binPath, ['--mode=cloud', '--host=127.0.0.1', '--window-enable-inspector=false'], {
            cwd: path.resolve(__dirname, '../bin')
        });

        child.stdout.on('data', d => { capturedLogs += d.toString(); });
        child.stderr.on('data', d => { capturedLogs += d.toString(); });

        try {
            const listenInfo = await waitForListen(child);
            assert.strictEqual(listenInfo.ip, '127.0.0.1', 'Cloud mode must restrict to 127.0.0.1 when configured');

            await new Promise(r => setTimeout(r, 200));
            assert.ok(!capturedLogs.includes('SECURITY WARNING'), 'No security warning should be logged for 127.0.0.1');
        } finally {
            child.kill();
        }
    });

    it('binds to loopback 127.0.0.1 by default in window mode', async function() {
        this.timeout(10000);
        const child = spawn(binPath, ['--load-dir-res', '--window-enable-inspector=false'], {
            cwd: path.resolve(__dirname, '../bin')
        });

        try {
            const listenInfo = await waitForListen(child);
            assert.strictEqual(listenInfo.ip, '127.0.0.1', 'Window mode must bind to 127.0.0.1 by default');
        } finally {
            child.kill();
        }
    });

    it('allows binding to 0.0.0.0 in window mode via --host=0.0.0.0 and logs warning', async function() {
        this.timeout(10000);
        let capturedLogs = '';
        const child = spawn(binPath, ['--load-dir-res', '--host=0.0.0.0', '--window-enable-inspector=false'], {
            cwd: path.resolve(__dirname, '../bin')
        });

        child.stdout.on('data', d => { capturedLogs += d.toString(); });
        child.stderr.on('data', d => { capturedLogs += d.toString(); });

        try {
            const listenInfo = await waitForListen(child);
            assert.ok(listenInfo.ip === '*' || listenInfo.ip === '0.0.0.0', 'Must bind to 0.0.0.0');

            await new Promise(r => setTimeout(r, 200));
            assert.ok(capturedLogs.includes('SECURITY WARNING'), 'Must log security warning when window mode binds 0.0.0.0');
        } finally {
            child.kill();
        }
    });
});
