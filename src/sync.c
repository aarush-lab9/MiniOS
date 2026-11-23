#include <stdio.h>                                 // Standard I/O library for console interaction (printf, scanf)
#include <stdlib.h>                                // Standard library functions
#include <string.h>                                // String manipulation functions (strcmp, strcat, snprintf)
#include "../include/sync.h"                       // Synchronization header defining constants and prototypes

#define RED     "\033[1;31m"                       // Bright Red: indicates blocked semaphore waits and unsafe states
#define GREEN   "\033[1;32m"                       // Bright Green: indicates successful production and safe states
#define YELLOW  "\033[1;33m"                       // Bright Yellow: indicates consumption operations and warnings
#define CYAN    "\033[1;36m"                       // Bright Cyan: styles matrix table headers
#define MAGENTA "\033[1;35m"                       // Bright Magenta: decorative section banners
#define RESET   "\033[0m"                          // Color Reset: restores default terminal foreground styling

void run_producer_consumer(void) {                 // Bounded-Buffer Producer-Consumer synchronization simulation
    printf(MAGENTA "\n========== Producer-Consumer Problem ==========\n" RESET); // Module execution banner
    printf("Buffer size: %d | Simulating 10 steps\n\n", BUFFER_SIZE); // Display buffer capacity and step count
    
    int buffer[BUFFER_SIZE];                       // Shared ring buffer array with BUFFER_SIZE (5) slots
    int in_idx = 0;                                // Circular write index pointing to next free slot for Producer
    int out_idx = 0;                               // Circular read index pointing to next available slot for Consumer
    int item = 1;                                  // Sequential counter generating monotonic numerical items (1, 2...)
    
    int empty = BUFFER_SIZE;                       // Counting Semaphore: available unallocated buffer slots (init=5)
    int full = 0;                                  // Counting Semaphore: occupied buffer slots containing data (init=0)
    int mutex = 1;                                 // Binary Semaphore (Mutex): mutual exclusion in critical section
    
    for (int i = 0; i < BUFFER_SIZE; i++) buffer[i] = 0; // Initialize shared buffer slots to 0 (empty cells)
    
    printf(CYAN "%-6s %-12s %-8s %-8s %-8s  Buffer State\n" RESET, // Tabular header showing semaphores and state
           "Step", "Action", "empty", "full", "mutex");
    printf("──────────────────────────────────────────────────────────\n"); // Dividing line
    
    char *actions[] = {"produce", "produce", "produce", "consume", // Interleaved sequence modeling concurrent actions
                       "produce", "consume", "produce", "consume",
                       "consume", "consume"};
    int num_steps = 10;                            // Total discrete simulation steps to execute
    
    for (int step = 0; step < num_steps; step++) { // Step-by-step discrete event loop
        char buf_str[64] = "[";                    // String buffer to build visual representation of ring buffer
        
        if (strcmp(actions[step], "produce") == 0) { // PRODUCER LOGIC: attempt to write item into shared buffer
            if (empty <= 0) {                      // If empty <= 0, the buffer is completely full
                printf("%-6d " RED "BLOCKED" RESET "     (buffer full, producer waits)\n", step + 1); // Producer blocks
                for (int i = 0; i < BUFFER_SIZE; i++) { // Render current buffer state string
                    char tmp[8];                   // Temp string
                    snprintf(tmp, sizeof(tmp), "%d", buffer[i]); // Format item number
                    strcat(buf_str, tmp);          // Append to buffer string
                    if (i < BUFFER_SIZE - 1) strcat(buf_str, ","); // Comma delimiter
                }
                strcat(buf_str, "]");              // Close bracket
                continue;                          // Skip remainder of step (producer suspended)
            }
            
            empty--;                               // wait(empty): atomically decrement empty slot counter
            mutex = 0;                             // wait(mutex): acquire critical section lock
            
            buffer[in_idx] = item;                 // CRITICAL SECTION: insert item into buffer at index in_idx
            in_idx = (in_idx + 1) % BUFFER_SIZE;   // Advance circular producer pointer: in = (in + 1) % BUFFER_SIZE
            
            mutex = 1;                             // signal(mutex): release critical section lock
            full++;                                // signal(full): atomically increment full item counter
            
            for (int i = 0; i < BUFFER_SIZE; i++) { // Construct visual buffer display string
                char tmp[8];                       // Temp string
                if (buffer[i] == 0) snprintf(tmp, sizeof(tmp), "_"); // Underscore for empty cell
                else snprintf(tmp, sizeof(tmp), "%d", buffer[i]); // Item number for occupied cell
                strcat(buf_str, tmp);              // Append to buffer string
                if (i < BUFFER_SIZE - 1) strcat(buf_str, ","); // Comma delimiter
            }
            strcat(buf_str, "]");                  // Close bracket
            
            printf("%-6d " GREEN "Produce %-4d" RESET " %-8d %-8d %-8d  %s\n", // Print green production confirmation
                   step + 1, item, empty, full, mutex, buf_str);
            item++;                                // Increment item value for next production
        } else {                                   // CONSUMER LOGIC: attempt to read item from shared buffer
            if (full <= 0) {                       // If full <= 0, the buffer is completely empty
                for (int i = 0; i < BUFFER_SIZE; i++) { // Build buffer string
                    char tmp[8];                   // Temp string
                    if (buffer[i] == 0) snprintf(tmp, sizeof(tmp), "_"); // Underscore for empty
                    else snprintf(tmp, sizeof(tmp), "%d", buffer[i]); // Item number
                    strcat(buf_str, tmp);          // Append
                    if (i < BUFFER_SIZE - 1) strcat(buf_str, ","); // Delimiter
                }
                strcat(buf_str, "]");              // Close bracket
                printf("%-6d " RED "BLOCKED" RESET "      (buffer empty, consumer waits)  %s\n", // Consumer blocks
                       step + 1, buf_str);
                continue;                          // Skip remainder of step (consumer suspended)
            }
            
            full--;                                // wait(full): atomically decrement occupied slot counter
            mutex = 0;                             // wait(mutex): acquire mutual exclusion lock
            
            int consumed = buffer[out_idx];        // CRITICAL SECTION: retrieve item from buffer at out_idx
            buffer[out_idx] = 0;                   // Clear buffer cell (reset to 0 representing empty)
            out_idx = (out_idx + 1) % BUFFER_SIZE; // Advance circular consumer pointer: out = (out + 1) % BUFFER_SIZE
            
            mutex = 1;                             // signal(mutex): release mutual exclusion lock
            empty++;                               // signal(empty): atomically increment free slot counter
            
            for (int i = 0; i < BUFFER_SIZE; i++) { // Construct visual buffer display string
                char tmp[8];                       // Temp string
                if (buffer[i] == 0) snprintf(tmp, sizeof(tmp), "_"); // Underscore for empty
                else snprintf(tmp, sizeof(tmp), "%d", buffer[i]); // Item number
                strcat(buf_str, tmp);              // Append
                if (i < BUFFER_SIZE - 1) strcat(buf_str, ","); // Delimiter
            }
            strcat(buf_str, "]");                  // Close bracket
            
            printf("%-6d " YELLOW "Consume %-4d" RESET " %-8d %-8d %-8d  %s\n", // Print yellow consumption confirmation
                   step + 1, consumed, empty, full, mutex, buf_str);
        }
    }
    printf("\n");                                  // Newline break after simulation
}

void run_bankers(void) {                           // Edsger Dijkstra's Banker's Algorithm for Deadlock Avoidance
    printf(MAGENTA "\n========== Banker's Algorithm ==========\n" RESET); // Algorithm execution banner
    
    int n, m;                                      // n = number of processes, m = number of resource types
    
    printf("Enter number of processes (max %d): ", MAX_SYNC_PROC); // Prompt user for process count
    if (scanf("%d", &n) != 1 || n <= 0 || n > MAX_SYNC_PROC) { // Read and validate integer input
        printf(RED "Invalid input\n" RESET);       // Alert invalid input
        while (getchar() != '\n');                 // Flush input buffer
        return;                                    // Return early on invalid input
    }
    
    printf("Enter number of resource types (max %d): ", MAX_RESOURCES); // Prompt user for resource types count
    if (scanf("%d", &m) != 1 || m <= 0 || m > MAX_RESOURCES) { // Read and validate resource count
        printf(RED "Invalid input\n" RESET);       // Alert invalid input
        while (getchar() != '\n');                 // Flush input buffer
        return;                                    // Return early on invalid input
    }
    
    int alloc[MAX_SYNC_PROC][MAX_RESOURCES];       // Allocation Matrix: instances currently allocated to processes
    int max[MAX_SYNC_PROC][MAX_RESOURCES];         // Max Matrix: maximum resource claim of each process
    int avail[MAX_RESOURCES];                      // Available Vector: count of unallocated available instances
    int need[MAX_SYNC_PROC][MAX_RESOURCES];        // Need Matrix: remaining resources needed (Need = Max - Alloc)
    
    printf("\nEnter Allocation Matrix (%d x %d):\n", n, m); // Read Allocation Matrix values
    for (int i = 0; i < n; i++) {                  // Process loop
        printf("  P%d: ", i);                      // Process label
        for (int j = 0; j < m; j++) {              // Resource loop
            scanf("%d", &alloc[i][j]);             // Read allocated resource instance count
        }
    }
    
    printf("Enter Max Matrix (%d x %d):\n", n, m); // Read Max Matrix values
    for (int i = 0; i < n; i++) {                  // Process loop
        printf("  P%d: ", i);                      // Process label
        for (int j = 0; j < m; j++) {              // Resource loop
            scanf("%d", &max[i][j]);               // Read maximum claim count
        }
    }
    
    printf("Enter Available Resources (%d values): ", m); // Read Available Vector values
    for (int j = 0; j < m; j++) {                  // Resource loop
        scanf("%d", &avail[j]);                    // Read available resource instances
    }
    
    while (getchar() != '\n');                     // Consume remaining newline character in standard input buffer
    
    printf(CYAN "\n--- Need Matrix (Max - Allocation) ---\n" RESET); // Compute and display Need Matrix
    printf("     ");                               // Indent
    for (int j = 0; j < m; j++) printf("R%-3d", j); // Print resource column headers (R0, R1, R2...)
    printf("\n");                                  // Newline
    
    for (int i = 0; i < n; i++) {                  // Calculate each entry in the Need Matrix
        printf("P%d:  ", i);                       // Print process row label
        for (int j = 0; j < m; j++) {              // Resource column loop
            need[i][j] = max[i][j] - alloc[i][j];  // Need invariant: remaining demand = peak demand - currently held
            printf("%-4d", need[i][j]);            // Print calculated need value
        }
        printf("\n");                              // Newline at end of row
    }
    
    int work[MAX_RESOURCES];                       // Work Vector: temporary working copy of available resources
    int finish[MAX_SYNC_PROC] = {0};               // Finish Vector: boolean flags tracking if process i can finish
    int safe_seq[MAX_SYNC_PROC];                   // Safe Sequence Array: records ordered sequence of completion
    int seq_count = 0;                             // Counter of processes successfully added to safe sequence
    
    for (int j = 0; j < m; j++) work[j] = avail[j]; // Step 1: Initialize Work = Available for all resource types j
    
    printf(CYAN "\n--- Safety Algorithm Execution ---\n" RESET); // Print safety trace header
    
    int found;                                     // Flag indicating if an eligible process was found in current pass
    do {                                           // Safety evaluation pass loop
        found = 0;                                 // Reset flag at start of each iteration
        for (int i = 0; i < n; i++) {              // Step 2: Search for an uncompleted process i whose Need <= Work
            if (finish[i]) continue;               // Skip process i if it has already been evaluated and finished
            
            int can_run = 1;                       // Assume process can run unless resource deficit is found
            for (int j = 0; j < m; j++) {          // Check condition: Need[i][j] <= Work[j] for ALL resource types j
                if (need[i][j] > work[j]) {        // If need exceeds currently available work vector
                    can_run = 0;                   // Process cannot execute at this point
                    break;                         // Exit resource check early
                }
            }
            
            if (can_run) {                         // If process i satisfies the safety condition
                printf("  P%d can run: Need(", i); // Print trace showing Need <= Work comparison
                for (int j = 0; j < m; j++) {      // Loop over resource needs
                    printf("%d", need[i][j]);      // Print need instance
                    if (j < m - 1) printf(",");    // Comma delimiter
                }
                printf(") <= Work(");              // Work vector label
                for (int j = 0; j < m; j++) {      // Loop over work vector
                    printf("%d", work[j]);         // Print work instance
                    if (j < m - 1) printf(",");    // Comma delimiter
                }
                printf(")\n");                     // Newline
                
                for (int j = 0; j < m; j++) {      // Step 3: Simulate completion: Work = Work + Allocation[i]
                    work[j] += alloc[i][j];        // Reclaim resources held by process i back into available pool
                }
                
                printf("         Work updated to ("); // Print updated Work vector after resource reclamation
                for (int j = 0; j < m; j++) {      // Loop over work vector
                    printf("%d", work[j]);         // Print updated work instance
                    if (j < m - 1) printf(",");    // Comma delimiter
                }
                printf(")\n");                     // Newline
                
                finish[i] = 1;                     // Mark process i as finished
                safe_seq[seq_count++] = i;         // Append process i to the Safe Sequence
                found = 1;                         // Set flag to true indicating progress was made
            }
        }
    } while (found);                               // Repeat search as long as at least one new process finishes
    
    if (seq_count == n) {                          // Step 4: If all n processes completed (seq_count == n)
        printf(GREEN "\nSystem is in a SAFE state!\n" RESET); // System is SAFE: deadlock is impossible
        printf(GREEN "Safe Sequence: < ");         // Announce safe sequence
        for (int i = 0; i < n; i++) {              // Print the calculated safe sequence
            printf("P%d", safe_seq[i]);            // Print process tag
            if (i < n - 1) printf(" -> ");         // Transition arrow
        }
        printf(" >\n\n" RESET);                    // Close bracket
    } else {                                       // System is in an UNSAFE state: deadlocks can potentially occur
        printf(RED "\nSystem is in an UNSAFE state! Deadlock possible.\n" RESET); // Unsafe alert
        printf(RED "Processes that cannot finish: "); // Identify starved processes
        for (int i = 0; i < n; i++) {              // Scan unfinished processes
            if (!finish[i]) printf("P%d ", i);     // Print starved process ID
        }
        printf("\n\n" RESET);                      // Newline
    }
}
