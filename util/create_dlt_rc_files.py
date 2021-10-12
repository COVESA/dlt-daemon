#!/usr/bin/python3
# This software has been developed by Advanced Driver Information Technology.
# Copyright(c) 2022 Advanced Driver Information Technology GmbH,
# Advanced Driver Information Technology Corporation, Robert Bosch GmbH,
# Robert Bosch Car Multimedia GmbH and DENSO Corporation.
# All rights reserved.
import pathlib
import argparse


def main(rc_in, rc_out, mode):
    file_in = pathlib.Path(rc_in)
    file_out = pathlib.Path(rc_out)
    with file_in.open() as fi, file_out.open('w') as fo:
        for line in fi:
            if "<dlt_android_auto_start>" in line:
                if mode == 'enabled':
                    print("Auto-start mode is enabled")
                    line = line.replace("<dlt_android_auto_start>", "")
                else:
                    print("Auto-start mode is disabled by default")
                    line = line.replace("<dlt_android_auto_start>", "disabled")
            fo.write(line)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('rc_in')
    parser.add_argument('rc_out')
    parser.add_argument('rc_mode')
    args = parser.parse_args()
    main(args.rc_in, args.rc_out, args.rc_mode)

