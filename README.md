<div align="center">
  <img src="https://cdn.rawgit.com/neutralinojs/neutralinojs.github.io/b667f2c2/docs/nllogo.png" style="width:300px;"/>
</div>

[![GitHub (pre-)release](https://img.shields.io/github/release/neutralinojs/neutralinojs/all.svg)](https://github.com/neutralinojs/neutralinojs/releases)
[![GitHub last commit](https://img.shields.io/github/last-commit/neutralinojs/neutralinojs.svg)](https://github.com/neutralinojs/neutralinojs/commits/master)
[![Twitter](https://img.shields.io/twitter/url/https/github.com/neutralinojs/neutralinojs.svg?style=social)](https://twitter.com/intent/tweet?text=NeutralinoJs%20is%20a%20portable%20and%20lightweight%20framework%20which%20lets%20you%20to%20develop%20apps%20with%20native%20functions%20that%20can%20run%20inside%20web%20browsers.%20Check%20it%20out:&url=https%3A%2F%2Fgithub.com%2Fneutralinojs%2Fneutralinojs)
[![OpenCollective](https://opencollective.com/neutralinojs/backers/badge.svg)](#backers) 
<a href="https://opencollective.com/neutralinojs"><img src="https://img.shields.io/badge/Support%20us-Open%20Collective-41B883.svg" alt="Support us"></a>

Neutralino is a lightweight and portable desktop application development framework. It lets you develop cross-platform desktop applications using JavaScript, HTML and CSS.

[Neutralinojs vs Electron vs NW.js](https://github.com/neutralinojs/evaluation)

Get started with CLI.

```bash
 $ npm i -g @neutralinojs/neu
 $ neu create hello-world
 $ cd hello-world
 $ neu run
```

This documentation is for contributors. If you would like to learn Neutralinojs application development, please check [this](https://neutralino.js.org/docs) documentation.
 
## Why Neutralinojs? 

In Electron and NWjs, you have to install NodeJs and hundreds of dependency libraries. Embedded Chromium and Node make simple apps bloaty. Neutralinojs offers a lightweight and portable SDK which is an alternative for Electron and NW.js also with many advantages.

Ask questions on StackOverflow using tag [neutralinojs](https://stackoverflow.com/questions/tagged/neutralinojs)

## Contribution

- Select existing [issues](https://github.com/neutralinojs/neutralinojs/issues) or create a [new issue](https://github.com/neutralinojs/neutralinojs/issues/new). Send us a pull request with your code changes. We are happy to review it ❤️
- Refer to [this guide](https://github.com/neutralinojs/neutralinojs/blob/master/CONTRIBUTING.md) for contribution guidelines.

### Collaboration and communication

- We use Slack for quick discussions. [get your invite](https://join.slack.com/t/neutralinojs/shared_invite/zt-b7mbivj5-pKpO6U5drmeT68vKD_pc6w)
- Join our [Google group](https://groups.google.com/forum/#!forum/neutralinojs) for feature proposals and long discussions.

## Framework Developer Guide

### Setup and compilation

**Linux**

No need for separate compilers because Linux distributions usually have GNU C/C++ compilers installed already.

Install GTk and webkit libraries

If you are using a newer version of Ubuntu, there is no need to run the following commands.

```bash
$ sudo apt-get install libgtk-3-dev
$ sudo add-apt-repository ppa:webkit-team/ppa
$ sudo apt-get update
$ sudo apt-get install libwebkit2gtk-4.0-37 libwebkit2gtk-4.0-dev
```

Compiling Neutralino server on Linux

```bash
$ cd core-linux
$ bash build.sh
```

**Windows**

Install the latest Visual Studio IDE. Neutralinojs compilation on Windows will use MSVC (aka `cl.exe`) C++ compiler. 

Compiling Neutralino server on Windows

```bash
> cd core-windows
> build
```

**macOS**

Install Xcode Command Line Tools.

Compiling Neutralino server on MacOS

```bash
$ cd core-macos
$ bash build.sh
```

## Sponsors 

[![MacStadium](https://assets-global.website-files.com/5ac3c046c82724970fc60918/5ac3c046c82724204dc6094e_macstadium-logo-white.png)](https://www.macstadium.com)

MacStadium offered us a free remotely-accessed physical mac-mini for Neutralinojs-mac development.

The following members are supporting us financially on [Patreon](https://www.patreon.com/shalithasuranga).

- Just Epic
- Brian McGonagill
- Brian
- Mortarwork Studios

If you like to support our work, you can donate to Neutralinojs via [Patreon](https://www.patreon.com/shalithasuranga).

[![Become a Patreon](https://c5.patreon.com/external/logo/become_a_patron_button.png)](https://www.patreon.com/shalithasuranga)

## Licenses and Copyrights

- Neutralinojs core: GPL-3.0
- HTTP server on Linux and macOS: GPL-3.0 from [yibo141/Servant](https://github.com/yibo141/Servant). Copyright (c) Broglie <yibo141@outlook.com>.
- HTTP server on Windows: MIT from [hckr/SimpleHTTP](https://github.com/hckr/SimpleHTTP). Copyright (c) Jakub Młokosiewicz.
- JSON parser library: MIT from [nlohmann/json](https://github.com/nlohmann/json). Copyright (c) Niels Lohmann.
- Cross-platform webview library: MIT from [webview/webview](https://github.com/webview/webview). Copyright (c) Serge Zaitsev. 

Please check all licenses from [this](LICENSE) link.

## Contributors

<a href="https://github.com/neutralinojs/neutralinojs/graphs/contributors">
  <img src="https://contributors-img.firebaseapp.com/image?repo=neutralinojs/neutralinojs" />
</a>

The above contribution image was generated with [contributors-img](https://contributors-img.firebaseapp.com).
