#include "failures.h"
#include "node.h"
#include "service_time.h"
#include "membership.h"
#include "communication.h"
#include "interface.h"
#include "log.h"
#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

void shutdown_node(){
	log_info("Shutting down node %u", SELF_ID);
	for(uint8_t nodeID = 0; nodeID < AMOUNT_NODES; nodeID++){
		free(node_states[nodeID].node_vv);
		free(node_states[nodeID].last_msg);
	}
	free(node_states);
	close(g_socket);
#if defined(LATENCY_BENCH) || defined(DELIVERY_BENCH)
	close_bench();
#endif
    close_interface();
	exit(0);
}

void send_operation(){
    service_msg* msg = calloc(1, sizeof(*msg));

    if(node_states[SELF_ID].stage == MEMBER){
        prepare_msg(msg, MEMBER);
#if !defined(LATENCY_BENCH) && !defined(DELIVERY_BENCH)
        fetch_message(msg, memb_count);
#else
        if(memb_count >= MIN_MEMBERS){
            fetch_message_bench(msg);
        }
#endif
        broadcast_message(msg);
        /*store msg for delivery in senders next membership slice*/
        free(node_states[SELF_ID].last_msg);
        node_states[SELF_ID].last_msg = msg;

    /*must have had the chance to recv every vv*/
    /*initialization in order to enter only if no other member is trying to join*/
    /*or if membership has not been established yet */
    } else if(context.cycle > 0 
            && (!membership_flag || !check_active_node())){  
        /*initialization message*/
        prepare_msg(msg, ACTIVE);
        broadcast_message(msg);
        node_states[SELF_ID].stage = ACTIVE;
        /*no need to keep the msg, won't be delivered*/
        free(msg);
    } else {
        log_trace("Too early to send, or other node is joining");
        log_trace("Membership flag is: %u", membership_flag);
        free(msg);
    }
}

void receive_operation(uint8_t senderID){
    service_msg* msg = malloc(sizeof(*msg));
    if(receive_message(msg) > 0){
        update_view(senderID, &msg->node_vector);
        /*if recv not member, recv must acknowledge members like this*/
        if(node_states[SELF_ID].stage != MEMBER){
            if(msg->stage == MEMBER) { 
                membership_flag = 1;
                if(node_states[senderID].stage != MEMBER){
                    memb_count++;
                }
            }
            node_states[senderID].stage = msg->stage; 
        } else if(node_states[senderID].stage == INACTIVE){
            node_states[senderID].stage = ACTIVE;
        }
        free(node_states[senderID].last_msg);
        node_states[senderID].last_msg = msg;
        node_states[senderID].sus = msg->sus;
    } else{
        update_view(senderID, NULL);
        free(node_states[senderID].last_msg);
        node_states[senderID].last_msg = NULL;
        if(node_states[senderID].stage == ACTIVE){
            node_states[senderID].stage = INACTIVE;
        /*receiving node assumes that sending node just left membership,*/
        /*if it didnt receiving node will not enter membership anyway*/
        } else if(node_states[senderID].stage == MEMBER 
                && node_states[SELF_ID].stage != MEMBER){
            remove_membership(senderID);
        }
        free(msg);
    }
}

void slice_switch(){
	uint8_t senderID = context.current_sender_ID;
	switch(context.slice){

		case MEMBERSHIP_SLICE:
			log_trace("node %u entering MEMBERSHIP SLICE of sender %u.", SELF_ID, senderID);

			if (node_states[senderID].stage == MEMBER 
					&& node_states[SELF_ID].stage == MEMBER && memb_count >= MIN_MEMBERS){
				membership_decision(senderID);
			} else if (node_states[senderID].stage == ACTIVE 
					&& (node_states[SELF_ID].stage == MEMBER || senderID == SELF_ID)){
				enter_membership(senderID);
			} else {
				// SENDER HAS NOT SENT MESSAGE YET, OR RECEIVER DOES NOT HAVE ENOUGH INFO
			}

			break;
		
		case SEND_SLICE:
            if(senderID == SELF_ID){
                log_trace("node %u entering SEND SLICE of sender %u.", SELF_ID, SELF_ID);
                send_operation();
			} else{
                log_trace("node %u entering RECEIVE SLICE of sender %u.", SELF_ID, senderID);
                receive_operation(senderID);
			}
			break;

		default:
			log_trace("node %u entering PAUSE SLICE.", SELF_ID);
			break;
	}
}
