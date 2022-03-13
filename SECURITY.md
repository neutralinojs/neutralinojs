# Security Policy

## Reporting Security Issues

**Please do not report security vulnerabilities through public GitHub issues.** Instead, please report them to the NeutralinoJs Email [neutralinojs@gmail.com](mailto:neutralinojs@gmail.com).

You should receive a response within 48 hours. If for some reason you do not, please follow up via email to ensure we received your original message.

Please include the requested information listed below (as much as you can provide) to help us better understand the nature and scope of the possible issue:

  * Type of issue (e.g. buffer overflow, cross-site scripting, etc.)
  * Full paths of source file(s) related to the manifestation of the issue
  * The location of the affected source code (tag/branch/commit or direct URL)
  * Any special configuration required to reproduce the issue
  * Step-by-step instructions to reproduce the issue
  * Proof-of-concept or exploit code (if possible)
  * Impact of the issue, including how an attacker might exploit the issue

This information will help us triage your report more quickly.

## How to download security patches

Neutralino binaries can be updated with [neu-cli](https://neutralino.js.org/docs/cli/neu-cli) from the developer's side. 
Therefore, if any vulnerability is reported against the Neutralinojs version you use, please update the existing binaries via CLI.

## Prebuilt binaries

The neu CLI downloads prebuilt binaries for each platform from the releases (according your project configuration's versions). On Windows, if your virus scanner software identified a Neutralinojs binary as malware via a false positive detection, please take the following actions.

- Do a full system scan with your antivirus software to verify whether a real malicious program is active.
- Send a false positive detection report to your antivirus software developers.
- Report to Neutralinojs developers via a Github [issue](https://github.com/neutralinojs/neutralinojs/issues).

You can download the Neutralinojs source, inspect, and build your binaries too.
