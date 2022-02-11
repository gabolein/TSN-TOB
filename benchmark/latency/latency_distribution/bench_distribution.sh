#!/usr/bin/env bash

traffic_load=$1
slice_duration=$2
modes=$3

date=$(date +"%d_%m_%Y_%H_%M")
cd ..

cycle_duration=$((slice_duration*9))

for mode in $modes
do
    echo "Now benchmarking for mode $mode"
    cd ../..
    # call setup twice just to make sure, weird stuff has happened...
    ./setup.sh 3 $cycle_duration $mode
    sleep 5
    ./setup.sh 3 $cycle_duration $mode
    sleep 5
    cd benchmark/latency
    
    cd ../../prototype
    
    make clean
    make LATENCY_BENCH=1

    if [ $mode == "t" ]; then
        node_mode="s"
    else
        node_mode=$mode
    fi

    sudo ip netns exec ns0 chrt -rr 1 ./synch_broadcast 0 3 $cycle_duration $node_mode &
    
    sudo ip netns exec ns1 chrt -rr 1 ./synch_broadcast 1 3 $cycle_duration $node_mode &

    sudo ip netns exec ns2 chrt -rr 1 ./synch_broadcast 2 3 $cycle_duration $node_mode &

    wait $(jobs -p)
    sudo killall synch_broadcast
    sleep 1

    cd ../benchmark/latency/
    echo "calculate latencies, average and deviation, write them to file"
    ./get_latencies.py $date $traffic_load $mode $slice_duration xD
done
