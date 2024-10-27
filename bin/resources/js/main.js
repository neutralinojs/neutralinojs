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

Neutralino.events.on("windowClose", () => {
    Neutralino.app.exit();
});

Neutralino.events.on("eventFromExtension", (evt) => {
    console.log(`INFO: Test extension said: ${evt.detail}`);
});

showInfo();

// New function to test execCommand behavior
async function funMain() {
    try {
        await Neutralino.os.execCommand("chcp 65001 & echo a"); // Ensure UTF-8 encoding on Windows
        var res = await Neutralino.os.execCommand("chcp 65001 & echo ä"); // Should now handle special characters
        console.log(res);
        await Neutralino.os.execCommand("echo a"); // This should now run
    } catch (error) {
        console.error("Error executing command:", error);
    }
}

funMain(); // Call the function to test it
