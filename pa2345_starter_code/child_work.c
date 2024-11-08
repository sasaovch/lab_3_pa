#include "child_work.h"
#include "ipc.h"
#include "pipes_const.h"
#include "time_work.h"
#include "work_with_pipes.h"
#include "common.h"
#include "pa2345.h"
#include "banking.h"

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>


int init_child_work(void* __child_state) {

    ChildState* child_state = (ChildState *) __child_state;
    local_id child_id = child_state->fork_id;
    int N = child_state->N;

    fprintf(elf, "-------------------- IN INIT CHILD 1 %d --------------\n", child_id);
    fflush(elf);

    fprintf(stdout, "-------------------- IN INIT CHILD 2 %d --------------\n", child_id);
    fflush(stdout);                

    pipe_info.fork_id = child_id;
    pipe_info.N = N;
    pipe_info.local_time = child_state->child_time;

    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 10; j++) {
            for(int k = 0; k < 2; k++) {
                pipe_info.pm[i][j][k] = pm[i][j][k];
            }
        }
    }

    // Info child_info = {.fork_id = child_id, .N = N, .local_time = child_state->child_time };
    // for(int i = 0; i < 10; i++) {
    //     for(int j = 0; j < 10; j++) {
    //         for(int k = 0; k < 2; k++) {
    //             child_info.pm[i][j][k] = pm[i][j][k];
    //         }
    //     }
    // }

    timestamp_t time_started = get_lamport_time();

    fprintf(elf, log_started_fmt, time_started, child_id, getpid(), getppid(), child_state->balance_history.s_history[child_state->balance_history.s_history_len - 1].s_balance);
    fflush(elf);

    fprintf(stdout, log_started_fmt, time_started, child_id, getpid(), getppid(), child_state->balance_history.s_history[child_state->balance_history.s_history_len - 1].s_balance);
    fflush(stdout);

    local_id line = 0;
    local_id column = 0;

    while (line < N) {
        column = 0;
        while (column < N) {
            if (line == column) {
                column++;
            } else {
                int to_close;
                
                if (column != child_id && pm[line][column][0] != -1) {
                    to_close = pm[line][column][0];
                    pm[line][column][0] = -1;
                    close(to_close);                    
                }
                
                if (line != child_id && pm[line][column][1] != -1) {
                    to_close = pm[line][column][1];
                    pm[line][column][1] = -1;
                    close(to_close);
                }
                
                column++;
            }
        }
        line++;
    }

    fprintf(elf, "-------------------- CLOSED PIPES 1 %d --------------\n", child_id);
    fflush(elf);

    fprintf(stdout, "-------------------- CLOSED PIPES 2 %d --------------\n", child_id);
    fflush(stdout);                    

    Message start_msg;
    char start_message[MAX_PAYLOAD_LEN];
    
    sprintf(start_message, log_started_fmt, time_started, child_id, getpid(), getppid(), child_state->balance_history.s_history[child_state->balance_history.s_history_len - 1].s_balance);
    memset(start_msg.s_payload, '\0', sizeof(char)*(MAX_PAYLOAD_LEN));
    memcpy(start_msg.s_payload, start_message, sizeof(char)*(MAX_PAYLOAD_LEN));

    pipe_info.local_time++;

    start_msg.s_header.s_magic = MESSAGE_MAGIC;
    start_msg.s_header.s_payload_len = MAX_PAYLOAD_LEN + 1;
    start_msg.s_header.s_type = STARTED;

    fprintf(elf, "-------------------- BEFORE TIME 1 %d --------------\n", child_id);
    fflush(elf);

    fprintf(stdout, "-------------------- BEFORE TIME 2 %d --------------\n", child_id);
    fflush(stdout);
    start_msg.s_header.s_local_time = get_lamport_time();

    fprintf(elf, "-------------------- AFTER TIME 1 %d --------------\n", child_id);
    fflush(elf);

    fprintf(stdout, "-------------------- AFTER TIME 2 %d --------------\n", child_id);
    fflush(stdout);    

    // send_multicast(&pipe_info, &start_msg);

    fprintf(elf, "-------------------- SENT START 1 %d --------------\n", child_id);
    fflush(elf);

    fprintf(stdout, "-------------------- SENT START 2 %d --------------\n", child_id);
    fflush(stdout);
    
    // local_id childs = 1;
    // while (childs < N) {
    //     if (childs == child_id) {
    //         childs++;
    //         continue;
    //     }
    //     Message msg;

    //     receive(&pipe_info, childs, &msg);
    //     if (msg.s_header.s_type == STARTED && msg.s_header.s_payload_len > 0) {
    //         // printf("Child %d received payload: %s\n", child_id, msg.s_payload);
    //         sync_lamport_time(&pipe_info, msg.s_header.s_local_time);
    //         childs++;
    //     }

    //     msg.s_header.s_payload_len = 0;
    //     memset(msg.s_payload, '\0', sizeof(char)*MAX_PAYLOAD_LEN);
    // }

    fprintf(elf, "-------------------- RECEIVED START 1 %d --------------\n", child_id);
    fflush(elf);

    fprintf(stdout, "-------------------- RECEIVED START 2 %d --------------\n", child_id);
    fflush(stdout);    

    timestamp_t all_time = get_lamport_time();
    fprintf(elf, log_received_all_started_fmt, all_time, child_id);
    fflush(elf);

    fprintf(stdout, log_received_all_started_fmt, all_time, child_id);
    fflush(stdout);

    child_state->child_time = pipe_info.local_time;

    return 0;
}

//FIXME: sasaovch
void update_state(ChildState* child_state, int sum, timestamp_t transfer_time) {
    int current_time = child_state->child_time;
    int balance_history_len = child_state->balance_history.s_history_len;

    if (current_time - balance_history_len > 0) {
        balance_t past_balance = child_state->balance_history.s_history[balance_history_len - 1].s_balance;

        int index = balance_history_len;
        while (index < current_time) {
            child_state->balance_history.s_history[index] = (BalanceState) {
                .s_balance = past_balance,
                .s_time = index,
                .s_balance_pending_in = 0,
            };
            
            index++;
        }

        if (sum > 0) {
            if (current_time > transfer_time) {
                for (timestamp_t time = transfer_time; time < current_time; time++) {
                    child_state->balance_history.s_history[time].s_balance_pending_in = sum;
                }
            }
        }

        child_state->balance_history.s_history[current_time] = (BalanceState) {
            .s_balance = past_balance + sum,
            .s_time = current_time,
            .s_balance_pending_in = 0,
        };

        child_state->balance_history.s_history_len = current_time + 1;        
    } else if (balance_history_len == current_time) {     
        if (sum > 0) {
            if (current_time > transfer_time) {
                for (timestamp_t time = transfer_time; time < current_time; time++) {
                    child_state->balance_history.s_history[time].s_balance_pending_in = sum;
                }
            }
        }

        child_state->balance_history.s_history[balance_history_len] = (BalanceState) {
            .s_balance = child_state->balance_history.s_history[balance_history_len - 1].s_balance + sum,
            .s_time = current_time,
            .s_balance_pending_in = 0,
        };
        
        child_state->balance_history.s_history_len++;
    } else if (balance_history_len - current_time == 1) {
        if (sum > 0) {
            if (current_time > transfer_time) {
                for (timestamp_t time = transfer_time; time < current_time; time++) {
                    child_state->balance_history.s_history[time].s_balance_pending_in = sum;
                }
            }
        }
        child_state->balance_history.s_history[balance_history_len - 1].s_balance += sum;

    }  
}

void transfer_handler(void* __child_state, Message* msg) {
    ChildState* child_state = (ChildState *) __child_state;
    local_id child_id = child_state->fork_id;
    // int N = child_state->N;

    // printf("Child %d wants to transfer in time %d\n", child_id, child_state->child_time);

    pipe_info.local_time = child_state->child_time;

    // Info child_info = {.fork_id = child_id, .N = N, .local_time = child_state->child_time};
    // for(int i = 0; i < 10; i++) {
    //     for(int j = 0; j < 10; j++) {
    //         for(int k = 0; k < 2; k++) {
    //             child_info.pm[i][j][k] = pm[i][j][k];
    //         }
    //     }
    // }

    timestamp_t transfer_time = msg->s_header.s_local_time;
    timestamp_t current_time = get_lamport_time();
    TransferOrder *order = (TransferOrder* ) msg->s_payload;

    if (child_id == order->s_dst) {    
        fprintf(elf, log_transfer_in_fmt, current_time, order->s_dst, order->s_amount, order->s_src);
        fflush(elf);

        // fprintf(stdout, log_transfer_in_fmt, current_time, order->s_dst, order->s_amount, order->s_src);
        // fflush(stdout);

        pipe_info.local_time++;

        Message msg_n;
        msg_n.s_header.s_magic = MESSAGE_MAGIC;
        msg_n.s_header.s_payload_len = MAX_PAYLOAD_LEN + 1;
        msg_n.s_header.s_type = ACK;
        msg_n.s_header.s_local_time = get_lamport_time();
        
        printf("----- %d process %d update state in %d from %d\n", get_lamport_time(), child_id, transfer_time, order->s_amount);
        child_state->child_time = pipe_info.local_time;
        update_state(child_state, order->s_amount, transfer_time);  
        send_to_pipe(&pipe_info, &msg_n, 0);     
    
    } else {         
        fprintf(elf,log_transfer_out_fmt, current_time, order->s_src, order->s_amount, order->s_dst);
        fflush(elf);

        // fprintf(stdout,log_transfer_out_fmt, current_time, order->s_src, order->s_amount, order->s_dst);
        // fflush(stdout);

        printf("----- %d process %d update state in %d from %d\n", get_lamport_time(), child_id, transfer_time, order->s_amount);
        transfer(&pipe_info, order->s_src, order->s_dst, order->s_amount);
        child_state->child_time = pipe_info.local_time;
        update_state(child_state, -order->s_amount, transfer_time); 
    }
}

int handle_transfers(void* __child_state) {
    ChildState* child_state = (ChildState *) __child_state;
    local_id child_id = child_state->fork_id;
    // int N = child_state->N;
    
    pipe_info.local_time = child_state->child_time;
    // Info child_info = {.fork_id = child_id, .N = N, .local_time = child_state->child_time};
    // for(int i = 0; i < 10; i++) {
    //     for(int j = 0; j < 10; j++) {
    //         for(int k = 0; k < 2; k++) {
    //             child_info.pm[i][j][k] = pm[i][j][k];
    //         }
    //     }
    // }


    // Message msg_r;
    // int16_t type = -1;

    // int wait_for_others_to_stop = N - 2;
    // printf("-------- Child %d Time before transfer %d\n", child_id, get_lamport_time());
    // while (type != STOP) {
        
    //     msg_r.s_header.s_payload_len = 0;
    //     memset(msg_r.s_payload, '\0', sizeof(char)*MAX_PAYLOAD_LEN);

    //     pipe_info.local_time++;
    //     type = receive_any(&pipe_info, &msg_r);

    //     printf("$$$$$ %d: %d parent received time %d\n", get_lamport_time(), child_id, msg_r.s_header.s_local_time);
    //     sync_lamport_time(&pipe_info, msg_r.s_header.s_local_time);
    //     printf("$$$$$ %d: %d updated time\n", get_lamport_time(), child_id);
        
    //     if (type == TRANSFER) {
    //         // printf("transfer %d\n", type);
    //         child_state->child_time = pipe_info.local_time;
    //         transfer_handler(child_state, &msg_r);
    //     } else if (type == DONE) {
    //         wait_for_others_to_stop--;
    //     }
    // }

    // Message done_msg;
    // char done_message[MAX_PAYLOAD_LEN];
    timestamp_t time = get_lamport_time();

    fprintf(elf, log_done_fmt, time, child_id, child_state->balance_history.s_history[child_state->balance_history.s_history_len - 1].s_balance);
    fflush(elf);

    fprintf(stdout, log_done_fmt, time, child_id, child_state->balance_history.s_history[child_state->balance_history.s_history_len - 1].s_balance);
    fflush(stdout);
    
    // sprintf(done_message, log_done_fmt, time, child_id, child_state->balance_history.s_history[child_state->balance_history.s_history_len - 1].s_balance);
    // memset(done_msg.s_payload, '\0', sizeof(char)*(MAX_PAYLOAD_LEN));
    // memcpy(done_msg.s_payload, done_message, sizeof(char)*(MAX_PAYLOAD_LEN));

    pipe_info.local_time++;

    // done_msg.s_header.s_magic = MESSAGE_MAGIC;
    // done_msg.s_header.s_payload_len = MAX_PAYLOAD_LEN + 1;
    // done_msg.s_header.s_type = DONE;
    // done_msg.s_header.s_local_time = get_lamport_time();

    // send_multicast(&pipe_info, &done_msg);
    
    // Message msg_d;
    // while (wait_for_others_to_stop > 0) {
    //     msg_d.s_header.s_payload_len = 0;
    //     memset(msg_d.s_payload, '\0', sizeof(char)*MAX_PAYLOAD_LEN);
        
    //     pipe_info.local_time++;
    //     type = receive_any(&pipe_info, &msg_d);
    //     sync_lamport_time(&pipe_info, msg_d.s_header.s_local_time);
    //     if (type == DONE) {
    //         wait_for_others_to_stop--;
    //     }
    // }

    timestamp_t history_time = get_lamport_time();
    fprintf(elf, log_received_all_done_fmt, history_time, child_id);
    fflush(elf);

    // fprintf(stdout, log_received_all_done_fmt, history_time, child_id);
    // fflush(stdout);

    pipe_info.local_time++;
    child_state->child_time = pipe_info.local_time;
    update_state(child_state, 0, history_time);
    
    Message history_msg;
    
    memset(history_msg.s_payload, '\0', sizeof(char)*(MAX_PAYLOAD_LEN));
    memcpy(history_msg.s_payload, &(child_state->balance_history), sizeof(BalanceHistory));

    pipe_info.local_time++;

    history_msg.s_header.s_magic = MESSAGE_MAGIC;
    history_msg.s_header.s_payload_len = sizeof(BalanceHistory);
    history_msg.s_header.s_type = BALANCE_HISTORY;
    history_msg.s_header.s_local_time = get_lamport_time();

    send_to_pipe(&pipe_info, &history_msg, 0);
    child_state->child_time = pipe_info.local_time;
    return 0;
}
