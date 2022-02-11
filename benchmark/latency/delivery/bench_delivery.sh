#!/usr/bin/env bash

for cycle_duration in {1000..1500..200}
#for cycle_duration in {1000..5501..500}
do
    for mode in v
    #for mode in b t
    do
        echo "Now benchmarking for mode $mode, cycle_duration $cycle_duration"
        cd ../../..
        ./setup.sh 3 $cycle_duration $mode
        sleep 5
        ./setup.sh 3 $cycle_duration $mode
        sleep 5
        
        cd prototype

        make clean
        make DELIVERY_BENCH=1

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

        cd ../benchmark/latency/delivery 
        ./get_delivery_latency.py $mode $cycle_duration
    done
done
