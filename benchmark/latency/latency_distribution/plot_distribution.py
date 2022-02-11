#!/usr/bin/python3

import sys
import matplotlib.pyplot as plt

input_dir=sys.argv[1]
slice_duration=sys.argv[2]
load=sys.argv[3]

b_latencies = []
s_latencies = []
t_latencies = []

# with open(input_dir + 'b', 'r') as b_file:
    # for line in b_file:
        # b_latencies.append(float(str(line))/1000)

with open(input_dir + 'v', 'r') as s_file:
    for line in s_file:
        s_latencies.append(float(str(line))/1000)

# with open(input_dir + 't', 'r') as t_file:
    # for line in t_file:
        # t_latencies.append(float(str(line))/1000)


#split data into ranges
bins = [x for x in range(50, 550, 10)]

t_latencies.sort()
s_latencies.sort()
b_latencies.sort()

# plt.hist(b_latencies, bins, label='Best-effort mode', alpha=0.5)
plt.hist(s_latencies, bins, label='virtual', alpha=0.5)
#plt.hist(t_latencies, bins, label='TSN mode', alpha=0.5, color='tab:orange')
plt.legend()
plt.xlabel(u"Message latency in \u03bcs")
plt.ylabel("Frequency of latency")


plt.savefig(load + '_' + slice_duration)
