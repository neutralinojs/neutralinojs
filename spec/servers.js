const { spawn } = require('child_process');
const path = require('path');

const START_TIMEOUT_MS = 5000;

/** Starts http-server.js and resolves with `{ port, stop }`. */
function startFixtureServer() {
    return new Promise((resolve, reject) => {
        const child = spawn(process.execPath, [path.join(__dirname, 'http-server.js')]);
        let started = false;
        let timer;

        const fail = (error) => {
            if(started) {
                return;
            }
            started = true;
            clearTimeout(timer);
            child.kill();
            reject(error);
        };

        timer = setTimeout(
            () => fail(new Error('Fixture server did not report a port in time')),
            START_TIMEOUT_MS);

        const onData = (chunk) => {
            const match = chunk.toString().match(/PORT:(\d+)/);
            if(!match || started) {
                return;
            }
            started = true;
            clearTimeout(timer);
            child.stdout.off('data', onData);
            resolve({
                port: Number(match[1]),
                stop: () => child.kill()
            });
        };

        child.stdout.on('data', onData);
        child.once('error', fail);
        child.once('exit', code => fail(new Error(`Fixture server exited unexpectedly (code ${code})`)));
    });
}

module.exports = { startFixtureServer };

