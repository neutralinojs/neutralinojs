#!/usr/bin/env python3

import sys
import os

if len(sys.argv) < 2:
    print('Please provide required arguments')
    sys.exit(1)

changelog_file = './CHANGELOG.md'
tmp_dir = './.tmp'
release_note_file = tmp_dir + '/release_notes.md'
release_version = 'v' + sys.argv[1]

release_note = '## What\'s new \n'
collect = False
updated_changelog = ''

with open(changelog_file, 'r') as cf:
    for line in cf:
        if '## Unreleased' in line:
            collect = True
            continue
        if('## v' in line):
            break
        if(collect):
            release_note += line

    if(not collect):
        print('No changelog found!')
        sys.exit(1)

    cf.seek(0)
    updated_changelog = cf.read().replace('## Unreleased', '## ' + release_version)

release_note += '\nGet started: https://neutralino.js.org/docs'

print('---- Release note %s ----' % release_version)
print(release_note)
print('----')

print('INFO: Creating release notes file...')
os.makedirs(tmp_dir, exist_ok = True)

with open(release_note_file, 'w') as rf:
    rf.write(release_note)

print('INFO: Updating change log...')
with open(changelog_file, 'w') as cf:
    cf.write(updated_changelog)

print('OK: All done. Delete %s after using it.' % release_note_file)
