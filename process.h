#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

#define MAX_PROCESSES 16

typedef enum {
    PROCESS_UNUSED = 0,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_TERMINATED
} ProcessState;

typedef struct {
    uint32_t pid;
    ProcessState state;
    uint32_t esp;              /* Saved Stack Pointer */
    void* stack_base;          /* free_stack-এর জন্য পয়েন্টার */
    char name[32];
} PCB;

/* Function prototypes */
void process_init(void);
int process_create(void (*entry_point)(), const char* name);
void process_exit(void);
void process_dump(void);

/* বর্তমান চলমান প্রসেস ট্র্যাকিং */
extern PCB process_table[MAX_PROCESSES];
extern int current_pid;

#endif