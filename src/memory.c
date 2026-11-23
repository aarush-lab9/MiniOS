/*
 * memory.c - Contiguous Memory Management Subsystem
 * 
 * Implements dynamic contiguous memory allocation over a 1024 MB physical RAM arena:
 * - Doubly-linked list node architecture (MemBlock) tracking partitions and holes
 * - Dynamic placement heuristics: First-Fit, Best-Fit, and Worst-Fit
 * - Partition hole splitting when requests are smaller than candidate blocks
 * - Bidirectional O(1) adjacent free-block coalescing upon deallocation
 * - Visual memory map rendering and fragmentation reporting
 */

#include <stdio.h>      // Standard I/O library for formatted console output (printf)
#include <stdlib.h>     // Standard library for dynamic heap memory allocation (malloc, free)
#include <string.h>     // String manipulation functions (strcpy, strcmp, strncpy)
#include "memory.h"     // Memory subsystem header defining MemBlock, AllocPolicy, constants

// ANSI terminal color escape sequence macros for highlighting memory map states
#define RED     "\033[1;31m"    // Bright Red: highlights errors and allocation failures
#define GREEN   "\033[1;32m"    // Bright Green: highlights allocated memory blocks
#define YELLOW  "\033[1;33m"    // Bright Yellow: highlights free memory holes
#define CYAN    "\033[1;36m"    // Bright Cyan: highlights memory map structural headers
#define MAGENTA "\033[1;35m"    // Bright Magenta: decorative divider styling
#define RESET   "\033[0m"       // Color Reset: restores default terminal foreground styling

// Pointer to the first physical memory block node (origin at offset 0 MB)
static MemBlock *mem_head = NULL;

// Initializes the 1024 MB simulated physical memory arena
void init_memory(void) {
    // Reclaim any existing linked list nodes to prevent heap memory leaks
    cleanup_memory();
    
    // Allocate the initial head node on the heap representing the entire arena
    mem_head = (MemBlock *)malloc(sizeof(MemBlock));
    // Verify that heap memory allocation succeeded
    if (mem_head == NULL) {
        // Display kernel error alert if malloc fails
        printf(RED "Error: Failed to initialize memory\n" RESET);
        // Abort initialization
        return;
    }
    // Set starting physical address offset to 0 MB
    mem_head->offset = 0;
    // Set initial partition capacity to the full 1024 MB arena size
    mem_head->size = TOTAL_MEMORY;
    // Mark block as free hole (0 = unallocated, 1 = allocated)
    mem_head->allocated = 0;
    // Clear process ownership tag for an unallocated free block
    strcpy(mem_head->process_name, "");
    // Initialize forward pointer to NULL (no higher-offset neighbor exists yet)
    mem_head->next = NULL;
    // Initialize backward pointer to NULL (no lower-offset neighbor exists)
    mem_head->prev = NULL;
}

// Splits a free hole: allocates 'size' MB, leaving the remaining portion as a new free hole
static void split_block(MemBlock *blk, int size) {
    // If the block size matches the requested size exactly
    if (blk->size == size) {
        // Exact fit: no remainder block needs to be created, return immediately
        return;
    }
    
    // Allocate a new heap node representing the residual unallocated free hole
    MemBlock *new_block = (MemBlock *)malloc(sizeof(MemBlock));
    // Set the starting offset of the residual hole immediately after the allocated chunk
    new_block->offset = blk->offset + size;
    // Calculate residual hole size: original block size minus allocated size
    new_block->size = blk->size - size;
    // Mark the residual chunk as an unallocated free hole
    new_block->allocated = 0;
    // Initialize residual hole owner string to empty
    strcpy(new_block->process_name, "");
    
    // Splice new_block into the doubly-linked list immediately following blk
    new_block->next = blk->next;        // Forward link: new_block points to blk's old next neighbor
    new_block->prev = blk;              // Backward link: new_block points backward to blk
    // If blk previously had a forward neighbor
    if (blk->next != NULL)
        // Update the forward neighbor's backward pointer to point to new_block
        blk->next->prev = new_block;
    // Update blk's forward pointer to establish new_block as its immediate successor
    blk->next = new_block;
    
    // Truncate blk's recorded size to match the exact allocated request
    blk->size = size;
}

// Merges adjacent free memory holes around 'blk' in O(1) time (bidirectional coalescing)
static void merge_blocks(MemBlock *blk) {
    // Forward Coalescing: check if the immediate successor exists AND is an unallocated hole
    if (blk->next != NULL && !blk->next->allocated) {
        // Store pointer to the adjacent successor node
        MemBlock *next = blk->next;
        // Absorb successor's size into current block
        blk->size += next->size;
        // Bypass the successor in the forward linked list
        blk->next = next->next;
        // If a subsequent neighbor exists beyond next
        if (next->next != NULL)
            // Update its backward pointer to point directly back to blk
            next->next->prev = blk;
        // Free the absorbed successor node from heap memory
        free(next);
    }
    
    // Backward Coalescing: check if the immediate predecessor exists AND is an unallocated hole
    if (blk->prev != NULL && !blk->prev->allocated) {
        // Store pointer to the adjacent predecessor node
        MemBlock *prev = blk->prev;
        // Absorb current block's size into predecessor
        prev->size += blk->size;
        // Bypass current block in the forward linked list
        prev->next = blk->next;
        // If a subsequent neighbor exists beyond current block
        if (blk->next != NULL)
            // Update its backward pointer to point directly to prev
            blk->next->prev = prev;
        // Free the absorbed current block node from heap memory
        free(blk);
    }
}

// Allocates contiguous physical memory according to the specified dynamic placement policy
int mem_allocate(const char *pname, int size, AllocPolicy policy) {
    // Boundary validation: verify requested size is between 1 MB and 1024 MB
    if (size <= 0 || size > TOTAL_MEMORY) {
        // Reject invalid memory allocation request sizes
        printf(RED "Error: Invalid size %d (must be 1-%d)\n" RESET, size, TOTAL_MEMORY);
        // Return failure code -1
        return -1;
    }
    
    // Uniqueness validation: check if process already holds an active memory allocation
    MemBlock *cur = mem_head; // Initialize cursor pointer to start of memory arena
    while (cur != NULL) {     // Traverse across all partition nodes in physical address order
        // If block is allocated AND owned by this process tag
        if (cur->allocated && strcmp(cur->process_name, pname) == 0) {
            // Reject duplicate allocation for the same process name
            printf(RED "Error: '%s' already has memory allocated\n" RESET, pname);
            // Return failure code -1
            return -1;
        }
        // Advance cursor to the next physical block
        cur = cur->next;
    }
    
    // Pointer to track the candidate block chosen by the placement heuristic
    MemBlock *chosen = NULL;
    
    // Heuristic 1: First-Fit Placement Policy
    if (policy == FIRST_FIT) {
        // Traverse arena from offset 0 upwards
        cur = mem_head;
        while (cur != NULL) {
            // Find the very first unallocated hole whose size satisfies the request
            if (!cur->allocated && cur->size >= size) {
                // Select this hole
                chosen = cur;
                // Terminate search immediately (fastest heuristic with minimal search overhead)
                break;
            }
            // Advance cursor to next block
            cur = cur->next;
        }
    }
    // Heuristic 2: Best-Fit Placement Policy
    else if (policy == BEST_FIT) {
        // Initialize minimum tracking variable to an upper bound larger than arena
        int min_size = TOTAL_MEMORY + 1;
        cur = mem_head; // Start at arena origin
        // Search the ENTIRE linked list to locate the closest fitting hole
        while (cur != NULL) {
            // If block is free, large enough, and strictly smaller than current minimum candidate
            if (!cur->allocated && cur->size >= size && cur->size < min_size) {
                // Update candidate minimum size
                min_size = cur->size;
                // Select this candidate block
                chosen = cur;
            }
            // Continue searching until end of arena
            cur = cur->next;
        }
    }
    // Heuristic 3: Worst-Fit Placement Policy
    else if (policy == WORST_FIT) {
        // Initialize maximum tracking variable to -1
        int max_size = -1;
        cur = mem_head; // Start at arena origin
        // Search the ENTIRE linked list to locate the largest available hole
        while (cur != NULL) {
            // If block is free, large enough, and strictly larger than current maximum candidate
            if (!cur->allocated && cur->size >= size && cur->size > max_size) {
                // Update candidate maximum size
                max_size = cur->size;
                // Select this candidate block
                chosen = cur;
            }
            // Continue searching until end of arena
            cur = cur->next;
        }
    }
    
    // Invariant check: ensure a suitable free hole was identified
    if (chosen == NULL) {
        // Emit error alerting External Fragmentation (insufficient contiguous space)
        printf(RED "Error: Not enough contiguous memory for %d MB\n" RESET, size);
        // Return failure code -1
        return -1;
    }
    
    // Split the selected free hole if it exceeds the requested size
    split_block(chosen, size);
    // Mark the partition status as allocated (occupied)
    chosen->allocated = 1;
    // Copy the owning process tag into the partition descriptor with bounds safety
    strncpy(chosen->process_name, pname, 15);
    // Ensure string null-termination
    chosen->process_name[15] = '\0';
    
    // Lookup table for printing human-readable policy names
    const char *policy_names[] = {"First Fit", "Best Fit", "Worst Fit"};
    // Print confirmation of successful memory allocation with offset location
    printf(GREEN "  [%s] Allocated at offset %d\n" RESET, policy_names[policy], chosen->offset);
    // Return success code 0
    return 0;
}

// Releases all memory partitions allocated to process 'pname' and triggers coalescing
int mem_free(const char *pname) {
    // Cursor to traverse memory list from beginning
    MemBlock *cur = mem_head;
    // Traverse all physical blocks in the arena
    while (cur != NULL) {
        // If block is allocated AND owned by target process
        if (cur->allocated && strcmp(cur->process_name, pname) == 0) {
            // Mark block as free hole
            cur->allocated = 0;
            // Clear process ownership string
            strcpy(cur->process_name, "");
            // Automatically merge with adjacent free holes (prev and next) in O(1) time
            merge_blocks(cur);
            // Return success code 0
            return 0;
        }
        // Advance cursor to next memory block
        cur = cur->next;
    }
    // Emit error if target process holds no memory allocations
    printf(RED "Error: No memory allocated to '%s'\n" RESET, pname);
    // Return failure code -1
    return -1;
}

// Visualizes the complete physical memory map showing allocation ranges and hole fragmentation
void mem_status(void) {
    // Print banner showing total arena capacity
    printf(CYAN "\n=== Memory Map (Total: %d MB) ===\n" RESET, TOTAL_MEMORY);
    // Horizontal dividing line
    printf("────────────────────────────────────────────────\n");
    
    // Cursor to iterate over all memory partitions
    MemBlock *cur = mem_head;
    // Accumulators for total free space and total allocated space
    int total_free = 0, total_alloc = 0;
    
    // Traverse all blocks in physical order
    while (cur != NULL) {
        // Calculate ending physical offset: start + size - 1
        int end = cur->offset + cur->size - 1;
        // If partition is allocated to an active process
        if (cur->allocated) {
            // Print green record showing address range, owner tag, and chunk size
            printf(GREEN "  [%4d - %4d]  %-10s  %4d MB  ALLOCATED\n" RESET,
                   cur->offset, end, cur->process_name, cur->size);
            // Accumulate into total allocated counter
            total_alloc += cur->size;
        } else {
            // Print yellow record showing free hole address range and available size
            printf(YELLOW "  [%4d - %4d]  %-10s  %4d MB  FREE\n" RESET,
                   cur->offset, end, "---", cur->size);
            // Accumulate into total free counter
            total_free += cur->size;
        }
        // Advance cursor to next physical block
        cur = cur->next;
    }
    
    // Print dividing line separating list from summary statistics
    printf("────────────────────────────────────────────────\n");
    // Display global memory utilization breakdown
    printf("  Allocated: %d MB | Free: %d MB\n\n", total_alloc, total_free);
}

// Reclaims all dynamically allocated heap nodes in the linked list
void cleanup_memory(void) {
    // Cursor starting at the arena head
    MemBlock *cur = mem_head;
    // Iterate through linked list
    while (cur != NULL) {
        // Save pointer to next block before freeing current node
        MemBlock *next = cur->next;
        // Free current MemBlock node from heap memory
        free(cur);
        // Advance cursor to saved next pointer
        cur = next;
    }
    // Set head pointer to NULL indicating empty list
    mem_head = NULL;
}
