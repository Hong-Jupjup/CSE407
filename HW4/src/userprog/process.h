#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

struct vm_entry;

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

bool expand_stack (void *addr);
bool verify_stack (void *fault_addr, void *esp);
bool handle_mm_fault (struct vm_entry *vme);

#endif /* userprog/process.h */
