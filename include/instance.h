#ifndef INSTANCE_H
#define INSTANCE_H

#define PID_FILENAME "/tmp/test_pid.lock"

typedef struct Instance {
    int unique;
    char pid[10];
    int pid_file;
} Instance;

int instance_lock(Instance *this);
int instance_is_unique(Instance *this);
void instance_signal(Instance *this, int signal_code);
void instance_kill(Instance *this);
void instance_free(Instance *this);

#endif
