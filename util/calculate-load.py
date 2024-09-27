import argparse
import re
import subprocess
import os
import json
import math
from datetime import datetime, timedelta, timezone


# Function to calculate average payload size and round up to the next full integer
def calculate_payload_size_in_window(payload_sizes):
    return sum(payload_sizes)


# Parse command-line arguments
def parse_arguments():
    parser = argparse.ArgumentParser(
        description=
        'Process DLT messages to calculate the highest average payload size.')

    # Add the mode argument with choices for 'online' and 'offline'
    parser.add_argument('--soft-limit-factor',
                        type=float,
                        default=1.5,
                        help='Factor to multiply the measured values with to get the soft limit recommendation (default: 1.1)')
    parser.add_argument('--hard-limit-factor',
                        type=float,
                        default=2,
                        help='Factor to multiply the measured values with to get the hard limit recommendation (default: 1.3)')
    parser.add_argument('--factor-multiplier',
                        type=int,
                        default=10,
                        help='Multiplier used for rounding the limits up. I.e. 13 -> 20, 123 -> 130 (default: 10)')

    parser.add_argument('--sum-contexts',
                        action='store_true',
                        default=False,
                        help='If set to true, all contexts will be summed, otherwise one measurement per context (default: True')

    # Add the mode argument with choices for 'online' and 'offline'
    parser.add_argument('-m', '--mode',
                        type=str,
                        choices=['online', 'offline'],
                        default='online',
                        help='Mode to process DLT messages, online will read logs directly via dlt-receive, '
                             'offline will expect a dlt offline trace converted '
                             'to text using dlt-convert -a file > dlt.txt (default: online)')

    parser.add_argument('-o', '--output',
                        type=str,
                        choices=['json', 'dlt'],
                        default='dlt',
                        help='Output mode of script (default: json)')

    parser.add_argument('-e', '--ecu',
                        type=str,
                        default=None,
                        required=True,
                        help='The ECU ID to filter DLT messages')

    parser.add_argument('-p', '--port',
                              type=int,
                              default=3490,
                              help='Port to receive DLT messages (default: 3490) (only used in online mode)')
    parser.add_argument('-a', '--address',
                              type=str,
                              default='127.0.0.1',
                              help='Address to receive DLT messages (default: 127.0.0.1) (only used in online mode)')
    parser.add_argument('-t', '--time',
                              type=int,
                              default=300,
                              help='Runtime duration in seconds (default: 300) (only used in online mode)')

    # Create a subgroup for offline mode arguments
    offline_group = parser.add_argument_group('Offline Mode Arguments')
    offline_group.add_argument('-f', '--file',
                               type=str,
                               help='Path to the DLT file when in offline mode')

    parser.add_argument(
        '-d',
        '--debug',
        action='store_true',
        help='Include debug logs in the average (default: false)')
    parser.add_argument(
        '-v',
        '--verbose',
        action='store_true',
        help='Include verbose logs in the average (default: false)')

    parser.add_argument('app_id',
                        type=str,
                        nargs='?',
                        help='Application ID to filter DLT messages, if not passed, all messages will be processed')

    parser.add_argument('context_id',
                        type=str,
                        default=None,
                        nargs='?',
                        help='Optional ID to filter DLT messages, needs application id!')

    args = parser.parse_args()
    if not args.ecu:
        parser.error("ECU ID is required.")

    # Validate the arguments based on the mode
    if args.mode == 'offline':
        if not args.file:
            parser.error("Offline mode requires a file path using the -f or --file option.")
        if not os.path.isfile(args.file):
            parser.error("File path provided does not exist.")
    else:
        # In online mode, the file should not be used
        if args.file:
            parser.error("File path is not used in online mode.")
    return args


def get_next_line_from_file(file):
    with open(file, 'r', errors='ignore') as file:
        for line in file:
            yield line.strip()


def get_next_line_from_dlt_receive(process):
    return process.stdout.readline().decode('utf-8', 'ignore').strip()

def keep_running(end_time):
    if end_time:
        return datetime.now(timezone.utc) < end_time
    return True

def round_up_to(n, multiplier):
    return math.ceil(n / multiplier) * multiplier

def main():
    args = parse_arguments()
    end_time = None
    load = {}

    if args.mode == 'offline':
        file_reader = get_next_line_from_file(args.file)
    else:
        # Prepare the command to receive messages with provided arguments
        command = f"dlt-receive -a {args.address} -p {args.port}"
        process = subprocess.Popen(command, stdout=subprocess.PIPE, shell=True)
        # Set the duration for which to receive messages
        end_time = datetime.now(timezone.utc) + timedelta(seconds=args.time)

    if not args.app_id:
        line_regex = re.compile(rf"{args.ecu}-{{0,3}}\s+(\w{{1,4}}-{{0,3}})\s+(\w{{1,4}}-{{0,3}})\s+log")
    else:
        if args.context_id:
            line_regex = re.compile(rf"{args.ecu}-{{0,3}}\s+({args.app_id}-{{0,3}})\s+({args.context_id}-{{0,3}})\s+log")
        else:
            line_regex = re.compile(rf"{args.ecu}-{{0,3}}\s+({args.app_id}-{{0,3}})\s+(\w{{1,4}}-{{0,3}})\s+log")

    try:
        # Process messages until the end time is reached
        while keep_running(end_time):
            if args.mode == 'offline':
                line = next(file_reader)
            else:
                line = get_next_line_from_dlt_receive(process)

            if not line:
                continue  # Skip empty lines

            match = line_regex.search(line)
            if not match:
                continue
            line_app_id = match.group(1).rstrip('-')
            line_context_id = match.group(2).rstrip('-')

            # Check if the line contains the Application ID
            if (('log debug' in line and not args.debug) or
                    ('log verbose' in line and not args.verbose)):
                continue  # Skip this line as it's a debug/verbose log and the flags are not set

            # Extract the payload size using regex
            match = re.search(r'\[(.*)$', line)
            if not match:
                continue
            if line_app_id not in load:
                load[line_app_id] = {}
            if line_context_id not in load[line_app_id]:
                load[line_app_id][line_context_id] = {}
                load[line_app_id][line_context_id]["payload_sizes"] = []
                load[line_app_id][line_context_id]["highest_average"] = 0
                load[line_app_id][line_context_id]["window_start_time"] = datetime.now(timezone.utc)

            payload = match.group(1).strip()[:-1]
            payload_size = len(payload)

            load[line_app_id][line_context_id]["payload_sizes"].append(payload_size)

            # Check if 60 seconds have passed or if we're at the end of the runtime
            current_time = datetime.now(timezone.utc)
            if (current_time - load[line_app_id][line_context_id]["window_start_time"]).total_seconds() >= 60 or not keep_running(end_time):
                average_payload_size = calculate_payload_size_in_window(load[line_app_id][line_context_id]["payload_sizes"])
                load[line_app_id][line_context_id]["highest_average"] = max(load[line_app_id][line_context_id]["highest_average"], average_payload_size)
                load[line_app_id][line_context_id]["payload_sizes"] = []  # Reset for the next window
                load[line_app_id][line_context_id]["window_start_time"] = current_time  # Reset window start time
    except StopIteration:
        # done reading file
        pass
    finally:
        results = {}

        # If there are any remaining payload sizes that have not been averaged, do so now
        for app_id, context_ids in load.items():
            app_id_total = 0
            results[app_id] = {}
            results[app_id]["contexts"] = {}
            for context_id, data in context_ids.items():
                payload_sizes = data["payload_sizes"]
                average_payload_size = calculate_payload_size_in_window(payload_sizes)
                highest_average = max(data["highest_average"], average_payload_size) / 60
                app_id_total += highest_average

                if not args.sum_contexts:
                    results[app_id]["contexts"][context_id] = {}
                    results[app_id]["contexts"][context_id]["measured"] = highest_average
                    results[app_id]["contexts"][context_id]["soft"] = round_up_to(highest_average * args.soft_limit_factor, args.factor_multiplier)
                    results[app_id]["contexts"][context_id]["hard"] = round_up_to(highest_average * args.hard_limit_factor, args.factor_multiplier)
                    results[app_id]["contexts"] = dict(sorted(results[app_id]["contexts"].items()))

            results[app_id]
            if not args.context_id:
                results[app_id]["measured"] = app_id_total
                results[app_id]["soft"] = round_up_to(app_id_total * args.soft_limit_factor, args.factor_multiplier)
                results[app_id]["hard"] = round_up_to(app_id_total * args.hard_limit_factor, args.factor_multiplier)

        results = dict(sorted(results.items()))
        if args.output == "json":
            print(json.dumps(results, indent=4))
        else:
            if args.sum_contexts:
              for app_id, data in results.items():
                    print(f"{app_id} {data['soft']} {data['hard']}")
            else:
                for app_id, context_ids in results.items():
                    for context_id, data in context_ids["contexts"].items():
                        print(f"{app_id} {context_id} {data['soft']} {data['hard']}")

        if args.mode == 'offline':
            pass
        else:
            # Ensure the subprocess is terminated
            process.terminate()


if __name__ == "__main__":
    main()
