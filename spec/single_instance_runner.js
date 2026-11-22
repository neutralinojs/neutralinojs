const { spawn } = require('child_process');
const fs = require('fs');
const path = require('path');

const TMP_DIR = '../bin/.tmp';
const OUTPUT_FILE = '../bin/.tmp/output.txt';
const READY_FILE = '../bin/.tmp/ready.txt';
const SOURCE_FILE = '../bin/resources/js/main_spec.js';
const BASE_CONFIG_FILE = '../bin/neutralino.config.json';
const CONFIG_FILE = '../bin/single_instance.config.json';

function getSource(expectedEvents) {
    return `
const receivedEvents = [];

Neutralino.events.on('secondInstance', async (evt) => {
    receivedEvents.push(evt.detail);
    if(receivedEvents.length !== ${expectedEvents}) return;
    try {
        await Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp');
    }
    catch(err) {
        // ignore
    }
    await Neutralino.filesystem.writeFile(
        NL_PATH + '/.tmp/output.txt', JSON.stringify(receivedEvents));
});

Neutralino.events.on('ready', () => {
    Neutralino.filesystem.createDirectory(NL_PATH + '/.tmp')
        .catch(() => {})
        .then(() => Neutralino.filesystem.writeFile(
            NL_PATH + '/.tmp/ready.txt', 'ready'))
        .catch(() => {});
    setTimeout(() => Neutralino.app.exit(1), 20000);
});

Neutralino.init();
`;
}

function getBinaryPath() {
    let binary = `..${path.sep}bin${path.sep}neutralino-`;
    if(process.platform == 'linux') binary += 'linux_' + process.arch;
    else if(process.platform == 'darwin') binary += 'mac_' + process.arch;
    else if(process.platform == 'win32') binary += 'win_x64.exe';
    return path.resolve(binary);
}

function getBinaryArgs() {
    return [
        '--load-dir-res',
        '--config-file=/single_instance.config.json',
        '--window-exit-process-on-close',
        '--url=/index_spec.html',
        '--window-enable-inspector=false'
    ];
}

function waitForExit(child) {
    return new Promise((resolve, reject) => {
        if(child.exitCode !== null || child.signalCode !== null) {
            resolve(child.exitCode);
            return;
        }
        child.once('error', reject);
        child.once('exit', resolve);
    });
}

async function waitForFile(file, timeoutMs = 10000) {
    const deadline = Date.now() + timeoutMs;
    while(Date.now() < deadline) {
        if(fs.existsSync(file)) return;
        await new Promise(resolve => setTimeout(resolve, 50));
    }
    throw new Error(`Timed out waiting for ${file}`);
}

async function waitForExitedChildren(children, expectedCount, timeoutMs = 10000) {
    const deadline = Date.now() + timeoutMs;
    while(Date.now() < deadline) {
        const exitedCount = children.filter(child =>
            child.exitCode !== null || child.signalCode !== null).length;
        if(exitedCount >= expectedCount) return;
        await new Promise(resolve => setTimeout(resolve, 50));
    }
    throw new Error(`Timed out waiting for ${expectedCount} child processes to exit`);
}

function cleanup() {
    try {
        fs.rmSync(TMP_DIR, { recursive: true, force: true });
    }
    catch(err) {
        // ignore
    }
    try {
        fs.rmSync(SOURCE_FILE, { force: true });
    }
    catch(err) {
        // ignore
    }
    try {
        fs.rmSync(CONFIG_FILE, { force: true });
    }
    catch(err) {
        // ignore
    }
}

function prepare(source) {
    cleanup();
    const config = JSON.parse(fs.readFileSync(BASE_CONFIG_FILE, 'utf8'));
    config.singleInstance = true;
    fs.writeFileSync(CONFIG_FILE, JSON.stringify(config));
    if(source) fs.writeFileSync(SOURCE_FILE, source);
}

async function runSecondInstance(forwardedArgs = [], options = {}) {
    prepare(getSource(1));

    const args = getBinaryArgs();
    const primary = spawn(getBinaryPath(), args, { stdio: 'ignore' });
    const primaryExit = waitForExit(primary);
    let secondary;
    const timeout = setTimeout(() => {
        if(secondary && secondary.exitCode === null) secondary.kill();
        if(primary.exitCode === null) primary.kill();
    }, 30000);

    try {
        await waitForFile(READY_FILE);
        const pathArgs = options.cwd ? [`--path=${path.resolve('../bin')}`] : [];
        secondary = spawn(getBinaryPath(), [...args, ...pathArgs, ...forwardedArgs], {
            stdio: 'ignore',
            cwd: options.cwd
        });
        const secondaryExitCode = await waitForExit(secondary);
        await waitForFile(OUTPUT_FILE);
        let output = '';
        try {
            output = fs.readFileSync(OUTPUT_FILE, 'utf8');
        }
        catch(err) {
            // ignore
        }
        return {
            secondaryExitCode,
            output,
            secondaryCwd: options.cwd || process.cwd()
        };
    }
    finally {
        clearTimeout(timeout);
        if(secondary && secondary.exitCode === null) secondary.kill();
        if(primary.exitCode === null) primary.kill();
        await primaryExit;
        cleanup();
    }
}

async function runSingleInstanceRace(count = 10) {
    prepare(getSource(count - 1));

    const args = getBinaryArgs();
    const processes = [];
    const markers = Array.from({ length: count },
        (_, index) => `race-token-${index}`);
    const timeout = setTimeout(() => {
        for(const child of processes) {
            if(child.exitCode === null) child.kill();
        }
    }, 30000);

    try {
        for(const marker of markers) {
            processes.push(spawn(getBinaryPath(), [...args, marker],
                { stdio: 'ignore' }));
        }
        await waitForFile(OUTPUT_FILE, 30000);
        await waitForExitedChildren(processes, count - 1);
        for(const child of processes) {
            if(child.exitCode === null) child.kill();
        }
        const exitCodes = await Promise.all(processes.map(waitForExit));
        let output = '';
        try {
            output = fs.readFileSync(OUTPUT_FILE, 'utf8');
        }
        catch(err) {
            // ignore
        }
        return { exitCodes, markers, output };
    }
    finally {
        clearTimeout(timeout);
        for(const child of processes) {
            if(child.exitCode === null) child.kill();
        }
        await Promise.all(processes.map(waitForExit));
        cleanup();
    }
}

async function runPrimaryTakeover() {
    prepare();
    const args = [
        ...getBinaryArgs(),
        '--mode=cloud',
        '--enable-server=false'
    ];
    const primary = spawn(getBinaryPath(), args, { stdio: 'ignore' });
    await new Promise(resolve => setTimeout(resolve, 750));
    primary.kill();
    await waitForExit(primary);

    const replacement = spawn(getBinaryPath(), args, { stdio: 'ignore' });
    try {
        await new Promise(resolve => setTimeout(resolve, 1000));
        return replacement.exitCode === null;
    }
    finally {
        if(replacement.exitCode === null) replacement.kill();
        await waitForExit(replacement);
        cleanup();
    }
}

async function runQueueOverflow() {
    prepare();
    const args = [
        ...getBinaryArgs(),
        '--mode=cloud',
        '--enable-server=false'
    ];
    const primary = spawn(getBinaryPath(), args, { stdio: 'ignore' });
    const acceptedExitCodes = [];
    let rejected;
    const timeout = setTimeout(() => {
        if(rejected && rejected.exitCode === null) rejected.kill();
        if(primary.exitCode === null) primary.kill();
    }, 30000);

    try {
        await new Promise(resolve => setTimeout(resolve, 750));
        for(let index = 0; index < 128; index++) {
            const child = spawn(getBinaryPath(), [...args, `queue-token-${index}`],
                { stdio: 'ignore' });
            acceptedExitCodes.push(await waitForExit(child));
        }
        rejected = spawn(getBinaryPath(), [...args, 'queue-overflow'],
            { stdio: 'ignore' });
        const rejectedExitCode = await waitForExit(rejected);
        return {
            acceptedExitCodes,
            rejectedExitCode,
            primaryStillRunning: primary.exitCode === null
        };
    }
    finally {
        clearTimeout(timeout);
        if(rejected && rejected.exitCode === null) rejected.kill();
        if(primary.exitCode === null) primary.kill();
        await waitForExit(primary);
        cleanup();
    }
}

module.exports = {
    runSecondInstance,
    runSingleInstanceRace,
    runPrimaryTakeover,
    runQueueOverflow
};
