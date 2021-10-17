<div align="center">
  <img src="https://cdn.rawgit.com/neutralinojs/neutralinojs.github.io/b667f2c2/docs/nllogo.png" style="width:300px;"/>
</div>

[![GitHub (pre-)release](https://img.shields.io/github/release/neutralinojs/neutralinojs/all.svg)](https://github.com/neutralinojs/neutralinojs/releases)
[![GitHub last commit](https://img.shields.io/github/last-commit/neutralinojs/neutralinojs.svg)](https://github.com/neutralinojs/neutralinojs/commits/main)
[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2Fneutralinojs%2Fneutralinojs.svg?type=shield)](https://app.fossa.com/projects/git%2Bgithub.com%2Fneutralinojs%2Fneutralinojs?ref=badge_shield)
[![Twitter](https://img.shields.io/twitter/url/https/github.com/neutralinojs/neutralinojs.svg?style=social)](https://twitter.com/intent/tweet?text=NeutralinoJs%20is%20a%20portable%20and%20lightweight%20framework%20which%20lets%20you%20to%20develop%20apps%20with%20native%20functions%20that%20can%20run%20inside%20web%20browsers.%20Check%20it%20out:&url=https%3A%2F%2Fgithub.com%2Fneutralinojs%2Fneutralinojs)
[![OpenCollective](https://opencollective.com/neutralinojs/backers/badge.svg)](#backers) 
<a href="https://opencollective.com/neutralinojs"><img src="https://img.shields.io/badge/Support%20us-Open%20Collective-41B883.svg" alt="Support us"></a>

Neutralinojs is a lightweight and portable desktop application development framework. It lets you develop lightweight cross-platform desktop applications using JavaScript, HTML and CSS. Apps built with Neutralinojs can run on Linux, macOS, Windows, and Web.

- [Neutralinojs vs Electron vs NW.JS vs Tauri vs NodeGui vs Flutter vs .Net MAUI](https://github.com/Elanis/web-to-desktop-framework-comparison)
- [Neutralinojs vs Electron vs NW.js (2018)](https://github.com/neutralinojs/evaluation)
- [Roadmap for 2021](https://github.com/neutralinojs/roadmap#roadmapplan-2021)

Get started with the neu CLI.

```bash
 # Creating a new app
 $ npm i -g @neutralinojs/neu
 $ neu create hello-world
 $ cd hello-world
 $ neu run
 
 # Building your app
 $ neu build
```

This documentation is for contributors. If you would like to learn Neutralinojs application development, please check [this](https://neutralino.js.org/docs) documentation.
 
## Why Neutralinojs? 

In Electron and NWjs, you have to install NodeJs and hundreds of dependency libraries. Embedded Chromium and Node make simple apps bloaty. Neutralinojs offers a lightweight and portable SDK which is an alternative for Electron and NW.js. Neutralinojs doesn't bundle Chromium and uses the existing web browser library in the operating system (Eg: gtk-webkit2 on Linux). Neutralinojs implements a WebSocket connection for native operations and embeds a static web server to serve the web content. Also, it offers a built-in [JavaScript client library](https://github.com/neutralinojs/neutralino.js) for developers.

Ask questions on StackOverflow using tag [neutralinojs](https://stackoverflow.com/questions/tagged/neutralinojs)

## Contribution

- Select existing [issues](https://github.com/neutralinojs/neutralinojs/issues) or create a [new issue](https://github.com/neutralinojs/neutralinojs/issues/new). Send us a pull request with your code changes. We are happy to review it ❤️
- Refer to [this guide](https://github.com/neutralinojs/neutralinojs/blob/master/CONTRIBUTING.md) for contribution guidelines.

### Collaboration and communication

- We use Slack and Discord for quick discussions. 
  * [Join on Slack](https://join.slack.com/t/neutralinojs/shared_invite/zt-b7mbivj5-pKpO6U5drmeT68vKD_pc6w)
  * [Join on Discord](https://discord.gg/cybpp4guTJ)
- Join our [Google group](https://groups.google.com/forum/#!forum/neutralinojs) for feature proposals and long discussions.

## Framework Developer Guide

### Setup and compilation

**Linux**

No need for separate compilers because Linux distributions usually have GNU C/C++ compilers installed already.

Install GTk and webkit libraries

**Ubuntu**

If you are using a newer version of Ubuntu, some libraries may be pre-installed.

```bash
sudo apt-get install \ 
    libboost-all-dev \
    libgtk-3-dev \ 
    libwebkit2gtk-4.0-37 \ 
    libwebkit2gtk-4.0-dev \
    libappindicator3-dev
```

**Fedora**

```bash
sudo dnf install \
    @development-tools \
    libappindicator-gtk3.x86_64 \
    gtk3 \
    webkit2gtk3.x86_64 \
    webkit2gtk3-devel.x86_64
```

Compiling Neutralino server on Linux

```bash
bash build_linux.sh # [ia32, x64, armhf, arm64]
```

**Windows**

Install the latest Visual Studio IDE. Neutralinojs compilation on Windows will use MSVC (aka `cl.exe`) C++ compiler. Install Boost libraries via Chocolatey.

```bash
choco install boost-msvc-14.1
```

Compiling Neutralino server on Windows

```bash
./build_windows.bat # [ia32, x64]
```

**macOS**

Install Xcode Command Line Tools. Install Boost libraries via Homebrew.

```bash
brew install boost@1.76
```

Compiling Neutralino server on MacOS

```bash
bash build_macos.sh # [ia32, x64, arm64]
```

## Sponsors and Donators

Organizations and individuals support Neutralinojs. See: https://codezri.org/sponsors

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
- Logo design credits: [IconsPng](https://www.iconspng.com/image/2688/atom-orange). Copyright free as mentioned in their website. 

Please check all licenses from [this](LICENSE) link.


[![FOSSA Status](https://app.fossa.com/api/projects/git%2Bgithub.com%2Fneutralinojs%2Fneutralinojs.svg?type=large)](https://app.fossa.com/projects/git%2Bgithub.com%2Fneutralinojs%2Fneutralinojs?ref=badge_large)

## Contributors

<a href="https://github.com/neutralinojs/neutralinojs/graphs/contributors">
  <img src="https://contributors-img.firebaseapp.com/image?repo=neutralinojs/neutralinojs" />
</a>

The above image was generated with [contributors-img](https://contributors-img.firebaseapp.com).
