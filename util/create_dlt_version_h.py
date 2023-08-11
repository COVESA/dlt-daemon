#!/usr/bin/python3
# This software has been developed by Advanced Driver Information Technology.
# Copyright(c) 2019 Advanced Driver Information Technology GmbH,
# Advanced Driver Information Technology Corporation, Robert Bosch GmbH,
# Robert Bosch Car Multimedia GmbH and DENSO Corporation.
#
# This file is part of COVESA Project DLT - Diagnostic Log and Trace.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.org/.
import pathlib
import subprocess
import argparse
import re


def get_cmd(cmd, cwd):
    return subprocess.check_output(cmd, cwd=cwd, shell=True,
                                   stderr=subprocess.STDOUT
                                   ).decode().strip()


def get_revision(git_dir):
    try:
        rev = get_cmd('git describe --tags', git_dir)
        if not rev.startswith("fatal:"):
            return rev

        rev = get_cmd('git rev-parse HEAD', git_dir)
        if not rev.startswith("fatal:"):
            return rev
    except subprocess.CalledProcessError:
        pass

    return get_cmd('date +%F', git_dir)


def main(cmake_file, header_in_file, header_out_file):
    cmakelists = pathlib.Path(cmake_file)
    header_in = pathlib.Path(header_in_file)
    header_out = pathlib.Path(header_out_file)
    git_dir = str(header_in.parent)
    cmake_vars = {}

    for m in re.finditer(
            'project\(\S+ VERSION (?P<major>\d+)\.(?P<minor>\d+)\.(?P<patch>\d+)(\.(?P<tweak>\d+))?',
            cmakelists.open().read()):
        cmake_vars['PROJECT_VERSION_MAJOR'] = m.group('major')
        cmake_vars['PROJECT_VERSION_MINOR'] = m.group('minor')
        cmake_vars['PROJECT_VERSION_PATCH'] = m.group('patch')
        cmake_vars['PROJECT_VERSION'] = "{}.{}.{}".format(m.group('major'), m.group('minor'), m.group('patch'))
        cmake_vars['PROJECT_VERSION_TWEAK'] = m.group('tweak')
        cmake_vars['DLT_REVISION'] = get_revision(git_dir)
        cmake_vars['DLT_VERSION_STATE'] = 'STABLE'

    header_out.parent.mkdir(parents=True, exist_ok=True)
    with header_in.open() as hi, header_out.open('w') as ho:
        for line in hi:
            text, _ = re.subn('@(?P<var_name>\w+)@', lambda x: cmake_vars.get(x.group('var_name'), "NONE"), line)
            ho.write(text)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('cmakelists')
    parser.add_argument('header_in')
    parser.add_argument('header_out')
    args = parser.parse_args()
    main(args.cmakelists, args.header_in, args.header_out)
