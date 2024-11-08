#include "banking.h"
#include "common.h"
#include "banking.h"
#include "pipes_const.h"
#include "time_work.h"
#include "work_with_pipes.h"

#include <string.h>

void transfer(void *data, local_id src, local_id dst, balance_t amount) {
    // Info *info = (Info *) data;
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
        // Message received_message;
        // int16_t type = -1;

        // send_to_pipe(&pipe_info, &msg, src);
        
        pipe_info.local_time++;
        // type = receive_any(&pipe_info, &received_message);
            
        // fprintf(elf, "Parent got message with type: %d\n", type);
        // fflush(elf);
        // fprintf(elf, "Parent got message with payload: %s\n", received_message.s_payload);
        // fflush(elf);

        // if (received_message.s_header.s_type == ACK) {
        //     printf("--- %d: %d parent received from %d to %d at time %d\n", get_lamport_time(), pipe_info.fork_id, src, dst, received_message.s_header.s_local_time);
        //     sync_lamport_time(&pipe_info, received_message.s_header.s_local_time);
        //     printf("--- %d: %d updated time\n", get_lamport_time(), pipe_info.fork_id);

        //     fprintf(elf, "Parent received ACK message\n");
        //     fflush(elf);
        //     type = ACK;
        //     return;
        // }
    }
    if (pipe_info.fork_id == src) {
        send_to_pipe(&pipe_info, &msg, dst);
    }
}
