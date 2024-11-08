#pragma once
#ifndef  __PIPES_CONST_H
#define  __PIPES_CONST_H

#include "ipc.h"
#include <stdio.h>

typedef struct {
  int pm[10][10][2];
  local_id fork_id;
  timestamp_t local_time;
  int N;
} Info;

extern FILE *elf;
extern FILE *plf;

extern Info pipe_info;

extern int pm[10][10][2];
// Message new_message_contructor(MessageType msg_type);

#endif
