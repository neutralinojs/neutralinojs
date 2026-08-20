const { execFileSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const SOURCE_TEMPLATE = `
{BEFORE_INIT_CODE}

Neutralino.init();

Neutralino.events.on("ready", async () => {
    await __init();
    {CODE}
});

async function __close(data = "", exitCode = 0) {
    try {
        if(data) {
            // Use storage.setData which bypasses filesystem scope checks
            // (it calls fs::writeFile directly, not the gated controller).
            await Neutralino.storage.setData('test_output', data);
        }
    }
    catch(err) {
        // ignore - storage may not be initialized; rely on filesystem fallback below
    }
    if(data) {
        // Also try filesystem paths for backward compatibility (when scopes
        // don't restrict those paths).
        await Neutralino.filesystem.writeFile(NL_PATH + "/.tmp/output.txt", data).catch(() => {});
        await Neutralino.filesystem.writeFile(NL_PATH + "/output.txt", data).catch(() => {});
    }
    setTimeout(async () => {
        await Neutralino.app.exit(exitCode); // normal exit
    }, 2000);
}

async function __init() {
    try {
        await Neutralino.filesystem.createDirectory(NL_PATH + "/.tmp");
    }
    catch(err) {
        // ignore
    }
    setTimeout(async () => {
        try {
            await Neutralino.storage.setData('test_output', 'NL_SP_MAXTIMT');
        }
        catch(err) {}
        await Neutralino.filesystem.writeFile(NL_PATH + "/.tmp/output.txt", 'NL_SP_MAXTIMT').catch(() => {});
        await Neutralino.filesystem.writeFile(NL_PATH + "/output.txt", 'NL_SP_MAXTIMT').catch(() => {});
        await Neutralino.app.exit(1); // max timeout force exit
    }, 20000);
}
`;

const TMP_DIR = '../bin/.tmp';
const OUTPUT_FILE = '../bin/.tmp/output.txt';
const FALLBACK_OUTPUT_FILE = '../bin/output.txt';
const SOURCE_FILE = '../bin/resources/js/main_spec.js';

function run(code, options = {}) {
    cleanup();
    if(options.debug) {
        console.log('INFO: Preparing app source...');
    }
    fs.writeFileSync(SOURCE_FILE, makeAppSource(code, options.beforeInitCode));

    if(options.debug) {
        console.log('INFO: Running the app...');
    }
    let exitCode = 0;
    try {
        let { executable, args } = makeCommand(options.args);
        if(options.debug) {
            console.log('INFO: Running command: ' + executable + ' ' + args.join(' '));
        }
        execFileSync(executable, args, {
            stdio: 'ignore',
            timeout: options.timeout || 30000,
            killSignal: 'SIGKILL',
            windowsHide: true
        });
    }
    catch(err) {
        if(err.signal === 'SIGKILL' || err.killed) {
            exitCode = -1; // killed by our timeout
        }
        else {
            exitCode = err.status || 0;
        }
    }

    if(options.debug) {
        console.log('INFO: Test app was closed...');
    }
    return exitCode;
}

function getOutput() {
    let content = ''
    try {
        content = fs.readFileSync(OUTPUT_FILE, 'utf8');
    }
    catch (err) {
        try {
            content = fs.readFileSync(FALLBACK_OUTPUT_FILE, 'utf8');
        }
        catch (err2) {
            // ignore
        }
    }
    if(!content) {
        // Storage writes to <dataLocation>/.storage/test_output.neustorage.
        // On Windows, getDataHome() returns %APPDATA%; on macOS/Linux,
        // $XDG_DATA_HOME or ~/.local/share. The Neutralino server's
        // dataLocation uses the app id from neutralino.config.json.
        try {
            const baseConfig = require('../bin/neutralino.config.json');
            const appId = baseConfig.applicationId || 'neutralinojs';
            const candidates = [
                '../bin/.storage/test_output.neustorage',
                path.join(process.env.APPDATA || '', appId, '.storage', 'test_output.neustorage'),
                path.join(process.env.APPDATA || '', 'neutralinojs', '.storage', 'test_output.neustorage'),
                path.join(process.env.HOME || '', '.local', 'share', appId, '.storage', 'test_output.neustorage'),
                path.join(process.env.HOME || '', '.local', 'share', 'neutralinojs', '.storage', 'test_output.neustorage')
            ];
            for(const p of candidates) {
                try { content = fs.readFileSync(p, 'utf8'); if(content) break; } catch(e) {}
            }
        }
        catch(err) {}
    }
    cleanup();
    return content;
}

function makeCommand(optArgs = '') {
    let executable;
    if(process.platform == 'linux') {
        executable = `..${path.sep}bin${path.sep}neutralino-linux_${process.arch}`;
    }
    else if(process.platform == 'darwin') {
        executable = `..${path.sep}bin${path.sep}neutralino-mac_${process.arch}`;
    }
    else if(process.platform == 'win32') {
        executable = `..${path.sep}bin${path.sep}neutralino-win_x64.exe`;
    }
    else {
        throw new Error('Unsupported platform: ' + process.platform);
    }
    const args = [
        '--load-dir-res',
        '--window-exit-process-on-close',
        '--url=/index_spec.html',
        '--window-enable-inspector=false'
    ];
    if(optArgs) {
        for(const a of optArgs.split(' ')) {
            if(a) args.push(a);
        }
    }
    return { executable, args };
}

function makeAppSource(code, beforeInitCode = '') {
    return SOURCE_TEMPLATE
        .replace('{CODE}', code)
        .replace('{BEFORE_INIT_CODE}', beforeInitCode);
}

function cleanup() {
    try {
        fs.rmSync(TMP_DIR, { recursive: true });
        fs.unlinkSync(SOURCE_FILE);
    }
    catch(err) {
        // ignore
    }
    try {
        fs.unlinkSync(FALLBACK_OUTPUT_FILE);
    }
    catch(err) {
        // ignore
    }
    try {
        const baseConfig = require('../bin/neutralino.config.json');
        const appId = baseConfig.applicationId || 'neutralinojs';
        const storageCandidates = [
            '../bin/.storage/test_output.neustorage',
            path.join(process.env.APPDATA || '', appId, '.storage', 'test_output.neustorage'),
            path.join(process.env.HOME || '', '.local', 'share', appId, '.storage', 'test_output.neustorage')
        ];
        for(const p of storageCandidates) {
            try { fs.unlinkSync(p); } catch(e) {}
        }
    }
    catch(err) {
        // ignore
    }
}

module.exports = {
    run,
    getOutput
}
