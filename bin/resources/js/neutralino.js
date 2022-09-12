var Neutralino = (function (exports) {
    'use strict';

    function __awaiter(thisArg, _arguments, P, generator) {
        function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
        return new (P || (P = Promise))(function (resolve, reject) {
            function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
            function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
            function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
            step((generator = generator.apply(thisArg, _arguments || [])).next());
        });
    }

    function on(event, handler) {
        window.addEventListener(event, handler);
        return Promise.resolve({
            success: true,
            message: 'Event listener added'
        });
    }
    function off(event, handler) {
        window.removeEventListener(event, handler);
        return Promise.resolve({
            success: true,
            message: 'Event listener removed'
        });
    }
    function dispatch$1(event, data) {
        let customEvent = new CustomEvent(event, { detail: data });
        window.dispatchEvent(customEvent);
        return Promise.resolve({
            success: true,
            message: 'Message dispatched'
        });
    }

    let ws;
    let nativeCalls = {};
    let offlineMessageQueue = [];
    let extensionMessageQueue = {};
    function init$1() {
        initAuth();
        ws = new WebSocket(`ws://${window.location.hostname}:${window.NL_PORT}`);
        registerLibraryEvents();
        registerSocketEvents();
    }
    function sendMessage(method, data) {
        return new Promise((resolve, reject) => {
            if ((ws === null || ws === void 0 ? void 0 : ws.readyState) != WebSocket.OPEN) {
                sendWhenReady({ method, data, resolve, reject });
                return;
            }
            const id = uuidv4();
            const accessToken = getAuthToken();
            nativeCalls[id] = { resolve, reject };
            ws.send(JSON.stringify({
                id,
                method,
                data,
                accessToken
            }));
        });
    }
    function sendWhenReady(message) {
        offlineMessageQueue.push(message);
    }
    function sendWhenExtReady(extensionId, message) {
        if (extensionId in extensionMessageQueue) {
            extensionMessageQueue[extensionId].push(message);
        }
        else {
            extensionMessageQueue[extensionId] = [message];
        }
    }
    function registerLibraryEvents() {
        Neutralino.events.on('ready', () => __awaiter(this, void 0, void 0, function* () {
            yield processQueue(offlineMessageQueue);
            if (!window.NL_EXTENABLED) {
                return;
            }
            let stats = yield Neutralino.extensions.getStats();
            for (let extension of stats.connected) {
                dispatch$1('extensionReady', extension);
            }
        }));
        Neutralino.events.on('extClientConnect', (evt) => {
            dispatch$1('extensionReady', evt.detail);
        });
        if (!window.NL_EXTENABLED) {
            return;
        }
        Neutralino.events.on('extensionReady', (evt) => __awaiter(this, void 0, void 0, function* () {
            if (evt.detail in extensionMessageQueue) {
                yield processQueue(extensionMessageQueue[evt.detail]);
                delete extensionMessageQueue[evt.detail];
            }
        }));
    }
    function registerSocketEvents() {
        ws.addEventListener('message', (event) => {
            var _a, _b;
            const message = JSON.parse(event.data);
            if (message.id && message.id in nativeCalls) {
                // Native call response
                if ((_a = message.data) === null || _a === void 0 ? void 0 : _a.error) {
                    nativeCalls[message.id].reject(message.data.error);
                    if (message.data.error.code == 'NE_RT_INVTOKN') {
                        // critical auth error
                        // Perhaps, someone tried to open app from anoher client,
                        // with 'one-time' token mode
                        handleAuthError();
                    }
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
                dispatch$1(message.event, message.data);
            }
        });
        ws.addEventListener('open', (event) => __awaiter(this, void 0, void 0, function* () {
            dispatch$1('ready');
        }));
        ws.addEventListener('close', (event) => __awaiter(this, void 0, void 0, function* () {
            let error = {
                code: 'NE_CL_NSEROFF',
                message: 'Neutralino server is offline. Try restarting the application'
            };
            dispatch$1('serverOffline', error);
        }));
    }
    function processQueue(messageQueue) {
        return __awaiter(this, void 0, void 0, function* () {
            while (messageQueue.length > 0) {
                let message = messageQueue.shift();
                try {
                    let response = yield sendMessage(message.method, message.data);
                    message.resolve(response);
                }
                catch (err) {
                    message.reject(err);
                }
            }
        });
    }
    function handleAuthError() {
        ws.close();
        document.body.innerText = '';
        document.write('<code>NE_RT_INVTOKN</code>: Neutralinojs application configuration' +
            ' prevents accepting native calls from this client.');
    }
    function initAuth() {
        if (window.NL_TOKEN) {
            sessionStorage.setItem('NL_TOKEN', window.NL_TOKEN);
        }
    }
    function getAuthToken() {
        return window.NL_TOKEN || sessionStorage.getItem('NL_TOKEN') || '';
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
    function appendFile(path, data) {
        return sendMessage('filesystem.appendFile', { path, data });
    }
    function writeBinaryFile(path, data) {
        return sendMessage('filesystem.writeBinaryFile', {
            path,
            data: arrayBufferToBase64(data)
        });
    }
    function appendBinaryFile(path, data) {
        return sendMessage('filesystem.appendBinaryFile', {
            path,
            data: arrayBufferToBase64(data)
        });
    }
    function readFile(path, options) {
        return sendMessage('filesystem.readFile', Object.assign({ path }, options));
    }
    function readBinaryFile(path, options) {
        return new Promise((resolve, reject) => {
            sendMessage('filesystem.readBinaryFile', Object.assign({ path }, options))
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
    function getStats$1(path) {
        return sendMessage('filesystem.getStats', { path });
    }
    function arrayBufferToBase64(data) {
        let bytes = new Uint8Array(data);
        let asciiStr = '';
        for (let byte of bytes) {
            asciiStr += String.fromCharCode(byte);
        }
        return window.btoa(asciiStr);
    }

    var filesystem = /*#__PURE__*/Object.freeze({
        __proto__: null,
        createDirectory: createDirectory,
        removeDirectory: removeDirectory,
        writeFile: writeFile,
        appendFile: appendFile,
        writeBinaryFile: writeBinaryFile,
        appendBinaryFile: appendBinaryFile,
        readFile: readFile,
        readBinaryFile: readBinaryFile,
        removeFile: removeFile,
        readDirectory: readDirectory,
        copyFile: copyFile,
        moveFile: moveFile,
        getStats: getStats$1
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
    function spawnProcess(command) {
        return sendMessage('os.spawnProcess', { command });
    }
    function updateSpawnedProcess(id, event, data) {
        return sendMessage('os.updateSpawnedProcess', { id, event, data });
    }
    function getSpawnedProcesses() {
        return sendMessage('os.getSpawnedProcesses');
    }
    function getEnv(key) {
        return sendMessage('os.getEnv', { key });
    }
    function getEnvs() {
        return sendMessage('os.getEnvs');
    }
    function showOpenDialog(title, options) {
        return sendMessage('os.showOpenDialog', Object.assign({ title }, options));
    }
    function showFolderDialog(title, options) {
        return sendMessage('os.showFolderDialog', Object.assign({ title }, options));
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
        spawnProcess: spawnProcess,
        updateSpawnedProcess: updateSpawnedProcess,
        getSpawnedProcesses: getSpawnedProcesses,
        getEnv: getEnv,
        getEnvs: getEnvs,
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
    function getArch() {
        return sendMessage('computer.getArch');
    }
    function getKernelInfo() {
        return sendMessage('computer.getKernelInfo');
    }
    function getOSInfo() {
        return sendMessage('computer.getOSInfo');
    }
    function getCPUInfo() {
        return sendMessage('computer.getCPUInfo');
    }
    function getDisplays() {
        return sendMessage('computer.getDisplays');
    }
    function getMousePosition() {
        return sendMessage('computer.getMousePosition');
    }

    var computer = /*#__PURE__*/Object.freeze({
        __proto__: null,
        getMemoryInfo: getMemoryInfo,
        getArch: getArch,
        getKernelInfo: getKernelInfo,
        getOSInfo: getOSInfo,
        getCPUInfo: getCPUInfo,
        getDisplays: getDisplays,
        getMousePosition: getMousePosition
    });

    function setData(key, data) {
        return sendMessage('storage.setData', { key, data });
    }
    function getData(key) {
        return sendMessage('storage.getData', { key });
    }
    function getKeys() {
        return sendMessage('storage.getKeys');
    }

    var storage = /*#__PURE__*/Object.freeze({
        __proto__: null,
        setData: setData,
        getData: getData,
        getKeys: getKeys
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

    function exit(code) {
        return sendMessage('app.exit', { code });
    }
    function killProcess() {
        return sendMessage('app.killProcess');
    }
    function restartProcess(options) {
        return new Promise((resolve) => __awaiter(this, void 0, void 0, function* () {
            let command = window.NL_ARGS.reduce((acc, arg) => {
                acc += ' ' + arg;
                return acc;
            }, '');
            if (options === null || options === void 0 ? void 0 : options.args) {
                command += ' ' + options.args;
            }
            yield Neutralino.os.execCommand(command, { background: true });
            Neutralino.app.exit();
            resolve();
        }));
    }
    function getConfig() {
        return sendMessage('app.getConfig');
    }
    function broadcast$2(event, data) {
        return sendMessage('app.broadcast', { event, data });
    }

    var app = /*#__PURE__*/Object.freeze({
        __proto__: null,
        exit: exit,
        killProcess: killProcess,
        restartProcess: restartProcess,
        getConfig: getConfig,
        broadcast: broadcast$2
    });

    const draggableRegions = new WeakMap();
    function setTitle(title) {
        return sendMessage('window.setTitle', { title });
    }
    function getTitle() {
        return sendMessage('window.getTitle');
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
    function setDraggableRegion(domElementOrId) {
        return new Promise((resolve, reject) => {
            const draggableRegion = domElementOrId instanceof Element ?
                domElementOrId : document.getElementById(domElementOrId);
            let initialClientX = 0;
            let initialClientY = 0;
            if (!draggableRegion) {
                return reject({
                    code: 'NE_WD_DOMNOTF',
                    message: 'Unable to find DOM element'
                });
            }
            if (draggableRegions.has(draggableRegion)) {
                return reject({
                    code: 'NE_WD_ALRDREL',
                    message: 'This DOM element is already an active draggable region'
                });
            }
            draggableRegion.addEventListener('pointerdown', startPointerCapturing);
            draggableRegion.addEventListener('pointerup', endPointerCapturing);
            draggableRegions.set(draggableRegion, { pointerdown: startPointerCapturing, pointerup: endPointerCapturing });
            function onPointerMove(evt) {
                return __awaiter(this, void 0, void 0, function* () {
                    yield Neutralino.window.move(evt.screenX - initialClientX, evt.screenY - initialClientY);
                });
            }
            function startPointerCapturing(evt) {
                if (evt.button !== 0)
                    return;
                initialClientX = evt.clientX;
                initialClientY = evt.clientY;
                draggableRegion.addEventListener('pointermove', onPointerMove);
                draggableRegion.setPointerCapture(evt.pointerId);
            }
            function endPointerCapturing(evt) {
                draggableRegion.removeEventListener('pointermove', onPointerMove);
                draggableRegion.releasePointerCapture(evt.pointerId);
            }
            resolve({
                success: true,
                message: 'Draggable region was activated'
            });
        });
    }
    function unsetDraggableRegion(domElementOrId) {
        return new Promise((resolve, reject) => {
            const draggableRegion = domElementOrId instanceof Element ?
                domElementOrId : document.getElementById(domElementOrId);
            if (!draggableRegion) {
                return reject({
                    code: 'NE_WD_DOMNOTF',
                    message: 'Unable to find DOM element'
                });
            }
            if (!draggableRegions.has(draggableRegion)) {
                return reject({
                    code: 'NE_WD_NOTDRRE',
                    message: 'DOM element is not an active draggable region'
                });
            }
            const { pointerdown, pointerup } = draggableRegions.get(draggableRegion);
            draggableRegion.removeEventListener('pointerdown', pointerdown);
            draggableRegion.removeEventListener('pointerup', pointerup);
            draggableRegions.delete(draggableRegion);
            resolve({
                success: true,
                message: 'Draggable region was deactivated'
            });
        });
    }
    function setSize(options) {
        return new Promise((resolve, reject) => __awaiter(this, void 0, void 0, function* () {
            let sizeOptions = yield Neutralino.window.getSize();
            options = Object.assign(Object.assign({}, sizeOptions), options); // merge prioritizing options arg
            sendMessage('window.setSize', options)
                .then((response) => {
                resolve(response);
            })
                .catch((error) => {
                reject(error);
            });
        }));
    }
    function getSize() {
        return sendMessage('window.getSize');
    }
    function getPosition() {
        return sendMessage('window.getPosition');
    }
    function setAlwaysOnTop(onTop) {
        return sendMessage('window.setAlwaysOnTop', { onTop });
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
            Neutralino.os.execCommand(command, { background: true })
                .then((processInfo) => {
                resolve(processInfo);
            })
                .catch((error) => {
                reject(error);
            });
        });
    }

    var window$1 = /*#__PURE__*/Object.freeze({
        __proto__: null,
        setTitle: setTitle,
        getTitle: getTitle,
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
        unsetDraggableRegion: unsetDraggableRegion,
        setSize: setSize,
        getSize: getSize,
        getPosition: getPosition,
        setAlwaysOnTop: setAlwaysOnTop,
        create: create
    });

    function broadcast$1(event, data) {
        return sendMessage('events.broadcast', { event, data });
    }

    var events = /*#__PURE__*/Object.freeze({
        __proto__: null,
        broadcast: broadcast$1,
        on: on,
        off: off,
        dispatch: dispatch$1
    });

    function dispatch(extensionId, event, data) {
        return new Promise((resolve, reject) => __awaiter(this, void 0, void 0, function* () {
            let stats = yield Neutralino.extensions.getStats();
            if (!stats.loaded.includes(extensionId)) {
                reject({
                    code: 'NE_EX_EXTNOTL',
                    message: `${extensionId} is not loaded`
                });
            }
            else if (stats.connected.includes(extensionId)) {
                try {
                    let result = yield sendMessage('extensions.dispatch', { extensionId, event, data });
                    resolve(result);
                }
                catch (err) {
                    reject(err);
                }
            }
            else {
                // loaded but not connected yet.
                sendWhenExtReady(extensionId, {
                    method: 'extensions.dispatch',
                    data: { extensionId, event, data }, resolve, reject
                });
            }
        }));
    }
    function broadcast(event, data) {
        return sendMessage('extensions.broadcast', { event, data });
    }
    function getStats() {
        return sendMessage('extensions.getStats');
    }

    var extensions = /*#__PURE__*/Object.freeze({
        __proto__: null,
        dispatch: dispatch,
        broadcast: broadcast,
        getStats: getStats
    });

    let manifest = null;
    function checkForUpdates(url) {
        function isValidManifest(manifest) {
            if (manifest.applicationId && manifest.applicationId == window.NL_APPID
                && manifest.version && manifest.resourcesURL) {
                return true;
            }
            return false;
        }
        return new Promise((resolve, reject) => __awaiter(this, void 0, void 0, function* () {
            if (!url) {
                return reject({
                    code: 'NE_RT_NATRTER',
                    message: 'Missing require parameter: url'
                });
            }
            try {
                let response = yield fetch(url);
                manifest = JSON.parse(yield response.text());
                if (isValidManifest(manifest)) {
                    resolve(manifest);
                }
                else {
                    reject({
                        code: 'NE_UP_CUPDMER',
                        message: 'Invalid update manifest or mismatching applicationId'
                    });
                }
            }
            catch (err) {
                reject({
                    code: 'NE_UP_CUPDERR',
                    message: 'Unable to fetch update manifest'
                });
            }
        }));
    }
    function install() {
        return new Promise((resolve, reject) => __awaiter(this, void 0, void 0, function* () {
            if (!manifest) {
                return reject({
                    code: 'NE_UP_UPDNOUF',
                    message: 'No update manifest loaded'
                });
            }
            try {
                let response = yield fetch(manifest.resourcesURL);
                let resourcesBuffer = yield response.arrayBuffer();
                yield Neutralino.filesystem.writeBinaryFile(window.NL_PATH + "/resources.neu", resourcesBuffer);
                resolve({
                    success: true,
                    message: 'Update installed. Restart the process to see updates'
                });
            }
            catch (err) {
                reject({
                    code: 'NE_UP_UPDINER',
                    message: 'Update installation error'
                });
            }
        }));
    }

    var updater = /*#__PURE__*/Object.freeze({
        __proto__: null,
        checkForUpdates: checkForUpdates,
        install: install
    });

    function readText(key, data) {
        return sendMessage('clipboard.readText', { key, data });
    }
    function writeText(data) {
        return sendMessage('clipboard.writeText', { data });
    }

    var clipboard = /*#__PURE__*/Object.freeze({
        __proto__: null,
        readText: readText,
        writeText: writeText
    });

    var version = "3.6.0";

    let initialized = false;
    function init() {
        if (initialized) {
            return;
        }
        init$1();
        if (window.NL_ARGS.find((arg) => arg == '--neu-dev-auto-reload')) {
            Neutralino.events.on('neuDev_reloadApp', () => __awaiter(this, void 0, void 0, function* () {
                yield Neutralino.debug.log('Reloading the application...');
                location.reload();
            }));
        }
        window.NL_CVERSION = version;
        window.NL_CCOMMIT = '07c5eacccecea385b689e14c6225b7944b275142'; // only the build server will update this
        initialized = true;
    }

    exports.app = app;
    exports.clipboard = clipboard;
    exports.computer = computer;
    exports.debug = debug;
    exports.events = events;
    exports.extensions = extensions;
    exports.filesystem = filesystem;
    exports.init = init;
    exports.os = os;
    exports.storage = storage;
    exports.updater = updater;
    exports.window = window$1;

    Object.defineProperty(exports, '__esModule', { value: true });

    return exports;

})({});

//# sourceMappingURL=neutralino.js.map