#!/usr/bin/python3

import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator

cycle_durations = [5000, 8000, 12000, 16000, 20000, 24000, 28000, 32000, 36000, 40000]
data_best_effort = [471, 487, 504, 490, 496, 484, 483, 485, 484, 480]
data_tsn = [736, 896, 927, 902, 941, 955, 955, 955, 955, 955]
measured_bandwidth = [955 for x in range(0,10)]

plt.plot(cycle_durations, data_best_effort, label='Best-effort mode', marker='v')
plt.plot(cycle_durations, data_tsn, label='TSN mode', marker='v')
plt.plot(cycle_durations, measured_bandwidth, label='Measured bandwidth')
plt.legend()
plt.xlabel(u'Cycle duration in \u03bcs')
plt.ylabel('Throughput in Mbit/s')
plt.xticks(range(4000,44000,4000))

ax = plt.axes()
ax.yaxis.set_minor_locator(AutoMinorLocator())

plt.savefig('throughput_plot')

