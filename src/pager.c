#include <stdio.h>                                 // Standard I/O library for formatted console printing (printf)
#include <string.h>                                // String manipulation library
#include "pager.h"                                 // Virtual memory paging header defining constants and prototypes

#define RED     "\033[1;31m"                       // Bright Red: indicates a Page Fault (victim eviction required)
#define GREEN   "\033[1;32m"                       // Bright Green: indicates a Page Hit (referenced page resident)
#define YELLOW  "\033[1;33m"                       // Bright Yellow: highlights statistical totals and hit ratios
#define CYAN    "\033[1;36m"                       // Bright Cyan: styles tabular step column headers
#define MAGENTA "\033[1;35m"                       // Bright Magenta: decorative algorithm banner borders
#define RESET   "\033[0m"                          // Color Reset: restores default terminal foreground styling

static int is_in_frames(int *frames, int n, int page) { // Linear scan: checks if logical page is in physical frames
    for (int i = 0; i < n; i++) {                  // Iterate through all n physical frame slots
        if (frames[i] == page)                     // If current frame slot holds requested page number
            return i;                              // Return frame slot index (indicating a Page Hit)
    }
    return -1;                                     // Return -1 if page absent from physical memory (Page Fault)
}

static void print_frames(int *frames, int n) {     // Formats and prints current physical memory frame occupancy
    printf("[");                                   // Open bracket delimiter for frame array visualization
    for (int i = 0; i < n; i++) {                  // Iterate across all n frame slots
        if (frames[i] == -1)                       // If frame is unallocated (-1 represents an empty frame slot)
            printf(" -");                          // Print dash placeholder for empty slot
        else                                       // If slot contains a valid resident page
            printf(" %d", frames[i]);              // Print resident page number
        if (i < n - 1) printf(",");                // Separate frame slots with comma if not final slot
    }
    printf(" ]");                                  // Close bracket delimiter
}

void page_replace_fifo(int frames, int *ref_string, int ref_len) { // First-In, First-Out (FIFO) page replacement
    printf(MAGENTA "\n========== FIFO Page Replacement (%d frames) ==========\n" RESET, frames); // Print header
    
    int frame[MAX_FRAMES];                         // Array representing physical memory frame slots
    int faults = 0;                                // Accumulator tracking total page fault occurrences
    int ptr = 0;                                   // Circular pointer pointing to oldest loaded frame (victim)
    
    for (int i = 0; i < frames; i++) frame[i] = -1; // Initialize all physical frame slots to -1 (initially empty)
    
    printf(CYAN "%-8s %-15s %-10s\n" RESET, "Ref", "Frames", "Status"); // Output tabular header columns
    printf("─────────────────────────────────\n"); // Horizontal dividing line
    
    for (int i = 0; i < ref_len; i++) {            // Iterate sequentially through page reference string
        int page = ref_string[i];                  // Retrieve current requested logical page number
        printf("  %-6d ", page);                   // Print reference number aligned to column
        
        if (is_in_frames(frame, frames, page) != -1) { // Check if requested page is already resident in memory
            print_frames(frame, frames);           // PAGE HIT: referenced page is present; display frames
            printf(GREEN "  HIT\n" RESET);         // Print green HIT status confirmation
        } else {                                   // PAGE FAULT: page is absent; must load from swap storage
            frame[ptr] = page;                     // Overwrite oldest resident frame slot pointed to by ptr
            ptr = (ptr + 1) % frames;              // Advance circular FIFO pointer: ptr = (ptr + 1) % frames
            faults++;                              // Increment page fault counter
            print_frames(frame, frames);           // Output updated frame array contents
            printf(RED "  FAULT\n" RESET);        // Print red FAULT alert
        }
    }
    
    printf("─────────────────────────────────\n"); // Dividing line
    printf(YELLOW "Total Page Faults: %d / %d\n" RESET, faults, ref_len); // Print total faults against references
    printf(YELLOW "Hit Ratio: %.2f%%\n\n" RESET, ((float)(ref_len - faults) / ref_len) * 100); // Print hit ratio %
}

void page_replace_lru(int frames, int *ref_string, int ref_len) { // Least Recently Used (LRU) page replacement
    printf(MAGENTA "\n========== LRU Page Replacement (%d frames) ==========\n" RESET, frames); // Print header
    
    int frame[MAX_FRAMES];                         // Physical memory frame array
    int last_used[MAX_FRAMES];                     // Timestamp array tracking simulation step when frame was accessed
    int faults = 0;                                // Accumulator for page faults
    
    for (int i = 0; i < frames; i++) {             // Initialize frames and timestamps
        frame[i] = -1;                             // Set frame slot to empty (-1)
        last_used[i] = -1;                         // Set access timestamp to uninitialized (-1)
    }
    
    printf(CYAN "%-8s %-15s %-10s\n" RESET, "Ref", "Frames", "Status"); // Output tabular column headers
    printf("─────────────────────────────────\n"); // Dividing line
    
    for (int i = 0; i < ref_len; i++) {            // Iterate sequentially through page reference string
        int page = ref_string[i];                  // Current requested logical page
        printf("  %-6d ", page);                   // Print reference number
        
        int pos = is_in_frames(frame, frames, page); // Search if page is already resident in physical memory
        
        if (pos != -1) {                           // If page is already resident in frame slot pos
            last_used[pos] = i;                    // PAGE HIT: update last-used timestamp to current step i
            print_frames(frame, frames);           // Print frame states
            printf(GREEN "  HIT\n" RESET);         // Print green HIT status
        } else {                                   // PAGE FAULT: locate frame slot whose last_used is minimal
            int lru_idx = 0;                       // Initialize candidate index to slot 0
            int min_time = last_used[0];           // Initialize candidate minimum timestamp
            
            for (int j = 1; j < frames; j++) {     // Linear scan across all physical frame slots
                if (last_used[j] < min_time) {     // If frame j was accessed further back in past than candidate
                    min_time = last_used[j];       // Update minimum timestamp
                    lru_idx = j;                   // Select frame j as new eviction victim candidate
                }
            }
            
            frame[lru_idx] = page;                 // Evict victim page and place requested page into frame lru_idx
            last_used[lru_idx] = i;                // Record current step i as last-used timestamp for this frame
            faults++;                              // Increment page fault counter
            print_frames(frame, frames);           // Output updated frame array
            printf(RED "  FAULT\n" RESET);        // Print red FAULT alert
        }
    }
    
    printf("─────────────────────────────────\n"); // Dividing line
    printf(YELLOW "Total Page Faults: %d / %d\n" RESET, faults, ref_len); // Output total fault statistics
    printf(YELLOW "Hit Ratio: %.2f%%\n\n" RESET, ((float)(ref_len - faults) / ref_len) * 100); // Hit ratio %
}

void page_replace_optimal(int frames, int *ref_string, int ref_len) { // Bélády's Optimal page replacement algorithm
    printf(MAGENTA "\n========== Optimal Page Replacement (%d frames) ==========\n" RESET, frames); // Header
    
    int frame[MAX_FRAMES];                         // Physical memory frame array
    int faults = 0;                                // Accumulator for page faults
    
    for (int i = 0; i < frames; i++) frame[i] = -1; // Initialize all physical frame slots to empty (-1)
    
    printf(CYAN "%-8s %-15s %-10s\n" RESET, "Ref", "Frames", "Status"); // Output tabular column headers
    printf("─────────────────────────────────\n"); // Dividing line
    
    for (int i = 0; i < ref_len; i++) {            // Iterate sequentially through page reference string
        int page = ref_string[i];                  // Current requested logical page
        printf("  %-6d ", page);                   // Print reference number
        
        if (is_in_frames(frame, frames, page) != -1) { // Check if page is already present in physical frames
            print_frames(frame, frames);           // PAGE HIT: display frame states
            printf(GREEN "  HIT\n" RESET);         // Print green HIT status
            continue;                              // Proceed to next reference
        }
        
        faults++;                                  // PAGE FAULT occurred: increment fault counter
        
        int empty_slot = -1;                       // First check: is there an empty frame slot available?
        for (int j = 0; j < frames; j++) {         // Scan physical frame slots
            if (frame[j] == -1) {                  // If slot is empty (-1)
                empty_slot = j;                    // Select empty slot
                break;                             // Stop search
            }
        }
        
        if (empty_slot != -1) {                    // If an empty slot exists
            frame[empty_slot] = page;              // Place page into empty frame without needing to evict
        } else {                                   // All frames occupied: scan future references for victim
            int farthest = -1;                     // Tracks furthest future reference distance
            int replace_idx = 0;                   // Index of frame chosen for eviction
            
            for (int j = 0; j < frames; j++) {     // Evaluate each currently resident frame j
                int next_use = ref_len;            // Default: assume page in frame j is never referenced again
                for (int k = i + 1; k < ref_len; k++) { // Look ahead through future references from i+1 to end
                    if (ref_string[k] == frame[j]) { // If future reference k accesses page in frame j
                        next_use = k;              // Record future access distance index k
                        break;                     // Break lookahead early (only care about FIRST future access)
                    }
                }
                if (next_use > farthest) {         // If frame j's next access occurs further in future
                    farthest = next_use;           // Update furthest future distance
                    replace_idx = j;               // Select frame j as victim
                }
            }
            
            frame[replace_idx] = page;             // Evict victim page and place requested page into replace_idx
        }
        
        print_frames(frame, frames);               // Output updated frame array
        printf(RED "  FAULT\n" RESET);            // Print red FAULT alert
    }
    
    printf("─────────────────────────────────\n"); // Dividing line
    printf(YELLOW "Total Page Faults: %d / %d\n" RESET, faults, ref_len); // Output total fault statistics
    printf(YELLOW "Hit Ratio: %.2f%%\n\n" RESET, ((float)(ref_len - faults) / ref_len) * 100); // Hit ratio %
}

void page_replace_compare(int frames, int *ref_string, int ref_len) { // Benchmark FIFO vs LRU vs Optimal
    printf(MAGENTA "\n╔══════════════════════════════════════════╗\n"); // Banner top border
    printf("║   Comparing Page Replacement Algorithms   ║\n"); // Banner title
    printf("╚══════════════════════════════════════════╝\n" RESET); // Banner bottom border
    
    page_replace_fifo(frames, ref_string, ref_len);    // Execute First-In, First-Out simulation
    page_replace_lru(frames, ref_string, ref_len);     // Execute Least Recently Used simulation
    page_replace_optimal(frames, ref_string, ref_len); // Execute Bélády's Optimal simulation
    
    printf(CYAN "Note: Optimal is theoretical (requires future knowledge)\n" RESET); // Explanatory takeaway note
    printf(CYAN "      and gives the minimum possible page faults.\n\n" RESET); // Minimum faults guarantee
}
