#!/usr/bin/env bash

clockprocs=$(ps -e | grep -o "phc2sys" | wc -l)
if [  $clockprocs != "3" ]; then
	sudo killall -q phc2sys
	sudo ip netns exec ns0 phc2sys -s CLOCK_REALTIME -c eno2 -O 0 &
	sudo ip netns exec ns1 phc2sys -s CLOCK_REALTIME -c eno3 -O 0 &
	sudo ip netns exec ns2 phc2sys -s CLOCK_REALTIME -c eno4 -O 0 &
fi
