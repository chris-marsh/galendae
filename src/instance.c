/******************************************************************************
 *                                                                            *
 *                galandae copyright (c) 2016 Chris Marsh                     *
 *               <https://github.com/chris-marsh/gcalendar                    *
 *                                                                            *
 * this program is free software: you can redistribute it and/or modify it    *
 * under the terms of the gnu general public license as published by the      *
 * free software foundation, either version 3 of the license, or any later    *
 * version.                                                                   *
 *                                                                            *
 * this program is distributed in the hope that it will be useful, but        *
 * without any warranty; without even the implied warranty of merchantability *
 * or fitness for a particular purpose.  see the gnu general public license   *
 * at <http://www.gnu.org/licenses/> for more details.                        *
 *                                                                            *
 ******************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/wait.h>
#include "instance.h"
#include "common.h"


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
 * 
 */
int instance_lock(Instance *this) {
    this->unique = FALSE;
    this->pid_file = open(PID_FILENAME, O_CREAT | O_RDWR, 0666);

    if (this->pid_file != -1){

        if (flock(this->pid_file, LOCK_EX | LOCK_NB)) {
            if (EWOULDBLOCK == errno) {
                /* Already running, get the PID */
                read(this->pid_file, this->pid, 10);
            } else {
                /* Deal with an error */
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
    return this->unique;
}


/*
 *
 */
int instance_is_unique(Instance *this)
{
    return this->unique;
}


/*
 *
 */
void instance_signal(Instance *this, int signal_code)
{
    send_sig(atoi(this->pid), signal_code);
}


/*
 *
 */
void instance_kill(Instance *this)
{
    instance_signal(this, SIGTERM);
}


/*
 *
 */
void instance_free(Instance *this)
{
    if (this->pid_file)
        close(this->pid_file);
    unlink(PID_FILENAME);
}

