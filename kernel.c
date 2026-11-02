/* kernel.c - Main kernel with null process */
#include "types.h"
#include "serial.h"
#include "string.h"
// memory add korchi
#include "memory.h"
// process er jonne
#include "process.h"

extern uint8_t __kernel_end[];
extern void context_switch(uint32_t* old_esp, uint32_t new_esp);

#define MAX_INPUT 128

/* Test Process Function */
void test_proc(void) {
    serial_puts("\n>>> SUCCESS: Entered test_proc via Context Switch! <<<\n");
    while (1) {
        __asm__ volatile ("hlt");
    }
}

void kmain(void)
{
    char input[MAX_INPUT];
    int pos = 0;

    /* Initialize hardware */
    serial_init();

    /* Print welcome message */
    serial_puts("\n");
    serial_puts("========================================\n");
    serial_puts("    kacchiOS - Minimal Baremetal OS\n");
    serial_puts("========================================\n");
    serial_puts("Hello from kacchiOS!\n");
    serial_puts("Running null process...\n\n");
    
    serial_puts("Roll Check: ");
    serial_put_int(55);
    serial_puts(" to ");
    serial_put_int(60);
    serial_puts("\nKernel Ends at Memory Address: ");
    serial_put_hex((uint32_t)__kernel_end);
    serial_puts("\n\n");

    // Memory manager test
    memory_init();

    serial_puts("1. Allocating 64 bytes (p1)...\n");
    void *p1 = kmalloc(64);
    serial_puts("   p1 Address: ");
    serial_put_hex((uint32_t)p1);
    serial_puts("\n");

    serial_puts("2. Allocating 128 bytes (p2)...\n");
    void *p2 = kmalloc(128);
    serial_puts("   p2 Address: ");
    serial_put_hex((uint32_t)p2);
    serial_puts("\n");

    memory_dump();

    serial_puts("\n3. Freeing p1...\n");
    kfree(p1);
    memory_dump();

    serial_puts("\n4. Allocating Process Stack...\n");
    void *s1 = alloc_stack();
    serial_puts("   Stack 1 Top: ");
    serial_put_hex((uint32_t)s1);
    serial_puts("\n\n");

    //----------------

    // external process function add korlam
    /* Process Manager Test */
    process_init();
    int pid = process_create(test_proc, "ProcA");
    serial_puts("Created Process with PID: ");
    serial_put_int(pid);
    serial_puts("\n");

    process_dump();

    serial_puts("Switching to ProcA...\n");
    uint32_t dummy_esp = 0;
    
    /* 
     * Ekhane null process theke test_proc e switch korbe. 
     * Erpor ar nicher code run korbe na, karon test_proc ekta infinite loop e chole jabe. 
     */
    context_switch(&dummy_esp, process_table[0].esp);


    /* Main loop - the "null process" */
    /* Nicher ei ongsho tuko context_switch er pore ar run hobe na jodi scheduler na thake */
    while (1)
    {
        serial_puts("kacchiOS> ");
        pos = 0;

        /* Read input line */
        while (1)
        {
            char c = serial_getc();

            /* Handle Enter key */
            if (c == '\r' || c == '\n')
            {
                input[pos] = '\0';
                serial_puts("\n");
                break;
            }
            /* Handle Backspace */
            else if ((c == '\b' || c == 0x7F) && pos > 0)
            {
                pos--;
                serial_puts("\b \b"); /* Erase character on screen */
            }
            /* Handle normal characters */
            else if (c >= 32 && c < 127 && pos < MAX_INPUT - 1)
            {
                input[pos++] = c;
                serial_putc(c); /* Echo character */
            }
        }

        /* Echo back the input */
        if (pos > 0)
        {
            serial_puts("You typed: ");
            serial_puts(input);
            serial_puts("\n");
        }
    }

    /* Should never reach here */
    for (;;)
    {
        __asm__ volatile("hlt");
    }
}