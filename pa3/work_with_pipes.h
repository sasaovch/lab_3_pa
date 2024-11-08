#pragma once
#ifndef  __WORK_WITH_PIPES_H
#define  __WORK_WITH_PIPES_H

#include "ipc.h"

int send_to_pipe(void *__info, const Message *msg, local_id pipe_id);

#endif
