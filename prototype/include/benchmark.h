#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "node.h"
#include "service_time.h"
#include "communication.h"

void fetch_message_bench(service_msg*);
void init_benchmark(void);
void close_bench(void);
void ts_sending(struct timespec*, int);
void deliver_bench(struct timespec*);
void ts_receive(struct timespec*, uint8_t, int);
void document_timefail(uint64_t, int);

#endif // BENCHMARK_H
