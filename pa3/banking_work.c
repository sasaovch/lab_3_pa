#include "banking.h"
#include "common.h"
#include "banking.h"
#include "pipes_const.h"
#include "time_work.h"
#include "work_with_pipes.h"

#include <string.h>

void transfer(void *data, local_id src, local_id dst, balance_t amount) {
    (void) data;
    pipe_info.local_time++;

    TransferOrder order;
    order.s_src = src;
    order.s_dst = dst;
    order.s_amount = amount;
    
    Message msg;
    msg.s_header.s_magic = MESSAGE_MAGIC;
    msg.s_header.s_payload_len = sizeof(TransferOrder);
    msg.s_header.s_type = TRANSFER;
    msg.s_header.s_local_time = get_lamport_time();

    printf("--- %d: %d transfer from %d to %d amount %d\n", get_lamport_time(), pipe_info.fork_id, src, dst, amount);

    memcpy(&msg.s_payload, &order,sizeof(TransferOrder));

    if (pipe_info.fork_id == 0) {
        Message received_message;

        send(&pipe_info, src, &msg);
        pipe_info.local_time++;

        while (receive(&pipe_info, dst, &received_message) != 0);
        sync_lamport_time(&pipe_info, received_message.s_header.s_local_time);
    }
    if (pipe_info.fork_id == src) {
        send(&pipe_info, dst, &msg);
    }
}
