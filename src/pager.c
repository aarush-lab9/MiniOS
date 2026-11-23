/*
 * pager.c - Virtual Memory Page Replacement Subsystem
 * 
 * Simulates operating system demand paging across physical memory frame slots:
 * - Page presence validation and Page Hit / Page Fault detection
 * - First-In, First-Out (FIFO) replacement using circular pointer queue logic
 * - Least Recently Used (LRU) replacement leveraging temporal locality timestamps
 * - Bélády's Optimal foresight replacement (theoretical minimum fault ceiling)
 * - Frame contents visualization at each step and comparative hit ratio metrics
 */

#include <stdio.h>      // Standard I/O library for formatted console printing (printf)
#include <string.h>     // String manipulation library
#include "pager.h"      // Virtual memory paging header defining constants and function prototypes

// ANSI terminal color escape sequence macros for output formatting
#define RED     "\033[1;31m"    // Bright Red: indicates a Page Fault (victim eviction required)
#define GREEN   "\033[1;32m"    // Bright Green: indicates a Page Hit (referenced page already resident)
#define YELLOW  "\033[1;33m"    // Bright Yellow: highlights statistical totals and hit ratios
#define CYAN    "\033[1;36m"    // Bright Cyan: styles tabular step column headers
#define MAGENTA "\033[1;35m"    // Bright Magenta: decorative algorithm banner borders
#define RESET   "\033[0m"       // Color Reset: restores default terminal foreground styling

// Linear scan helper: checks if logical 'page' is already resident in physical 'frames'
static int is_in_frames(int *frames, int n, int page) {
    // Iterate through all n physical frame slots
    for (int i = 0; i < n; i++) {
        // If current frame slot holds the requested page number
        if (frames[i] == page) 
            // Return frame slot index (indicating a Page Hit)
            return i;
    }
    // Return -1 if page is not found in physical memory (indicating a Page Fault)
    return -1;
}

// Formats and prints current physical memory frame occupancy across all slots
static void print_frames(int *frames, int n) {
    // Open bracket delimiter for frame array visualization
    printf("[");
    // Iterate across all n frame slots
    for (int i = 0; i < n; i++) {
        // If frame is unallocated (-1 represents an empty frame slot)
        if (frames[i] == -1)
            // Print dash placeholder for empty slot
            printf(" -");
        else
            // Print resident page number
            printf(" %d", frames[i]);
        // Separate frame slots with a comma if not the final slot
        if (i < n - 1) printf(",");
    }
    // Close bracket delimiter
    printf(" ]");
}

/* ==============================================================================
 * FIFO (First-In, First-Out) Page Replacement Algorithm
 * ============================================================================== */
void page_replace_fifo(int frames, int *ref_string, int ref_len) {
    // Print algorithm execution header with configured physical frame capacity
    printf(MAGENTA "\n========== FIFO Page Replacement (%d frames) ==========\n" RESET, frames);
    
    // Array representing physical memory frame slots
    int frame[MAX_FRAMES];
    // Accumulator tracking total page fault occurrences
    int faults = 0;
    // Circular pointer pointing to the oldest loaded frame (victim candidate)
    int ptr = 0;
    
    // Initialize all physical frame slots to -1 (indicating initially empty RAM)
    for (int i = 0; i < frames; i++) frame[i] = -1;
    
    // Output tabular header columns
    printf(CYAN "%-8s %-15s %-10s\n" RESET, "Ref", "Frames", "Status");
    // Horizontal dividing line
    printf("─────────────────────────────────\n");
    
    // Iterate sequentially through the page reference string
    for (int i = 0; i < ref_len; i++) {
        // Retrieve current requested logical page number
        int page = ref_string[i];
        // Print reference number aligned to column
        printf("  %-6d ", page);
        
        // Check if requested page is already resident in physical memory
        if (is_in_frames(frame, frames, page) != -1) {
            // PAGE HIT: referenced page is present; no I/O transfer needed
            print_frames(frame, frames);
            // Print green HIT status confirmation
            printf(GREEN "  HIT\n" RESET);
        } else {
            // PAGE FAULT: page is absent; must load from simulated secondary swap storage
            // Overwrite the oldest resident frame slot pointed to by ptr
            frame[ptr] = page;
            // Advance circular FIFO pointer: ptr = (ptr + 1) % frames
            ptr = (ptr + 1) % frames;
            // Increment page fault counter
            faults++;
            // Output updated frame array contents
            print_frames(frame, frames);
            // Print red FAULT alert
            printf(RED "  FAULT\n" RESET);
        }
    }
    
    // Dividing line
    printf("─────────────────────────────────\n");
    // Print total page fault summary against total references
    printf(YELLOW "Total Page Faults: %d / %d\n" RESET, faults, ref_len);
    // Calculate and print Hit Ratio percentage: ((Hits) / Total) * 100
    printf(YELLOW "Hit Ratio: %.2f%%\n\n" RESET, ((float)(ref_len - faults) / ref_len) * 100);
}

/* ==============================================================================
 * LRU (Least Recently Used) Page Replacement Algorithm
 * ============================================================================== */
void page_replace_lru(int frames, int *ref_string, int ref_len) {
    // Print algorithm execution header
    printf(MAGENTA "\n========== LRU Page Replacement (%d frames) ==========\n" RESET, frames);
    
    // Physical memory frame array
    int frame[MAX_FRAMES];
    // Timestamp array tracking the simulation timeline step index when each frame was last accessed
    int last_used[MAX_FRAMES];
    // Accumulator for page faults
    int faults = 0;
    
    // Initialize all frames to -1 (empty) and all last_used timestamps to -1
    for (int i = 0; i < frames; i++) {
        frame[i] = -1;
        last_used[i] = -1;
    }
    
    // Output tabular column headers
    printf(CYAN "%-8s %-15s %-10s\n" RESET, "Ref", "Frames", "Status");
    // Dividing line
    printf("─────────────────────────────────\n");
    
    // Iterate sequentially through the page reference string
    for (int i = 0; i < ref_len; i++) {
        // Current requested logical page
        int page = ref_string[i];
        // Print reference number
        printf("  %-6d ", page);
        
        // Search if page is already resident in physical memory
        int pos = is_in_frames(frame, frames, page);
        
        // If page is already resident in frame slot pos
        if (pos != -1) {
            // PAGE HIT: update the last-used timestamp of this frame to current timeline step i
            last_used[pos] = i;
            // Print frame states
            print_frames(frame, frames);
            // Print green HIT status
            printf(GREEN "  HIT\n" RESET);
        } else {
            // PAGE FAULT: locate the frame slot whose last_used timestamp is minimal (oldest access)
            int lru_idx = 0;                    // Initialize candidate index to slot 0
            int min_time = last_used[0];        // Initialize candidate minimum timestamp
            
            // Linear scan across all physical frame slots
            for (int j = 1; j < frames; j++) {
                // If frame j was accessed further back in the past than current candidate
                if (last_used[j] < min_time) {
                    // Update minimum timestamp
                    min_time = last_used[j];
                    // Select frame j as new eviction victim candidate
                    lru_idx = j;
                }
            }
            
            // Evict victim page and place requested page into frame lru_idx
            frame[lru_idx] = page;
            // Record current timeline step i as the last-used timestamp for this frame
            last_used[lru_idx] = i;
            // Increment page fault counter
            faults++;
            // Output updated frame array
            print_frames(frame, frames);
            // Print red FAULT alert
            printf(RED "  FAULT\n" RESET);
        }
    }
    
    // Dividing line
    printf("─────────────────────────────────\n");
    // Output total fault statistics
    printf(YELLOW "Total Page Faults: %d / %d\n" RESET, faults, ref_len);
    // Output Hit Ratio percentage
    printf(YELLOW "Hit Ratio: %.2f%%\n\n" RESET, ((float)(ref_len - faults) / ref_len) * 100);
}

/* ==============================================================================
 * Optimal (Bélády's Min) Page Replacement Algorithm
 * ============================================================================== */
void page_replace_optimal(int frames, int *ref_string, int ref_len) {
    // Print algorithm execution header
    printf(MAGENTA "\n========== Optimal Page Replacement (%d frames) ==========\n" RESET, frames);
    
    // Physical memory frame array
    int frame[MAX_FRAMES];
    // Accumulator for page faults
    int faults = 0;
    
    // Initialize all physical frame slots to empty (-1)
    for (int i = 0; i < frames; i++) frame[i] = -1;
    
    // Output tabular column headers
    printf(CYAN "%-8s %-15s %-10s\n" RESET, "Ref", "Frames", "Status");
    // Dividing line
    printf("─────────────────────────────────\n");
    
    // Iterate sequentially through the page reference string
    for (int i = 0; i < ref_len; i++) {
        // Current requested logical page
        int page = ref_string[i];
        // Print reference number
        printf("  %-6d ", page);
        
        // Check if page is already present in physical frames
        if (is_in_frames(frame, frames, page) != -1) {
            // PAGE HIT: display frame states and proceed to next reference
            print_frames(frame, frames);
            printf(GREEN "  HIT\n" RESET);
            continue;
        }
        
        // PAGE FAULT occurred
        faults++;
        
        // First check: is there an empty frame slot available?
        int empty_slot = -1;
        for (int j = 0; j < frames; j++) {
            // If slot is empty (-1)
            if (frame[j] == -1) {
                // Select empty slot
                empty_slot = j;
                break;
            }
        }
        
        // If an empty slot exists
        if (empty_slot != -1) {
            // Place page into empty frame without needing to evict any existing page
            frame[empty_slot] = page;
        } else {
            // All frames are occupied: scan future references to find the page not used for the longest time
            int farthest = -1;          // Tracks the furthest future reference distance
            int replace_idx = 0;        // Index of frame chosen for eviction
            
            // Evaluate each currently resident frame j
            for (int j = 0; j < frames; j++) {
                // Default: assume page in frame j is never referenced again in the future (distance = ref_len)
                int next_use = ref_len;
                // Look ahead through future references from i + 1 to end of string
                for (int k = i + 1; k < ref_len; k++) {
                    // If future reference k accesses the page currently resident in frame j
                    if (ref_string[k] == frame[j]) {
                        // Record future access distance index k
                        next_use = k;
                        // Break lookahead early (only care about the FIRST future access)
                        break;
                    }
                }
                // If frame j's next access occurs further in the future than current candidate
                if (next_use > farthest) {
                    // Update furthest future distance
                    farthest = next_use;
                    // Select frame j as victim
                    replace_idx = j;
                }
            }
            
            // Evict victim page and place requested page into frame replace_idx
            frame[replace_idx] = page;
        }
        
        // Output updated frame array
        print_frames(frame, frames);
        // Print red FAULT alert
        printf(RED "  FAULT\n" RESET);
    }
    
    // Dividing line
    printf("─────────────────────────────────\n");
    // Output total fault statistics
    printf(YELLOW "Total Page Faults: %d / %d\n" RESET, faults, ref_len);
    // Output Hit Ratio percentage
    printf(YELLOW "Hit Ratio: %.2f%%\n\n" RESET, ((float)(ref_len - faults) / ref_len) * 100);
}

/* ==============================================================================
 * Comparative Benchmark Execution (FIFO vs LRU vs Optimal)
 * ============================================================================== */
void page_replace_compare(int frames, int *ref_string, int ref_len) {
    // Banner header
    printf(MAGENTA "\n╔══════════════════════════════════════════╗\n");
    printf("║   Comparing Page Replacement Algorithms   ║\n");
    printf("╚══════════════════════════════════════════╝\n" RESET);
    
    // Execute First-In, First-Out simulation
    page_replace_fifo(frames, ref_string, ref_len);
    // Execute Least Recently Used simulation
    page_replace_lru(frames, ref_string, ref_len);
    // Execute Bélády's Optimal simulation
    page_replace_optimal(frames, ref_string, ref_len);
    
    // Explanatory pedagogical takeaway note
    printf(CYAN "Note: Optimal is theoretical (requires future knowledge)\n" RESET);
    printf(CYAN "      and gives the minimum possible page faults.\n\n" RESET);
}
