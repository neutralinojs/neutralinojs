


Neutralino.init();

Neutralino.events.on("ready", async () => {
    await __init();
    
                let displays = await Neutralino.computer.getDisplays();
                await __close(JSON.stringify(displays));
            
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
