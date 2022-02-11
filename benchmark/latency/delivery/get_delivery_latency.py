#!/usr/bin/python3

import sys
import string
import numpy as np

def parse_timestamp(timestamp_str):
    try:
        timestamp_sec, timestamp_nsec = timestamp_str.split(sep='+')
    except Exception:
        print("Could not handle timestamp properly")
        print("timestamp str: " + repr(timestamp_str))

    try:
        timestamp_sec = int(timestamp_sec[:-2])
        timestamp_nsec = int(timestamp_nsec[:-3])
    except Exception:
        print("Could not take slice of stamp")
        print("timestamp str: " + repr(timestamp_str))

    assert timestamp_sec > -1 and timestamp_nsec > -1
    return timestamp_sec, timestamp_nsec

def calc_diff_ts(recv_sec, recv_nsec, sent_sec, sent_nsec):
    if recv_sec < sent_sec or (recv_sec == sent_sec and recv_nsec < sent_nsec):
        print("Something is majorly fucked!!!!")
        #sys.exit(0)


    if (recv_nsec - sent_nsec) < 0:
        result_sec = recv_sec - sent_sec - 1
        result_nsec = recv_nsec - sent_nsec + 1000000000
    else:
        result_sec = recv_sec - sent_sec
        result_nsec = recv_nsec - sent_nsec

    if result_sec != 0:
        raise Exception("Latency higher than 1s")

    return result_nsec


mode = sys.argv[1]
cycle_duration = sys.argv[2]

list_latencies = []

with open("../latency_node_0/sent_ts", "r") as sent_ts:
    for line in sent_ts:
        if line != "\n":

            try:
                msg_ID, timestamp = line.split(maxsplit=1)
            except ValueError:
                print("Could not split line properly")
                print("sender line causing error:" + repr(line))

            sent_s, sent_ns = parse_timestamp(timestamp)
            with open("deliver_ts", "r") as recv_ts:
                for recv_line in  recv_ts:
                    if recv_line != "\n" and not "NOT RECV" in recv_line:

                        try:
                            recv_msg_ID, recv_stamp = recv_line.split(maxsplit=1)
                        except ValueError:
                            print("Could not split line properly")
                            print("Line that caused error:" + repr(recv_line))

                        if recv_msg_ID == msg_ID:
                            recv_s, recv_ns = parse_timestamp(recv_stamp)
                            try:
                                diff = calc_diff_ts(recv_s, recv_ns, sent_s, sent_ns)
                            except Exception:
                                print("Latency higher than 1 s!!")
                                print("sender " + str(node) + " line " + repr(line))
                                print("receiver " + str(receiver) + " line " + repr(recv_line))
                                print("sec values: " + str(recv_s) + " " + str(sent_s))
                                print("nsec values: " + str(recv_ns) + " " + str(sent_ns))

                            if diff > 0:
                                list_latencies.append(diff)



if list_latencies:
    with open(mode, "a") as avg_file:
              avg_file.write(cycle_duration + "," + str(len(list_latencies)) + "," + str(np.median(list_latencies)) + "\n")
else:
    print("No latencies!")
