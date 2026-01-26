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
if(NL_MODE == "window") {
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


window.addEventListener('keydown', (e) => {
    //check if ctrlkey is click with 'w' or 'W'
    if (e.ctrlKey && (e.key === 'w' || e.key === 'W')) {         
        e.preventDefault(); // it can prevent browser default behavior 
        Neutralino.app.exit(); //this close application
    }
});

showInfo();
