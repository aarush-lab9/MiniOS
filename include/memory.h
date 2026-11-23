#ifndef MEMORY_H                               // Include guard: prevents duplicate inclusion of memory interface
#define MEMORY_H                               // Define MEMORY_H symbol

#define TOTAL_MEMORY 1024                      // Total physical memory arena size in Megabytes (1024 MB pool)

typedef struct MemBlock {                      // Memory partition node in bidirectional doubly-linked list
    int offset;                                // Starting physical address offset in MB within the 1024 MB arena
    int size;                                  // Size of this contiguous memory partition in Megabytes
    int allocated;                             // Allocation status flag: 1 = occupied by process, 0 = free hole
    char process_name[16];                     // Name of the occupying process, or empty string if unallocated
    struct MemBlock *next;                     // Pointer to logically adjacent block at higher physical memory address
    struct MemBlock *prev;                     // Pointer to logically adjacent block at lower physical memory address
} MemBlock;                                    // Typedef alias for contiguous memory partition block

typedef enum {                                 // Dynamic contiguous partition allocation placement heuristic policies
    FIRST_FIT,                                 // Allocate first free hole large enough from beginning of list
    BEST_FIT,                                  // Allocate smallest free hole that satisfies the requested size
    WORST_FIT                                  // Allocate largest free hole available to leave largest remaining split
} AllocPolicy;                                 // Typedef alias for memory placement policy enum

void init_memory(void);                        // Initializes heap as single 1024 MB free unallocated partition
int mem_allocate(const char *pname, int size, AllocPolicy policy); // Allocates partition; returns 0 on success, -1 on failure
int mem_free(const char *pname);               // Frees partition owned by pname and merges adjacent free holes in O(1)
void mem_status(void);                         // Visualizes partition map, hole fragmentation, and total allocation
void cleanup_memory(void);                     // Releases all heap-allocated MemBlock nodes and reclaims system memory

#endif                                         // End of include guard MEMORY_H
