#!/usr/bin/env bash

if [ $# -lt 3 ]; then
    echo "Usage :"
    echo "./setup.sh <number of nodes> <cycle duration> <mode (v|b|s|t)> "
	exit 1
fi

N_NODES=$1
CYCLE_DURATION=$2
# cycle duration needs to be divisible by slice duration bc of how taprio works
SLICES_IN_CYCLE=$((N_NODES*3))
if [ $((CYCLE_DURATION % SLICES_IN_CYCLE)) != 0 ]; then
	APPROX_SLICE_DURATION=$((CYCLE_DURATION/SLICES_IN_CYCLE))
	CYCLE_DURATION=$((SLICES_IN_CYCLE*(APPROX_SLICE_DURATION+1)))
fi
MODE=$3

get_base_time() {
	curr_time=$(date +%s%N)
	curr_time=$((curr_time/1000))
	cycle_time=$((curr_time % $CYCLE_DURATION))
	echo "$(((curr_time-$cycle_time)*1000))"
}

set_qdisc(){
	interface=$1
	namespace=$2
	start=$(get_base_time)
	slice_duration=$(((CYCLE_DURATION*1000)/9))
	sudo ip netns exec $namespace tc qdisc replace dev $interface parent root handle 100 taprio \
		num_tc 2 \
		map 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 \
		queues 1@0 1@1 \
		base-time $start \
		sched-entry S 01 $slice_duration \
		sched-entry S 02 $slice_duration \
		sched-entry S 01 $slice_duration \
		sched-entry S 01 $slice_duration \
		sched-entry S 02 $slice_duration \
		sched-entry S 01 $slice_duration \
		sched-entry S 01 $slice_duration \
		sched-entry S 02 $slice_duration \
		sched-entry S 01 $slice_duration \
		clockid CLOCK_REALTIME
}

set_qdisc_tsn(){
	interface=$1
	namespace=$2
	start=$(get_base_time)
	slice_duration=$(((CYCLE_DURATION*1000)/9))

	sudo ip netns exec $namespace tc qdisc replace dev $interface parent root handle 100 taprio \
		num_tc 2 \
		map 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 \
		queues 1@0 1@1 \
		base-time $start \
		sched-entry S 01 $slice_duration \
		sched-entry S 02 $slice_duration \
		sched-entry S 01 $slice_duration \
		sched-entry S 01 $slice_duration \
		sched-entry S 02 $slice_duration \
		sched-entry S 01 $slice_duration \
		sched-entry S 01 $slice_duration \
		sched-entry S 02 $slice_duration \
		sched-entry S 01 $slice_duration \
		flags 0x1 \
		txtime-delay 50000 \
		clockid CLOCK_REALTIME

	sudo ip netns exec $namespace tc qdisc replace dev $interface parent 100:1 etf \
		clockid CLOCK_TAI \
		delta 50000 \
		offload \
		skip_sock_check
}

setup_files(){
    mkdir -p "interface_files/in" "interface_files/out"
    cp -r "interface_files/templates/in" "interface_files/"
    cp -r "interface_files/templates/out/" "interface_files/"
}

setup_files

if [ "$MODE" == "v" ]; then
	echo "Virtual mode"
	#create bridge
	sudo ip link add br0 type bridge
	sudo ip link set br0 up
	for ((ID=0;ID<N_NODES;ID++)); do
		echo "Setting up node $ID ..."
		sudo ip netns add "ns$ID"
		sudo ip link add "veth$ID" type veth peer name "vethpeer$ID"
		sudo ip link set "veth$ID" netns "ns$ID"
		sudo ip netns exec "ns$ID" ip link set dev "veth$ID" up
		sudo ip link set dev "vethpeer$ID" up
		sudo ip link set "vethpeer$ID" master br0
	done
fi

# semi hardcoded towards SuperMicro interface names
if [ "$MODE" != "v" ]; then
	# for eno2, eno3, eno4
	for ((ID=0;ID<N_NODES;ID++)); do
		interface=eno$((ID+2))
		echo "Setting up node $ID ..."
		if ! ip netns list | grep -q "ns$ID"; then
			sudo ip netns add "ns$ID"
		fi
		if ip link | grep -q $interface; then
			sudo ip link set $interface netns "ns$ID"
			sudo ip netns exec "ns$ID" ip link set dev $interface up
			sudo ip netns exec "ns$ID" ip address add "192.168.178.20$ID/24" dev $interface
            sudo ip netns exec "ns$ID" ip route add default via 192.168.178.1 dev $interface
		fi
        # reset qdisc
        sudo ip netns exec ns$ID tc qdisc del dev $interface root
		if [ "$MODE" == "t" ] || [ "$MODE" == "s" ]; then
			echo "setting qdisc ..."
			if ! sudo ip netns exec ns$ID ip link | grep -q ${interface}.1; then
				sudo ip netns exec ns$ID ip link add link $interface name ${interface}.1 type vlan id 1 egress-qos-map 4:4 5:5
				sudo ip netns exec ns$ID ip link set dev ${interface}.1 up
			fi

			if [ "$MODE" == "s" ]; then
				set_qdisc $interface ns$ID
			fi
			if [ "$MODE" == "t" ]; then
				set_qdisc_tsn $interface ns$ID
			fi
		fi
	done

	if [ "$MODE" == "t" ]; then
		echo "Synchronising clocks ..."
		./synch_clocks.sh
	fi
fi

