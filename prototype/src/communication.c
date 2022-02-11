#define _GNU_SOURCE  
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <poll.h>
#include "communication.h"
#include "service_time.h"
#include "time_util.h"
#include "interface.h"
#include "log.h"
#include "benchmark.h"

#define BROADCAST_TRAFFIC_ID 5
#define MAX_RECV_DELAY SLICE_DURATION/10 /* chosen arbitrarily */

void broadcast_message(service_msg* msg){
	log_info("broadcasting message");
	if(node_states[SELF_ID].stage == MEMBER){
		log_info("Node %u sending msg '%s'", SELF_ID, msg->data);
	}
#if defined(LATENCY_BENCH) || defined(DELIVERY_BENCH)
	if(memb_count >= MIN_MEMBERS){
		struct timespec time_struct = {};
		clock_gettime(CLOCK_REALTIME, &time_struct);
		ts_sending(&time_struct, atoi(msg->data));
	}
#endif
	/* send twice (reduces  probability of omission failure)*/
	if(sendto(g_socket, msg, sizeof(*msg), 0, (struct sockaddr*)&g_addr, sizeof(g_addr)) < 0){
		log_warn("FIRST BROADCAST FAILED");
	}
	if(sendto(g_socket, msg, sizeof(*msg), 0, (struct sockaddr*)&g_addr, sizeof(g_addr)) < 0){
		log_warn("SECOND BROADCAST FAILED");
	}
}

int set_recv_timeout(struct timespec* ts){
	uint64_t time_to_next_slice;
	if((time_to_next_slice = next_slice_time(0, 0)) <= 0){
		return -1;
	} else {
		*ts = us_to_ts(time_to_next_slice+MAX_RECV_DELAY);
		return 0;
	}
}

 /* TODO msgs that arrived too late may get read in next cycle? */
 /*May be solved through sequence numbers*/
int receive_message(service_msg* buffer){
	struct pollfd fd;
	int poll_retv;
	fd.fd = g_socket;
	fd.events = POLLIN;
	struct timespec ts;
	if(set_recv_timeout(&ts)){
		log_warn("error in setting timeout. will not attempt to receive");
		return -1;
	}
	int correct_sender = 0;
	while(!correct_sender){
		switch(poll_retv = ppoll(&fd, 1, &ts, NULL)){
			case -1:
				log_warn("poll error in receiving, will not attempt to receive");
				return -1;
			case 0:
#ifdef LATENCY_BENCH
				if(memb_count >= MIN_MEMBERS){
					ts_receive(NULL, context.current_sender_ID, -1);
					//shutdown_node();
				}
#endif
				log_info("Receive timed out.");
				return -1;
			default:
				recv(g_socket, buffer, sizeof(*buffer), 0);
				if(buffer->nodeID == context.current_sender_ID){
					correct_sender = 1;
#ifdef LATENCY_BENCH
					if(memb_count >= MIN_MEMBERS){
						struct timespec ts;
						clock_gettime(CLOCK_REALTIME, &ts);
						ts_receive(&ts, context.current_sender_ID, atoi(buffer->data));
					}
#endif
					log_info("Received msg from node %u", buffer->nodeID);
				}
				break;
		}
	}
	return 1;
}

void array_to_byte(uint8_t* vector){
	*vector = 0;
	for(uint8_t nodeID = 0; nodeID < AMOUNT_NODES; nodeID++){
		if(node_states[SELF_ID].node_vv[nodeID] == RECEIVED){
			*vector |= 1 << nodeID;
		}
	}
}

void prepare_msg(service_msg* msg, node_stage stage){
	msg->nodeID = SELF_ID;
	msg->stage = stage;
	msg->sus = node_states[SELF_ID].sus;	
	array_to_byte(&msg->node_vector);
}

 /*inits socket and address*/
 /*TODO: so far interface names are hard coded*/
void init_broadcasting(mode mode){
	// init global socket
	if((g_socket = socket(AF_PACKET, SOCK_DGRAM, htons(0x88b5))) == -1){
		perror("socket");
	}

	// build ifr struct, needed to "bind" to interface
	char if_name[10];

	switch(mode){
		case VIRTUAL:
			snprintf(if_name, 10, "veth%u", SELF_ID);
			break;
		case BEST_EFFORT:
			snprintf(if_name, 10, "eno%u", SELF_ID+2);
			break;
		case SCHEDULED:
			snprintf(if_name, 10, "eno%u.1", SELF_ID+2);
			int prio = BROADCAST_TRAFFIC_ID;
			setsockopt(g_socket, SOL_SOCKET, SO_PRIORITY, &prio, sizeof(prio));
			break;
		default:
            puts("Invalid mode");
            shutdown_node();
			break;
	}
	log_trace("connecting to interface %s", if_name);
	struct ifreq ifr;
	strcpy(ifr.ifr_name, if_name);
	if(ioctl(g_socket, SIOCGIFINDEX, &ifr) == -1){
		perror("ioctl");
	}

	g_addr.sll_family = AF_PACKET;
	g_addr.sll_ifindex = ifr.ifr_ifindex;
	g_addr.sll_halen = ETHER_ADDR_LEN;
	g_addr.sll_protocol = htons(0x88b5);
     /*11111111111... eth broadcast*/
	uint8_t dest[] = {0xff,0xff,0xff,0xff,0xff,0xff};
	memcpy(g_addr.sll_addr, dest, ETHER_ADDR_LEN);
}
