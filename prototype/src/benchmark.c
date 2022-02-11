#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include "benchmark.h"
#include "node.h"
#include "service_time.h"
#include "log.h"

#define MAX_NUM_MSGS 300

FILE* fp_sent_ts = NULL;
FILE* fp_delivered = NULL;
FILE* fp_recv_ts = NULL;

void fetch_message_bench(service_msg* msg){
	static uint64_t counter = 40;
	if(SELF_ID == 0 && counter > MAX_NUM_MSGS+counter){
		shutdown_node();
	}
	sprintf(msg->data, "%lu", counter);
	counter++;
}

void close_bench(){
	fclose(fp_sent_ts);
	fclose(fp_delivered);
	fclose(fp_recv_ts);
}

void init_benchmark(){
	char path[200];
	sprintf(path, "../benchmark/latency/latency_node_%u/sent_ts", SELF_ID);
	fp_sent_ts = fopen(path, "w");
	assert(fp_sent_ts != NULL);
	sprintf(path, "../benchmark/latency/delivery/deliver_ts");
	fp_delivered = fopen(path, "w");
	assert(fp_delivered != NULL);
	sprintf(path, "../benchmark/latency/latency_node_%u/recv_ts", SELF_ID);
	fp_recv_ts = fopen(path, "w");
	assert(fp_recv_ts != NULL);
}

void ts_sending(struct timespec* time_struct, int count){
    if(SELF_ID == 0){
        fprintf(fp_sent_ts, "%u %lus + %luns\n", count, time_struct->tv_sec, time_struct->tv_nsec);
    }
}

void ts_receive(struct timespec* time_struct, uint8_t ID, int counter){
    if(ID == 0 && SELF_ID == 2){
	counter == -1 ? fprintf(fp_recv_ts, "%u NOT RECV\n", ID) 
		: fprintf(fp_recv_ts, "%u %u %lus + %luns\n", ID, counter, time_struct->tv_sec, time_struct->tv_nsec);
    }
}

void deliver_bench(struct timespec* time_struct){
    if(SELF_ID == 0){
		fprintf(fp_delivered, "%lu %lus + %luns\n", strtoul(node_states[SELF_ID].last_msg->data, NULL, 10), time_struct->tv_sec, time_struct->tv_nsec);
    }
	free(node_states[SELF_ID].last_msg);
	node_states[SELF_ID].last_msg = NULL;
}

void document_timefail(uint64_t slice_count, int slice_fail){
    log_warn("documenting failure");
    FILE* fp_timefail = fopen("../benchmark/failures/timefail_tmp", "a");
    slice_fail? fprintf(fp_timefail, "%lu\n", slice_count)
        : fprintf(fp_timefail, "other_fail\n");
    fclose(fp_timefail);
}

