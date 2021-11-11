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
Neutralino.events.on("ready", () => {
    // Immediate native calls
});

Neutralino.events.on("windowClose", () => {
    Neutralino.app.exit();
});

Neutralino.events.on("eventFromExtension", (evt) => {
    console.log(evt.detail);
});

showInfo();
