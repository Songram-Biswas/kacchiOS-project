#include "process.h"
#include "memory.h"
#include "string.h"
#include "serial.h"

PCB process_table[MAX_PROCESSES];
int current_pid = -1;
static uint32_t next_pid = 1;

void process_init(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = PROCESS_UNUSED;
        process_table[i].esp = 0;
        process_table[i].stack_base = NULL;
        process_table[i].name[0] = '\0';
    }
    current_pid = -1;
    next_pid = 1;
    serial_puts("Process Manager Initialized.\n");
}

/* 
 * Fake Context তৈরি: x86 context switch-এর স্ট্যাক ফ্রেম
 * যখন popa এবং iret/ret চলবে, তখন CPU এগুলো তুলে নেবে।
 */
int process_create(void (*entry_point)(), const char* name) {
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCESS_UNUSED) {
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

    /* স্ট্যাক সাজানো (উপর থেকে নিচে) */
    uint32_t* stk = (uint32_t*)stack_top;

    /* 1. ফাংশন রিটার্ন অ্যাড্রেস বা এক্সিট হ্যান্ডলার */
    *(--stk) = (uint32_t)process_exit; 

    /* 2. এন্ট্রি পয়েন্ট (যেখানে ফাংশন শুরু হবে) */
    *(--stk) = (uint32_t)entry_point;

    /* 3. pusha রেজিস্টার সেট (EAX, ECX, EDX, EBX, Temp ESP, EBP, ESI, EDI) */
    *(--stk) = 0; /* EAX */
    *(--stk) = 0; /* ECX */
    *(--stk) = 0; /* EDX */
    *(--stk) = 0; /* EBX */
    *(--stk) = 0; /* ESP */
    *(--stk) = 0; /* EBP */
    *(--stk) = 0; /* ESI */
    *(--stk) = 0; /* EDI */

    /* PCB সেটআপ */
    process_table[slot].pid = next_pid++;
    process_table[slot].state = PROCESS_READY;
    process_table[slot].esp = (uint32_t)stk;
    process_table[slot].stack_base = stack_top;
    strcpy(process_table[slot].name, name);

    return process_table[slot].pid;
}

void process_exit(void) {
    if (current_pid != -1) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (process_table[i].pid == current_pid) {
                process_table[i].state = PROCESS_TERMINATED;
                if (process_table[i].stack_base != NULL) {
                    free_stack(process_table[i].stack_base);
                    process_table[i].stack_base = NULL;
                }
                break;
            }
        }
    }
    /* প্রসেস শেষ হলে ইনফিনিট ওয়েট (পরবর্তী শিডিউল হওয়া পর্যন্ত) */
    while (1) {
        __asm__ volatile ("hlt");
    }
}

void process_dump(void) {
    serial_puts("\n--- Process Table ---\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != PROCESS_UNUSED) {
            serial_puts("PID: ");
            serial_put_int(process_table[i].pid);
            serial_puts(" | Name: ");
            serial_puts(process_table[i].name);
            serial_puts(" | State: ");
            if (process_table[i].state == PROCESS_READY) serial_puts("READY");
            else if (process_table[i].state == PROCESS_RUNNING) serial_puts("RUNNING");
            else if (process_table[i].state == PROCESS_TERMINATED) serial_puts("TERMINATED");
            serial_puts(" | ESP: ");
            serial_put_hex(process_table[i].esp);
            serial_puts("\n");
        }
    }
    serial_puts("----------------------\n\n");
}