#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "service_time.h"
#include "membership.h"
#include "node.h"
#include "communication.h"
#include "interface.h"
#include "log.h"
#if defined(LATENCY_BENCH) || defined(DELIVERY_BENCH)
#include "benchmark.h"
#endif


#define MAX_NODES 8

void init_service(uint8_t id, uint8_t amount_nodes, uint32_t cycle_duration, int verbose, mode mode){
    if(amount_nodes > MAX_NODES){
        puts("service_msg struct currently cannot handle more than 8 nodes (vector is a byte)");
        shutdown_node();
    }
    SELF_ID = id; /*global, constant (or rather "final"...)*/
#if defined(LATENCY_BENCH) || defined(DELIVERY_BENCH)
    init_benchmark();
#endif
    init_log(verbose);
    init_time(amount_nodes, cycle_duration);
    init_membership(amount_nodes);
    init_broadcasting(mode);
    init_interface();
    fcntl(0, F_SETFL, O_NONBLOCK); /* non blocking stdin for easy graceful shutdown, hacky*/
}

int main(int argc, char *argv[]){
    if(argc < 5){
        wrong_usage_log();
        return 0;
    }

    mode mode = VIRTUAL;
    switch(*argv[4]){
        case 'v': 
            mode = VIRTUAL;
            break;
        case 'b': 
            mode = BEST_EFFORT;
            break;
        case 's':
        case 't':
            mode = SCHEDULED;
            break;
        default:
            puts("Invalid mode. Modes: v: virtual, b: best-effort, t or s: scheduled (tsn)");
            return 0;
    }

    int verbose = argc > 5 && !strncmp(argv[5], "-verbose", 10);
    init_service(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), verbose, mode); 
    log_info("Starting node %u ...\n", SELF_ID);


    for(;;){
#if !defined(LATENCY_BENCH) && !defined(DELIVERY_BENCH)
        /*for graceful shutdown when running program interactively*/
        if(getchar() == 'q'){
            shutdown_node();
        }
#endif
        update_time();
        slice_switch();
        sleep_next_slice();
    }
}
