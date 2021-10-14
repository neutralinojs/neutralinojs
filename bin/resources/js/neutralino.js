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

    var RequestType;
    (function (RequestType) {
        RequestType["GET"] = "GET";
        RequestType["POST"] = "POST";
    })(RequestType || (RequestType = {}));
    function request(options) {
        return new Promise((resolve, reject) => {
            if (options.isNativeMethod)
                options.url = 'http://localhost:' + window.NL_PORT + '/__nativeMethod_' + options.url;
            if (options.data)
                options.data = JSON.stringify(options.data);
            let headers = new Headers();
            headers.append('Content-Type', 'application/json');
            headers.append('Authorization', 'Basic ' + window.NL_TOKEN);
            fetch(options.url, {
                method: options.type,
                headers,
                body: options.data
            })
                .then((resp) => __awaiter(this, void 0, void 0, function* () {
                let respData = yield resp.text();
                let respObj = null;
                if (respData) {
                    respObj = JSON.parse(respData);
                }
                if (respObj && respObj.success) {
                    resolve(respObj.hasOwnProperty('returnValue')
                        ? respObj.returnValue
                        : respObj);
                }
                if (respObj && respObj.error)
                    reject(respObj.error);
            }))
                .catch((e) => {
                let error = {
                    code: 'NE_CL_NSEROFF',
                    message: 'Neutralino server is offline. Try restarting the application'
                };
                dispatch('serverOffline', error);
                reject(error);
            });
        });
    }

    function createDirectory(path) {
        return request({
            url: 'filesystem.createDirectory',
            type: RequestType.POST,
            data: {
                path
            },
            isNativeMethod: true
        });
    }
    function removeDirectory(path) {
        return request({
            url: 'filesystem.removeDirectory',
            type: RequestType.POST,
            data: {
                path
            },
            isNativeMethod: true
        });
    }
    function writeFile(path, data) {
        return request({
            url: 'filesystem.writeFile',
            type: RequestType.POST,
            data: {
                path,
                data
            },
            isNativeMethod: true
        });
    }
    function writeBinaryFile(path, data) {
        let bytes = new Uint8Array(data);
        let asciiStr = '';
        for (let byte of bytes) {
            asciiStr += String.fromCharCode(byte);
        }
        return request({
            url: 'filesystem.writeBinaryFile',
            type: RequestType.POST,
            data: {
                path,
                data: window.btoa(asciiStr)
            },
            isNativeMethod: true
        });
    }
    function readFile(path) {
        return request({
            url: 'filesystem.readFile',
            type: RequestType.POST,
            data: {
                path
            },
            isNativeMethod: true
        });
    }
    function readBinaryFile(path) {
        return new Promise((resolve, reject) => {
            request({
                url: 'filesystem.readBinaryFile',
                type: RequestType.POST,
                data: {
                    path
                },
                isNativeMethod: true
            })
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
        return request({
            url: 'filesystem.removeFile',
            type: RequestType.POST,
            data: {
                path
            },
            isNativeMethod: true
        });
    }
    function readDirectory(path) {
        return request({
            url: 'filesystem.readDirectory',
            type: RequestType.POST,
            data: {
                path
            },
            isNativeMethod: true
        });
    }
    function copyFile(source, destination) {
        return request({
            url: 'filesystem.copyFile',
            type: RequestType.POST,
            data: {
                source,
                destination
            },
            isNativeMethod: true
        });
    }
    function moveFile(source, destination) {
        return request({
            url: 'filesystem.moveFile',
            type: RequestType.POST,
            data: {
                source,
                destination
            },
            isNativeMethod: true
        });
    }
    function getStats(path) {
        return request({
            url: 'filesystem.getStats',
            type: RequestType.POST,
            data: {
                path
            },
            isNativeMethod: true
        });
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
        return request({
            url: 'os.execCommand',
            type: RequestType.POST,
            data: Object.assign({ command }, options),
            isNativeMethod: true
        });
    }
    function getEnv(key) {
        return request({
            url: 'os.getEnv',
            type: RequestType.POST,
            data: {
                key
            },
            isNativeMethod: true
        });
    }
    function showOpenDialog(title, options) {
        return request({
            url: 'os.showOpenDialog',
            type: RequestType.POST,
            data: Object.assign({ title }, options),
            isNativeMethod: true
        });
    }
    function showFolderDialog(title) {
        return request({
            url: 'os.showFolderDialog',
            type: RequestType.POST,
            data: {
                title
            },
            isNativeMethod: true
        });
    }
    function showSaveDialog(title, options) {
        return request({
            url: 'os.showSaveDialog',
            type: RequestType.POST,
            data: Object.assign({ title }, options),
            isNativeMethod: true
        });
    }
    function showNotification(title, content, icon) {
        return request({
            url: 'os.showNotification',
            type: RequestType.POST,
            data: {
                title,
                content,
                icon
            },
            isNativeMethod: true
        });
    }
    function showMessageBox(title, content, choice, icon) {
        return request({
            url: 'os.showMessageBox',
            type: RequestType.POST,
            data: {
                title,
                content,
                choice,
                icon
            },
            isNativeMethod: true
        });
    }
    function setTray(options) {
        return request({
            url: 'os.setTray',
            type: RequestType.POST,
            data: options,
            isNativeMethod: true
        });
    }
    function open(url) {
        return request({
            url: 'os.open',
            type: RequestType.POST,
            data: {
                url
            },
            isNativeMethod: true
        });
    }
    function getPath(name) {
        return request({
            url: 'os.getPath',
            type: RequestType.POST,
            data: {
                name
            },
            isNativeMethod: true
        });
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
        return request({
            url: 'computer.getMemoryInfo',
            type: RequestType.GET,
            isNativeMethod: true
        });
    }

    var computer = /*#__PURE__*/Object.freeze({
        __proto__: null,
        getMemoryInfo: getMemoryInfo
    });

    function setData(key, data) {
        return request({
            url: 'storage.setData',
            type: RequestType.POST,
            data: {
                key,
                data
            },
            isNativeMethod: true
        });
    }
    function getData(key) {
        return request({
            url: 'storage.getData',
            type: RequestType.POST,
            data: {
                key
            },
            isNativeMethod: true
        });
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
        return request({
            url: 'debug.log',
            type: RequestType.POST,
            data: {
                message,
                type
            },
            isNativeMethod: true
        });
    }

    var debug = /*#__PURE__*/Object.freeze({
        __proto__: null,
        get LoggerType () { return LoggerType; },
        log: log
    });

    function exit(code) {
        return request({
            url: 'app.exit',
            type: RequestType.POST,
            data: {
                code
            },
            isNativeMethod: true
        });
    }
    function killProcess() {
        return request({
            url: 'app.killProcess',
            type: RequestType.GET,
            isNativeMethod: true
        });
    }
    function restartProcess(options) {
        return new Promise((resolve, reject) => __awaiter(this, void 0, void 0, function* () {
            let command = window.NL_ARGS.reduce((acc, arg, index) => {
                acc += ' ' + arg;
                return acc;
            }, '');
            if (options.args) {
                command += ' ' + options.args;
            }
            yield Neutralino.os.execCommand(command, { shouldRunInBackground: true });
            Neutralino.app.exit();
            resolve();
        }));
    }
    function keepAlive() {
        return request({
            url: 'app.keepAlive',
            type: RequestType.GET,
            isNativeMethod: true
        });
    }
    function getConfig() {
        return request({
            url: 'app.getConfig',
            type: RequestType.GET,
            isNativeMethod: true
        });
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
        return request({
            url: 'window.setTitle',
            type: RequestType.POST,
            data: {
                title
            },
            isNativeMethod: true
        });
    }
    function maximize() {
        return request({
            url: 'window.maximize',
            type: RequestType.GET,
            isNativeMethod: true
        });
    }
    function unmaximize() {
        return request({
            url: 'window.unmaximize',
            type: RequestType.GET,
            isNativeMethod: true
        });
    }
    function isMaximized() {
        return request({
            url: 'window.isMaximized',
            type: RequestType.GET,
            isNativeMethod: true
        });
    }
    function minimize() {
        return request({
            url: 'window.minimize',
            type: RequestType.GET,
            isNativeMethod: true
        });
    }
    function setFullScreen() {
        return request({
            url: 'window.setFullScreen',
            type: RequestType.GET,
            isNativeMethod: true
        });
    }
    function exitFullScreen() {
        return request({
            url: 'window.exitFullScreen',
            type: RequestType.GET,
            isNativeMethod: true
        });
    }
    function isFullScreen() {
        return request({
            url: 'window.isFullScreen',
            type: RequestType.GET,
            isNativeMethod: true
        });
    }
    function show() {
        return request({
            url: 'window.show',
            type: RequestType.GET,
            isNativeMethod: true
        });
    }
    function hide() {
        return request({
            url: 'window.hide',
            type: RequestType.GET,
            isNativeMethod: true
        });
    }
    function isVisible() {
        return request({
            url: 'window.isVisible',
            type: RequestType.GET,
            isNativeMethod: true
        });
    }
    function focus() {
        return request({
            url: 'window.focus',
            type: RequestType.GET,
            isNativeMethod: true
        });
    }
    function setIcon(icon) {
        return request({
            url: 'window.setIcon',
            type: RequestType.POST,
            isNativeMethod: true,
            data: {
                icon
            }
        });
    }
    function move(x, y) {
        return request({
            url: 'window.move',
            type: RequestType.POST,
            isNativeMethod: true,
            data: {
                x, y
            }
        });
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
        return request({
            url: 'window.setSize',
            type: RequestType.POST,
            isNativeMethod: true,
            data: options
        });
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

    const PING_INTERVAL_MS = 5000;
    let ping = {
        start: () => {
            setInterval(() => __awaiter(void 0, void 0, void 0, function* () {
                try {
                    yield keepAlive();
                }
                catch (e) {
                    console.error('Unable to keep Neutralino server online. The server is not reachable.');
                }
            }), PING_INTERVAL_MS);
        }
    };

    let devClient = {
        start: function () {
            setInterval(() => __awaiter(this, void 0, void 0, function* () {
                try {
                    let response = yield request({
                        url: 'http://localhost:5050',
                        type: RequestType.GET
                    });
                    if (response.needsReload) {
                        location.reload();
                    }
                }
                catch (e) {
                    console.error('Unable to communicate with neu devServer');
                }
            }), 1000);
        }
    };

    var version = "1.5.0";

    function init() {
        if (window.NL_MODE && window.NL_MODE == 'browser')
            ping.start();
        if (typeof window.NL_ARGS != 'undefined') {
            for (let i = 0; i < window.NL_ARGS.length; i++) {
                if (window.NL_ARGS[i] == '--debug-mode') {
                    devClient.start();
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
