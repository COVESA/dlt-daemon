#!/usr/bin/python3
# This software has been developed by Advanced Driver Information Technology.
# Copyright(c) 2020 Advanced Driver Information Technology GmbH,
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
import argparse


def main(header_in_file, header_out_file):
    header_in = pathlib.Path(header_in_file)
    header_out = pathlib.Path(header_out_file)
    with header_in.open() as hi, header_out.open('w') as ho:
        for line in hi:
            if line.startswith("#cmakedefine"):
                continue
            ho.write(line)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('header_in')
    parser.add_argument('header_out')
    args = parser.parse_args()
    main(args.header_in, args.header_out)
