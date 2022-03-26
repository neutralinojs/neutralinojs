#!/usr/bin/env python3

import sys
import os
import subprocess
import platform
import json
import glob

C = {}

BZ_VERSION = '1.0.0'
BZ_CONFIG_FILE = 'buildzri.config.json'
BZ_OS = platform.system().lower()
BZ_ISLINUX = BZ_OS == 'linux'
BZ_ISDARWIN = BZ_OS == 'darwin'
BZ_ISWIN = BZ_OS == 'windows'
BZ_ISVERBOSE = '--verbose' in sys.argv

def get_arch(short_names = True, use_mac_rosetta = True):
    arch = platform.machine().lower()

    if BZ_ISDARWIN and arch == 'arm64':
        arch = 'x86_64'

    if short_names and (arch in ['x86_64', 'amd64']):
        arch = 'x64'
    return arch

def get_os_shortname():
    osnames = {'linux': 'linux', 'windows': 'win', 'darwin': 'mac'}
    return osnames[BZ_OS]

def get_compiler():
    compilers = {'linux': 'g++', 'windows': 'cl', 'darwin': 'c++'}
    return compilers[BZ_OS] + ' '

def apply_template_vars(text):
    return text\
        .replace('${BZ_OS}', get_os_shortname()) \
        .replace('${BZ_VERSION}', C['version']) \
        .replace('${BZ_ARCH}', get_arch()) \
        .replace('${BZ_ARCHL}', get_arch(short_names = False))

def get_target_name():
    out_file = C['output']

    out_file = apply_template_vars(out_file)

    if BZ_ISWIN:
        out_file += '.exe'

    return out_file

def configure_vs_tools():
    vsw_path = ''
    vsw_path_f = '%s\\Microsoft Visual Studio\\Installer\\vswhere.exe'
    for prog_path in [os.getenv('ProgramFiles(x86)'), os.getenv('ProgramFiles')]:
        path = vsw_path_f % prog_path
        if os.path.exists(path):
            vsw_path = path
            break

    if vsw_path == '':
        print('ERR: Unable to find vswhere.exe')
        sys.exit(1)
    
    if BZ_ISVERBOSE:
        print('vswhere.exe path: %s' % vsw_path)

    vs_paths = subprocess\
        .getoutput('"%s" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath' % vsw_path) \
        .strip().split('\r\n')
    
    vs_path = vs_paths[-1]
    vs_devcmd_file = '%s\\Common7\\Tools\\vsdevcmd.bat' % vs_path
    
    if not os.path.exists(vs_devcmd_file):
        print('ERR: Unable to find VS dev command-line tools')
        sys.exit(1)
    
    return '"%s" -host_arch=%s -arch=%s' % (vs_devcmd_file, get_arch(), get_arch())


def get_std():
    if 'std' not in C:
        return ''

    std_prefix = '--std='
    if BZ_ISWIN:
        std_prefix = '/std:'
    return '%s%s ' % (std_prefix, C['std'])

def get_source_files():
    file_defs = ['*', BZ_OS]
    files = ''

    for file_def in file_defs:
        if file_def not in C['source']:
            continue
        for entry in C['source'][file_def]:
            glob_files = glob.glob(entry)
            if len(glob_files) > 0:
                files += ' '.join(glob_files) + ' '

    return files

def get_includes():
    if 'include' not in C:
        return ''

    inc_defs = ['*', BZ_OS]
    incs = ''
    inc_prefix = '-I'
    if BZ_ISWIN:
        inc_prefix = '/I'

    for inc_def in inc_defs:
        if inc_def not in C['include']:
            continue
        for entry in C['include'][inc_def]:
            incs += '%s %s ' % (inc_prefix, entry)

    return incs

def get_definitions():
    if 'definitions' not in C:
        return ''

    defs = ''
    def_defs = ['*', BZ_OS]
    def_prefix = '-D'
    if BZ_ISWIN:
        def_prefix = '/D'

    for def_def in def_defs:
        if def_def not in C['definitions']:
            continue
        for entry in C['definitions'][def_def]:
            defs += '%s%s ' % (def_prefix, apply_template_vars(entry))

    return defs

def get_target():
    if 'output' not in C:
        return ''

    out_prefix = '-o '
    if BZ_ISWIN:
        out_prefix = '/OUT:'
    out_file = get_target_name()

    out_path = os.path.dirname(out_file)

    if out_path != '' and not os.path.isdir(out_path):
        os.makedirs(out_path, exist_ok = True)

    if os.path.exists(out_file):
        os.remove(out_file)

    return '%s%s ' % (out_prefix, out_file)

def get_options():
    if 'options' not in C:
        return ''

    opt_defs = ['*', BZ_OS]
    opts = ''

    for opt_def in opt_defs:
        if opt_def not in C['options']:
            continue
        for entry in C['options'][opt_def]:
            opts += '%s ' % apply_template_vars(entry)
    return opts

def build_compiler_cmd():
    cmd = ''
    if BZ_ISWIN:
        cmd += '%s && ' % configure_vs_tools()
    cmd += get_compiler()
    cmd += get_std()
    cmd += get_includes()
    cmd += get_source_files()
    cmd += get_definitions()
    cmd += get_options()
    cmd += get_target()
    return cmd

def compile(cmd):
    print('Compiling %s...' % C['name'])

    if BZ_ISVERBOSE:
        print('Running command: %s' % cmd)

    exit_code = subprocess.call(cmd, shell = True)
    msg = ''
    if exit_code == 0:
        msg = 'OK: %s compiled into %s' % (C['name'], get_target_name())
    else:
        msg = 'ERR: Unable to compile %s' % C['name']

    print(msg)

def print_ascii_art():
    print('''
  ____        _ _     _ ______     _
 |  _ \      (_) |   | |___  /    (_)
 | |_) |_   _ _| | __| |  / / _ __ _
 |  _ <| | | | | |/ _` | / / | '__| |
 | |_) | |_| | | | (_| |/ /__| |  | |
 |____/ \__,_|_|_|\__,_/_____|_|  |_|

 BuildZri v%s - A minimal build automation tool for C++

    ''' % BZ_VERSION)

if __name__ == '__main__':
    with open(BZ_CONFIG_FILE) as configFile:
        print_ascii_art()
        C = json.loads(configFile.read())
        cmd = build_compiler_cmd()
        compile(cmd)
