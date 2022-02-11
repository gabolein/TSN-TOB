#!/usr/bin/env python3

import matplotlib.pyplot as plt

slice_durations = [200, 300, 400]
tsn= [2.48, 45.8, 1995]
best_effort=[4.41,63.7,1053]


plt.plot(slice_durations, best_effort, label='Best-effort mode', marker='v')
plt.plot(slice_durations, tsn, label='TSN mode', marker='v')
plt.legend()
plt.xlabel(u"Slice duration in \u03bcs")
plt.ylabel("MTBF in seconds (timing failures)")
#ax = plt.gca()
#ax.set_yscale('log')
plt.savefig('MTBF_plot.png')
