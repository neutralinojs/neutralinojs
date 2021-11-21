const { execSync } = require('child_process');
const fs = require('fs');

const SOURCE_TEMPLATE = `
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
    }, 10000);
}
`;
const TMP_DIR = '../bin/.tmp';
const OUTPUT_FILE = '../bin/.tmp/output.txt';
const SOURCE_FILE = '../bin/resources/js/main_spec.js';

function run(code, options = {args: ''}) {
    cleanup();
    if(options.debug) {
        console.log('INFO: Preparing app source...');
    }
    fs.writeFileSync(SOURCE_FILE, makeAppSource(code));

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
    catch (err) {
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
    catch(err) {
        // ignore
    }
    cleanup();
    return content;
}

function makeCommand(optArgs = '') {
    let command = '../bin/neutralino-';
    if(process.platform == 'linux') {
        command += 'linux_x64'
    }
    else if(process.platform == 'darwin') {
        command += 'mac_x64'
    }
    else if(process.platform == 'win32') {
        command += 'win_x64.exe'
    }
    command += ' --load-dir-res --window-exit-process-on-close ' +
            '--url=/resources/index_spec.html --window-enable-inspector=false ' + optArgs;
    return command;
}

function makeAppSource(code) {
    return SOURCE_TEMPLATE.replace('{CODE}', code);
}

function cleanup() {
    try {
        fs.rmSync(TMP_DIR, {recursive: true});
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
