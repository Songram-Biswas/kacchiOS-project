#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/* Memory Initialization */
void memory_init(void);

/* Dynamic Heap Allocation */
void* kmalloc(size_t size);
void kfree(void* ptr);

/* Process Stack Allocation (4KB aligned) */
void* alloc_stack(void);
void free_stack(void* stack_top);

/* Debug Dump via Serial Console */
void memory_dump(void);

#endif