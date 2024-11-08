#include "time_work.h"
#include "pipes_const.h"
#include "banking.h"

timestamp_t get_lamport_time() {
    return pipe_info.local_time;
}

void sync_lamport_time(void *__info, timestamp_t t) {
    Info *info = (Info *) __info;
    if (info->local_time > t) {
        info->local_time++;
    } else {
        info->local_time = t + 1;
    }
}
