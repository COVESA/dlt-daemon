#!/usr/bin/python3
# This file is part of COVESA Project DLT - Diagnostic Log and Trace.
#
# This Source Code Form is subject to the terms of the
# Mozilla Public License (MPL), v. 2.0.
# If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.org/.
#********************************************************************************
#**                      Author Identity                                       **
#********************************************************************************
#**                                                                            **
#** Initials     Name                       Company                            **
#** --------     -------------------------  ---------------------------------- **
#**  aw          LUU QUANG MINH             BOSCH GLOBAL SOFTWARE TECHNOLOGY   **
#********************************************************************************
# DLT actuator is constructed here for utesting
# due to the fact that some components need
# setting up environment in it own scripts.
# Some scripts is not good handled with exit
# condition or control, honestly no exit at all,
# and this can lead to an infinite loop or
# bash recursive calls making system hang.
import os
import subprocess
import sys
def run_command(command, logfile, mode='w'):
    command_name = os.path.basename(command[0])
    print(f"----INFO: Executing {command_name}")
    print(f"----INFO: Logging to {logfile}")
    with open(logfile, mode) as log_file:
        return subprocess.call(command, stdout=log_file, stderr=log_file)
def run_gtest_teardown():
    gtest_teardown_path = os.path.join(os.getcwd(), "gtest_dlt_teardown")
    bash_script = gtest_teardown_path + ".sh"
    log_file = gtest_teardown_path + ".log"
    if not os.path.isfile(bash_script):
        print(f"----ERROR: gtest script {bash_script} does not exist")
        sys.exit(1)
    ret_code = run_command([bash_script], log_file)
    if ret_code != 0:
        print(f"----ERROR: Teardown failed with exit code {ret_code}")
        with open(log_file, 'r') as log:
            print("----INFO: Teardown Log Output:")
            print("\n===== LOG OUTPUT START =====\n")
            print(log.read())
            print("\n===== LOG OUTPUT END =====\n")
        sys.exit(ret_code)
    else:
        print(f"----INFO: Trigger gtest_dlt_teardown.......... SUCCESS")
    return ret_code
def run_gtest_binary(gtest_binary):
    binary_name = os.path.basename(gtest_binary)
    gtest_binary_path = os.path.join(os.getcwd(), gtest_binary)
    if not os.path.isfile(gtest_binary_path):
        print(f"----ERROR: gtest binary {gtest_binary_path} does not exist")
        sys.exit(1)
    bash_script = gtest_binary_path + ".sh"
    log_file = gtest_binary_path + ".log"
    if os.path.isfile(bash_script):
        print(f"----INFO: Preparing environment")
        log_mode = 'a'
        ret_code = run_command([bash_script], log_file)
        if ret_code != 0:
            print(f"----ERROR: Setup failed with exit code {ret_code}")
            with open(log_file, 'r') as log:
                print("----INFO: Test Log Output:")
                print("\n===== LOG OUTPUT START =====\n")
                print(log.read())
                print("\n===== LOG OUTPUT END =====\n")
            sys.exit(ret_code)
    else:
        print(f"----INFO: No environment setup, running test")
        log_mode = 'w'
    ret_code = run_command([gtest_binary_path], log_file, mode=log_mode)
    if ret_code != 0:
        print(f"----INFO: Test {binary_name} .......... FAILED")
        with open(log_file, 'r') as log:
            print("----INFO: Test Log Output:")
            print("\n===== LOG OUTPUT START =====\n")
            print(log.read())
            print("\n===== LOG OUTPUT END =====\n")
    else:
        print(f"----INFO: Test {binary_name} .......... PASSED")
    run_gtest_teardown()
    return ret_code
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("----INFO: Usage: python gtest_dlt_actuator.py <gtest_dlt_binary_path>")
        sys.exit(1)
    gtest_binary = sys.argv[1]
    if not os.path.isfile(gtest_binary):
        print(f"----ERROR: gtest binary {gtest_binary} does not exist")
        sys.exit(1)
    exit_code = run_gtest_binary(gtest_binary)
    sys.exit(exit_code)