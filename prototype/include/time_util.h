#ifndef TIME_UTIL_H
#define TIME_UTIL_H
#include <stdint.h>

struct timeval us_to_tv(int64_t);
struct timespec us_to_ts(int64_t);
struct timeval ts_to_tv(struct timespec);
uint64_t calc_epoch_us();

#endif // TIME_UTIL_H
