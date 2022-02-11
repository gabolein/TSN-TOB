#include "interface.h"
#include "node.h"
#include "communication.h"
#include "log.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/*return file pointer to dir in requested mode*/
FILE* get_node_file(char* dir, char* mode){
	// generate path string
	char path[100];
	sprintf(path, "../interface_files/%s/%u", dir, SELF_ID);
	return fopen(path, mode);
}

static FILE* to_send = NULL;
static FILE* node_file = NULL;
void init_interface(){
	to_send = get_node_file("in", "r");
	node_file = get_node_file("out", "a");
}

void close_interface(){
    if(to_send){
        fclose(to_send);
    }
    if(node_file){
        fclose(node_file);
    }
}

void set_empty_msg(service_msg* msg){
    *(msg->data) = '\0';
}

void fetch_message(service_msg* msg, uint8_t memb_count){
    if(memb_count < MIN_MEMBERS) {
        log_trace("Not enough nodes to deliver a message. Sending empty msg");
        set_empty_msg(msg);
        return;
    } 
	char* line = NULL;
	size_t len = 0;
	log_trace("Taking line from file interface_files/in/%u, adding it to msg.", SELF_ID);
	if(getline(&line, &len, to_send) >= 0){
		assert(line != NULL);

		if(len){
			strncpy(msg->data, line, DATA_SIZE);
		} 
		if(line){
			free(line);
		}
	} else {
			log_trace("No line found in input file. Sending empty msg");
            set_empty_msg(msg);
	}
}

/*adds last message received from nodeID to the nodes delivery file*/
void deliver_message(uint8_t nodeID){
/*do not deliver if empty msg*/
	if(*(node_states[nodeID].last_msg->data) == '\0'){
		log_info("msg received from node %u was empty. Not delivering");
		return;
	}
	log_info("Writing last message from %u to file interface_files/out/%u", nodeID, SELF_ID);
	print_output_file(node_file, nodeID, node_states[nodeID].last_msg->data);
	free(node_states[nodeID].last_msg);
	node_states[nodeID].last_msg = NULL;
}

/* Can be used to change input file dynamically, not recommended as it is very slow*/
void file_delete_line(uint8_t ID, FILE* file_fp){
	pid_t process_id = fork();
	if(!process_id){
		char command[50];
		sprintf(command, "sed -i '1d' ../interface_files/in/%u &", ID); 
		system(command);
		fclose(file_fp);
		exit(0);
	}
}
