#include "process.h"
#include "memory.h"
#include "string.h"
#include "serial.h"
#include "scheduler.h"

PCB process_table[MAX_PROCESSES];
int current_slot = -1;
static uint32_t next_pid = 1;

void process_init(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = PROCESS_UNUSED;
        process_table[i].esp = 0;
        process_table[i].stack_base = NULL;
        process_table[i].name[0] = '\0';
    }
    current_slot = -1;
    next_pid = 1;
    serial_puts("Process Manager Initialized.\n");
}

int process_create(void (*entry_point)(void), const char* name) {
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCESS_UNUSED || process_table[i].state == PROCESS_TERMINATED) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        serial_puts("process_create: Process table full!\n");
        return -1;
    }

    void* stack_top = alloc_stack();
    if (stack_top == NULL) {
        serial_puts("process_create: Failed to allocate stack!\n");
        return -1;
    }

    uint32_t* stk = (uint32_t*)stack_top;

    *(--stk) = (uint32_t)process_exit;
    *(--stk) = (uint32_t)entry_point;

    *(--stk) = 0; /* EAX */
    *(--stk) = 0; /* ECX */
    *(--stk) = 0; /* EDX */
    *(--stk) = 0; /* EBX */
    *(--stk) = 0; /* ESP */
    *(--stk) = 0; /* EBP */
    *(--stk) = 0; /* ESI */
    *(--stk) = 0; /* EDI */

    process_table[slot].pid = next_pid++;
    process_table[slot].state = PROCESS_READY;
    process_table[slot].esp = (uint32_t)stk;
    process_table[slot].stack_base = stack_top;
    strcpy(process_table[slot].name, name);

    return (int)process_table[slot].pid;
}

void process_exit(void) {
    if (current_slot >= 0 && current_slot < MAX_PROCESSES) {
        serial_puts("\n[Process ");
        serial_put_int((int32_t)process_table[current_slot].pid);
        serial_puts(" (");
        serial_puts(process_table[current_slot].name);
        serial_puts(") finished & freeing stack]\n");

        process_table[current_slot].state = PROCESS_TERMINATED;
        if (process_table[current_slot].stack_base != NULL) {
            free_stack(process_table[current_slot].stack_base);
            process_table[current_slot].stack_base = NULL;
        }
    }
    schedule_yield();

    while (1) {
        __asm__ volatile ("hlt");
    }
}

int process_terminate(uint32_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != PROCESS_UNUSED && process_table[i].pid == pid) {
            process_table[i].state = PROCESS_TERMINATED;
            if (process_table[i].stack_base != NULL) {
                free_stack(process_table[i].stack_base);
                process_table[i].stack_base = NULL;
            }
            return 0;
        }
    }
    return -1;
}

void process_dump(void) {
    serial_puts("\n--- Process Table (PCB) ---\n");
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != PROCESS_UNUSED) {
            count++;
            serial_puts("PID: ");
            serial_put_int((int32_t)process_table[i].pid);
            serial_puts(" | Name: ");
            serial_puts(process_table[i].name);
            serial_puts(" | State: ");
            if (process_table[i].state == PROCESS_READY) serial_puts("READY     ");
            else if (process_table[i].state == PROCESS_RUNNING) serial_puts("RUNNING   ");
            else if (process_table[i].state == PROCESS_TERMINATED) serial_puts("TERMINATED");
            serial_puts(" | ESP: ");
            serial_put_hex(process_table[i].esp);
            serial_puts("\n");
        }
    }
    if (count == 0) {
        serial_puts("(No processes created yet)\n");
    }
    serial_puts("---------------------------\n");
}