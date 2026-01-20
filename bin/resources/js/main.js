function showInfo() {
    document.getElementById('info').innerHTML = `${NL_APPID} is running on port ${NL_PORT}
        inside ${NL_OS}<br/><br/><span>v${NL_VERSION}</span>`;
}

function openDocs() {
    Neutralino.os.open("https://neutralino.js.org/docs");
}

function openInBrowser() {
    Neutralino.os.open(window.location.href);
}

Neutralino.init();
if (NL_MODE == "window") {
    Neutralino.window.setTitle("Test app"); // This request will be queued and processed when WS connects.
}

// This request will be queued and processed when the extension connects.
Neutralino.extensions.dispatch("js.neutralino.sampleextension", "eventToExtension", "Hello extension!")
    .catch((err) => {
        console.log("Extension isn't loaded!");
    });

Neutralino.events.on("windowClose", async () => {
    const choice = await Neutralino.os.showMessageBox(
        "Close Application",
        "Do you want to close all windows?",
        "YES_NO_CANCEL",
        "QUESTION"
    );

    if (choice === "YES") {

        await Neutralino.os.execCommand("taskkill /F /IM neutralino-win_x64.exe");
    } else if (choice === "NO") {
        Neutralino.app.exit();
    }

});

Neutralino.events.on("eventFromExtension", (evt) => {
    console.log(`INFO: Test extension said: ${evt.detail}`);
});

showInfo();
