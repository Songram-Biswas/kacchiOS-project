/* kernel.c - Main kernel with Memory, Process, Scheduler & Interactive Shell */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "memory.h"
#include "process.h"
#include "scheduler.h"

extern uint8_t __kernel_end[];

#define MAX_INPUT 128

void process_alpha(void) {
    void* dyn_mem = kmalloc(64);
    for (int step = 1; step <= 3; step++) {
        serial_puts("  [Proc Alpha] Step ");
        serial_put_int(step);
        serial_puts("/3 | Heap Block: ");
        serial_put_hex((uint32_t)dyn_mem);
        serial_puts("\n");
        schedule_yield();
    }
    kfree(dyn_mem);
}

void process_beta(void) {
    void* dyn_mem = kmalloc(128);
    for (int step = 1; step <= 3; step++) {
        serial_puts("  [Proc Beta ] Step ");
        serial_put_int(step);
        serial_puts("/3 | Heap Block: ");
        serial_put_hex((uint32_t)dyn_mem);
        serial_puts("\n");
        schedule_yield();
    }
    kfree(dyn_mem);
}

void process_gamma(void) {
    for (int step = 1; step <= 2; step++) {
        serial_puts("  [Proc Gamma] Step ");
        serial_put_int(step);
        serial_puts("/2 (Computing...)\n");
        schedule_yield();
    }
}

static void spawn_demo_processes(void) {
    int p1 = process_create(process_alpha, "Alpha");
    int p2 = process_create(process_beta,  "Beta");
    int p3 = process_create(process_gamma, "Gamma");
    serial_puts("Spawned Demo Processes -> PIDs: ");
    serial_put_int(p1);
    serial_puts(", ");
    serial_put_int(p2);
    serial_puts(", ");
    serial_put_int(p3);
    serial_puts("\n");
}

static void print_help(void) {
    serial_puts("\nAvailable kacchiOS Shell Commands:\n");
    serial_puts("  help  - Show this command list\n");
    serial_puts("  mem   - Display heap blocks and stack usage\n");
    serial_puts("  ps    - Display Process Control Block (PCB) table\n");
    serial_puts("  spawn - Create 3 new demo processes (Alpha, Beta, Gamma)\n");
    serial_puts("  run   - Run Round-Robin Scheduler on READY processes\n");
    serial_puts("  demo  - Run full automated memory + process + scheduler demo\n\n");
}

void kmain(void) {
    char input[MAX_INPUT];
    int pos = 0;

    serial_init();

    serial_puts("\n");
    serial_puts("========================================\n");
    serial_puts("    kacchiOS - Minimal Baremetal OS\n");
    serial_puts("========================================\n");
    serial_puts("Roll Group: 55 to 60\n");
    serial_puts("Kernel End Address: ");
    serial_put_hex((uint32_t)__kernel_end);
    serial_puts("\n\n");

    memory_init();
    process_init();
    scheduler_init();

    spawn_demo_processes();
    process_dump();
    scheduler_run();
    process_dump();
    memory_dump();

    print_help();

    while (1) {
        serial_puts("kacchiOS> ");
        pos = 0;

        while (1) {
            char c = serial_getc();

            if (c == '\r' || c == '\n') {
                input[pos] = '\0';
                serial_puts("\n");
                break;
            } else if ((c == '\b' || c == 0x7F) && pos > 0) {
                pos--;
                serial_puts("\b \b");
            } else if (c >= 32 && c < 127 && pos < MAX_INPUT - 1) {
                input[pos++] = c;
                serial_putc(c);
            }
        }

        if (pos > 0) {
            if (strcmp(input, "help") == 0) {
                print_help();
            } else if (strcmp(input, "mem") == 0) {
                memory_dump();
            } else if (strcmp(input, "ps") == 0) {
                process_dump();
            } else if (strcmp(input, "spawn") == 0) {
                spawn_demo_processes();
            } else if (strcmp(input, "run") == 0) {
                scheduler_run();
            } else if (strcmp(input, "demo") == 0) {
                spawn_demo_processes();
                process_dump();
                scheduler_run();
                process_dump();
                memory_dump();
            } else {
                serial_puts("Unknown command: ");
                serial_puts(input);
                serial_puts(" (Type 'help' for commands)\n");
            }
        }
    }
}