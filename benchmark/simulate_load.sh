#!/usr/bin/env bash


traffic_load=$1
N_NODES=3
ip=192.168.178.77

sudo killall -q iperf3
iperf3 -s -p 5203 > /dev/null &
iperf3 -s -p 5204 > /dev/null &
iperf3 -s -p 5205 > /dev/null &

if [ $(ip netns list | wc -l) != 3 ]; then
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
		fi
	done
fi

sudo ip netns exec ns0 iperf3 -c $ip -p 5203 -u -b ${traffic_load}m -t 20000 > /dev/null &
sudo ip netns exec ns1 iperf3 -c $ip -p 5204 -u -b ${traffic_load}m -t 20000 > /dev/null &
sudo ip netns exec ns2 iperf3 -c $ip -p 5205 -u -b ${traffic_load}m -t 20000 > /dev/null &
