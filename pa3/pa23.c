#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "common.h"
#include "banking.h"
#include "ipc.h"
#include "pa2345.h"
#include "pipes_const.h"
#include "parent_work.h"
#include "child_work.h"
#include "work_with_pipes.h"

int is_not_child(int fork_id) {
    return fork_id == 0;
}

int main(int argc, char * argv[])
{
    int N = atoi(argv[2]) + 1;
    
    int *array = (int *) malloc((N) * sizeof(int));
    array[0] = N;
    for (int i = 1; i < N; i++) array[i] = atoi(argv[i + 2]);

    elf = fopen(events_log, "a");
    plf = fopen(pipes_log, "a");

    fprintf(elf, "-------------------- VERSION 1.5.1 --------------\n");
    fflush(elf);

    fprintf(stdout, "-------------------- VERSION 1.5.2 --------------\n");
    fflush(stdout);

    local_id line = 0;
    local_id column = 0;
    int all_pipes_number = N;

    while (line < all_pipes_number) {
        column = 0;
        while (column < all_pipes_number) {
            if (line == column) {
                pm[line][column][0] = -1;
                pm[line][column][1] = -1;
                column++;
            } else {
                int descriptors[2];
                pipe(descriptors);

                fcntl(descriptors[0], F_SETFL, fcntl(descriptors[0], F_GETFL, 0) | O_NONBLOCK);
                fcntl(descriptors[1], F_SETFL, fcntl(descriptors[1], F_GETFL, 0) | O_NONBLOCK);

                for (int i = 0; i < 2; i++) {
                    pm[line][column][i] = descriptors[i]; // 0 - read and 1 - write
                }

                fprintf(plf, "Pipe %d -> %d. Fd %d -> %d\n", line, column, descriptors[0], descriptors[1]);
                fflush(plf);
                column++;
            }
        }
        line++;
    }

    local_id number_id = 1;
    while (number_id < N) {
        int fork_id = fork();
        if (!is_not_child(fork_id)) {
            number_id++;
        } else {
            ChildState child_state = {
                .fork_id = number_id,
                .child_time = 0,
                .N = N,
                .balance_history = {
                    .s_id = number_id,
                    .s_history_len = 1,
                    .s_history[0] = {
                        .s_balance = array[number_id],
                        .s_time = 0,
                        .s_balance_pending_in = 0,
                    }
                }
            };       

            init_child_work(&child_state);

            handle_transfers(&child_state);
            
            return 0;
        }
    }

    // Start parent
    pipe_info.fork_id = 0;
    pipe_info.N = N;
    pipe_info.local_time = 0;
    for(int i = 0; i < 10; i++) {
        for(int j = 0; j < 10; j++) {
            for(int k = 0; k < 2; k++) {
                pipe_info.pm[i][j][k] = pm[i][j][k];
            }
        }
    }

    init_parent_work(&pipe_info, N);

    do_parent_work(&pipe_info, N);

    print_history_from_all_children(&pipe_info, N);

    parent_are_waiting(&pipe_info, N);
    
    free(array);

    return 0;
}
