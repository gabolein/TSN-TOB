#ifndef SERVICE_TIME_H
#define SERVICE_TIME_H
#include <stdint.h>
#include <time.h>
#include "node.h"

typedef struct service_time{
    uint64_t cycle; // cycle since first wake up, not absolute
    uint64_t cycle_start_time; // refers to cycle above
    uint64_t epoch_us;
    uint64_t time_remaining;
    uint8_t turn;
    slice_type slice;
    uint8_t current_sender_ID;
} service_time;

/*++++++++*/
/* global */
service_time context;

/* "final" */
uint32_t CYCLE_DURATION;
uint32_t SLICE_DURATION;
uint8_t AMOUNT_NODES;
/*++++++++*/

void init_time(uint8_t, uint32_t);
void update_time(void);
int64_t next_slice_time(int, uint64_t);
void sleep_next_slice(void);

#endif // SERVICE_TIME_H
