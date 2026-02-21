export declare enum LoggerType {
	WARNING = "WARNING",
	ERROR = "ERROR",
	INFO = "INFO"
}
export declare enum Icon {
	WARNING = "WARNING",
	ERROR = "ERROR",
	INFO = "INFO",
	QUESTION = "QUESTION"
}
export declare enum MessageBoxChoice {
	OK = "OK",
	OK_CANCEL = "OK_CANCEL",
	YES_NO = "YES_NO",
	YES_NO_CANCEL = "YES_NO_CANCEL",
	RETRY_CANCEL = "RETRY_CANCEL",
	ABORT_RETRY_IGNORE = "ABORT_RETRY_IGNORE"
}
export declare enum ClipboardFormat {
	unknown = "unknown",
	text = "text",
	image = "image"
}
export declare enum Mode {
	window = "window",
	browser = "browser",
	cloud = "cloud",
	chrome = "chrome"
}
export declare enum OperatingSystem {
	Linux = "Linux",
	Windows = "Windows",
	Darwin = "Darwin",
	FreeBSD = "FreeBSD",
	Unknown = "Unknown"
}
export declare enum Architecture {
	x64 = "x64",
	arm = "arm",
	itanium = "itanium",
	ia32 = "ia32",
	unknown = "unknown"
}
export interface DirectoryEntry {
	entry: string;
	path: string;
	type: string;
}
export interface FileReaderOptions {
	pos: number;
	size: number;
}
export interface DirectoryReaderOptions {
	recursive: boolean;
}
export interface OpenedFile {
	id: number;
	eof: boolean;
	pos: number;
	lastRead: number;
}
export interface Stats {
	size: number;
	isFile: boolean;
	isDirectory: boolean;
	createdAt: number;
	modifiedAt: number;
}
export interface Watcher {
	id: number;
	path: string;
}
export interface CopyOptions {
	recursive: boolean;
	overwrite: boolean;
	skip: boolean;
}
export interface PathParts {
	rootName: string;
	rootDirectory: string;
	rootPath: string;
	relativePath: string;
	parentPath: string;
	filename: string;
	stem: string;
	extension: string;
}
interface Permissions$1 {
	all: boolean;
	ownerAll: boolean;
	ownerRead: boolean;
	ownerWrite: boolean;
	ownerExec: boolean;
	groupAll: boolean;
	groupRead: boolean;
	groupWrite: boolean;
	groupExec: boolean;
	othersAll: boolean;
	othersRead: boolean;
	othersWrite: boolean;
	othersExec: boolean;
}
export type PermissionsMode = "ADD" | "REPLACE" | "REMOVE";
declare function createDirectory(path: string): Promise<void>;
declare function remove(path: string): Promise<void>;
declare function writeFile(path: string, data: string): Promise<void>;
declare function appendFile(path: string, data: string): Promise<void>;
declare function writeBinaryFile(path: string, data: ArrayBuffer): Promise<void>;
declare function appendBinaryFile(path: string, data: ArrayBuffer): Promise<void>;
declare function readFile(path: string, options?: FileReaderOptions): Promise<string>;
declare function readBinaryFile(path: string, options?: FileReaderOptions): Promise<ArrayBuffer>;
declare function openFile(path: string): Promise<number>;
declare function createWatcher(path: string): Promise<number>;
declare function removeWatcher(id: number): Promise<number>;
declare function getWatchers(): Promise<Watcher[]>;
declare function updateOpenedFile(id: number, event: string, data?: any): Promise<void>;
declare function getOpenedFileInfo(id: number): Promise<OpenedFile>;
declare function readDirectory(path: string, options?: DirectoryReaderOptions): Promise<DirectoryEntry[]>;
declare function copy(source: string, destination: string, options?: CopyOptions): Promise<void>;
declare function move(source: string, destination: string): Promise<void>;
declare function getStats(path: string): Promise<Stats>;
declare function getAbsolutePath(path: string): Promise<string>;
declare function getRelativePath(path: string, base?: string): Promise<string>;
declare function getPathParts(path: string): Promise<PathParts>;
declare function getPermissions(path: string): Promise<Permissions$1>;
declare function setPermissions(path: string, permissions: Permissions$1, mode: PermissionsMode): Promise<void>;
declare function getJoinedPath(...paths: string[]): Promise<string>;
declare function getNormalizedPath(path: string): Promise<string>;
declare function getUnnormalizedPath(path: string): Promise<string>;
export interface ExecCommandOptions {
	stdIn?: string;
	background?: boolean;
	cwd?: string;
}
export interface ExecCommandResult {
	pid: number;
	stdOut: string;
	stdErr: string;
	exitCode: number;
}
export interface SpawnedProcess {
	id: number;
	pid: number;
}
export interface SpawnedProcessOptions {
	cwd?: string;
	envs?: Record<string, string>;
}
export interface Envs {
	[key: string]: string;
}
export interface OpenDialogOptions {
	multiSelections?: boolean;
	filters?: Filter[];
	defaultPath?: string;
}
export interface FolderDialogOptions {
	defaultPath?: string;
}
export interface SaveDialogOptions {
	forceOverwrite?: boolean;
	filters?: Filter[];
	defaultPath?: string;
}
export interface Filter {
	name: string;
	extensions: string[];
}
export interface TrayOptions {
	icon: string;
	menuItems: TrayMenuItem[];
}
export interface TrayMenuItem {
	id?: string;
	text: string;
	isDisabled?: boolean;
	isChecked?: boolean;
}
export type KnownPath = "config" | "data" | "cache" | "documents" | "pictures" | "music" | "video" | "downloads" | "savedGames1" | "savedGames2" | "temp";
declare function execCommand(command: string, options?: ExecCommandOptions): Promise<ExecCommandResult>;
declare function spawnProcess(command: string, options?: SpawnedProcessOptions): Promise<SpawnedProcess>;
declare function updateSpawnedProcess(id: number, event: string, data?: any): Promise<void>;
declare function getSpawnedProcesses(): Promise<SpawnedProcess[]>;
declare function getEnv(key: string): Promise<string>;
declare function getEnvs(): Promise<Envs>;
declare function showOpenDialog(title?: string, options?: OpenDialogOptions): Promise<string[]>;
declare function showFolderDialog(title?: string, options?: FolderDialogOptions): Promise<string>;
declare function showSaveDialog(title?: string, options?: SaveDialogOptions): Promise<string>;
declare function showNotification(title: string, content: string, icon?: Icon): Promise<void>;
declare function showMessageBox(title: string, content: string, choice?: MessageBoxChoice, icon?: Icon): Promise<string>;
declare function setTray(options: TrayOptions): Promise<void>;
declare function open$1(url: string): Promise<void>;
declare function getPath(name: KnownPath): Promise<string>;
export interface MemoryInfo {
	physical: {
		total: number;
		available: number;
	};
	virtual: {
		total: number;
		available: number;
	};
}
export interface KernelInfo {
	variant: string;
	version: string;
}
export interface OSInfo {
	name: string;
	description: string;
	version: string;
}
export interface CPUInfo {
	vendor: string;
	model: string;
	frequency: number;
	architecture: string;
	logicalThreads: number;
	physicalCores: number;
	physicalUnits: number;
}
export interface Display {
	id: number;
	resolution: Resolution;
	dpi: number;
	bpp: number;
	refreshRate: number;
}
export interface Resolution {
	width: number;
	height: number;
}
export interface MousePosition {
	x: number;
	y: number;
}
declare function getMemoryInfo(): Promise<MemoryInfo>;
declare function getArch(): Promise<string>;
declare function getKernelInfo(): Promise<KernelInfo>;
declare function getOSInfo(): Promise<OSInfo>;
declare function getCPUInfo(): Promise<CPUInfo>;
declare function getDisplays(): Promise<Display[]>;
declare function getMousePosition(): Promise<MousePosition>;
declare function setData(key: string, data: string | null): Promise<void>;
declare function getData(key: string): Promise<string>;
declare function removeData(key: string): Promise<void>;
declare function getKeys(): Promise<string[]>;
declare function clear(): Promise<void>;
declare function log(message: string, type?: LoggerType): Promise<void>;
export interface OpenActionOptions {
	url: string;
}
export interface RestartOptions {
	args: string;
}
declare function exit(code?: number): Promise<void>;
declare function killProcess(): Promise<void>;
declare function restartProcess(options?: RestartOptions): Promise<void>;
declare function getConfig(): Promise<any>;
declare function broadcast(event: string, data?: any): Promise<void>;
declare function readProcessInput(readAll?: boolean): Promise<string>;
declare function writeProcessOutput(data: string): Promise<void>;
declare function writeProcessError(data: string): Promise<void>;
export interface WindowOptions extends WindowSizeOptions, WindowPosOptions {
	title?: string;
	icon?: string;
	fullScreen?: boolean;
	alwaysOnTop?: boolean;
	enableInspector?: boolean;
	borderless?: boolean;
	maximize?: boolean;
	hidden?: boolean;
	maximizable?: boolean;
	useSavedState?: boolean;
	exitProcessOnClose?: boolean;
	extendUserAgentWith?: string;
	injectGlobals?: boolean;
	injectClientLibrary?: boolean;
	injectScript?: string;
	processArgs?: string;
}
export interface WindowSizeOptions {
	width?: number;
	height?: number;
	minWidth?: number;
	minHeight?: number;
	maxWidth?: number;
	maxHeight?: number;
	resizable?: boolean;
}
export interface WindowPosOptions {
	x?: number;
	y?: number;
	center?: boolean;
}
export interface WindowMenu extends Array<WindowMenuItem> {
}
export interface WindowMenuItem {
	id?: string;
	text: string;
	action?: string;
	shortcut?: string;
	isDisabled?: boolean;
	isChecked?: boolean;
	menuItems?: WindowMenuItem[];
}
declare function setTitle(title: string): Promise<void>;
declare function getTitle(): Promise<string>;
declare function maximize(): Promise<void>;
declare function unmaximize(): Promise<void>;
declare function isMaximized(): Promise<boolean>;
declare function minimize(): Promise<void>;
declare function unminimize(): Promise<void>;
declare function isMinimized(): Promise<boolean>;
declare function setFullScreen(): Promise<void>;
declare function exitFullScreen(): Promise<void>;
declare function isFullScreen(): Promise<boolean>;
declare function show(): Promise<void>;
declare function hide(): Promise<void>;
declare function isVisible(): Promise<boolean>;
declare function focus$1(): Promise<void>;
declare function setIcon(icon: string): Promise<void>;
declare function move$1(x: number, y: number): Promise<void>;
declare function center(): Promise<void>;
declare function beginDrag(screenX?: number, screenY?: number): Promise<void>;
declare function setDraggableRegion(DOMElementOrId: string | HTMLElement, options?: {
	exclude?: Array<string | HTMLElement>;
}): Promise<{
	success: true;
	message: string;
	exclusions: {
		add(elements: Array<string | HTMLElement>): void;
		remove(elements: Array<string | HTMLElement>): void;
		removeAll(): void;
	};
}>;
declare function unsetDraggableRegion(DOMElementOrId: string | HTMLElement): Promise<{
	success: true;
	message: string;
}>;
declare function setSize(options: WindowSizeOptions): Promise<void>;
declare function getSize(): Promise<WindowSizeOptions>;
declare function getPosition(): Promise<WindowPosOptions>;
declare function setAlwaysOnTop(onTop: boolean): Promise<void>;
declare function setBorderless(borderless: boolean): Promise<void>;
declare function create(url: string, options?: WindowOptions): Promise<void>;
declare function snapshot(path: string): Promise<void>;
declare function setMainMenu(options: WindowMenu): Promise<void>;
declare function print$1(): Promise<void>;
interface Response$1 {
	success: boolean;
	message: string;
}
export type Builtin = "ready" | "trayMenuItemClicked" | "windowClose" | "serverOffline" | "clientConnect" | "clientDisconnect" | "appClientConnect" | "appClientDisconnect" | "extClientConnect" | "extClientDisconnect" | "extensionReady" | "neuDev_reloadApp";
declare function on(event: string, handler: (ev: CustomEvent) => void): Promise<Response$1>;
declare function off(event: string, handler: (ev: CustomEvent) => void): Promise<Response$1>;
declare function dispatch(event: string, data?: any): Promise<Response$1>;
declare function broadcast$1(event: string, data?: any): Promise<void>;
export interface ExtensionStats {
	loaded: string[];
	connected: string[];
}
declare function dispatch$1(extensionId: string, event: string, data?: any): Promise<void>;
declare function broadcast$2(event: string, data?: any): Promise<void>;
declare function getStats$1(): Promise<ExtensionStats>;
export interface Manifest {
	applicationId: string;
	version: string;
	resourcesURL: string;
}
declare function checkForUpdates(url: string): Promise<Manifest>;
declare function install(): Promise<void>;
export interface ClipboardImage {
	width: number;
	height: number;
	bpp: number;
	bpr: number;
	redMask: number;
	greenMask: number;
	blueMask: number;
	redShift: number;
	greenShift: number;
	blueShift: number;
	data: ArrayBuffer;
}
declare function getFormat(): Promise<ClipboardFormat>;
declare function readText(): Promise<string>;
declare function readImage(format?: string): Promise<ClipboardImage | null>;
declare function writeText(data: string): Promise<void>;
declare function writeImage(image: ClipboardImage): Promise<void>;
declare function readHTML(): Promise<string>;
declare function writeHTML(data: string): Promise<void>;
declare function clear$1(): Promise<void>;
interface Stats$1 {
	size: number;
	isFile: boolean;
	isDirectory: boolean;
}
declare function getFiles(): Promise<string[]>;
declare function getStats$2(path: string): Promise<Stats$1>;
declare function extractFile(path: string, destination: string): Promise<void>;
declare function extractDirectory(path: string, destination: string): Promise<void>;
declare function readFile$1(path: string): Promise<string>;
declare function readBinaryFile$1(path: string): Promise<ArrayBuffer>;
declare function mount(path: string, target: string): Promise<void>;
declare function unmount(path: string): Promise<void>;
declare function getMounts(): Promise<Record<string, string>>;
declare function getMethods(): Promise<string[]>;
export interface InitOptions {
	exportCustomMethods?: boolean;
}
export declare function init(options?: InitOptions): void;
export type ErrorCode = "NE_FS_DIRCRER" | "NE_FS_RMDIRER" | "NE_FS_FILRDER" | "NE_FS_FILWRER" | "NE_FS_FILRMER" | "NE_FS_NOPATHE" | "NE_FS_COPYFER" | "NE_FS_MOVEFER" | "NE_OS_INVMSGA" | "NE_OS_INVKNPT" | "NE_ST_INVSTKY" | "NE_ST_STKEYWE" | "NE_RT_INVTOKN" | "NE_RT_NATPRME" | "NE_RT_APIPRME" | "NE_RT_NATRTER" | "NE_RT_NATNTIM" | "NE_CL_NSEROFF" | "NE_EX_EXTNOTC" | "NE_UP_CUPDMER" | "NE_UP_CUPDERR" | "NE_UP_UPDNOUF" | "NE_UP_UPDINER";
interface Error$1 {
	code: ErrorCode;
	message: string;
}
declare global {
	interface Window {
		/** Mode of the application: window, browser, cloud, or chrome */
		NL_MODE: Mode;
		/** Application port */
		NL_PORT: number;
		/** Command-line arguments */
		NL_ARGS: string[];
		/** Basic authentication token */
		NL_TOKEN: string;
		/** Neutralinojs client version */
		NL_CVERSION: string;
		/** Application identifier */
		NL_APPID: string;
		/** Application version */
		NL_APPVERSION: string;
		/** Application path */
		NL_PATH: string;
		/** Application data path */
		NL_DATAPATH: string;
		/** Returns true if extensions are enabled */
		NL_EXTENABLED: boolean;
		/** Returns true if the client library is injected */
		NL_GINJECTED: boolean;
		/** Returns true if globals are injected */
		NL_CINJECTED: boolean;
		/** Operating system name: Linux, Windows, Darwin, FreeBSD, or Uknown */
		NL_OS: OperatingSystem;
		/** CPU architecture: x64, arm, itanium, ia32, or unknown */
		NL_ARCH: Architecture;
		/** Neutralinojs server version */
		NL_VERSION: string;
		/** Current working directory */
		NL_CWD: string;
		/** Identifier of the current process */
		NL_PID: string;
		/** Source of application resources: bundle or directory */
		NL_RESMODE: string;
		/** Release commit of the client library */
		NL_CCOMMIT: string;
		/** An array of custom methods */
		NL_CMETHODS: string[];
	}
	/** Neutralino global object for custom methods **/
	const Neutralino: any;
}

declare namespace custom {
	export { getMethods };
}
declare namespace filesystem {
	export { appendBinaryFile, appendFile, copy, createDirectory, createWatcher, getAbsolutePath, getJoinedPath, getNormalizedPath, getOpenedFileInfo, getPathParts, getPermissions, getRelativePath, getStats, getUnnormalizedPath, getWatchers, move, openFile, readBinaryFile, readDirectory, readFile, remove, removeWatcher, setPermissions, updateOpenedFile, writeBinaryFile, writeFile };
}
declare namespace os {
	export { execCommand, getEnv, getEnvs, getPath, getSpawnedProcesses, open$1 as open, setTray, showFolderDialog, showMessageBox, showNotification, showOpenDialog, showSaveDialog, spawnProcess, updateSpawnedProcess };
}
declare namespace computer {
	export { getArch, getCPUInfo, getDisplays, getKernelInfo, getMemoryInfo, getMousePosition, getOSInfo };
}
declare namespace storage {
	export { clear, getData, getKeys, removeData, setData };
}
declare namespace debug {
	export { log };
}
declare namespace app {
	export { broadcast, exit, getConfig, killProcess, readProcessInput, restartProcess, writeProcessError, writeProcessOutput };
}
declare namespace window$1 {
	export { beginDrag, center, create, exitFullScreen, focus$1 as focus, getPosition, getSize, getTitle, hide, isFullScreen, isMaximized, isMinimized, isVisible, maximize, minimize, move$1 as move, print$1 as print, setAlwaysOnTop, setBorderless, setDraggableRegion, setFullScreen, setIcon, setMainMenu, setSize, setTitle, show, snapshot, unmaximize, unminimize, unsetDraggableRegion };
}
declare namespace events {
	export { broadcast$1 as broadcast, dispatch, off, on };
}
declare namespace extensions {
	export { broadcast$2 as broadcast, dispatch$1 as dispatch, getStats$1 as getStats };
}
declare namespace updater {
	export { checkForUpdates, install };
}
declare namespace clipboard {
	export { clear$1 as clear, getFormat, readHTML, readImage, readText, writeHTML, writeImage, writeText };
}
declare namespace resources {
	export { extractDirectory, extractFile, getFiles, getStats$2 as getStats, readBinaryFile$1 as readBinaryFile, readFile$1 as readFile };
}
declare namespace server {
	export { getMounts, mount, unmount };
}

export {
	Error$1 as Error,
	Permissions$1 as Permissions,
	Response$1 as Response,
	app,
	clipboard,
	computer,
	custom,
	debug,
	events,
	extensions,
	filesystem,
	os,
	resources,
	server,
	storage,
	updater,
	window$1 as window,
};

export as namespace Neutralino;

export {};
