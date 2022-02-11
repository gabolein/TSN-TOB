#!/usr/bin/python3


import sys
import matplotlib.pyplot as plt
from csv import reader
import os

b_data_median = []
t_data_median = []
v_data_median = []

# with open("b", "r") as b_file:
    # csv_reader = reader(b_file)
    # for row in csv_reader:
        # if int(row[1]) > 20:
            # tuple_median = (int(row[0]), float(row[2])/1000)
            # b_data_median.append(tuple_median)

# with open('t', 'r') as t_file:
    # csv_reader = reader(t_file)
    # for row in csv_reader:
        # if int(row[1]) > 20:
            # tuple_median = (int(row[0]), float(row[2])/1000)
            # t_data_median.append(tuple_median)

with open('v', 'r') as t_file:
    csv_reader = reader(t_file)
    for row in csv_reader:
        if int(row[1]) > 1000:
            tuple_median = (int(row[0]), float(row[2])/1000)
            v_data_median.append(tuple_median)


zip(*v_data_median)
plt.plot(*zip(*v_data_median), color='g', marker='*', label='virtual')

# zip(*b_data_median)
# plt.plot(*zip(*b_data_median), label='Best-effort mode')

# zip(*t_data_median)
# plt.plot(*zip(*t_data_median), label='TSN mode')

plt.legend()
plt.xlabel(u"Cycle duration in \u03bcs")
plt.ylabel(u"Delivery latency in \u03bcs")
x1, x2, y1, y2 = plt.axis()
plt.axis((x1,x2,x1,x2))
plt.savefig("delivery_latency_plot")
