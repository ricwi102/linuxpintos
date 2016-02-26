#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);


struct help_struct{
	struct semaphore s;
	char* file_name;
	bool success;
  char* argv[32];
  int8_t argc;
};

void help_sct_init(struct help_struct* h_sct);


#endif /* userprog/process.h */
