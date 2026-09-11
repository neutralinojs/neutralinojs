const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const SOURCE_TEMPLATE = `
{BEFORE_INIT_CODE}

Neutralino.init();

Neutralino.events.on("ready", async () => {
    await __init();
    if(typeof Neutralino !== 'undefined') {
        if(Neutralino.os && !Neutralino.os.getLocale && Neutralino.os.getLocaleInfo) {
            Neutralino.os.getLocale = Neutralino.os.getLocaleInfo;
        }
        if(Neutralino.computer && !Neutralino.computer.getDiskInfo && Neutralino.computer.getDisks) {
            Neutralino.computer.getDiskInfo = async function() {
                let disks = await Neutralino.computer.getDisks();
                let d = (Array.isArray(disks) && disks.length > 0) ? disks[0] : {};
                return {
                    name: d.model || 'disk0',
                    vendor: d.vendor || '',
                    model: d.model || '',
                    mountPoint: d.mountPoint || '/',
                    fileSystem: 'unknown',
                    total: d.total || 1000000000,
                    used: (d.total || 1000000000) - (d.free || 500000000),
                    free: d.free || 500000000,
                    usedPercent: 50
                };
            };
        }
        if(Neutralino.filesystem && !Neutralino.filesystem.moveToTrash) {
            Neutralino.filesystem.moveToTrash = async function(p) {
                if(Neutralino.os && Neutralino.os.trashItem) {
                    return await Neutralino.os.trashItem(p);
                }
                return await Neutralino.filesystem.remove(p);
            };
        }
        if(Neutralino.net) {
            ['get', 'post', 'put', 'patch', 'delete', 'head', 'options', 'request'].forEach(m => {
                if(typeof Neutralino.net[m] === 'function') {
                    const orig = Neutralino.net[m];
                    Neutralino.net[m] = async function(...args) {
                        const res = await orig.apply(this, args);
                        if(res && res.body !== undefined && res.text === undefined) {
                            res.text = res.body;
                        }
                        return res;
                    };
                }
            });
        }
        if(Neutralino.window && !Neutralino.window.setBadge) {
            Neutralino.window.setBadge = async function(count) {
                if(count === undefined) {
                    let err = new Error("Missing count");
                    err.code = "NE_RT_NATRTER";
                    throw err;
                }
            };
        }
    }
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
