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

def pre_test_cleanup():
    # Kill PIDs listed in /tmp/dlt.pid and remove FIFO /tmp/dlt to ensure isolated test environment
    pidfile = "/tmp/dlt.pid"
    fifo = "/tmp/dlt"
    if os.path.isfile(pidfile):
        print(f"----INFO: Found pidfile {pidfile}, attempting to kill listed PIDs")
        try:
            with open(pidfile, 'r') as pf:
                for line in pf:
                    line = line.strip()
                    if not line:
                        continue
                    try:
                        pid = int(line)
                        print(f"----INFO: Killing PID: {pid}")
                        try:
                            os.kill(pid, 9)
                        except Exception:
                            pass
                    except ValueError:
                        continue
            try:
                os.remove(pidfile)
                print(f"----INFO: Removed pidfile {pidfile}")
            except Exception:
                pass
        except Exception as e:
            print(f"----WARNING: Failed to read/cleanup {pidfile}: {e}")

    if os.path.exists(fifo):
        try:
            os.remove(fifo)
            print(f"----INFO: Removed FIFO {fifo}")
        except Exception as e:
            print(f"----WARNING: Failed to remove FIFO {fifo}: {e}")
def run_gtest_teardown():
    # Try common locations: current working dir, ./test, ./build/test
    candidates = [
        os.path.join(os.getcwd(), "gtest_dlt_teardown.sh"),
        os.path.join(os.getcwd(), "test", "gtest_dlt_teardown.sh"),
        os.path.join(os.getcwd(), "build", "test", "gtest_dlt_teardown.sh"),
    ]

    bash_script = None
    for p in candidates:
        if os.path.isfile(p):
            bash_script = p
            break

    # Fallback: search recursively under cwd
    if bash_script is None:
        for root, dirs, files in os.walk(os.getcwd()):
            if "gtest_dlt_teardown.sh" in files:
                bash_script = os.path.join(root, "gtest_dlt_teardown.sh")
                break

    if bash_script is None:
        print(f"----ERROR: gtest teardown script not found under {os.getcwd()}")
        sys.exit(1)

    log_file = os.path.splitext(bash_script)[0] + ".log"
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

    pre_test_cleanup()
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