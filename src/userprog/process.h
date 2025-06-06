#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

void get_file_name(char *src, char *dest);
void stacking_arg(char *file_name, void **esp);

#endif /* userprog/process.h */
