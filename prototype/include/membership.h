#ifndef MEMBERSHIP_H
#define MEMBERSHIP_H

#include "service_time.h"

#define MIN_MEMBERS 3

// NO_INFO only for view matrix
typedef enum{
    NO_INFO,
    NOT_RECEIVED,
    RECEIVED
} node_information;

typedef enum{
    INACTIVE,
    ACTIVE,
    MEMBER
} node_stage;

typedef struct{
    uint8_t suspect_flag;
    uint8_t trigger_ID;
} suspicion;

typedef struct service_msg service_msg;
typedef struct{
    node_stage stage;
    service_msg* last_msg;
    node_information* node_vv;
    suspicion sus;
} node_state;

node_state* node_states; 
/* sorry */
int membership_flag;
int memb_count;

void init_membership(uint8_t);
uint8_t check_active_node(void);
void enter_membership(uint8_t);
void membership_decision(uint8_t);
uint8_t mval(uint8_t);
uint8_t check_receiver_omission(uint8_t);
uint8_t check_sender_omission(uint8_t);
void update_view(uint8_t, uint8_t*);

#endif // MEMBERSHIP_H
