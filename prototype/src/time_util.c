#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include "time_util.h"

uint64_t calc_epoch_us(){
    struct timespec time_struct;
	clock_gettime(CLOCK_REALTIME, &time_struct);
	uint64_t s_in_us = time_struct.tv_sec * 1e6;
	uint64_t ns_in_us = time_struct.tv_nsec / 1e3;
	return s_in_us + ns_in_us;
}
struct timeval us_to_tv(int64_t time_span){
	struct timeval time_struct = {0, 0};
	if(time_span > 0){
		time_struct.tv_sec = time_span / 1e6;
		time_struct.tv_usec = time_span - time_struct.tv_sec*1e6;
	}
	return time_struct;
}

struct timespec us_to_ts(int64_t time_span){
	struct timespec time_struct = {0, 0};
	if(time_span > 0){
		time_struct.tv_sec = time_span / 1e6;
		time_struct.tv_nsec = time_span*1e3 - time_struct.tv_sec*1e9;
	}
	return time_struct;
}

struct timeval ts_to_tv(struct timespec ts){
	struct timeval tv = {ts.tv_sec, ts.tv_nsec/1e3};
	return tv;
}
