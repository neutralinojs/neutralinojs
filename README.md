
<div align="center">
  <img src="https://cdn.rawgit.com/neutralinojs/neutralinojs.github.io/b667f2c2/docs/nllogo.png" style="width:300px;"/>
</div>

[![GitHub license](https://img.shields.io/github/license/neutralinojs/neutralinojs.svg)](https://github.com/neutralinojs/neutralinojs/blob/master/LICENSE)
[![HitCount](http://hits.dwyl.io/neutralinojs/neutralinojs.svg)](http://hits.dwyl.io/neutralinojs/neutralinojs)
[![Twitter](https://img.shields.io/twitter/url/https/github.com/neutralinojs/neutralinojs.svg?style=social)](https://twitter.com/intent/tweet?text=NeutralinoJs%20is%20a%20portable%20and%20lightweight%20framework%20which%20lets%20you%20to%20develop%20apps%20with%20native%20functions%20that%20can%20run%20inside%20web%20browsers.%20Check%20it%20out:&url=https%3A%2F%2Fgithub.com%2Fneutralinojs%2Fneutralinojs)

<h1><a href="hackathons/hacktoberfest.md">Hacktoberfest with Neutralinojs</a></h1>

NeutralinoJs is a portable and lightweight framework which lets you to develop apps with native functions that can run inside web browsers. 

In electron and NWjs you have to install NodeJs and hundreds of dependency libraries. Embedded Chromium and Node creates large overhead and makes even simple apps like “hello world” considerable in size. Neutralino offers a solution for this issue.

Interested in the NeutralinoJs idea? Start with our [docs](https://neutralino.js.org/docs/#/) and build your first [Neutralino App](https://neutralino.js.org/docs/#/gettingstarted/firstapp) 

Read the story of NeutralinoJs [here](https://medium.com/@shalithasuranga/why-we-built-neutralinojs-framework-part-i-1d6c667951d5)

Ask questions on Stackoverlow using tag [neutralinojs](https://stackoverflow.com/questions/tagged/neutralinojs)

### How to Develop Neutralino apps

- Download latest Neutralino lightweight SDK from [releases](https://github.com/neutralinojs/neutralinojs/releases)
- Write code for your application
- Double click on the neutralino binary, it will run the app on your default browser
- Close the browser tab (Neutralino runtime will terminate automatically)

### Deploying your app

- Zip the content and share anywhere. It's fully portable

### Neutralinojs can be used for

- Web apps with native functions
- Lightweight cross-platform apps
- Control VMs, servers or computers in a private/public network - [NeutralinoJs cloud](https://medium.com/@shalithasuranga/getting-started-with-neutralinojs-cloud-preview-version-aws-ec2-6e618d8a2ddb)

### Platform Architecture

<div align="center">
  <img src="media/architecture.png">
</div>

## Contribution

Help Neutralino!

- Give us a ★ :+1:
- Fork and Clone! Awesome
- Select existing [issues](https://github.com/neutralinojs/neutralinojs/issues) or create a [new issue](https://github.com/neutralinojs/neutralinojs/issues/new) and give us a PR with your bugfix or improvement after. We love it :) ❤️
- Refer to this [link](https://github.com/neutralinojs/neutralinojs/blob/master/CONTRIBUTING.md) for Contribution Procedures and Standards.

### Realtime Chat Room - Slack

- We use Slack for quick discussions. [get your invite](https://join.slack.com/t/neutralinojs/shared_invite/enQtMzk0MDU5ODMyNzM4LTc1ZjJmMzFjNjEzNjk2ODkyYWJiMTAxY2Q2OTA0MGYxNTNiMWFhMjAxMjc1M2E2NGI2OTM1ZjA1ZWNjZDFmZGU)

### Mainling List - Google Group

- Join our [Google group](https://groups.google.com/forum/#!forum/neutralinojs) for feature proposals and requests.
- You can help us by writing sample neutralino apps,by composing or improving documentation or by writing code.

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

Neutralino server will be compiled in `/bin`

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
- [x] Security improvements
- [x] Experimental Release
- [ ] Other required features
- [ ] Stable release
- [ ] MacOS support

## License

[MIT](LICENSE)
