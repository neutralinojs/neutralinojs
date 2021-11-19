<div align="center">
  <img src="https://cdn.rawgit.com/neutralinojs/neutralinojs.github.io/b667f2c2/docs/nllogo.png" style="width:300px;"/>
</div>

[![GitHub (pre-)release](https://img.shields.io/github/release/neutralinojs/neutralinojs/all.svg)](https://github.com/neutralinojs/neutralinojs/releases)
[![GitHub last commit](https://img.shields.io/github/last-commit/neutralinojs/neutralinojs.svg)](https://github.com/neutralinojs/neutralinojs/commits/main)
![Build status](https://github.com/neutralinojs/neutralinojs/actions/workflows/test_suite.yml/badge.svg)
[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2Fneutralinojs%2Fneutralinojs.svg?type=shield)](https://app.fossa.com/projects/git%2Bgithub.com%2Fneutralinojs%2Fneutralinojs?ref=badge_shield)

Neutralinojs is a lightweight and portable desktop application development framework. It lets you develop lightweight cross-platform desktop applications using JavaScript, HTML and CSS. Apps built with Neutralinojs can run on Linux, macOS, Windows, and Web. Also, you can extend Neutralinojs with any programming language (with extensions) and use Neutralinojs as part of your source code (via child processes). 

- [Neutralinojs vs Electron vs NW.JS vs Tauri vs NodeGui vs Flutter vs .Net MAUI](https://github.com/Elanis/web-to-desktop-framework-comparison)
- [Neutralinojs vs Electron vs NW.js (2018)](https://github.com/neutralinojs/evaluation)
- [Roadmap for 2021](https://github.com/neutralinojs/roadmap#roadmapplan-2021)

Get started with the neu CLI.

```bash
 # Creating a new app
 npm i -g @neutralinojs/neu
 neu create hello-world
 cd hello-world
 neu run
 
 # Building your app (No compilation - takes less than a second)
 neu build
```

Start building apps: [neutralino.js.org/docs](https://neutralino.js.org/docs)
 
## Why Neutralinojs? 

In Electron and NWjs, you have to install NodeJs and hundreds of dependency libraries. Embedded Chromium and Node make simple apps bloaty. Neutralinojs offers a lightweight and portable SDK which is an alternative for Electron and NW.js. Neutralinojs doesn't bundle Chromium and uses the existing web browser library in the operating system (Eg: gtk-webkit2 on Linux). Neutralinojs implements a WebSocket connection for native operations and embeds a static web server to serve the web content. Also, it offers a built-in [JavaScript client library](https://github.com/neutralinojs/neutralino.js) for developers.

Ask questions on StackOverflow using tag [neutralinojs](https://stackoverflow.com/questions/tagged/neutralinojs)

## Contributing

Please check the [constribution guide](https://neutralino.js.org/docs/contributing/framework-developer-guide). We use GitHub Discussions, Slack, and Discord for quick discussions. 
  * [Start a thread on Discussions](https://github.com/neutralinojs/neutralinojs/discussions)
  * [Join on Slack](https://join.slack.com/t/neutralinojs/shared_invite/zt-b7mbivj5-pKpO6U5drmeT68vKD_pc6w)
  * [Join on Discord](https://discord.gg/cybpp4guTJ)

Neutralinojs contributors:

<a href="https://github.com/neutralinojs/neutralinojs/graphs/contributors">
  <img src="https://contributors-img.firebaseapp.com/image?repo=neutralinojs/neutralinojs" />
</a>

Image created with [contributors-img](https://contributors-img.firebaseapp.com).

## Sponsors and Donators

Organizations and individuals support Neutralinojs development. See: https://codezri.org/sponsors

If you like to support our work, you can donate to Neutralinojs via [Patreon](https://www.patreon.com/shalithasuranga).

[![Become a Patreon](https://c5.patreon.com/external/logo/become_a_patron_button.png)](https://www.patreon.com/shalithasuranga)

## Licenses and Copyrights

- Neutralinojs core: MIT. Copyright (c) 2021 Neutralinojs and contributors.
- C++ websocket client/server library: BSD-3-Clause from [zaphoyd/websocketpp](https://github.com/zaphoyd/websocketpp). Copyright (c) 2014, Peter Thorson. All rights reserved.
- JSON parser library: MIT from [nlohmann/json](https://github.com/nlohmann/json). Copyright (c) 2013-2021 Niels Lohmann.
- Cross-platform webview library: MIT from [webview/webview](https://github.com/webview/webview). Copyright (c) 2017 Serge Zaitsev. 
- Cross-platform tray library: MIT from [zserge/tray](https://github.com/zserge/tray). Copyright (c) 2017 Serge Zaitsev. 
- Cross-platform GUI dialogs library: WTFPL from [samhocevar/portable-file-dialogs](https://github.com/samhocevar/portable-file-dialogs). Copyright (c) 2018—2020 Sam Hocevar <sam@hocevar.net>
- Base64 encoder/decoder library: MIT from [tobiaslocker/base64](https://github.com/tobiaslocker/base64). Copyright (c) 2019 Tobias Locker.
- Cross-platform known platform directories API: MIT from [sago007/PlatformFolders](https://github.com/sago007/PlatformFolders). Copyright (c) 2015 Poul Sander.
- C++ logging library: MIT from [amrayn/easyloggingpp](https://github.com/amrayn/easyloggingpp). Copyright (c) 2012-2018 Amrayn Web Services. Copyright (c) 2012-2018 @abumusamq
- Cross-platform process library: MIT from [eidheim/tiny-process-library](https://gitlab.com/eidheim/tiny-process-library). Copyright (c) 2015-2020 Ole Christian Eidheim.
- Logo design credits: [IconsPng](https://www.iconspng.com/image/2688/atom-orange). Copyright free as mentioned in their website. 

[Complete license details](LICENSE)

[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2Fneutralinojs%2Fneutralinojs.svg?type=large)](https://app.fossa.com/projects/git%2Bgithub.com%2Fneutralinojs%2Fneutralinojs?ref=badge_large)

