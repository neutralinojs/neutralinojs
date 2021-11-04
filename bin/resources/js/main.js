function showInfo() {
    document.getElementById('info').innerHTML = `${NL_APPID} is running on port ${NL_PORT}
        inside ${NL_OS}<br/><br/><span>v${NL_VERSION}</span>`;
}

function openDocs() {
    Neutralino.app.open("https://neutralino.js.org/docs");
}

Neutralino.init();
Neutralino.events.on("ready", () => {
    // Immediate native calls
});
Neutralino.events.on("windowClose", () => {
    Neutralino.app.exit();
});

showInfo();
