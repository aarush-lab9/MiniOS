#include <stdio.h>                                 // Standard I/O library for formatted console printing
#include <string.h>                                // String manipulation routines (memset, strncpy, strcmp)
#include "../include/process.h"                    // Process management header defining PCB, states, and prototypes

#define RED     "\033[1;31m"                       // ANSI escape code: Bright Red for error messages
#define GREEN   "\033[1;32m"                       // ANSI escape code: Bright Green for success confirmation
#define YELLOW  "\033[1;33m"                       // ANSI escape code: Bright Yellow for warnings and notices
#define CYAN    "\033[1;36m"                       // ANSI escape code: Bright Cyan for table headers
#define RESET   "\033[0m"                          // ANSI escape code: Reset formatting to default style

PCB process_table[MAX_PROCESSES];                  // Global Process Table array storing resident PCB structures
int process_count = 0;                             // Global integer tracking number of active processes in memory

void init_process_table(void) {                    // Kernel routine to initialize the Process Table
    memset(process_table, 0, sizeof(process_table)); // Zero-fill entire Process Table memory block to clear garbage
    process_count = 0;                             // Reset resident process count to zero
}

const char* state_to_string(ProcessState s) {      // Maps ProcessState enum value to human-readable string
    switch (s) {                                   // Inspect numerical state enum value
        case STATE_NEW:        return "NEW";       // NEW: Created PCB not yet admitted to ready queue
        case STATE_READY:      return "READY";     // READY: Resident in memory awaiting CPU core assignment
        case STATE_RUNNING:    return "RUNNING";   // RUNNING: Actively executing instructions on the processor
        case STATE_WAITING:    return "WAITING";   // WAITING: Blocked waiting for an I/O or sync event
        case STATE_TERMINATED: return "TERMINATED";// TERMINATED: Execution finished; awaiting reclamation
        default:               return "UNKNOWN";   // Fallback label for uninitialized or invalid state codes
    }
}

int create_process(const char *name, int burst, int priority, int arrival) { // Allocates and initializes new PCB
    if (process_count >= MAX_PROCESSES) {          // Guard: verify process table has not reached capacity (20)
        printf(RED "Error: Process table full (max %d)\n" RESET, MAX_PROCESSES); // Emit table saturation error
        return -1;                                 // Return failure code -1
    }
    
    if (burst <= 0) {                              // Guard: ensure requested CPU burst time is strictly positive
        printf(RED "Error: Burst time must be positive\n" RESET); // Emit invalid burst time error
        return -1;                                 // Return failure code -1
    }
    
    for (int i = 0; i < MAX_PROCESSES; i++) {       // Check all table slots to prevent duplicate process names
        if (process_table[i].is_active && strcmp(process_table[i].name, name) == 0) { // If name matches an active PCB
            printf(RED "Error: Process '%s' already exists\n" RESET, name); // Alert naming collision
            return -1;                             // Return failure code -1
        }
    }
    
    int slot = -1;                                 // Initialize target slot index to -1 (not found)
    for (int i = 0; i < MAX_PROCESSES; i++) {       // Linear scan to find the first available free PCB slot
        if (!process_table[i].is_active) {         // Slot is inactive/free
            slot = i;                              // Record index of available slot
            break;                                 // Terminate search upon finding first free slot
        }
    }
    
    if (slot == -1) {                              // Invariant check: verify that a free slot was located
        printf(RED "Error: No free slots\n" RESET);// Emit error if table has no available slot
        return -1;                                 // Return failure code -1
    }
    
    process_table[slot].pid = slot;                // Assign unique PID matching slot index in process table
    strncpy(process_table[slot].name, name, MAX_NAME_LEN - 1); // Safely copy name string into PCB buffer
    process_table[slot].name[MAX_NAME_LEN - 1] = '\0'; // Guarantee null-termination of process name
    process_table[slot].burst_time = burst;        // Set total CPU burst time requested by the job
    process_table[slot].remaining_time = burst;    // Initialize remaining execution time to total burst time
    process_table[slot].arrival_time = arrival;    // Record arrival timestamp in simulation timeline
    process_table[slot].priority = priority;      // Set static priority integer (lower value = higher priority)
    process_table[slot].state = STATE_READY;       // Set lifecycle state to READY for scheduler dispatch
    process_table[slot].waiting_time = 0;          // Initialize waiting time accumulator to zero
    process_table[slot].turnaround_time = 0;       // Initialize turnaround time accumulator to zero
    process_table[slot].completion_time = 0;       // Initialize completion timestamp to zero
    process_table[slot].is_active = 1;             // Mark PCB slot as active and occupied
    
    process_count++;                               // Increment total active process counter
    return slot;                                   // Return assigned Process Identifier (PID)
}

int kill_process(int pid) {                        // Terminates active process by PID and reclaims PCB
    if (pid < 0 || pid >= MAX_PROCESSES) {         // Validate PID falls within legal array index bounds
        printf(RED "Error: Invalid PID %d\n" RESET, pid); // Emit invalid PID error
        return -1;                                 // Return failure code -1
    }
    if (!process_table[pid].is_active) {           // Validate that the target slot contains an active process
        printf(RED "Error: Process %d does not exist\n" RESET, pid); // Emit process not found error
        return -1;                                 // Return failure code -1
    }
    
    process_table[pid].state = STATE_TERMINATED;   // Transition lifecycle state to TERMINATED
    process_table[pid].is_active = 0;              // Mark slot as inactive so it can be re-allocated
    process_count--;                               // Decrement total active process counter
    return 0;                                      // Return 0 indicating successful termination
}

void list_processes(void) {                        // Formats and displays all resident processes in table
    if (process_count == 0) {                      // Check if process table currently has zero active jobs
        printf(YELLOW "No processes in the system.\n" RESET); // Inform user table is empty
        return;                                    // Return early to shell
    }
    
    printf(CYAN "\n%-5s %-12s %-8s %-10s %-10s %-8s %-10s\n" RESET, // Print formatted cyan column headers
           "PID", "Name", "Burst", "Remaining", "Arrival", "Priority", "State");
    printf("───────────────────────────────────────────────────────────────\n"); // Print horizontal divider
    
    for (int i = 0; i < MAX_PROCESSES; i++) {       // Iterate through all slots in the Process Table
        if (process_table[i].is_active) {          // Only print rows for active process entries
            printf("%-5d %-12s %-8d %-10d %-10d %-8d %-10s\n", // Print aligned row for active PCB
                   process_table[i].pid,           // Output PID
                   process_table[i].name,          // Output process name string
                   process_table[i].burst_time,     // Output total CPU burst time
                   process_table[i].remaining_time, // Output remaining CPU burst time
                   process_table[i].arrival_time,   // Output arrival timestamp
                   process_table[i].priority,       // Output static priority value
                   state_to_string(process_table[i].state)); // Output human-readable state string
        }
    }
    printf("\nTotal processes: %d\n\n", process_count); // Print summary of total active processes
}

void reset_scheduling_fields(void) {               // Resets scheduling metrics across all active processes
    for (int i = 0; i < MAX_PROCESSES; i++) {       // Loop through all slots in the Process Table
        if (process_table[i].is_active) {          // Check if slot holds an active process
            process_table[i].remaining_time = process_table[i].burst_time; // Reset remaining time to full burst
            process_table[i].waiting_time = 0;     // Clear cumulative waiting time
            process_table[i].turnaround_time = 0;  // Clear calculated turnaround time
            process_table[i].completion_time = 0;  // Clear completion timestamp
            process_table[i].state = STATE_READY;  // Re-arm state to READY for scheduler dispatch
        }
    }
}
