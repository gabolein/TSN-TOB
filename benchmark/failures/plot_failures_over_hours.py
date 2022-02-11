#!/usr/bin/python3

import matplotlib.pyplot as plt
import numpy as np

bins = range(0,8)
t = [0, 3, 2, 0, 2, 2, 1, 4]

fig = plt.figure()
fig.set_figheight(4)
fig.set_figwidth(10)
plt.bar(bins, t, color='tab:orange', label='TSN mode', alpha=0.5)
plt.legend()
plt.xlabel('Runtime of benchmark in hours')
plt.ylabel('Timing failures within timeframe')
plt.yticks(np.arange(0, 7, 1.0))
plt.savefig("plot_tsn_3600_long.png")

plt.clf()

b = [1, 1, 3, 4, 6, 4, 4, 1]
fig = plt.figure()
fig.set_figheight(4)
fig.set_figwidth(10)
plt.bar(bins, b, alpha=0.5, label='Best-effort mode')
plt.legend()
plt.xlabel('Runtime of benchmark in hours')
plt.ylabel('Timing failures within timeframe')
plt.yticks(np.arange(0, 7, 1.0))
plt.savefig("plot_best_3600_long.png")
