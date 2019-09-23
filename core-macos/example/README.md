# Example Application

You can directly run this application in your mac
Refer to **Info.plist** to see how to configure the App Bundle.

## Directory structure

```bash
├── Neut.app
│   └── Contents
│       ├── Info.plist
│       ├── MacOS
│       │   └── neutralino
│       └── Resources
│           ├── app
│           │   ├── assets
│           │   │   ├── app.css
│           │   │   ├── app.js
│           │   │   └── neutralino.js
│           │   ├── index.html
│           │   └── settings.json
│           └── storage
```

## Where to put binary

Put your binary in `Contents/MacOS` directory

## Where to put html/js/css

You need to put all the resources in `Contents/Resources` directory. 

