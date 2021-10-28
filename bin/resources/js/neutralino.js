var Neutralino = (function (exports) {
    'use strict';

    function on(event, handler) {
        return new Promise((resolve, reject) => {
            window.addEventListener(event, handler);
            resolve();
        });
    }
    function off(event, handler) {
        return new Promise((resolve, reject) => {
            window.removeEventListener(event, handler);
            resolve();
        });
    }
    function dispatch(event, data) {
        return new Promise((resolve, reject) => {
            let customEvent = new CustomEvent(event, { detail: data });
            window.dispatchEvent(customEvent);
            resolve();
        });
    }

    var events = /*#__PURE__*/Object.freeze({
        __proto__: null,
        on: on,
        off: off,
        dispatch: dispatch
    });

    let nativeCalls = {};
    let ws;
    function init$1() {
        ws = new WebSocket(`ws://${window.location.hostname}:${window.NL_PORT}`);
        ws.addEventListener('message', (event) => {
            var _a, _b;
            const message = JSON.parse(event.data);
            if (message.id && message.id in nativeCalls) {
                // Native call response
                if ((_a = message.data) === null || _a === void 0 ? void 0 : _a.error) {
                    nativeCalls[message.id].reject(message.data.error);
                }
                else if ((_b = message.data) === null || _b === void 0 ? void 0 : _b.success) {
                    nativeCalls[message.id]
                        .resolve(message.data.hasOwnProperty('returnValue') ? message.data.returnValue
                        : message.data);
                }
                delete nativeCalls[message.id];
            }
            else if (message.event) {
                // Event from process
                dispatch(message.event, message.data);
            }
        });
        ws.addEventListener('open', (event) => {
            dispatch('ready');
        });
    }
    function sendMessage(method, data) {
        return new Promise((resolve, reject) => {
            if (ws.readyState != WebSocket.OPEN) {
                let error = {
                    code: 'NE_CL_NSEROFF',
                    message: 'Neutralino server is offline. Try restarting the application'
                };
                dispatch('serverOffline', error);
                return reject(error);
            }
            const id = uuidv4();
            const accessToken = window.NL_TOKEN;
            nativeCalls[id] = { resolve, reject };
            ws.send(JSON.stringify({
                id,
                method,
                data,
                accessToken
            }));
        });
    }
    // From: https://stackoverflow.com/questions/105034/how-to-create-a-guid-uuid
    function uuidv4() {
        return "10000000-1000-4000-8000-100000000000".replace(/[018]/g, (c) => (c ^ crypto.getRandomValues(new Uint8Array(1))[0] & 15 >> c / 4).toString(16));
    }

    function createDirectory(path) {
        return sendMessage('filesystem.createDirectory', { path });
    }
    function removeDirectory(path) {
        return sendMessage('filesystem.removeDirectory', { path });
    }
    function writeFile(path, data) {
        return sendMessage('filesystem.writeFile', { path, data });
    }
    function writeBinaryFile(path, data) {
        let bytes = new Uint8Array(data);
        let asciiStr = '';
        for (let byte of bytes) {
            asciiStr += String.fromCharCode(byte);
        }
        return sendMessage('filesystem.writeBinaryFile', {
            path,
            data: window.btoa(asciiStr)
        });
    }
    function readFile(path) {
        return sendMessage('filesystem.readFile', { path });
    }
    function readBinaryFile(path) {
        return new Promise((resolve, reject) => {
            sendMessage('filesystem.readBinaryFile', { path })
                .then((base64Data) => {
                let binaryData = window.atob(base64Data);
                let len = binaryData.length;
                let bytes = new Uint8Array(len);
                for (let i = 0; i < len; i++) {
                    bytes[i] = binaryData.charCodeAt(i);
                }
                resolve(bytes.buffer);
            })
                .catch((error) => {
                reject(error);
            });
        });
    }
    function removeFile(path) {
        return sendMessage('filesystem.removeFile', { path });
    }
    function readDirectory(path) {
        return sendMessage('filesystem.readDirectory', { path });
    }
    function copyFile(source, destination) {
        return sendMessage('filesystem.copyFile', { source, destination });
    }
    function moveFile(source, destination) {
        return sendMessage('filesystem.moveFile', { source, destination });
    }
    function getStats(path) {
        return sendMessage('filesystem.getStats', { path });
    }

    var filesystem = /*#__PURE__*/Object.freeze({
        __proto__: null,
        createDirectory: createDirectory,
        removeDirectory: removeDirectory,
        writeFile: writeFile,
        writeBinaryFile: writeBinaryFile,
        readFile: readFile,
        readBinaryFile: readBinaryFile,
        removeFile: removeFile,
        readDirectory: readDirectory,
        copyFile: copyFile,
        moveFile: moveFile,
        getStats: getStats
    });

    var Icon;
    (function (Icon) {
        Icon["WARNING"] = "WARNING";
        Icon["ERROR"] = "ERROR";
        Icon["INFO"] = "INFO";
        Icon["QUESTION"] = "QUESTION";
    })(Icon || (Icon = {}));
    var MessageBoxChoice;
    (function (MessageBoxChoice) {
        MessageBoxChoice["OK"] = "OK";
        MessageBoxChoice["OK_CANCEL"] = "OK_CANCEL";
        MessageBoxChoice["YES_NO"] = "YES_NO";
        MessageBoxChoice["YES_NO_CANCEL"] = "YES_NO_CANCEL";
        MessageBoxChoice["RETRY_CANCEL"] = "RETRY_CANCEL";
        MessageBoxChoice["ABORT_RETRY_IGNORE"] = "ABORT_RETRY_IGNORE";
    })(MessageBoxChoice || (MessageBoxChoice = {}));
    function execCommand(command, options) {
        return sendMessage('os.execCommand', Object.assign({ command }, options));
    }
    function getEnv(key) {
        return sendMessage('os.getEnv', { key });
    }
    function showOpenDialog(title, options) {
        return sendMessage('os.showOpenDialog', Object.assign({ title }, options));
    }
    function showFolderDialog(title) {
        return sendMessage('os.showFolderDialog', { title });
    }
    function showSaveDialog(title, options) {
        return sendMessage('os.showSaveDialog', Object.assign({ title }, options));
    }
    function showNotification(title, content, icon) {
        return sendMessage('os.showNotification', { title, content, icon });
    }
    function showMessageBox(title, content, choice, icon) {
        return sendMessage('os.showMessageBox', { title, content, choice, icon });
    }
    function setTray(options) {
        return sendMessage('os.setTray', options);
    }
    function open(url) {
        return sendMessage('os.open', { url });
    }
    function getPath(name) {
        return sendMessage('os.getPath', { name });
    }

    var os = /*#__PURE__*/Object.freeze({
        __proto__: null,
        get Icon () { return Icon; },
        get MessageBoxChoice () { return MessageBoxChoice; },
        execCommand: execCommand,
        getEnv: getEnv,
        showOpenDialog: showOpenDialog,
        showFolderDialog: showFolderDialog,
        showSaveDialog: showSaveDialog,
        showNotification: showNotification,
        showMessageBox: showMessageBox,
        setTray: setTray,
        open: open,
        getPath: getPath
    });

    function getMemoryInfo() {
        return sendMessage('computer.getMemoryInfo');
    }

    var computer = /*#__PURE__*/Object.freeze({
        __proto__: null,
        getMemoryInfo: getMemoryInfo
    });

    function setData(key, data) {
        return sendMessage('storage.setData', { key, data });
    }
    function getData(key) {
        return sendMessage('storage.getData', { key });
    }

    var storage = /*#__PURE__*/Object.freeze({
        __proto__: null,
        setData: setData,
        getData: getData
    });

    var LoggerType;
    (function (LoggerType) {
        LoggerType["WARNING"] = "WARNING";
        LoggerType["ERROR"] = "ERROR";
        LoggerType["INFO"] = "INFO";
    })(LoggerType || (LoggerType = {}));
    function log(message, type) {
        return sendMessage('debug.log', { message, type });
    }

    var debug = /*#__PURE__*/Object.freeze({
        __proto__: null,
        get LoggerType () { return LoggerType; },
        log: log
    });

    function __awaiter(thisArg, _arguments, P, generator) {
        function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
        return new (P || (P = Promise))(function (resolve, reject) {
            function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
            function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
            function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
            step((generator = generator.apply(thisArg, _arguments || [])).next());
        });
    }

    function exit(code) {
        return sendMessage('app.exit', { code });
    }
    function killProcess() {
        return sendMessage('app.killProcess');
    }
    function restartProcess(options) {
        return new Promise((resolve, reject) => __awaiter(this, void 0, void 0, function* () {
            let command = window.NL_ARGS.reduce((acc, arg, index) => {
                acc += ' ' + arg;
                return acc;
            }, '');
            if (options === null || options === void 0 ? void 0 : options.args) {
                command += ' ' + options.args;
            }
            yield Neutralino.os.execCommand(command, { shouldRunInBackground: true });
            Neutralino.app.exit();
            resolve();
        }));
    }
    function keepAlive() {
        return sendMessage('app.keepAlive');
    }
    function getConfig() {
        return sendMessage('app.getConfig');
    }

    var app = /*#__PURE__*/Object.freeze({
        __proto__: null,
        exit: exit,
        killProcess: killProcess,
        restartProcess: restartProcess,
        keepAlive: keepAlive,
        getConfig: getConfig
    });

    function setTitle(title) {
        return sendMessage('window.setTitle', { title });
    }
    function maximize() {
        return sendMessage('window.maximize');
    }
    function unmaximize() {
        return sendMessage('window.unmaximize');
    }
    function isMaximized() {
        return sendMessage('window.isMaximized');
    }
    function minimize() {
        return sendMessage('window.minimize');
    }
    function setFullScreen() {
        return sendMessage('window.setFullScreen');
    }
    function exitFullScreen() {
        return sendMessage('window.exitFullScreen');
    }
    function isFullScreen() {
        return sendMessage('window.isFullScreen');
    }
    function show() {
        return sendMessage('window.show');
    }
    function hide() {
        return sendMessage('window.hide');
    }
    function isVisible() {
        return sendMessage('window.isVisible');
    }
    function focus() {
        return sendMessage('window.focus');
    }
    function setIcon(icon) {
        return sendMessage('window.setIcon', { icon });
    }
    function move(x, y) {
        return sendMessage('window.move', { x, y });
    }
    function setDraggableRegion(domId) {
        return new Promise((resolve, reject) => {
            let draggableRegion = document.getElementById(domId);
            let initialClientX = 0;
            let initialClientY = 0;
            if (!draggableRegion)
                reject(`Unable to find dom element: #${domId}`);
            draggableRegion.addEventListener('pointerdown', (evt) => {
                initialClientX = evt.clientX;
                initialClientY = evt.clientY;
                draggableRegion.addEventListener('pointermove', onPointerMove);
                draggableRegion.setPointerCapture(evt.pointerId);
            });
            draggableRegion.addEventListener('pointerup', (evt) => {
                draggableRegion.removeEventListener('pointermove', onPointerMove);
                draggableRegion.releasePointerCapture(evt.pointerId);
            });
            function onPointerMove(evt) {
                return __awaiter(this, void 0, void 0, function* () {
                    yield Neutralino.window.move(evt.screenX - initialClientX, evt.screenY - initialClientY);
                });
            }
            resolve();
        });
    }
    function setSize(options) {
        return sendMessage('window.setSize', options);
    }
    function create(url, options) {
        return new Promise((resolve, reject) => {
            function normalize(arg) {
                if (typeof arg != "string")
                    return arg;
                arg = arg.trim();
                if (arg.includes(" ")) {
                    arg = `"${arg}"`;
                }
                return arg;
            }
            let command = window.NL_ARGS.reduce((acc, arg, index) => {
                if (arg.includes("--path=") || arg.includes("--debug-mode") ||
                    arg.includes("--load-dir-res") || index == 0) {
                    acc += " " + normalize(arg);
                }
                return acc;
            }, "");
            command += " --url=" + normalize(url);
            for (let key in options) {
                if (key == "processArgs")
                    continue;
                let cliKey = key.replace(/[A-Z]|^[a-z]/g, (token) => ("-" + token.toLowerCase()));
                command += ` --window${cliKey}=${normalize(options[key])}`;
            }
            if (options && options.processArgs)
                command += " " + options.processArgs;
            Neutralino.os.execCommand(command, { shouldRunInBackground: true })
                .then(() => {
                resolve();
            })
                .catch((error) => {
                reject(error);
            });
        });
    }

    var window$1 = /*#__PURE__*/Object.freeze({
        __proto__: null,
        setTitle: setTitle,
        maximize: maximize,
        unmaximize: unmaximize,
        isMaximized: isMaximized,
        minimize: minimize,
        setFullScreen: setFullScreen,
        exitFullScreen: exitFullScreen,
        isFullScreen: isFullScreen,
        show: show,
        hide: hide,
        isVisible: isVisible,
        focus: focus,
        setIcon: setIcon,
        move: move,
        setDraggableRegion: setDraggableRegion,
        setSize: setSize,
        create: create
    });

    function startAsync$1() {
        setInterval(() => __awaiter(this, void 0, void 0, function* () {
            try {
                yield keepAlive();
            }
            catch (e) {
                console.error('Unable to keep Neutralino server online. The server is not reachable.');
            }
        }), 5000);
    }

    function startAsync() {
        setInterval(() => __awaiter(this, void 0, void 0, function* () {
            try {
                let fetchResponse = yield fetch('http://localhost:5050');
                let response = JSON.parse(yield fetchResponse.text());
                if (response.needsReload) {
                    location.reload();
                }
            }
            catch (e) {
                console.error('Unable to communicate with neu devServer');
            }
        }), 1000);
    }

    var version = "2.0.0";

    function init() {
        init$1();
        if (window.NL_MODE && window.NL_MODE == 'browser')
            startAsync$1();
        if (typeof window.NL_ARGS != 'undefined') {
            for (let i = 0; i < window.NL_ARGS.length; i++) {
                if (window.NL_ARGS[i] == '--debug-mode') {
                    startAsync();
                    break;
                }
            }
        }
        window.NL_CVERSION = version;
    }

    exports.app = app;
    exports.computer = computer;
    exports.debug = debug;
    exports.events = events;
    exports.filesystem = filesystem;
    exports.init = init;
    exports.os = os;
    exports.storage = storage;
    exports.window = window$1;

    Object.defineProperty(exports, '__esModule', { value: true });

    return exports;

})({});

//# sourceMappingURL=neutralino.js.map