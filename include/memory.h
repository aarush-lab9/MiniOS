#ifndef MEMORY_H
#define MEMORY_H

// Total simulated physical memory arena capacity (1024 Megabytes)
#define TOTAL_MEMORY 1024

// Memory Allocation Block (MemBlock): Node structure in the doubly-linked memory list
typedef struct MemBlock {
    int offset;                 // Starting byte/MB physical offset of this block in the 1024 MB arena
    int size;                   // Total contiguous size of this memory block in Megabytes
    int allocated;              // Partition occupancy flag: 1 = currently allocated, 0 = free hole
    char process_name[16];      // Tag of the owning process if allocated, or empty string if free
    struct MemBlock *next;      // Pointer to the adjacent memory block at a higher physical address
    struct MemBlock *prev;      // Pointer to the adjacent memory block at a lower physical address
} MemBlock;

// Dynamic memory allocation placement policies
typedef enum {
    FIRST_FIT,  // Scan from arena beginning and allocate the first free hole that is large enough
    BEST_FIT,   // Search the entire arena to allocate the smallest hole that satisfies the request
    WORST_FIT   // Search the entire arena and allocate from the largest available free hole
} AllocPolicy;

// Initializes the 1024 MB physical memory pool as a single unallocated contiguous hole
void init_memory(void);

// Requests 'size' MB of contiguous memory for 'pname' using the specified placement policy
// Returns 0 on successful allocation, or -1 if no contiguous free hole is large enough
int mem_allocate(const char *pname, int size, AllocPolicy policy);

// Releases all memory blocks owned by 'pname' and merges adjacent free blocks in O(1) time
// Returns the number of blocks freed, or -1 if the process has no allocations
int mem_free(const char *pname);

// Visualizes the complete physical memory map showing allocated chunks, free holes, and fragmentation
void mem_status(void);

// Reclaims all dynamically allocated heap memory nodes and resets the arena
void cleanup_memory(void);

#endif
