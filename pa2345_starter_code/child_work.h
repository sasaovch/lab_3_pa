#pragma once
#ifndef  __CHILD_WORK_H
#define  __CHILD_WORK_H

#include "ipc.h"
#include "banking.h"

typedef struct {
    local_id fork_id;
    timestamp_t child_time;
    int N;
    BalanceHistory balance_history;
} ChildState;

int init_child_work(void* __child_state);
void update_state(ChildState* child_state, int sum, timestamp_t time);
void transfer_handler(void* __child_state, Message* msg);
int handle_transfers(void* __child_state);

#endif
