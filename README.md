
<div align="center">
  <img src="https://cdn.rawgit.com/neutralinojs/neutralinojs.github.io/b667f2c2/docs/nllogo.png" style="width:300px;"/>
</div>

[![GitHub license](https://img.shields.io/github/license/neutralinojs/neutralinojs.svg)](https://github.com/neutralinojs/neutralinojs/blob/master/LICENSE)
 [![GitHub (pre-)release](https://img.shields.io/github/release/neutralinojs/neutralinojs/all.svg)](https://github.com/neutralinojs/neutralinojs/releases)
  [![GitHub last commit](https://img.shields.io/github/last-commit/neutralinojs/neutralinojs.svg)](https://github.com/neutralinojs/neutralinojs/commits/master)
[![HitCount](http://hits.dwyl.io/neutralinojs/neutralinojs.svg)](http://hits.dwyl.io/neutralinojs/neutralinojs)
[![Twitter](https://img.shields.io/twitter/url/https/github.com/neutralinojs/neutralinojs.svg?style=social)](https://twitter.com/intent/tweet?text=NeutralinoJs%20is%20a%20portable%20and%20lightweight%20framework%20which%20lets%20you%20to%20develop%20apps%20with%20native%20functions%20that%20can%20run%20inside%20web%20browsers.%20Check%20it%20out:&url=https%3A%2F%2Fgithub.com%2Fneutralinojs%2Fneutralinojs)
[![OpenCollective](https://opencollective.com/neutralinojs/backers/badge.svg)](#backers) 
<a href="https://opencollective.com/neutralinojs"><img src="https://img.shields.io/badge/Support%20us-Open%20Collective-41B883.svg" alt="Support us"></a>

Neutralino is a lightweight and portable application development framework. It lets you develop cross-platform applications using JavaScript, HTML and CSS.

[Neutralinojs vs Electron vs NW.js](https://github.com/neutralinojs/evaluation)


<div align="center">
  <img src="media/demo_njs.gif"/>
</div>
 
## Why Neutralinojs? 

In Electron and NWjs you have to install NodeJs and hundreds of dependency libraries. Embedded Chromium and Node creates large overhead and makes even simple apps like “hello world” considerable in size. Neutralino offers a lightweight and portable SDK which is an alternative for Electron and NW.js also with many advantages.

Ask questions on Stackoverflow using tag [neutralinojs](https://stackoverflow.com/questions/tagged/neutralinojs)

See how it [works](https://www.99xtechnology.com/blog/research/neutralinojs-internals/)

### How it works?

<div align="center">
  <img src="media/architecture.png">
</div>

## Contribution

Help Neutralino!

- Give us a star :star:
- Fork and Clone! Awesome
- Select existing [issues](https://github.com/neutralinojs/neutralinojs/issues) or create a [new issue](https://github.com/neutralinojs/neutralinojs/issues/new) and give us a PR with your bugfix or improvement after. We love it ❤️
- Refer to this [guide](https://github.com/neutralinojs/neutralinojs/blob/master/CONTRIBUTING.md) for Contribution Procedures and Standards.

### Neutralinojs cloud

This experimental mode can be used to expose native functions for clients

```bash
./neutralino  /dev/null 2>&1 &
```
Check this [article](https://medium.com/@shalithasuranga/getting-started-with-neutralinojs-cloud-preview-version-aws-ec2-6e618d8a2ddb)

### Realtime Chat Room - Slack

- We use Slack for quick discussions. [get your invite](https://join.slack.com/t/neutralinojs/shared_invite/enQtMzk0MDU5ODMyNzM4LTc1ZjJmMzFjNjEzNjk2ODkyYWJiMTAxY2Q2OTA0MGYxNTNiMWFhMjAxMjc1M2E2NGI2OTM1ZjA1ZWNjZDFmZGU)

### Mailing List - Google Group

- Join our [Google group](https://groups.google.com/forum/#!forum/neutralinojs) for feature proposals and requests.
- You can help us by writing sample Neutralino apps,by composing or improving documentation or by writing code.

## Developer Guide

### Required tools 

To create Neutralino developer environment in your pc, install these tool kits.

- [MinGW Compiler for C++](http://mingw.org/)
- [NodeJs](https://nodejs.org/en/download/)
- [Webpack Bundler](https://webpack.js.org/)
- [WebKitGTK+](https://webkitgtk.org/)

### Build Neutralino platform in Windows

```bash
> build
```

### Build Neutralino platform in Linux

Install GTk and webkit libraries

```bash
$ sudo apt-get install libgtk-3-dev
$ sudo add-apt-repository ppa:webkit-team/ppa
$ sudo apt-get update
$ sudo apt-get install libwebkit2gtk-4.0-37 libwebkit2gtk-4.0-dev
```

```bash
$ bash build.sh
```
Neutralino platform components will be assembled in `/dist`

### Build Neutralino server in Windows

```bash
> cd core-windows
> build
```

### Build Neutralino server in Linux

```bash
$ cd core-linux
$ bash build.sh
```

Neutralino server will be compiled in `/bin`

### MacOS Build

Neutralino now supports MacOSX

#### Build Requirenments

- Xcode Command Line Tools
- CMake 3.15+

#### Debug Builds

Debug builds are unoptimized fat binaries containing debug information. This is the type you should use for any debugging purpose.

- `mkdir build`
- `cd build`
- `cmake ..`
- `make -j 4`

The resulting binary will be placed in **build** directory

#### Release Builds

Release builds are optimized binaries with a very small footprint. Suitable for distributions

- `mkdir release`
- `cd release`
- `cmake .. -DCMAKE_BUILD_TYPE=Release`
- `make -j 4`

The resulting binary will be placed in **release** directory

To install resulting binary to `bin` directory use

- `make install`


### Build Neutralino.js with webpack

```bash
$ cd neutralino.js
$ npm install
$ npm run build
```

## Become a Backer

If you like to contribute by funding for sustaining our work, Please visit https://opencollective.com/neutralinojs

[![Become a Patron](https://c5.patreon.com/external/logo/become_a_patron_button.png)](https://www.patreon.com/shalithasuranga)

## License

[MIT](LICENSE)

## Contributors

<a href="https://github.com/neutralinojs/neutralinojs/graphs/contributors">
  <img src="https://contributors-img.firebaseapp.com/image?repo=neutralinojs/neutralinojs" />
</a>

Made with [contributors-img](https://contributors-img.firebaseapp.com).
