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


#ifndef INSTANCE_H
#define INSTANCE_H

#define PID_FILENAME "/tmp/galendae_pid.lock"

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
