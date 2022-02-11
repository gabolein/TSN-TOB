#!/usr/bin/env bash

ID=$1
prio=1
amount_nodes=$2
cycle_duration=$3
mode='v'
verbose=-verbose

make clean
make && sudo ip netns exec ns$ID chrt -rr $prio ./synch_broadcast $ID $amount_nodes $cycle_duration $mode $verbose 

