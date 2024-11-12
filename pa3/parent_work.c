#include "ipc.h"
#include "pipes_const.h"
#include "pa2345.h"
#include "banking.h"
#include "time_work.h"

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

int init_parent_work(void *__info, int N) {
    local_id line = 0;
    local_id column = 0;

    while (line < N) {
        column = 0;
        while (column < N) {
            if (line == column) {
                column++;
            } else {
                int to_close;
                
                if (column != 0 && pm[line][column][0] != -1) {
                    to_close = pm[line][column][0];
                    pm[line][column][0] = -1;
                    close(to_close);                    
                }
                
                if (line != 0 && pm[line][column][1] != -1) {
                    to_close = pm[line][column][1];
                    pm[line][column][1] = -1;
                    close(to_close);
                }
                
                column++;
            }
        }
        line++;
    }

    fprintf(elf, log_started_fmt, get_lamport_time(), 0, getpid(), getppid(), 0);
    fflush(elf);

    fprintf(stdout, log_started_fmt, get_lamport_time(), 0, getpid(), getppid(), 0);
    fflush(stdout);
  
    local_id child_i = 1;
    while (child_i < N) {
        Info info = {.fork_id = 0, .N = N};

        for(int i = 0; i < 10; i++) {
            for(int j = 0; j < 10; j++) {
                for(int k = 0; k < 2; k++) {
                    info.pm[i][j][k] = pm[i][j][k];
                }
            }
        }
        Message started_msg;

        int status = receive(&info, child_i, &started_msg);
        if (status != 0) continue;

        if (started_msg.s_header.s_type == STARTED) {
            fprintf(elf, "%d: parent got message: %s", get_lamport_time(), started_msg.s_payload);
            fflush(elf);

            fprintf(stdout, "%d: parent got message: %s", get_lamport_time(), started_msg.s_payload);
            fflush(stdout);

            sync_lamport_time(&pipe_info, started_msg.s_header.s_local_time);
            child_i++;
        }

        started_msg.s_header.s_payload_len = 0;
        memset(started_msg.s_payload, '\0', sizeof(char)*MAX_PAYLOAD_LEN);
    }

    fprintf(elf, log_received_all_started_fmt, get_lamport_time(), 0);
    fflush(elf);

    fprintf(stdout, log_received_all_started_fmt, get_lamport_time(), 0);
    fflush(stdout);

    return 0;
}

void do_parent_work(void *__info, int N) {
    Info *parent_info = (Info *) __info;
    bank_robbery(parent_info, N - 1);

    Message stop_msg;
    // char message[MAX_PAYLOAD_LEN];
    
    // sprintf(message, "STOP children\n");
    // memset(stop_msg.s_payload, '\0', sizeof(char)*(MAX_PAYLOAD_LEN));
    // memcpy(stop_msg.s_payload, message, sizeof(char)*(MAX_PAYLOAD_LEN));

    parent_info->local_time++;

    stop_msg.s_header.s_magic = MESSAGE_MAGIC;
    stop_msg.s_header.s_payload_len = 0;
    stop_msg.s_header.s_type = STOP;
    stop_msg.s_header.s_local_time = parent_info->local_time;

    send_multicast(parent_info, &stop_msg);

    local_id child_i = 1;
    while (child_i < N) {
        Message receive_msg;
        receive(parent_info, child_i, &receive_msg);
        sync_lamport_time(parent_info, receive_msg.s_header.s_local_time);

        if (receive_msg.s_header.s_type == DONE) {
        // && receive_msg.s_header.s_payload_len > 0) {
            fprintf(elf, "Stop parent message from %d with payload: %s\n", child_i, receive_msg.s_payload);
            fflush(elf);
            child_i++;
        }

        receive_msg.s_header.s_payload_len = 0;
        receive_msg.s_header.s_type = 0;
        // memset(receive_msg.s_payload, '\0', sizeof(char)*MAX_PAYLOAD_LEN);
    }

    fprintf(elf, log_received_all_done_fmt, get_lamport_time(), 0);
    fflush(elf);

    fprintf(stdout, log_received_all_done_fmt, get_lamport_time(), 0);
    fflush(stdout);
}

void print_history_from_all_children(void *__info, int N) {
    AllHistory a_hs;
    a_hs.s_history_len = N - 1;

    Info *parent_info = (Info *) __info;

    Message msg_hs;
    local_id child = 1;
    while (child < N) {
        int status = receive(parent_info, child, &msg_hs);
        if (status != 0) continue;

        sync_lamport_time(parent_info, msg_hs.s_header.s_local_time);
        
        if (msg_hs.s_header.s_type == BALANCE_HISTORY) {
            BalanceHistory* balance_history = (BalanceHistory*) msg_hs.s_payload;
            fprintf(elf, "Check history: %d\n", balance_history->s_id);
            fflush(elf);
            a_hs.s_history[balance_history->s_id - 1] = *balance_history;
            child++;

            msg_hs.s_header.s_payload_len = 0;
            msg_hs.s_header.s_type = 0;
            memset(msg_hs.s_payload, '\0', sizeof(char)*MAX_PAYLOAD_LEN);
        }
    }
    
    print_history(&a_hs);
}

void parent_are_waiting(void *__info, int N) {
    local_id child = 1;
    while (child < N) {
        wait(NULL);
        child++;
    }
}
