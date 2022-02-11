#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "benchmark.h"
#include "service_time.h"
#include "time_util.h"
#include "log.h"

#define TURN_DURATION (CYCLE_DURATION/AMOUNT_NODES)
#define TWO_NODES_THRESHOLD 20
#define SLICES_IN_TURN 3

void set_cycle_duration(uint32_t cycle_duration){
    /* needs to be divisible by a whole (integer) slice duration*/ 
	uint32_t slices_in_cycle = AMOUNT_NODES*SLICES_IN_TURN; 
	uint32_t approx_slice_duration = cycle_duration/slices_in_cycle;
	CYCLE_DURATION = cycle_duration % slices_in_cycle ? 
		slices_in_cycle * (approx_slice_duration + 1) : cycle_duration;
}

void init_time(uint8_t amount_nodes, uint32_t cycle_duration){
    if(amount_nodes < 3 || cycle_duration < 1) {
        puts("Too few nodes or invalid cycle duration.");
        shutdown_node();
    }

	AMOUNT_NODES = amount_nodes;
    set_cycle_duration(cycle_duration);
	SLICE_DURATION = (TURN_DURATION / SLICES_IN_TURN);

	context.cycle = 0;
	context.epoch_us = calc_epoch_us();
	context.cycle_start_time = context.epoch_us;
}

void check_slice_skip(uint64_t current_epoch){
	if(current_epoch >= (context.epoch_us + context.time_remaining + SLICE_DURATION)){
		log_warn("A slice was skipped. Try increasing the cycle duration.");
		if(context.slice == SEND_SLICE
				&& current_epoch < (context.epoch_us + context.time_remaining + 2 * SLICE_DURATION)){
			log_info("Skipped slice was pause slice. Continuing ...");
		} else {
			shutdown_node();
		}
	}
}

void increase_cycle(){
    context.cycle_start_time = context.epoch_us;
    context.cycle++;
    log_info("Entering cycle number %lu (since node's first wake up)", context.cycle);
    /* so that faulty member does not block operations due to not recognizing third member: */
    static uint8_t stuck_membership = 0;
    if(memb_count == 2){
        if (stuck_membership == TWO_NODES_THRESHOLD){
        log_warn("There have been only two nodes in the membership for too long. Shutting down");
        shutdown_node();
        }
        stuck_membership++;
    } else if(stuck_membership){
        stuck_membership = 0;
    }
}

void update_time(){

#if defined(LATENCY_BENCH) || defined(DELIVERY_BENCH)
	/* such that benchmarks stops early if one node fails*/
	if(context.cycle > 50 && memb_count != 3){
		shutdown_node();
	}
#endif

	uint64_t current_epoch = calc_epoch_us();
    check_slice_skip(current_epoch);
	context.epoch_us = current_epoch;
    context.time_remaining = next_slice_time(1,current_epoch);
	uint64_t cycle_time = context.epoch_us % CYCLE_DURATION;
	context.turn = cycle_time / TURN_DURATION;
	uint64_t turn_time = cycle_time % TURN_DURATION;
	context.slice = turn_time / SLICE_DURATION;
	context.current_sender_ID = context.turn;

	if(context.cycle_start_time + CYCLE_DURATION <= context.epoch_us){
        increase_cycle();
	}
}

 /*returns time remaining to next slice*/
 /*if abs flag is set, returns absolute time when next slice commences*/
 /*receives time in us as parameter, function calculates time itself if current_epoch set to 0*/
int64_t next_slice_time(int abs, uint64_t current_epoch){
    if(!current_epoch) {    
        current_epoch = calc_epoch_us();
    }
	uint8_t next_slice = (context.slice + 1) % SLICES_IN_TURN;
	uint8_t next_turn = next_slice ? context.turn : context.turn + 1;
	uint64_t start_current_cycle = context.epoch_us - context.epoch_us % CYCLE_DURATION;
	uint64_t start_next_slice = start_current_cycle + SLICE_DURATION * (next_turn * SLICES_IN_TURN + next_slice);

	if(!abs){
		int64_t next_slice_diff = start_next_slice - current_epoch;
		if(next_slice_diff <= 0){
			log_warn("Slice computations took too long");
		}
		return next_slice_diff;
	} else {
		return start_next_slice;
	}
}

struct timespec next_slice_time_ts(int abs){
	return us_to_ts(next_slice_time(abs,0));
}


struct timeval next_slice_time_tv(int abs){
	return ts_to_tv(next_slice_time_ts(abs));	
}

void sleep_next_slice(){
	struct timespec time_struct = next_slice_time_ts(1);
	if(clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &time_struct, NULL)){
		log_error("nanosleep error\n");
	}
}
