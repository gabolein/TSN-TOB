#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include "failures.h"
#include "membership.h"
#include "service_time.h"
#include "time_util.h"
#include "node.h"
#include "interface.h"
#include "communication.h"

typedef enum {
  TIMING_FAILURE,
  CRASH_FAILURE,
  SENDER_OMISSION,
  RECEIVER_OMISSION
} FAILURE_TYPE;

/* untested */
int simulate_failure (uint64_t cycle, uint8_t nodeID, FAILURE_TYPE fail_t){
  if (context.cycle >= cycle && SELF_ID == nodeID){
    switch(fail_t){
      case TIMING_FAILURE:;
        struct timespec ts = us_to_ts(CYCLE_DURATION);
        clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
        break;

      case CRASH_FAILURE:
        shutdown_node();
        break;

      case SENDER_OMISSION:
        if(context.current_sender_ID == SELF_ID 
            && context.slice == SEND_SLICE){
          uint8_t senderID = context.current_sender_ID;
          service_msg* msg = calloc(1, sizeof(*msg));
          if(node_states[senderID].stage == MEMBER){
              prepare_msg(msg, MEMBER);
          }
          fetch_message(msg, memb_count);
          /*omitting broadcast here*/
          free(node_states[senderID].last_msg);
          node_states[senderID].last_msg = msg;
          sleep_next_slice();
        }
        break;

      case RECEIVER_OMISSION:
        if(context.current_sender_ID != SELF_ID 
            && context.slice == SEND_SLICE){
          uint8_t senderID = context.current_sender_ID;
          update_view(senderID, NULL);
          free(node_states[senderID].last_msg);
          node_states[senderID].last_msg = NULL;
          if(node_states[senderID].stage == ACTIVE){
              node_states[senderID].stage = INACTIVE;
          } else if(node_states[senderID].stage == MEMBER 
                  && node_states[SELF_ID].stage != MEMBER){
              remove_membership(senderID);
          }
          sleep_next_slice();
        }
      }
  }
  return 0;
}


void document_failure(uint8_t nodeID, uint64_t slices){
  FILE* fp = fopen("../benchmark/failures/recv_fail", "a");
  fprintf(fp, "node:%u,slice:%lu\n", nodeID, slices);
}
