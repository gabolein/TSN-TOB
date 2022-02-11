#ifndef NODE_H
#define NODE_H

#include <stdint.h>

typedef enum{
	MEMBERSHIP_SLICE,
	SEND_SLICE,
	PAUSE_SLICE
} slice_type;

void shutdown_node(void);
void slice_switch(void);

uint8_t SELF_ID;

#endif // NODE_H
