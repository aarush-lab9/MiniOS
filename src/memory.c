#include <stdio.h>                                 // Standard I/O library for formatted console output (printf)
#include <stdlib.h>                                // Standard library for dynamic heap memory allocation (malloc, free)
#include <string.h>                                // String manipulation functions (strcpy, strcmp, strncpy)
#include "memory.h"                                // Memory subsystem header defining MemBlock, AllocPolicy, constants

#define RED     "\033[1;31m"                       // Bright Red: highlights errors and allocation failures
#define GREEN   "\033[1;32m"                       // Bright Green: highlights allocated memory blocks
#define YELLOW  "\033[1;33m"                       // Bright Yellow: highlights free memory holes
#define CYAN    "\033[1;36m"                       // Bright Cyan: highlights memory map structural headers
#define MAGENTA "\033[1;35m"                       // Bright Magenta: decorative divider styling
#define RESET   "\033[0m"                          // Color Reset: restores default terminal foreground styling

static MemBlock *mem_head = NULL;                  // Pointer to first physical memory block node (origin at offset 0 MB)

void init_memory(void) {                           // Initializes the 1024 MB simulated physical memory arena
    cleanup_memory();                              // Reclaim any existing linked list nodes to prevent heap leaks
    
    mem_head = (MemBlock *)malloc(sizeof(MemBlock)); // Allocate initial head node representing entire arena
    if (mem_head == NULL) {                        // Verify that heap memory allocation succeeded
        printf(RED "Error: Failed to initialize memory\n" RESET); // Display kernel error alert if malloc fails
        return;                                    // Abort initialization on allocation failure
    }
    mem_head->offset = 0;                          // Set starting physical address offset to 0 MB
    mem_head->size = TOTAL_MEMORY;                 // Set initial partition capacity to full 1024 MB arena
    mem_head->allocated = 0;                       // Mark block as free hole (0 = unallocated, 1 = allocated)
    strcpy(mem_head->process_name, "");            // Clear process ownership tag for unallocated free block
    mem_head->next = NULL;                         // Forward pointer to NULL (no higher-offset neighbor exists)
    mem_head->prev = NULL;                         // Backward pointer to NULL (no lower-offset neighbor exists)
}

static void split_block(MemBlock *blk, int size) { // Splits a free hole: allocates 'size' MB, leaving residual hole
    if (blk->size == size) {                       // If block size matches requested size exactly
        return;                                    // Exact fit: no remainder block needed, return immediately
    }
    
    MemBlock *new_block = (MemBlock *)malloc(sizeof(MemBlock)); // Allocate heap node representing residual free hole
    new_block->offset = blk->offset + size;        // Starting offset of residual hole immediately after allocated chunk
    new_block->size = blk->size - size;            // Residual hole size: original block size minus allocated size
    new_block->allocated = 0;                      // Mark residual chunk as unallocated free hole
    strcpy(new_block->process_name, "");           // Initialize residual hole owner string to empty
    
    new_block->next = blk->next;                   // Forward link: new_block points to blk's old next neighbor
    new_block->prev = blk;                         // Backward link: new_block points backward to blk
    if (blk->next != NULL)                         // If blk previously had a forward neighbor
        blk->next->prev = new_block;               // Update forward neighbor's backward pointer to point to new_block
    blk->next = new_block;                         // Update blk's forward pointer to establish new_block as successor
    
    blk->size = size;                              // Truncate blk's recorded size to match exact allocated request
}

static void merge_blocks(MemBlock *blk) {          // Merges adjacent free memory holes around blk in O(1) time
    if (blk->next != NULL && !blk->next->allocated) { // Forward Coalescing: check if successor exists and is free hole
        MemBlock *next = blk->next;                // Store pointer to adjacent successor node
        blk->size += next->size;                   // Absorb successor's size into current block
        blk->next = next->next;                    // Bypass successor in forward linked list
        if (next->next != NULL)                    // If a subsequent neighbor exists beyond next
            next->next->prev = blk;                // Update its backward pointer to point directly back to blk
        free(next);                                // Free absorbed successor node from heap memory
    }
    
    if (blk->prev != NULL && !blk->prev->allocated) { // Backward Coalescing: check if predecessor exists and is free hole
        MemBlock *prev = blk->prev;                // Store pointer to adjacent predecessor node
        prev->size += blk->size;                   // Absorb current block's size into predecessor
        prev->next = blk->next;                    // Bypass current block in forward linked list
        if (blk->next != NULL)                     // If a subsequent neighbor exists beyond current block
            blk->next->prev = prev;                // Update its backward pointer to point directly to prev
        free(blk);                                 // Free absorbed current block node from heap memory
    }
}

int mem_allocate(const char *pname, int size, AllocPolicy policy) { // Allocates contiguous physical memory via policy
    if (size <= 0 || size > TOTAL_MEMORY) {        // Boundary validation: verify requested size is between 1-1024 MB
        printf(RED "Error: Invalid size %d (must be 1-%d)\n" RESET, size, TOTAL_MEMORY); // Reject invalid request
        return -1;                                 // Return failure code -1
    }
    
    MemBlock *cur = mem_head;                      // Initialize cursor pointer to start of memory arena
    while (cur != NULL) {                          // Traverse across all partition nodes in physical address order
        if (cur->allocated && strcmp(cur->process_name, pname) == 0) { // Check if process already holds allocation
            printf(RED "Error: '%s' already has memory allocated\n" RESET, pname); // Reject duplicate allocation
            return -1;                             // Return failure code -1
        }
        cur = cur->next;                           // Advance cursor to next physical block
    }
    
    MemBlock *chosen = NULL;                       // Pointer to track candidate block chosen by placement heuristic
    
    if (policy == FIRST_FIT) {                     // Heuristic 1: First-Fit Placement Policy
        cur = mem_head;                            // Traverse arena from offset 0 upwards
        while (cur != NULL) {                      // Search for first hole meeting size requirement
            if (!cur->allocated && cur->size >= size) { // Find first unallocated hole whose size satisfies request
                chosen = cur;                      // Select this hole
                break;                             // Terminate search immediately (minimal search overhead)
            }
            cur = cur->next;                       // Advance cursor to next block
        }
    } else if (policy == BEST_FIT) {               // Heuristic 2: Best-Fit Placement Policy
        int min_size = TOTAL_MEMORY + 1;           // Initialize minimum tracking variable to upper bound
        cur = mem_head;                            // Start at arena origin
        while (cur != NULL) {                      // Search ENTIRE linked list to locate closest fitting hole
            if (!cur->allocated && cur->size >= size && cur->size < min_size) { // Block is free, valid, and smaller
                min_size = cur->size;              // Update candidate minimum size
                chosen = cur;                      // Select this candidate block
            }
            cur = cur->next;                       // Continue searching until end of arena
        }
    } else if (policy == WORST_FIT) {              // Heuristic 3: Worst-Fit Placement Policy
        int max_size = -1;                         // Initialize maximum tracking variable to -1
        cur = mem_head;                            // Start at arena origin
        while (cur != NULL) {                      // Search ENTIRE linked list to locate largest available hole
            if (!cur->allocated && cur->size >= size && cur->size > max_size) { // Block is free, valid, and larger
                max_size = cur->size;              // Update candidate maximum size
                chosen = cur;                      // Select this candidate block
            }
            cur = cur->next;                       // Continue searching until end of arena
        }
    }
    
    if (chosen == NULL) {                          // Invariant check: ensure a suitable free hole was identified
        printf(RED "Error: Not enough contiguous memory for %d MB\n" RESET, size); // Alert external fragmentation
        return -1;                                 // Return failure code -1
    }
    
    split_block(chosen, size);                     // Split selected free hole if it exceeds requested size
    chosen->allocated = 1;                         // Mark partition status as allocated (occupied)
    strncpy(chosen->process_name, pname, 15);      // Copy owning process tag into partition descriptor safely
    chosen->process_name[15] = '\0';               // Ensure string null-termination
    
    const char *policy_names[] = {"First Fit", "Best Fit", "Worst Fit"}; // Lookup table for readable policy names
    printf(GREEN "  [%s] Allocated at offset %d\n" RESET, policy_names[policy], chosen->offset); // Print confirmation
    return 0;                                      // Return success code 0
}

int mem_free(const char *pname) {                  // Releases all memory partitions allocated to process pname
    MemBlock *cur = mem_head;                      // Cursor to traverse memory list from beginning
    while (cur != NULL) {                          // Traverse all physical blocks in the arena
        if (cur->allocated && strcmp(cur->process_name, pname) == 0) { // If block is allocated and owned by process
            cur->allocated = 0;                    // Mark block as free hole
            strcpy(cur->process_name, "");         // Clear process ownership string
            merge_blocks(cur);                     // Automatically merge with adjacent free holes in O(1) time
            return 0;                              // Return success code 0
        }
        cur = cur->next;                           // Advance cursor to next memory block
    }
    printf(RED "Error: No memory allocated to '%s'\n" RESET, pname); // Emit error if process holds no allocation
    return -1;                                     // Return failure code -1
}

void mem_status(void) {                            // Visualizes physical memory map showing ranges and fragmentation
    printf(CYAN "\n=== Memory Map (Total: %d MB) ===\n" RESET, TOTAL_MEMORY); // Print banner showing arena capacity
    printf("────────────────────────────────────────────────\n"); // Horizontal dividing line
    
    MemBlock *cur = mem_head;                      // Cursor to iterate over all memory partitions
    int total_free = 0, total_alloc = 0;           // Accumulators for total free space and total allocated space
    
    while (cur != NULL) {                          // Traverse all blocks in physical order
        int end = cur->offset + cur->size - 1;     // Calculate ending physical offset: start + size - 1
        if (cur->allocated) {                      // If partition is allocated to an active process
            printf(GREEN "  [%4d - %4d]  %-10s  %4d MB  ALLOCATED\n" RESET, // Print green record with address range
                   cur->offset, end, cur->process_name, cur->size);
            total_alloc += cur->size;              // Accumulate into total allocated counter
        } else {                                   // If partition is an unallocated free hole
            printf(YELLOW "  [%4d - %4d]  %-10s  %4d MB  FREE\n" RESET, // Print yellow record showing hole range
                   cur->offset, end, "---", cur->size);
            total_free += cur->size;               // Accumulate into total free counter
        }
        cur = cur->next;                           // Advance cursor to next physical block
    }
    
    printf("────────────────────────────────────────────────\n"); // Dividing line separating list from summary
    printf("  Allocated: %d MB | Free: %d MB\n\n", total_alloc, total_free); // Display global memory breakdown
}

void cleanup_memory(void) {                        // Reclaims all dynamically allocated heap nodes in linked list
    MemBlock *cur = mem_head;                      // Cursor starting at the arena head
    while (cur != NULL) {                          // Iterate through linked list
        MemBlock *next = cur->next;                // Save pointer to next block before freeing current node
        free(cur);                                 // Free current MemBlock node from heap memory
        cur = next;                                // Advance cursor to saved next pointer
    }
    mem_head = NULL;                               // Set head pointer to NULL indicating empty list
}
