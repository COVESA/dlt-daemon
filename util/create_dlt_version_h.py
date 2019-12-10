#!/usr/bin/python3
# This software has been developed by Advanced Driver Information Technology.
# Copyright(c) 2019 Advanced Driver Information Technology GmbH,
# Advanced Driver Information Technology Corporation, Robert Bosch GmbH,
# Robert Bosch Car Multimedia GmbH and DENSO Corporation.
#
# This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.genivi.org/.
import pathlib
import argparse
import re


# CMake variables without references to other vars
determined_vars = {}


# Resolves variable reverences from CMakeLists e.g.:
#       ${DLT_MAJOR_VERSION}.${DLT_MINOR_VERSION}
#    -> 2.${DLT_MINOR_VERSION}
def resolve_variables(undetermined_vars):
    for key, val in undetermined_vars.items():
        m = re.search('\$\{(?P<var_name>\w+)\}', val)
        if m:
            if m.group('var_name') in determined_vars:
                yield key, val.replace(m.group(0), determined_vars[m.group('var_name')])
                continue
            yield key, val
        else:
            determined_vars[key] = val


def main(cmake_file, header_in_file, header_out_file):
    cmakelists = pathlib.Path(cmake_file)
    header_in = pathlib.Path(header_in_file)
    header_out = pathlib.Path(header_out_file)

    src = cmakelists.open().read()
    undetermined_vars = {}

    # Find all cmake variable assignments
    for match in re.finditer('set\s*\(\s*(?P<key>\w+)\s*(?P<value>\S+)\s*\)', src):
        key = match.group('key')
        val = match.group('value')
        if '${' not in val:
            determined_vars[key] = val
        else:
            undetermined_vars[key] = val

    # Try to resolve all variables referencing other vars e.g.:
    # set(DLT_VERSION ${DLT_MAJOR_VERSION}.${DLT_MINOR_VERSION}.${DLT_PATCH_LEVEL})
    no_changes = 5
    def_len = len(undetermined_vars)
    while no_changes > 0:
        undetermined_vars = dict(resolve_variables(undetermined_vars))
        if len(undetermined_vars) == def_len:
            no_changes -= 1
        else:
            def_len = len(undetermined_vars)
            no_changes = 5

    header_out.parent.mkdir(parents=True, exist_ok=True)
    with header_in.open() as hi, header_out.open('w') as ho:
        for line in hi:
            text, _ = re.subn('@(?P<var_name>\w+)@', lambda x: determined_vars.get(x.group('var_name'), "NONE"), line)
            ho.write(text)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('cmakelists')
    parser.add_argument('header_in')
    parser.add_argument('header_out')
    args = parser.parse_args()
    main(args.cmakelists, args.header_in, args.header_out)
