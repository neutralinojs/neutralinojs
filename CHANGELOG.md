# Changelog

Add all code changes (features, deprecations, and enhancements) under the `Unreleased` topic to track changes for 
the next release. Once the changes are released,
rename `Unreleased` topic with the new version tag. Finally, create a new `Unreleased` topic for future changes.

## Unreleased

### API: os.execCommand

- Renamed `shouldRunInBackground` option to `background`.
- Supports `stdIn` as an optional input.
- Returns multiple values: `stdOut`, `stdErr`, `pid`, and `exitCode`.


