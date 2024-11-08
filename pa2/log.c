#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"
#include "common.h"
#include "pa2345.h"
#include "io.h"

IpcLocal local;
size_t num_processes;
int reader[MAX_PROCESSES][MAX_PROCESSES];
int writer[MAX_PROCESSES][MAX_PROCESSES];

void flog_argsfmt(FILE* file, const char *format, va_list args) {
    if (file == NULL) {
        fprintf(stderr, error_log_message_fmt, format);
        return;
    }

    vfprintf(file, format, args);
}

void flogfmt(FILE* file, const char *format, ...) {
    va_list args;

    va_start(args, format);
    flog_argsfmt(file, format, args);
    va_end(args);
}

void log_argsfmt(const char *format, va_list args) {
    vprintf(format, args);
}

void logfmt(const char *format, ...) {
    va_list args;

    va_start(args, format);
    log_argsfmt(format, args);
    va_end(args);
}

void log_errorfmt(const char *format, ...) {
    va_list args;

    va_start(args, format);
    fprintf(stderr, format, args);
    va_end(args);
}

void log_events(const char *format, ...) {
    va_list args;

    va_start(args, format);
    log_argsfmt(format, args);
    flog_argsfmt(events_log_file, format, args);
    va_end(args);
}

void log_pipes(const char *format, ...) {
    va_list args;

    va_start(args, format);
    log_argsfmt(format, args);
    flog_argsfmt(pipes_log_file, format, args);
    va_end(args);
}

void log_init() {
    if ((events_log_file = fopen(events_log, "w")) == NULL)
        log_errorfmt(error_open_file_fmt, events_log);

    if ((pipes_log_file = fopen(pipes_log, "w")) == NULL)
        log_errorfmt(error_open_file_fmt, pipes_log);
}

void started() {
    pid_t pid = getpid();
    pid_t parent_pid = getppid();
    balance_t balance = local.balance_history.s_history[local.balance_history.s_history_len - 1].s_balance;    
    log_events(
        log_started_fmt, 
        get_physical_time(),
        local.ipc_id,
        pid, 
        parent_pid, 
        balance
    );
}

void done() {
    balance_t balance = local.balance_history.s_history[local.balance_history.s_history_len - 1].s_balance;
    log_events(
        log_done_fmt,
        get_physical_time(), 
        local.ipc_id, 
        balance
    );
}

void received_all_started() {
    log_events(
        log_received_all_started_fmt, 
        get_physical_time(), 
        local.ipc_id
    );
}

void received_all_done() {
    log_events(
        log_received_all_done_fmt, 
        get_physical_time(), 
        local.ipc_id
    );
}


void transfer_out_fmt(local_id from, local_id to, balance_t amount) {
    log_events(
        log_transfer_out_fmt, 
        get_physical_time(), 
        from, 
        amount, 
        to
    );
}

void transfer_in_fmt(local_id from, local_id to, balance_t amount) {
    log_events(
        log_transfer_in_fmt, 
        get_physical_time(), 
        to, 
        amount, 
        from
    );
}


void pipe_opened(int from, int to) {
    log_pipes("Pipe from process %1d to %1d OPENED\n", from, to);
}

void log_end() {
    fclose(events_log_file);
    fclose(pipes_log_file);
}
