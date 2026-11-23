/*
 * sync.c - Process Synchronization & Deadlock Avoidance Subsystem
 * 
 * Implements two foundational operating system concurrency paradigms:
 * 1. Bounded-Buffer Producer-Consumer Problem:
 *    - Modeled over a 5-slot ring buffer
 *    - Synchronized via counting semaphores (empty, full) and binary semaphore (mutex)
 *    - Prevents race conditions and buffer overflow/underflow hazards
 * 2. Edsger Dijkstra's Banker's Algorithm:
 *    - Multi-resource deadlock avoidance via state-space safety evaluation
 *    - Tracks Allocation, Max, Available, and Need matrices (Need = Max - Allocation)
 *    - Dynamically searches for a safe process execution sequence
 */

#include <stdio.h>      // Standard I/O library for console interaction (printf, scanf, getchar)
#include <stdlib.h>     // Standard library functions
#include <string.h>     // String manipulation functions (strcmp, strcat, snprintf)
#include "sync.h"       // Synchronization header defining BUFFER_SIZE, MAX_RESOURCES, prototypes

// ANSI terminal color escape sequence macros for highlighting execution states
#define RED     "\033[1;31m"    // Bright Red: indicates blocked semaphore waits and unsafe deadlock states
#define GREEN   "\033[1;32m"    // Bright Green: indicates successful production and safe system states
#define YELLOW  "\033[1;33m"    // Bright Yellow: indicates consumption operations and warnings
#define CYAN    "\033[1;36m"    // Bright Cyan: styles matrix table headers
#define MAGENTA "\033[1;35m"    // Bright Magenta: decorative section banners
#define RESET   "\033[0m"       // Color Reset: restores default terminal foreground styling

/* ==============================================================================
 * Bounded-Buffer Producer-Consumer Simulation
 * ============================================================================== */
void run_producer_consumer(void) {
    // Print module execution banner
    printf(MAGENTA "\n========== Producer-Consumer Problem ==========\n" RESET);
    // Display buffer capacity and step count
    printf("Buffer size: %d | Simulating 10 steps\n\n", BUFFER_SIZE);
    
    // Shared ring buffer array with BUFFER_SIZE (5) slots
    int buffer[BUFFER_SIZE];
    // Circular write index pointing to next free slot for the Producer
    int in_idx = 0;
    // Circular read index pointing to next available slot for the Consumer
    int out_idx = 0;
    // Sequential counter generating monotonic numerical items (1, 2, 3...)
    int item = 1;
    
    // Simulated Counting Semaphore: tracks count of available unallocated buffer slots
    int empty = BUFFER_SIZE;  // Initialized to 5 (entire buffer is initially empty)
    // Simulated Counting Semaphore: tracks count of occupied buffer slots containing data
    int full = 0;             // Initialized to 0 (no items in buffer initially)
    // Simulated Binary Semaphore (Mutex): guarantees mutual exclusion in the critical section
    int mutex = 1;            // Initialized to 1 (lock is initially unlocked)
    
    // Initialize all shared buffer slots to 0 (representing empty cells)
    for (int i = 0; i < BUFFER_SIZE; i++) buffer[i] = 0;
    
    // Output tabular header showing step number, action, semaphore values, and buffer map
    printf(CYAN "%-6s %-12s %-8s %-8s %-8s  Buffer State\n" RESET,
           "Step", "Action", "empty", "full", "mutex");
    // Horizontal dividing line
    printf("──────────────────────────────────────────────────────────\n");
    
    // Pre-scripted interleaved sequence of produce and consume actions modeling concurrency
    char *actions[] = {"produce", "produce", "produce", "consume",
                       "produce", "consume", "produce", "consume",
                       "consume", "consume"};
    // Total discrete simulation steps to execute
    int num_steps = 10;
    
    // Step-by-step discrete event loop
    for (int step = 0; step < num_steps; step++) {
        // String buffer to build visual representation of the ring buffer
        char buf_str[64] = "[";
        
        // PRODUCER LOGIC: Producer attempts to write an item into the shared buffer
        if (strcmp(actions[step], "produce") == 0) {
            // Check condition: if empty <= 0, the buffer is completely full
            if (empty <= 0) {
                // Producer blocks: cannot produce into a full buffer
                printf("%-6d " RED "BLOCKED" RESET "     (buffer full, producer waits)\n", step + 1);
                // Render current buffer state string
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    char tmp[8];
                    snprintf(tmp, sizeof(tmp), "%d", buffer[i]);
                    strcat(buf_str, tmp);
                    if (i < BUFFER_SIZE - 1) strcat(buf_str, ",");
                }
                strcat(buf_str, "]");
                // Skip remainder of step (producer is suspended in waiting queue)
                continue;
            }
            
            // wait(empty): atomically decrement empty slot counter
            empty--;
            // wait(mutex): acquire critical section lock (set mutex to 0)
            mutex = 0;
            
            // CRITICAL SECTION: insert item into buffer at circular index in_idx
            buffer[in_idx] = item;
            // Advance circular producer pointer: in = (in + 1) % BUFFER_SIZE
            in_idx = (in_idx + 1) % BUFFER_SIZE;
            
            // signal(mutex): release critical section lock (restore mutex to 1)
            mutex = 1;
            // signal(full): atomically increment full item counter
            full++;
            
            // Construct visual buffer display string showing items and underscores for empty slots
            for (int i = 0; i < BUFFER_SIZE; i++) {
                char tmp[8];
                if (buffer[i] == 0) snprintf(tmp, sizeof(tmp), "_");
                else snprintf(tmp, sizeof(tmp), "%d", buffer[i]);
                strcat(buf_str, tmp);
                if (i < BUFFER_SIZE - 1) strcat(buf_str, ",");
            }
            strcat(buf_str, "]");
            
            // Print green confirmation row showing action, semaphores, and resulting buffer
            printf("%-6d " GREEN "Produce %-4d" RESET " %-8d %-8d %-8d  %s\n",
                   step + 1, item, empty, full, mutex, buf_str);
            // Increment item value for next production
            item++;
        }
        // CONSUMER LOGIC: Consumer attempts to read an item from the shared buffer
        else {
            // Check condition: if full <= 0, the buffer is completely empty
            if (full <= 0) {
                // Build buffer string
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    char tmp[8];
                    if (buffer[i] == 0) snprintf(tmp, sizeof(tmp), "_");
                    else snprintf(tmp, sizeof(tmp), "%d", buffer[i]);
                    strcat(buf_str, tmp);
                    if (i < BUFFER_SIZE - 1) strcat(buf_str, ",");
                }
                strcat(buf_str, "]");
                // Consumer blocks: cannot extract from an empty buffer
                printf("%-6d " RED "BLOCKED" RESET "      (buffer empty, consumer waits)  %s\n",
                       step + 1, buf_str);
                // Skip remainder of step (consumer is suspended in waiting queue)
                continue;
            }
            
            // wait(full): atomically decrement occupied slot counter
            full--;
            // wait(mutex): acquire mutual exclusion lock
            mutex = 0;
            
            // CRITICAL SECTION: retrieve item from buffer at index out_idx
            int consumed = buffer[out_idx];
            // Clear buffer cell (reset to 0 representing empty)
            buffer[out_idx] = 0;
            // Advance circular consumer pointer: out = (out + 1) % BUFFER_SIZE
            out_idx = (out_idx + 1) % BUFFER_SIZE;
            
            // signal(mutex): release mutual exclusion lock
            mutex = 1;
            // signal(empty): atomically increment free slot counter
            empty++;
            
            // Construct visual buffer display string
            for (int i = 0; i < BUFFER_SIZE; i++) {
                char tmp[8];
                if (buffer[i] == 0) snprintf(tmp, sizeof(tmp), "_");
                else snprintf(tmp, sizeof(tmp), "%d", buffer[i]);
                strcat(buf_str, tmp);
                if (i < BUFFER_SIZE - 1) strcat(buf_str, ",");
            }
            strcat(buf_str, "]");
            
            // Print yellow confirmation row showing action, semaphores, and resulting buffer
            printf("%-6d " YELLOW "Consume %-4d" RESET " %-8d %-8d %-8d  %s\n",
                   step + 1, consumed, empty, full, mutex, buf_str);
        }
    }
    // Newline break after simulation
    printf("\n");
}

/* ==============================================================================
 * Edsger Dijkstra's Banker's Algorithm for Deadlock Avoidance
 * ============================================================================== */
void run_bankers(void) {
    // Print algorithm execution banner
    printf(MAGENTA "\n========== Banker's Algorithm ==========\n" RESET);
    
    int n, m;  // n = number of processes, m = number of resource types
    
    // Prompt user for process count
    printf("Enter number of processes (max %d): ", MAX_SYNC_PROC);
    // Read and validate integer input
    if (scanf("%d", &n) != 1 || n <= 0 || n > MAX_SYNC_PROC) {
        printf(RED "Invalid input\n" RESET);
        while (getchar() != '\n'); // Flush input buffer
        return;
    }
    
    // Prompt user for resource types count
    printf("Enter number of resource types (max %d): ", MAX_RESOURCES);
    // Read and validate resource count
    if (scanf("%d", &m) != 1 || m <= 0 || m > MAX_RESOURCES) {
        printf(RED "Invalid input\n" RESET);
        while (getchar() != '\n'); // Flush input buffer
        return;
    }
    
    // Allocation Matrix (n x m): instances of each resource currently allocated to each process
    int alloc[MAX_SYNC_PROC][MAX_RESOURCES];
    // Max Matrix (n x m): maximum resource claim of each process over its entire execution
    int max[MAX_SYNC_PROC][MAX_RESOURCES];
    // Available Vector (m): count of unallocated available instances of each resource type
    int avail[MAX_RESOURCES];
    // Need Matrix (n x m): remaining resources needed by each process (Need = Max - Allocation)
    int need[MAX_SYNC_PROC][MAX_RESOURCES];
    
    // Read Allocation Matrix values from user input
    printf("\nEnter Allocation Matrix (%d x %d):\n", n, m);
    for (int i = 0; i < n; i++) {
        printf("  P%d: ", i);
        for (int j = 0; j < m; j++) {
            scanf("%d", &alloc[i][j]);
        }
    }
    
    // Read Max Matrix values from user input
    printf("Enter Max Matrix (%d x %d):\n", n, m);
    for (int i = 0; i < n; i++) {
        printf("  P%d: ", i);
        for (int j = 0; j < m; j++) {
            scanf("%d", &max[i][j]);
        }
    }
    
    // Read Available Vector values from user input
    printf("Enter Available Resources (%d values): ", m);
    for (int j = 0; j < m; j++) {
        scanf("%d", &avail[j]);
    }
    
    // Consume remaining newline character in standard input buffer
    while (getchar() != '\n');
    
    // Compute and display the Need Matrix: Need[i][j] = Max[i][j] - Allocation[i][j]
    printf(CYAN "\n--- Need Matrix (Max - Allocation) ---\n" RESET);
    printf("     ");
    // Print resource column headers (R0, R1, R2...)
    for (int j = 0; j < m; j++) printf("R%-3d", j);
    printf("\n");
    
    // Calculate each entry in the Need Matrix
    for (int i = 0; i < n; i++) {
        printf("P%d:  ", i);
        for (int j = 0; j < m; j++) {
            // Need invariant: remaining demand equals peak demand minus currently held instances
            need[i][j] = max[i][j] - alloc[i][j];
            // Print calculated need value
            printf("%-4d", need[i][j]);
        }
        printf("\n");
    }
    
    // BANKER'S SAFETY ALGORITHM EXECUTION
    // Work Vector (m): temporary working copy of available resources
    int work[MAX_RESOURCES];
    // Finish Vector (n): boolean flags tracking whether process i can safely finish
    int finish[MAX_SYNC_PROC] = {0};
    // Safe Sequence Array: records the ordered sequence of process completion
    int safe_seq[MAX_SYNC_PROC];
    // Counter of processes successfully added to the safe sequence
    int seq_count = 0;
    
    // Step 1: Initialize Work = Available for all resource types j
    for (int j = 0; j < m; j++) work[j] = avail[j];
    
    // Print safety trace header
    printf(CYAN "\n--- Safety Algorithm Execution ---\n" RESET);
    
    int found; // Flag indicating if an eligible process was found in the current pass
    do {
        found = 0; // Reset flag at start of each iteration
        // Step 2: Search for an uncompleted process i whose Need <= Work
        for (int i = 0; i < n; i++) {
            // Skip process i if it has already been evaluated and finished
            if (finish[i]) continue;
            
            // Check condition: Need[i][j] <= Work[j] for ALL resource types j
            int can_run = 1; // Assume process can run unless a resource deficit is found
            for (int j = 0; j < m; j++) {
                // If need exceeds currently available work vector
                if (need[i][j] > work[j]) {
                    can_run = 0; // Process cannot execute at this point
                    break;
                }
            }
            
            // If process i satisfies the safety condition
            if (can_run) {
                // Print detailed step trace showing Need <= Work comparison
                printf("  P%d can run: Need(", i);
                for (int j = 0; j < m; j++) {
                    printf("%d", need[i][j]);
                    if (j < m - 1) printf(",");
                }
                printf(") <= Work(");
                for (int j = 0; j < m; j++) {
                    printf("%d", work[j]);
                    if (j < m - 1) printf(",");
                }
                printf(")\n");
                
                // Step 3: Simulate completion of process i: Work = Work + Allocation[i]
                for (int j = 0; j < m; j++) {
                    // Reclaim resources held by process i back into the available pool
                    work[j] += alloc[i][j];
                }
                
                // Print updated Work vector after resource reclamation
                printf("         Work updated to (");
                for (int j = 0; j < m; j++) {
                    printf("%d", work[j]);
                    if (j < m - 1) printf(",");
                }
                printf(")\n");
                
                // Mark process i as finished
                finish[i] = 1;
                // Append process i to the Safe Sequence
                safe_seq[seq_count++] = i;
                // Set flag to true indicating progress was made
                found = 1;
            }
        }
    } while (found); // Repeat search as long as at least one new process finishes per pass
    
    // Step 4: Evaluate overall system safety state
    // If all n processes successfully completed (seq_count == n)
    if (seq_count == n) {
        // System is in a SAFE state: deadlock is impossible under this execution order
        printf(GREEN "\nSystem is in a SAFE state!\n" RESET);
        printf(GREEN "Safe Sequence: < ");
        // Print the calculated safe sequence
        for (int i = 0; i < n; i++) {
            printf("P%d", safe_seq[i]);
            if (i < n - 1) printf(" -> ");
        }
        printf(" >\n\n" RESET);
    } else {
        // System is in an UNSAFE state: deadlocks can potentially occur
        printf(RED "\nSystem is in an UNSAFE state! Deadlock possible.\n" RESET);
        printf(RED "Processes that cannot finish: ");
        // Identify processes that were starved due to circular dependency
        for (int i = 0; i < n; i++) {
            if (!finish[i]) printf("P%d ", i);
        }
        printf("\n\n" RESET);
    }
}
