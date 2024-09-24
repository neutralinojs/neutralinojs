# Security Policy

## Reporting a vulnerability

If you found a vulnerability in one of Neutralinojs components, please [create a security advisory](https://github.com/neutralinojs/neutralinojs/security/advisories/new) via GitHub. We'll review them and implement necessary patches with upcoming releases.

## How to download security patches

Neutralino binaries can be updated with [neu-cli](https://neutralino.js.org/docs/cli/neu-cli) from the developer's side. 
Therefore, if any vulnerability is reported against the Neutralinojs version you use, please update the existing binaries via CLI.

## Prebuilt binaries

The neu CLI downloads prebuilt binaries for each platform from the releases (according your project configuration's versions). On Windows, if your virus scanner software identified a Neutralinojs binary as malware via a false positive detection, please take the following actions.

- Do a full system scan with your antivirus software to verify whether a real malicious program is active.
- Send a false positive detection report to your antivirus software developers.
- Report to Neutralinojs developers via a Github [issue](https://github.com/neutralinojs/neutralinojs/issues).

You can download the Neutralinojs source, inspect, and build your binaries too.
