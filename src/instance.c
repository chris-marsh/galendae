/*
 * Project: Galendae Popup Calendar
 * URL:     https://github.com/chris-marsh/galendae
 * License: The MIT License
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/wait.h>
#include "instance.h"
#include "common.h"


struct Instance {
    int unique;
    char pid[10];
    int pid_file;
};


/*
 * C99 Alternative for POSIX 'kill'
*/
static int send_sig(int pid, int sig)
{
    char exec_str[128];
    sprintf(exec_str, "kill -%d %d >/dev/null 2>&1", sig, pid);
    int result = WEXITSTATUS(system(exec_str));
    if (result == 0)
        return 0;
    else
        return -1;
}


/*
 * Try to create a unqiue instance of the program. 
 * Create a temporary file and attempt a lock on it. If succesful, write
 * the pid into the file. If the lock fails, another instance is already
 * running, so read its pid from the existing lock file.
 */
InstancePtr instance_create() {
    InstancePtr this = malloc(sizeof *this);

    this->unique = FALSE;
    this->pid_file = open(PID_FILENAME, O_CREAT | O_RDWR, 0666);

    if (this->pid_file != -1){

        if (flock(this->pid_file, LOCK_EX | LOCK_NB)) {
            if (EWOULDBLOCK == errno) {
                /* Already running, get the PID */
                read(this->pid_file, this->pid, 10);
            }
        } else {
            /* Get lock */
            if (this->pid_file) {
                sprintf(this->pid, "%d", getpid());
                write(this->pid_file, this->pid, strlen(this->pid));
                this->unique = TRUE;
            }
        }
    }
    return this;
}


/*
 * Enuf said
 */
int instance_is_unique(InstancePtr this)
{
    return this->unique;
}


/*
 * Send a signal to the instance with the lock.
 */
void instance_signal(InstancePtr this, int signal_code)
{
    send_sig(atoi(this->pid), signal_code);
}


/*
 * Shorthand fuunction to send the SIGTERM signal
 */
void instance_kill(InstancePtr this)
{
    instance_signal(this, SIGTERM);
}


/*
 * Cleanup the temp file and used memory
 */
void instance_free(InstancePtr this)
{
    if (this->pid_file)
        close(this->pid_file);
    unlink(PID_FILENAME);
    free(this);
}

