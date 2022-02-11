#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "benchmark.h"
#include "membership.h"
#include "node.h" 
#include "interface.h"
#include "log.h"

/*init view vectors*/
void init_membership(uint8_t amount_nodes){
    membership_flag = 0;
    memb_count = 0;
    node_states = calloc(amount_nodes, sizeof(*node_states));
    for(int i = 0; i < amount_nodes; i++){
        node_states[i].node_vv = calloc(amount_nodes, sizeof(node_information));
    }
}

uint8_t check_active_node(){
    for(int i = 0; i < AMOUNT_NODES; i++){
       if(node_states[i].stage == ACTIVE && i != SELF_ID){
           log_trace("Node %u is joining. Cannot join.", i);
           return 1;
       }
    }
    return 0;
}


int check_ack_init_msg(uint8_t nodeID){
    for(uint8_t i = 0; i < AMOUNT_NODES; i++){
        log_trace("Checking if member %u acknowledges node %u ...", i, nodeID);
        if(node_states[i].stage == MEMBER 
                && node_states[i].node_vv[nodeID] != RECEIVED){
            log_warn("NODE %u FAILED TO ENTER MEMBERSHIP. MEMBER DOES NOT RECEIVE MESSAGE", nodeID);
            return -1;
        }
    }
    return 0;
}

int establish_membership(uint8_t nodeID){
    for(uint8_t i = 0; i < AMOUNT_NODES; i++){
        if(node_states[i].stage != INACTIVE && node_states[i].node_vv[nodeID] == RECEIVED){
            return 0;
        }
    }
    return -2;
}

void enter_membership(uint8_t new_node){
    int join = membership_flag ? check_ack_init_msg(new_node) : establish_membership(new_node);
    switch(join){
        case 0:
            log_info("Node %u entering membership", new_node);
            node_states[new_node].stage = MEMBER;
            membership_flag = 1;
            memb_count++;
            break;
        case -1:
            log_warn("NODE %u FAILED TO ENTER MEMBERSHIP. MEMBER DOES NOT RECEIVE MESSAGE", new_node);
            if(new_node == SELF_ID){
                shutdown_node();
            }
            break;
        case -2:
            log_warn("NODE %u FAILED TO ENTER MEMBERSHIP. NO OTHER NODES RECV MESSAGE", new_node);
            /* no shutdown because this case occurs when other nodes have not been started yet */
            break;
        default:
            /* dead */
            break;
    }
}

#define MOD(a,b) ((((a)%(b))+(b))%(b))
// returns nodeID if no prev member found
uint8_t get_prev_member(uint8_t nodeID){
    for(uint8_t i = MOD((int)nodeID-1,AMOUNT_NODES); i != nodeID; i = MOD((int)i-1,AMOUNT_NODES)){
        if(node_states[i].stage == MEMBER){
            return i;
        }
    }
    return nodeID;
}

uint8_t get_next_member(uint8_t nodeID){
    for(uint8_t i = (nodeID+1)%AMOUNT_NODES; i != nodeID; i = (i+1)%AMOUNT_NODES){
        if(node_states[i].stage == MEMBER){
            return i;
        }
    }
    return nodeID;
}

void remove_membership(uint8_t nodeID){
    uint8_t prev_member = get_next_member(nodeID);
    if(node_states[prev_member].sus.suspect_flag 
            && (node_states[prev_member].sus.trigger_ID == nodeID)){
        log_trace("Removing suspicion of node %u", prev_member);
        node_states[prev_member].sus.suspect_flag = 0;
    }
    node_states[nodeID].stage = INACTIVE;
    memb_count--;
}

/*----------------------*/
/*-MEMBERSHIP DECISIONS-*/
/*----------------------*/

uint8_t mval(uint8_t nodeID){
    return check_sender_omission(nodeID) && check_receiver_omission(nodeID);
}

void handle_suspicion(uint8_t nodeID){
    // if predecessor experienced sender omission, predecessor must be shut off by now
    uint8_t prev_member = node_states[nodeID].sus.trigger_ID;
    log_debug("prev member is %u", prev_member);
    if(node_states[prev_member].stage != INACTIVE){
        log_warn("NODE %u HAD RECEIVER OMISSION FAILURE LAST CYCLE. SHUT DOWN", nodeID);
        remove_membership(nodeID);
        if(nodeID == SELF_ID){
            shutdown_node();
        }
        return;
    } else{
        // no receiver omission failure by node ID
        node_states[nodeID].sus.suspect_flag = 0;
    }
}

void membership_decision(uint8_t nodeID){
    // in case of suspected receiver omission failure in last cycle
    if(node_states[nodeID].sus.suspect_flag){
        log_trace("Checking node %u suspicion ...", nodeID);
        handle_suspicion(nodeID);
    }
    if(mval(nodeID)){
        if(node_states[SELF_ID].node_vv[nodeID] == RECEIVED || SELF_ID == nodeID){
            log_trace("%u is still a member, delivering its last message ...", nodeID);
#ifdef DELIVERY_BENCH
            if(memb_count >= MIN_MEMBERS && nodeID == 0){
                struct timespec ts;
                clock_gettime(CLOCK_REALTIME, &ts);
                deliver_bench(&ts);
            }
#elif !defined(LATENCY_BENCH)
            deliver_message(nodeID);
#endif
        }
    } else {
        log_warn("NODE %u FAILURE. NODE SHOULD BE SHUTTING DOWN", nodeID);
        remove_membership(nodeID);
        if(nodeID == SELF_ID){
            shutdown_node();
        }
    }
}

uint8_t compare_vv(uint8_t prevID, uint8_t nodeID){
    for(uint8_t i = 0; i < AMOUNT_NODES; i++){
        if(i != prevID && i != nodeID){
            if(node_states[prevID].node_vv[i] == RECEIVED 
                    && node_states[nodeID].node_vv[i] != RECEIVED){
                log_warn("NODE %u DID NOT RECEIVE MESSAGE THAT GOT RECEIVED BY PREDECESSOR. NOT RECEIVED ID:%u", nodeID, i);
                return -1;
            }
        }
    }
    return 0;
}

/* checks whether a missing message can be safely accredited to 
 * nodeID's receiver omission failure*/
uint8_t check_receiver_omission(uint8_t nodeID){
    /* rely on view vector of previous member for decision. if received */
    uint8_t prev_member = get_prev_member(nodeID);
    if(node_states[nodeID].node_vv[prev_member] == RECEIVED){
        if(compare_vv(prev_member, nodeID) < 0){
            return 0;
        } else {
            /* might have still missed active node if prevID < active id < node id*/
            /*but: if more than two members should notice in next cycle*/
            /* if 2 members, will shutdown eventually */
        }

    } else {
        /*check if another member message was lost*/
        for(uint8_t senderID = 0; senderID < AMOUNT_NODES; senderID++){
            if(senderID != nodeID && senderID != prev_member && node_states[senderID].stage == MEMBER){
                if(node_states[nodeID].node_vv[senderID] == NOT_RECEIVED){
                    log_warn("NODE %u MISSED TWO MESSAGES FROM MEMBERS. FAILURE BY FAULT HYPOTHESIS", nodeID);
                    return 0;
                }
            }
        }
        /*node gets here if prev had sender omission failure*/
        /*cannot say yet who is responsible, needs to be dealt with next cycle*/
        log_warn("NODE %u ENTERING SUSPICIOUS MODE", nodeID);
        node_states[nodeID].sus.suspect_flag = 1;
        node_states[nodeID].sus.trigger_ID = prev_member;
    }
    return 1;
}

uint8_t count_omitted_receptions(uint8_t nodeID){
    // if more than 1 member did not receive, sender must be at fault 
    uint8_t amount_not_received = 0;
    for(uint8_t view_vector_ID = 0; amount_not_received < 2 && view_vector_ID < AMOUNT_NODES; view_vector_ID++){
        if(node_states[view_vector_ID].stage == MEMBER 
                && nodeID != view_vector_ID 
                && node_states[view_vector_ID].node_vv[nodeID] == NOT_RECEIVED){
            amount_not_received++;
            log_trace("%u did not receive last message from %u", view_vector_ID, nodeID);
        }
    }
    if(amount_not_received > 1) {
        log_warn("NODE %u SENDER OMISSION FAILURE", nodeID);
    }
    return amount_not_received < 2;
}

// returns 0 if more than one node didnt receive senders last message
uint8_t check_sender_omission(uint8_t nodeID){
    log_trace("Checking for sender omission failure by node %u", nodeID);
    return (nodeID != SELF_ID 
            && node_states[SELF_ID].node_vv[nodeID] == RECEIVED) ? 1 : 
        count_omitted_receptions(nodeID);  
}

void update_view(uint8_t nodeID, uint8_t* vector){
    if(vector == NULL){
        node_states[SELF_ID].node_vv[nodeID] = NOT_RECEIVED;
        node_information* unknown = calloc(AMOUNT_NODES, sizeof(node_information));
        memcpy(node_states[nodeID].node_vv, unknown, AMOUNT_NODES*sizeof(node_information));
        free(unknown);
    } else {
        // convert byte to array
        for(uint8_t i = 0; i < AMOUNT_NODES; i++){
            node_states[nodeID].node_vv[i] = (*vector >> i) & 1 ? 
                RECEIVED : NOT_RECEIVED; 
        }
        node_states[SELF_ID].node_vv[nodeID] = RECEIVED;
    }
}

