
<div align="center">
  <img src="https://cdn.rawgit.com/neutralinojs/neutralinojs.github.io/b667f2c2/docs/nllogo.png" style="width:300px;"/>
</div>

[![GitHub license](https://img.shields.io/github/license/neutralinojs/neutralinojs.svg)](https://github.com/neutralinojs/neutralinojs/blob/master/LICENSE)
[![HitCount](http://hits.dwyl.io/neutralinojs/neutralinojs.svg)](http://hits.dwyl.io/neutralinojs/neutralinojs)
[![Twitter](https://img.shields.io/twitter/url/https/github.com/neutralinojs/neutralinojs.svg?style=social)](https://twitter.com/intent/tweet?text=Wow:&url=https%3A%2F%2Fgithub.com%2Fneutralinojs%2Fneutralinojs)



NeutralinoJs is a portable and lightweight framework to develop apps with native functions and it allows you to run developed app inside web browsers. 

In electron and NWjs you have to install NodeJs, hundreds of dependence libraries and also the bundled app size is large even we write `hello world` due to embedded chromium and Node runtime. Neutralino is lightweight solution for this issue.

Interested in NeutralinoJs idea? Get started with our [docs](https://neutralinojs.github.io/docs/#/) and build your first [Neutralino App](https://neutralinojs.github.io/docs/#/gettingstarted/firstapp) 

### How to Develop Neutralino apps

- Download latest Neutralino lightweight SDK from [releases](https://github.com/neutralinojs/neutralinojs/releases)
- Write code for your application
- Double click on neutralino binary, it will run your app on default browser
- Close the browser tab (Neutralino runtime will be terminated automatically)

### Deploying your app

- zip the content and share your app anywhere. it's fully portable

### Neutralinojs can be used for

- Web apps with native functions
- Lightweight cross platform apps

### Platform Architecture

<div align="center">
  <img src="https://rawgit.com/neutralinojs/neutralinojs.github.io/master/docs/architecture.png">
</div>

## Contribution

Help Neutralino!.

- Give us a star 😋
- Fork and Clone! Awesome
- Select existing [issues](https://github.com/neutralinojs/neutralinojs/issues) or create a [new issue](https://github.com/neutralinojs/neutralinojs/issues/new) thereafter give us a PR with your bugfix or improvement. We love it ❤️ 
- We use Slack for discussions. [get your invite](https://join.slack.com/t/neutralinojs/shared_invite/enQtMzk0MDU5ODMyNzM4LTc1ZjJmMzFjNjEzNjk2ODkyYWJiMTAxY2Q2OTA0MGYxNTNiMWFhMjAxMjc1M2E2NGI2OTM1ZjA1ZWNjZDFmZGU)
- Join our [Google group](https://groups.google.com/forum/#!forum/neutralinojs) for feature proposals and requests.

## Developer Guide

### Required tools 

To create Neutralino developer environment in your pc, install these tool kits.

- [MinGW Compiler for C++](http://mingw.org/)
- [NodeJs](https://nodejs.org/en/download/)
- [Webpack Bundler](https://webpack.js.org/)

### Build Neutralino platform in Windows

```bash
> build
```

### Build Neutralino platform in Linux

```bash
$ bash build.sh
```
Neutralino platform components will be assembled in to `/dist`

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

Neutralino server will be compiled in to `/bin`

### Build Neutralino.js with webpack

```bash
$ cd neutralino.js
$ npm install
$ npm run build
```

## Development Milestones

- [x] Solution Design
- [x] Windows Server 
- [x] Linux Server
- [x] Javascript SDK
- [x] Documentation
- [x] Web site
- [ ] Security improvements
- [ ] Other milestones as per planed
- [x] Experimental Release
- [ ] Stable Release






