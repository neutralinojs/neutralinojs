# Security Policy

## Reporting a vulnerability

If you found a vulnerability in Neutralinojs components, please report as a Github [issue](https://github.com/neutralinojs/neutralinojs/issues).
After that, we'll add the `security` label to those issues and apply patches with a future release.

## How to download security patches

Neutralino binaries can be updated with [neu-cli](https://neutralino.js.org/docs/developer-environment/cli) from the developer's side. 
Therefore, if any vulnerability is reported against the Neutralinojs version you use, please update the existing binaries via CLI.

## Prebuilt binaries

The neu CLI downloads prebuilt binaries for each platform from the latest release (according to the CLI version). On Windows, if your virus scanner software identified a Neutralinojs binary as malware via a false positive detection, please take the following actions.

- Do a full system scan with your antivirus software to verify that a real malicious program is active.
- Send a false positive detection report to your antivirus software developers.
- Report to Neutralinojs developers via a Github issue.

You can download the Neutralinojs source, inspect, and build your binaries too.
