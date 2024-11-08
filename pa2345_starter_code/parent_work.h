#pragma once
#ifndef  __PARENT_WORK_H
#define  __PARENT_WORK_H

#include "ipc.h"
#include "pipes_const.h"

int init_parent_work(void *__info, int N);
void do_parent_work(void *__info, int N);
void print_history_from_all_children(void *__info, int N);
void parent_are_waiting(void *__info, int N);

#endif
