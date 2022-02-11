#ifndef INTERFACE_H
#define INTERFACE_H

#include "communication.h"
#include <stdint.h>
#include <stdio.h>

void init_interface(void);
void close_interface();
void deliver_message(uint8_t);
FILE* get_node_file(char*, char*);
void fetch_message(service_msg*, uint8_t);

#endif // INTERFACE_H


