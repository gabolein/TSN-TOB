#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <netpacket/packet.h>
#include <stdint.h>
#include "membership.h"

#define MAX_NODES 8
#define DATA_SIZE 300

typedef enum {
    VIRTUAL,
    BEST_EFFORT,
    SCHEDULED
} mode;

typedef struct service_msg{
    uint8_t nodeID;
    uint8_t node_vector;  
    node_stage stage;
    suspicion sus;
    char data[DATA_SIZE];
} service_msg;

/* global, "final" */
int g_socket;
struct sockaddr_ll g_addr;

void init_broadcasting(mode);
void broadcast_message(service_msg*);
int receive_message(service_msg*);
void prepare_msg(service_msg*, node_stage);
void remove_membership(uint8_t);

#endif // COMMUNICATION_H
