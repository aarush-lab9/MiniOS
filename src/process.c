/*
 * process.c - Process Management Subsystem
 * 
 * Implements the core operating system process control mechanisms:
 * - Process Control Block (PCB) instantiation and tracking
 * - 5-state process lifecycle transitions (NEW, READY, RUNNING, WAITING, TERMINATED)
 * - Process Table allocation, deallocation, and tabular listing
 * - Scheduling field state reset for fair multi-algorithm benchmarking
 */

#include <stdio.h>      // Standard I/O library for console formatting (printf)
#include <string.h>     // String manipulation functions (memset, strncpy, strcmp)
#include "process.h"    // Process management header defining PCB, states, and constants

// ANSI terminal color escape sequence macros for structured, highlighted console output
#define RED     "\033[1;31m"    // Bright Red: indicates error conditions and invalid states
#define GREEN   "\033[1;32m"    // Bright Green: indicates successful allocation and transitions
#define YELLOW  "\033[1;33m"    // Bright Yellow: highlights warnings and summary headers
#define CYAN    "\033[1;36m"    // Bright Cyan: styles tabular column headers
#define RESET   "\033[0m"       // Color Reset: restores default terminal foreground styling

// The primary Process Table: static array storing all active Process Control Blocks
PCB process_table[MAX_PROCESSES];

// Monotonically tracked counter of active processes currently resident in memory
int process_count = 0;

// Initializes the Process Table during kernel boot sequence
void init_process_table(void) {
    // Zero-out all bytes in the Process Table memory block to ensure no garbage data exists
    memset(process_table, 0, sizeof(process_table));
    // Reset the active process count to zero
    process_count = 0;
}

// Maps a ProcessState enum integer to an authentic operating system state string
const char* state_to_string(ProcessState s) {
    // Switch on the numerical lifecycle state enum value
    switch (s) {
        // STATE_NEW: Job has been created but not yet scheduled
        case STATE_NEW:        return "NEW";
        // STATE_READY: Job is loaded into memory, awaiting CPU dispatcher allocation
        case STATE_READY:      return "READY";
        // STATE_RUNNING: Job instructions are actively executing on the CPU core
        case STATE_RUNNING:    return "RUNNING";
        // STATE_WAITING: Job is blocked waiting for an event, lock, or I/O completion
        case STATE_WAITING:    return "WAITING";
        // STATE_TERMINATED: Job has finished execution; waiting to be reaped by parent
        case STATE_TERMINATED: return "TERMINATED";
        // Fallback for an uninitialized or invalid state code
        default:               return "UNKNOWN";
    }
}

// Instantiates a new process and reserves an active slot in the Process Table
int create_process(const char *name, int burst, int priority, int arrival) {
    // Defensive check: verify that the process table has not exceeded maximum capacity (20)
    if (process_count >= MAX_PROCESSES) {
        // Display kernel error alert indicating table saturation
        printf(RED "Error: Process table full (max %d)\n" RESET, MAX_PROCESSES);
        // Return failure code -1 to calling function
        return -1;
    }
    
    // Validation check: ensure CPU burst requirement is strictly positive
    if (burst <= 0) {
        // Reject zero or negative CPU burst allocations
        printf(RED "Error: Burst time must be positive\n" RESET);
        // Return failure code -1
        return -1;
    }
    
    // Uniqueness validation: iterate over all table slots to prevent duplicate process names
    for (int i = 0; i < MAX_PROCESSES; i++) {
        // Check if the current slot is occupied AND matches the requested process name
        if (process_table[i].is_active && strcmp(process_table[i].name, name) == 0) {
            // Emit error alerting duplicate naming collision
            printf(RED "Error: Process '%s' already exists\n" RESET, name);
            // Return failure code -1
            return -1;
        }
    }
    
    // Linear scan to locate the lowest-indexed inactive slot in the Process Table
    int slot = -1; // Initialize candidate slot indicator to -1 (not found)
    for (int i = 0; i < MAX_PROCESSES; i++) {
        // If this slot is marked inactive (free)
        if (!process_table[i].is_active) {
            // Assign the candidate slot index
            slot = i;
            // Break early upon finding the first available slot
            break;
        }
    }
    
    // Invariant check: ensure a valid free slot was located
    if (slot == -1) {
        // Emit error message if no inactive slot was available
        printf(RED "Error: No free slots\n" RESET);
        // Return failure code -1
        return -1;
    }
    
    // Populate the newly allocated Process Control Block (PCB) fields
    process_table[slot].pid = slot; // Set unique Process ID matching the table index
    // Safely copy the process name string into the PCB buffer with bounds protection
    strncpy(process_table[slot].name, name, MAX_NAME_LEN - 1);
    // Explicitly guarantee null-termination of the process name string
    process_table[slot].name[MAX_NAME_LEN - 1] = '\0';
    // Set total processor burst time requested by the job
    process_table[slot].burst_time = burst;
    // Initialize remaining execution time equal to total burst time
    process_table[slot].remaining_time = burst;
    // Record timeline instant of process submission (arrival time)
    process_table[slot].arrival_time = arrival;
    // Set static integer priority (lower numerical value represents higher priority)
    process_table[slot].priority = priority;
    // Transition the newly created process into the READY state
    process_table[slot].state = STATE_READY;
    // Initialize cumulative waiting time counter to zero
    process_table[slot].waiting_time = 0;
    // Initialize turnaround time counter to zero
    process_table[slot].turnaround_time = 0;
    // Initialize timeline completion instant to zero
    process_table[slot].completion_time = 0;
    // Set active flag to 1, marking this slot as occupied in the Process Table
    process_table[slot].is_active = 1;
    
    // Increment the active process count in the system
    process_count++;
    // Return the assigned Process Identifier (PID)
    return slot;
}

// Terminates an active process by PID and reclaims its Process Table slot
int kill_process(int pid) {
    // Bounds validation: verify that the target PID falls within legal table boundaries
    if (pid < 0 || pid >= MAX_PROCESSES) {
        // Emit error for out-of-range PID
        printf(RED "Error: Invalid PID %d\n" RESET, pid);
        // Return failure code -1
        return -1;
    }
    // Liveness check: verify that the targeted slot is actually an active process
    if (!process_table[pid].is_active) {
        // Emit error if attempting to terminate a non-existent or already dead process
        printf(RED "Error: Process %d does not exist\n" RESET, pid);
        // Return failure code -1
        return -1;
    }
    
    // Transition process lifecycle state to TERMINATED (zombie stage prior to reaping)
    process_table[pid].state = STATE_TERMINATED;
    // Mark the table slot inactive (reclaimed for future process creation)
    process_table[pid].is_active = 0;
    // Decrement the active process counter
    process_count--;
    // Return 0 indicating successful process termination
    return 0;
}

// Formats and prints the active Process Table in tabular ANSI format
void list_processes(void) {
    // Check if the system currently has zero active processes
    if (process_count == 0) {
        // Inform the user that the process table is empty
        printf(YELLOW "No processes in the system.\n" RESET);
        // Return early
        return;
    }
    
    // Print the tabular header row with aligned column widths and cyan highlight
    printf(CYAN "\n%-5s %-12s %-8s %-10s %-10s %-8s %-10s\n" RESET,
           "PID", "Name", "Burst", "Remaining", "Arrival", "Priority", "State");
    // Print horizontal dividing line separating column headers from data records
    printf("───────────────────────────────────────────────────────────────\n");
    
    // Iterate through all slots in the static Process Table
    for (int i = 0; i < MAX_PROCESSES; i++) {
        // Only display records for currently active process slots
        if (process_table[i].is_active) {
            // Print formatted row displaying all PCB attributes for process i
            printf("%-5d %-12s %-8d %-10d %-10d %-8d %-10s\n",
                   process_table[i].pid,                     // Process Identifier
                   process_table[i].name,                    // Process Name Tag
                   process_table[i].burst_time,               // Original CPU Burst
                   process_table[i].remaining_time,           // Remaining CPU Time
                   process_table[i].arrival_time,             // Arrival Timeline Instant
                   process_table[i].priority,                 // Priority Rating
                   state_to_string(process_table[i].state));  // Lifecycle State String
        }
    }
    // Output total count of active processes resident in the kernel table
    printf("\nTotal processes: %d\n\n", process_count);
}

// Resets scheduling execution metrics across all active processes prior to a benchmark run
void reset_scheduling_fields(void) {
    // Iterate across all process table entries
    for (int i = 0; i < MAX_PROCESSES; i++) {
        // Only reset fields for active processes
        if (process_table[i].is_active) {
            // Restore remaining execution time back to original burst time demand
            process_table[i].remaining_time = process_table[i].burst_time;
            // Reset cumulative waiting time accumulator to zero
            process_table[i].waiting_time = 0;
            // Reset calculated turnaround time to zero
            process_table[i].turnaround_time = 0;
            // Reset timeline completion instant to zero
            process_table[i].completion_time = 0;
            // Re-arm process state machine to READY for CPU dispatching
            process_table[i].state = STATE_READY;
        }
    }
}
