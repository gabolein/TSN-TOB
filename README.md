# Time Aware Total Order Broadcast

This is a TSN based proof-of-concept implementation of Kopetz et. al.'s synchronous Total Order Broadcast algorithm. It is an experimental prototype that serves towards showcasing how TSN can enable the implementation of synchronous distributed algorithms.
The prototype was created for the SuperMicro target device. Big portions of the implementation (especially the setup script) would have to be adapted in order to make the prototype portable.

## Overview

The source code and Makefile for the application can be found in the `prototype` directory. The `interface_files` directory contains the input files, from which the application fetches messages that are to be delivered and the output files, to which these messages are then delivered by the application. The `benchmark` directory contains scripts to collect benchmark data and to plot the data.
The `setup.sh` and `synch_clocks.sh` serve to create an environment in which to run several nodes. 
A copy of my thesis is supplied additionally, as the implementation is described more thoroughly therein.

## Setup

The application allows for 3 different modes: 

- **v**irtual mode: nodes communicate via virtual ethernet (especially useful for testing purposes, as it allows for up to 8 nodes)
- **b**est-effort mode: nodes send messages over the wire, but the standard qdisc settings are kept (useful for comparison with TSN)
- **t**SN mode: nodes have an exclusive slots and hardware offload is active (IEEE 802.1Qbv)

The setup script located in the repository root directory is to be called as follows: 
`./setup.sh <max participating nodes> <cycle duration in us> <mode>`, where the mode is set by passing either of 'v','b' and 't'.

The setup script also sets up the input and output files by copying an input template (new templates can be created in the `interface_files` directory).

A separate namespace is created for every node. The virtual mode creates `veth`s that are moved to their respective namespace. The non-virtual nodes are hardcoded to the interfaces eno2 eno3 eno4. This results from working on hardware that only has 4 TSN-capable nics and eno1 being used for ssh.
In addition, the qdisc modes create a vlan interface that maps socket priority to VLAN tag priority, partly such that a .1Q switch is able to prioritize the Total Order Broadcast traffic. The modes thereafter create a qdisc that exclusively allows Total Order Broadcast traffic to be sent out during the sending slices. The schedules are set in accordance to the [TSN documentation](https://tsn.readthedocs.io/).

In TSN mode, the script also forks processes that synchronize the Hardware Clocks to CLOCK_REALTIME.

## Running a node

The lower the cycle duration the more probable it is that important actions will be ommited due to preemption. To avoid such complications, the service should be run with realtime priority. Refer to the `run_node.sh` script for an example on how best to run a node. It is run by calling `./run_node.sh <node ID> <max participating nodes> <cycle duration>`. The ID must be in the interval [0, number of nodes -1] and unique. The cycle duration is to be stated in microseconds. In the script, one can also manually alter the realtime priority, the mode and disable/enable verbosity.

The cycle duration should be chosen such that the resulting slice duration is not too brief to do the necessary computations. The slice duration is obtained by calculating (cycle duration / 9). More or less thorough testing has shown that on the smicro machine the slice duration can be as low as ~300 usec, however delays due to context switches, syscalls etc. have too high variances to really say which slice duration is optimal.

For this reason, especially when benchmarking, it is recommendable to set the CPU frequency with the [performance governor](https://wiki.archlinux.org/title/CPU_frequency_scaling#Scaling_governors).

## Benchmarking

Automatic benchmark scripts are provided for communication latency and message delivery latency in the `benchmark/latency` directory.

The `bench_distribution.sh` script (Usage: `./bench_distribution.sh <load> <slice duration> <modes>`) runs three nodes that collect timestamps whenever a message is sent and received. The `get_latencies.sh`script (that requires a rewrite very badly) is then called to match the timestamps and the message exchange latencies are written to a file. The `load` parameter is only relevant for naming of the result files. Load can be generated with the `iperf3` tool to observe how background traffic affects the communication latency. The `modes` parameter should be passed as a string of whitespace separated modes, e.g. `"v b t"`.

The `bench_delivery.sh` script runs three nodes that collect timestamps when they broadcast a message and when they deliver one, and then subsequently computes the latencies. It does this for different cycle durations and modes set in the script.

## Compiling

A trivial Makefile is provided in the `prototype` directory. If one wishes to add files to the compilation process, they must be explicitly listed in the Makefile. To compile a node, simply run `make`. For benchmarking purposes, certain smaller bits of the program can compiled differently by running either `make LATENCY_BENCH=1` and `make DELIVERY_BENCH=1`.
