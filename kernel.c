/* kernel.c - Main kernel with null process */
#include "types.h"
#include "serial.h"
#include "string.h"
//memory add korchi
#include "memory.h"
extern uint8_t __kernel_end[];

#define MAX_INPUT 128

void kmain(void) {
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
    void* p1 = kmalloc(64);
    serial_puts("   p1 Address: ");
    serial_put_hex((uint32_t)p1);
    serial_puts("\n");

    serial_puts("2. Allocating 128 bytes (p2)...\n");
    void* p2 = kmalloc(128);
    serial_puts("   p2 Address: ");
    serial_put_hex((uint32_t)p2);
    serial_puts("\n");

    memory_dump();

    serial_puts("\n3. Freeing p1...\n");
    kfree(p1);
    memory_dump();

    serial_puts("\n4. Allocating Process Stack...\n");
    void* s1 = alloc_stack();
    serial_puts("   Stack 1 Top: ");
    serial_put_hex((uint32_t)s1);
    serial_puts("\n\n");

    //----------------
    
    /* Main loop - the "null process" */
    while (1) {
        serial_puts("kacchiOS> ");
        pos = 0;
        
        /* Read input line */
        while (1) {
            char c = serial_getc();
            
            /* Handle Enter key */
            if (c == '\r' || c == '\n') {
                input[pos] = '\0';
                serial_puts("\n");
                break;
            }
            /* Handle Backspace */
            else if ((c == '\b' || c == 0x7F) && pos > 0) {
                pos--;
                serial_puts("\b \b");  /* Erase character on screen */
            }
            /* Handle normal characters */
            else if (c >= 32 && c < 127 && pos < MAX_INPUT - 1) {
                input[pos++] = c;
                serial_putc(c);  /* Echo character */
            }
        }
        
        /* Echo back the input */
        if (pos > 0) {
            serial_puts("You typed: ");
            serial_puts(input);
            serial_puts("\n");
        }
    }
    
    /* Should never reach here */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}