/*
 * Project: Galendae Popup Calendar
 * URL:     https://github.com/chris-marsh/galendae
 * License: The MIT License
 */


#ifndef INSTANCE_H
#define INSTANCE_H

typedef struct Instance *InstancePtr;

InstancePtr instance_create();
int instance_is_unique(InstancePtr this);
void instance_signal(InstancePtr this, int signal_code);
void instance_kill(InstancePtr this);
void instance_free(InstancePtr this);

#endif
