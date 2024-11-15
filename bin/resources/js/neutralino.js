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
    typeof SuppressedError === "function" ? SuppressedError : function (error, suppressed, message) {
        var e = new Error(message);
        return e.name = "SuppressedError", e.error = error, e.suppressed = suppressed, e;
    };

    function dispatch$1(extensionId, event, data) {
        return new Promise((resolve, reject) => __awaiter(this, void 0, void 0, function* () {
            const stats = yield getStats$1();
            if (!stats.loaded.includes(extensionId)) {
                reject({
                    code: 'NE_EX_EXTNOTL',
                    message: `${extensionId} is not loaded`
                });
            }
            else if (stats.connected.includes(extensionId)) {
                try {
                    const result = yield sendMessage('extensions.dispatch', { extensionId, event, data });
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
    function broadcast$2(event, data) {
        return sendMessage('extensions.broadcast', { event, data });
    }
    function getStats$1() {
        return sendMessage('extensions.getStats');
    }

    var extensions = {
        __proto__: null,
        broadcast: broadcast$2,
        dispatch: dispatch$1,
        getStats: getStats$1
    };

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
    function dispatch(event, data) {
        const customEvent = new CustomEvent(event, { detail: data });
        window.dispatchEvent(customEvent);
        return Promise.resolve({
            success: true,
            message: 'Message dispatched'
        });
    }

    function base64ToBytesArray(data) {
        const binaryData = window.atob(data);
        const len = binaryData.length;
        const bytes = new Uint8Array(len);
        for (let i = 0; i < len; i++) {
            bytes[i] = binaryData.charCodeAt(i);
        }
        return bytes.buffer;
    }
    function arrayBufferToBase64(data) {
        let bytes = new Uint8Array(data);
        let asciiStr = '';
        for (let byte of bytes) {
            asciiStr += String.fromCharCode(byte);
        }
        return window.btoa(asciiStr);
    }

    let ws;
    const nativeCalls = {};
    const offlineMessageQueue = [];
    const extensionMessageQueue = {};
    function init$1() {
        initAuth();
        const connectToken = getAuthToken().split('.')[1];
        const hostname = (window.NL_GINJECTED || window.NL_CINJECTED) ?
            'localhost' : window.location.hostname;
        ws = new WebSocket(`ws://${hostname}:${window.NL_PORT}?connectToken=${connectToken}`);
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
        on('ready', () => __awaiter(this, void 0, void 0, function* () {
            yield processQueue(offlineMessageQueue);
            if (!window.NL_EXTENABLED) {
                return;
            }
            const stats = yield getStats$1();
            for (const extension of stats.connected) {
                dispatch('extensionReady', extension);
            }
        }));
        on('extClientConnect', (evt) => {
            dispatch('extensionReady', evt.detail);
        });
        if (!window.NL_EXTENABLED) {
            return;
        }
        on('extensionReady', (evt) => __awaiter(this, void 0, void 0, function* () {
            if (evt.detail in extensionMessageQueue) {
                yield processQueue(extensionMessageQueue[evt.detail]);
                delete extensionMessageQueue[evt.detail];
            }
        }));
    }
    function registerSocketEvents() {
        ws.addEventListener('message', (event) => {
            var _a, _b, _c;
            const message = JSON.parse(event.data);
            if (message.id && message.id in nativeCalls) {
                // Native call response
                if ((_a = message.data) === null || _a === void 0 ? void 0 : _a.error) {
                    nativeCalls[message.id].reject(message.data.error);
                    if (message.data.error.code == 'NE_RT_INVTOKN') {
                        // Invalid native method token
                        handleNativeMethodTokenError();
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
                if (message.event == 'openedFile' && ((_c = message === null || message === void 0 ? void 0 : message.data) === null || _c === void 0 ? void 0 : _c.action) == 'dataBinary') {
                    message.data.data = base64ToBytesArray(message.data.data);
                }
                dispatch(message.event, message.data);
            }
        });
        ws.addEventListener('open', (event) => __awaiter(this, void 0, void 0, function* () {
            dispatch('ready');
        }));
        ws.addEventListener('close', (event) => __awaiter(this, void 0, void 0, function* () {
            const error = {
                code: 'NE_CL_NSEROFF',
                message: 'Neutralino server is offline. Try restarting the application'
            };
            dispatch('serverOffline', error);
        }));
        ws.addEventListener('error', (event) => __awaiter(this, void 0, void 0, function* () {
            handleConnectError();
        }));
    }
    function processQueue(messageQueue) {
        return __awaiter(this, void 0, void 0, function* () {
            while (messageQueue.length > 0) {
                const message = messageQueue.shift();
                try {
                    const response = yield sendMessage(message.method, message.data);
                    message.resolve(response);
                }
                catch (err) {
                    message.reject(err);
                }
            }
        });
    }
    function handleNativeMethodTokenError() {
        ws.close();
        document.body.innerText = '';
        document.write('<code>NE_RT_INVTOKN</code>: Neutralinojs application cannot' +
            ' execute native methods since <code>NL_TOKEN</code> is invalid.');
    }
    function handleConnectError() {
        document.body.innerText = '';
        document.write('<code>NE_CL_IVCTOKN</code>: Neutralinojs application cannot' +
            ' connect with the framework core using <code>NL_TOKEN</code>.');
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
    function remove(path) {
        return sendMessage('filesystem.remove', { path });
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
    function readFile$1(path, options) {
        return sendMessage('filesystem.readFile', Object.assign({ path }, options));
    }
    function readBinaryFile$1(path, options) {
        return new Promise((resolve, reject) => {
            sendMessage('filesystem.readBinaryFile', Object.assign({ path }, options))
                .then((base64Data) => {
                resolve(base64ToBytesArray(base64Data));
            })
                .catch((error) => {
                reject(error);
            });
        });
    }
    function openFile(path) {
        return sendMessage('filesystem.openFile', { path });
    }
    function createWatcher(path) {
        return sendMessage('filesystem.createWatcher', { path });
    }
    function removeWatcher(id) {
        return sendMessage('filesystem.removeWatcher', { id });
    }
    function getWatchers() {
        return sendMessage('filesystem.getWatchers');
    }
    function updateOpenedFile(id, event, data) {
        return sendMessage('filesystem.updateOpenedFile', { id, event, data });
    }
    function getOpenedFileInfo(id) {
        return sendMessage('filesystem.getOpenedFileInfo', { id });
    }
    function readDirectory(path, options) {
        return sendMessage('filesystem.readDirectory', Object.assign({ path }, options));
    }
    function copy(source, destination, options) {
        return sendMessage('filesystem.copy', Object.assign({ source, destination }, options));
    }
    function move$1(source, destination) {
        return sendMessage('filesystem.move', { source, destination });
    }
    function getStats(path) {
        return sendMessage('filesystem.getStats', { path });
    }
    function getAbsolutePath(path) {
        return sendMessage('filesystem.getAbsolutePath', { path });
    }
    function getRelativePath(path, base) {
        return sendMessage('filesystem.getRelativePath', { path, base });
    }
    function getPathParts(path) {
        return sendMessage('filesystem.getPathParts', { path });
    }

    var filesystem = {
        __proto__: null,
        appendBinaryFile: appendBinaryFile,
        appendFile: appendFile,
        copy: copy,
        createDirectory: createDirectory,
        createWatcher: createWatcher,
        getAbsolutePath: getAbsolutePath,
        getOpenedFileInfo: getOpenedFileInfo,
        getPathParts: getPathParts,
        getRelativePath: getRelativePath,
        getStats: getStats,
        getWatchers: getWatchers,
        move: move$1,
        openFile: openFile,
        readBinaryFile: readBinaryFile$1,
        readDirectory: readDirectory,
        readFile: readFile$1,
        remove: remove,
        removeWatcher: removeWatcher,
        updateOpenedFile: updateOpenedFile,
        writeBinaryFile: writeBinaryFile,
        writeFile: writeFile
    };

    function execCommand(command, options) {
        return sendMessage('os.execCommand', Object.assign({ command }, options));
    }
    function spawnProcess(command, cwd) {
        return sendMessage('os.spawnProcess', { command, cwd });
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

    var os = {
        __proto__: null,
        execCommand: execCommand,
        getEnv: getEnv,
        getEnvs: getEnvs,
        getPath: getPath,
        getSpawnedProcesses: getSpawnedProcesses,
        open: open,
        setTray: setTray,
        showFolderDialog: showFolderDialog,
        showMessageBox: showMessageBox,
        showNotification: showNotification,
        showOpenDialog: showOpenDialog,
        showSaveDialog: showSaveDialog,
        spawnProcess: spawnProcess,
        updateSpawnedProcess: updateSpawnedProcess
    };

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

    var computer = {
        __proto__: null,
        getArch: getArch,
        getCPUInfo: getCPUInfo,
        getDisplays: getDisplays,
        getKernelInfo: getKernelInfo,
        getMemoryInfo: getMemoryInfo,
        getMousePosition: getMousePosition,
        getOSInfo: getOSInfo
    };

    function setData(key, data) {
        return sendMessage('storage.setData', { key, data });
    }
    function getData(key) {
        return sendMessage('storage.getData', { key });
    }
    function getKeys() {
        return sendMessage('storage.getKeys');
    }

    var storage = {
        __proto__: null,
        getData: getData,
        getKeys: getKeys,
        setData: setData
    };

    function log(message, type) {
        return sendMessage('debug.log', { message, type });
    }

    var debug = {
        __proto__: null,
        log: log
    };

    function exit(code) {
        return sendMessage('app.exit', { code });
    }
    function killProcess() {
        return sendMessage('app.killProcess');
    }
    function restartProcess(options) {
        return new Promise((resolve) => __awaiter(this, void 0, void 0, function* () {
            let command = window.NL_ARGS.reduce((acc, arg) => {
                if (arg.includes(' ')) {
                    arg = `"${arg}"`;
                }
                acc += ' ' + arg;
                return acc;
            }, '');
            if (options === null || options === void 0 ? void 0 : options.args) {
                command += ' ' + options.args;
            }
            yield execCommand(command, { background: true });
            exit();
            resolve();
        }));
    }
    function getConfig() {
        return sendMessage('app.getConfig');
    }
    function broadcast$1(event, data) {
        return sendMessage('app.broadcast', { event, data });
    }
    function readProcessInput(readAll) {
        return sendMessage('app.readProcessInput', { readAll });
    }
    function writeProcessOutput(data) {
        return sendMessage('app.writeProcessOutput', { data });
    }
    function writeProcessError(data) {
        return sendMessage('app.writeProcessError', { data });
    }

    var app = {
        __proto__: null,
        broadcast: broadcast$1,
        exit: exit,
        getConfig: getConfig,
        killProcess: killProcess,
        readProcessInput: readProcessInput,
        restartProcess: restartProcess,
        writeProcessError: writeProcessError,
        writeProcessOutput: writeProcessOutput
    };

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
    function unminimize() {
        return sendMessage('window.unminimize');
    }
    function isMinimized() {
        return sendMessage('window.isMinimized');
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
    function center() {
        return sendMessage('window.center');
    }
    function setDraggableRegion(domElementOrId, options = {}) {
        return new Promise((resolve, reject) => {
            const draggableRegion = domElementOrId instanceof Element ?
                domElementOrId : document.getElementById(domElementOrId);
            let initialClientX = 0;
            let initialClientY = 0;
            let absDragMovementDistance = 0;
            let shouldReposition = false;
            let lastMoveTimestamp = performance.now();
            let isPointerCaptured = options.alwaysCapture;
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
            draggableRegion.addEventListener('pointercancel', endPointerCapturing);
            draggableRegions.set(draggableRegion, { pointerdown: startPointerCapturing, pointerup: endPointerCapturing });
            function onPointerMove(evt) {
                return __awaiter(this, void 0, void 0, function* () {
                    var _a;
                    // Get absolute drag distance from the starting point
                    const dx = evt.clientX - initialClientX, dy = evt.clientY - initialClientY;
                    absDragMovementDistance = Math.sqrt(dx * dx + dy * dy);
                    // Only start pointer capturing when the user dragged more than a certain amount of distance
                    // This ensures that the user can also click on the dragable area, e.g. if the area is menu / navbar
                    if (absDragMovementDistance >= ((_a = options.dragMinDistance) !== null && _a !== void 0 ? _a : 10)) {
                        shouldReposition = true;
                        if (!isPointerCaptured) {
                            draggableRegion.setPointerCapture(evt.pointerId);
                            isPointerCaptured = true;
                        }
                    }
                    if (shouldReposition) {
                        const currentMilliseconds = performance.now();
                        const timeTillLastMove = currentMilliseconds - lastMoveTimestamp;
                        // Limit move calls to 1 per every 5ms - TODO: introduce constant instead of magic number?
                        if (timeTillLastMove < 5) {
                            // Do not execute move more often than 1x every 5ms or performance will drop
                            return;
                        }
                        // Store current time minus the offset
                        lastMoveTimestamp = currentMilliseconds - (timeTillLastMove - 5);
                        yield move(evt.screenX - initialClientX, evt.screenY - initialClientY);
                        return;
                    }
                });
            }
            function startPointerCapturing(evt) {
                if (evt.button !== 0)
                    return;
                initialClientX = evt.clientX;
                initialClientY = evt.clientY;
                draggableRegion.addEventListener('pointermove', onPointerMove);
                if (options.alwaysCapture) {
                    draggableRegion.setPointerCapture(evt.pointerId);
                }
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
            draggableRegion.removeEventListener('pointercancel', pointerup);
            draggableRegions.delete(draggableRegion);
            resolve({
                success: true,
                message: 'Draggable region was deactivated'
            });
        });
    }
    function setSize(options) {
        return new Promise((resolve, reject) => __awaiter(this, void 0, void 0, function* () {
            let sizeOptions = yield getSize();
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
            options = Object.assign(Object.assign({}, options), { useSavedState: false });
            // useSavedState: false -> Child windows won't save their states
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
                let cliKey = '-' + key.replace(/([a-z])([A-Z])/g, '$1-$2').toLowerCase();
                command += ` --window${cliKey}=${normalize(options[key])}`;
            }
            if (options && options.processArgs)
                command += " " + options.processArgs;
            execCommand(command, { background: true })
                .then((processInfo) => {
                resolve(processInfo);
            })
                .catch((error) => {
                reject(error);
            });
        });
    }

    var window$1 = {
        __proto__: null,
        center: center,
        create: create,
        exitFullScreen: exitFullScreen,
        focus: focus,
        getPosition: getPosition,
        getSize: getSize,
        getTitle: getTitle,
        hide: hide,
        isFullScreen: isFullScreen,
        isMaximized: isMaximized,
        isMinimized: isMinimized,
        isVisible: isVisible,
        maximize: maximize,
        minimize: minimize,
        move: move,
        setAlwaysOnTop: setAlwaysOnTop,
        setDraggableRegion: setDraggableRegion,
        setFullScreen: setFullScreen,
        setIcon: setIcon,
        setSize: setSize,
        setTitle: setTitle,
        show: show,
        unmaximize: unmaximize,
        unminimize: unminimize,
        unsetDraggableRegion: unsetDraggableRegion
    };

    function broadcast(event, data) {
        return sendMessage('events.broadcast', { event, data });
    }

    var events = {
        __proto__: null,
        broadcast: broadcast,
        dispatch: dispatch,
        off: off,
        on: on
    };

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
                const response = yield fetch(url);
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
                const response = yield fetch(manifest.resourcesURL);
                const resourcesBuffer = yield response.arrayBuffer();
                yield writeBinaryFile(window.NL_PATH + "/resources.neu", resourcesBuffer);
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

    var updater = {
        __proto__: null,
        checkForUpdates: checkForUpdates,
        install: install
    };

    function getFormat() {
        return sendMessage('clipboard.getFormat');
    }
    function readText() {
        return sendMessage('clipboard.readText');
    }
    function readImage() {
        return new Promise((resolve, reject) => {
            sendMessage('clipboard.readImage')
                .then((image) => {
                if (image) {
                    image.data = base64ToBytesArray(image.data);
                }
                resolve(image);
            })
                .catch((error) => {
                reject(error);
            });
        });
    }
    function writeText(data) {
        return sendMessage('clipboard.writeText', { data });
    }
    function writeImage(image) {
        const props = Object.assign({}, image);
        if (image === null || image === void 0 ? void 0 : image.data) {
            props.data = arrayBufferToBase64(image.data);
        }
        return sendMessage('clipboard.writeImage', props);
    }
    function clear() {
        return sendMessage('clipboard.clear');
    }

    var clipboard = {
        __proto__: null,
        clear: clear,
        getFormat: getFormat,
        readImage: readImage,
        readText: readText,
        writeImage: writeImage,
        writeText: writeText
    };

    function getFiles() {
        return sendMessage('resources.getFiles');
    }
    function extractFile(path, destination) {
        return sendMessage('resources.extractFile', { path, destination });
    }
    function readFile(path) {
        return sendMessage('resources.readFile', { path });
    }
    function readBinaryFile(path) {
        return new Promise((resolve, reject) => {
            sendMessage('resources.readBinaryFile', { path })
                .then((base64Data) => {
                resolve(base64ToBytesArray(base64Data));
            })
                .catch((error) => {
                reject(error);
            });
        });
    }

    var resources = {
        __proto__: null,
        extractFile: extractFile,
        getFiles: getFiles,
        readBinaryFile: readBinaryFile,
        readFile: readFile
    };

    function getMethods() {
        return sendMessage('custom.getMethods');
    }

    var custom = {
        __proto__: null,
        getMethods: getMethods
    };

    var version = "5.5.0";

    let initialized = false;
    function init(options = {}) {
        options = Object.assign({ exportCustomMethods: true }, options);
        if (initialized) {
            return;
        }
        init$1();
        if (window.NL_ARGS.find((arg) => arg == '--neu-dev-auto-reload')) {
            on('neuDev_reloadApp', () => __awaiter(this, void 0, void 0, function* () {
                yield log('Reloading the application...');
                location.reload();
            }));
        }
        if (options.exportCustomMethods && window.NL_CMETHODS && window.NL_CMETHODS.length > 0) {
            for (const method of window.NL_CMETHODS) {
                Neutralino.custom[method] = (...args) => {
                    let data = {};
                    for (const [argi, argv] of args.entries()) {
                        if (typeof argv == 'object' && !Array.isArray(argv) && argv != null) {
                            data = Object.assign(Object.assign({}, data), argv);
                        }
                        else {
                            data = Object.assign(Object.assign({}, data), { ['arg' + argi]: argv });
                        }
                    }
                    return sendMessage('custom.' + method, data);
                };
            }
        }
        window.NL_CVERSION = version;
        window.NL_CCOMMIT = '425c526c318342e0e5d0f17caceef2a53049eda4'; // only the build server will update this
        initialized = true;
    }

    exports.app = app;
    exports.clipboard = clipboard;
    exports.computer = computer;
    exports.custom = custom;
    exports.debug = debug;
    exports.events = events;
    exports.extensions = extensions;
    exports.filesystem = filesystem;
    exports.init = init;
    exports.os = os;
    exports.resources = resources;
    exports.storage = storage;
    exports.updater = updater;
    exports.window = window$1;

    return exports;

})({});
//# sourceMappingURL=neutralino.js.map

//# sourceMappingURL=neutralino.js.map