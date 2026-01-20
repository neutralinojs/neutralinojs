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
// Create the exit button dynamically
function createExitButton() {
    const btn = document.createElement("div");
    btn.id = "fullscreen-exit-btn";
    btn.innerHTML = "✕";
    btn.style.cssText = `
        display: none;
        position: fixed;
        top: 20px;
        left: 50%;
        transform: translateX(-50%);
        width: 40px;
        height: 40px;
        background: rgba(0, 0, 0, 0.7);
        border-radius: 50%;
        cursor: pointer;
        z-index: 9999;
        font-size: 24px;
        color: white;
        text-align: center;
        line-height: 40px;
    `;
    btn.onclick = exitFullScreen;
    document.body.appendChild(btn);
}

<<<<<<< Updated upstream
Neutralino.events.on("windowClose", () => {
    Neutralino.app.exit();
});

Neutralino.events.on("eventFromExtension", (evt) => {
    console.log(`INFO: Test extension said: ${evt.detail}`);
});

=======
// Call this when app loads
createExitButton();
// Show/hide fullscreen exit button
function showExitButton() {
    document.getElementById("fullscreen-exit-btn").style.display = "block";
}

function hideExitButton() {
    document.getElementById("fullscreen-exit-btn").style.display = "none";
}

// Exit fullscreen function (called by X button click)
async function exitFullScreen() {
    await Neutralino.window.exitFullScreen();
    isInFullScreen = false;  // Update state
    hideExitButton();
}

// Modify existing F11 handler to show/hide button:
// When entering fullscreen: call showExitButton()
// When exiting fullscreen: call hideExitButton()
// Track Esc key hold time
let escPressedTime = null;
let escHoldTimer = null;
const ESC_HOLD_DURATION = 2000; // 2 seconds hold to exit fullscreen

// F11 to toggle fullscreen
// Track if we're in fullscreen
let isInFullScreen = false;

// Show X button only when mouse is at top of screen (in fullscreen)
document.addEventListener("mousemove", (event) => {
    if (isInFullScreen) {
        if (event.clientY < 50) {
            // Mouse near top - show button
            showExitButton();
        } else {
            // Mouse moved away - hide button
            hideExitButton();
        }
    }
});
document.addEventListener("keydown", async (event) => {
    // F11 - Toggle fullscreen
    if (event.key === "F11") {
        event.preventDefault();
        const fullScreen = await Neutralino.window.isFullScreen();
        if (fullScreen) {
            await Neutralino.window.exitFullScreen();
            isInFullScreen = false;
            hideExitButton();
        } else {
            await Neutralino.window.setFullScreen();
            isInFullScreen = true;
            // Don't show button yet - wait for mouse to go to top
        }
    }

    // Esc - Start tracking hold time and check continuously
    if (event.key === "Escape") {
        if (!escPressedTime) {
            escPressedTime = Date.now();

            // Start checking if held long enough (every 100ms)
            escHoldTimer = setInterval(async () => {
                if (Date.now() - escPressedTime >= ESC_HOLD_DURATION) {
                    clearInterval(escHoldTimer);  // Stop checking
                    escHoldTimer = null;
                    escPressedTime = null;

                    const fullScreen = await Neutralino.window.isFullScreen();
                    if (fullScreen) {
                        await Neutralino.window.exitFullScreen();
                        isInFullScreen = false;
                        hideExitButton();
                    }
                }
            }, 100);
        }
    }
});

// Esc release - Cleanup timer if released early
document.addEventListener("keyup", (event) => {
    if (event.key === "Escape") {
        escPressedTime = null;
        if (escHoldTimer) {
            clearInterval(escHoldTimer);
            escHoldTimer = null;
        }
    }
});

Neutralino.events.on("eventFromExtension", (evt) => {
    console.log(`INFO: Test extension said: ${evt.detail}`);
});

>>>>>>> Stashed changes
showInfo();
