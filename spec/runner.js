const { execSync } = require('child_process');
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
    if(data) {
        await Neutralino.filesystem.writeFile(NL_PATH + "/.tmp/output.txt", data);
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
        await Neutralino.filesystem.writeFile(NL_PATH + "/.tmp/output.txt", 'NL_SP_MAXTIMT');
        await Neutralino.app.exit(1); // max timeout force exit
    }, 20000);
}
`;

const TMP_DIR = '../bin/.tmp';
const OUTPUT_FILE = '../bin/.tmp/output.txt';
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
        let command = makeCommand(options.args);
        if(options.debug) {
            console.log('INFO: Running command: ' + command);
        }
        execSync(command);
    }
    catch(err) {
        exitCode = err.status;
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
        // ignore
    }
    cleanup();
    return content;
}

function makeCommand(optArgs = '') {
    let command = `..${path.sep}bin${path.sep}neutralino-`;
    if(process.platform == 'linux') {
        command += 'linux_' + process.arch
    }
    else if(process.platform == 'darwin') {
        command += 'mac_' + process.arch
    }
    else if(process.platform == 'win32') {
        command += 'win_x64.exe'
    }
    command += ' --load-dir-res --window-exit-process-on-close ' +
        '--url=/index_spec.html --window-enable-inspector=false ' + optArgs;
    return command;
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
}

module.exports = {
    run,
    getOutput
}
