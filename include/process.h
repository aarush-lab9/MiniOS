#ifndef PROCESS_H                              // Include guard: prevents duplicate definition of process types
#define PROCESS_H                              // Define PROCESS_H macro symbol

#define MAX_PROCESSES 20                       // Maximum number of concurrent processes supported in Process Table
#define MAX_NAME_LEN 16                        // Maximum character length for a process name string

typedef enum {                                 // Enumeration defining the 5-state process lifecycle model
    STATE_NEW,                                 // Initial state: Process Control Block allocated but unadmitted
    STATE_READY,                               // In main memory ready queue waiting for CPU core allocation
    STATE_RUNNING,                             // Currently occupying CPU and executing instructions
    STATE_WAITING,                             // Blocked waiting for an I/O completion or synchronization event
    STATE_TERMINATED                           // Execution finished; resources reclaimed pending table cleanup
} ProcessState;                                // Typedef alias for process lifecycle state enumeration

typedef struct {                               // Process Control Block (PCB): Kernel metadata for an active job
    int pid;                                   // Unique Process Identifier assigned sequentially by kernel
    char name[MAX_NAME_LEN];                   // Human-readable process label (e.g., "P1", "P2")
    int burst_time;                            // Total CPU execution duration required to complete (in ms)
    int remaining_time;                        // Remaining CPU execution time needed (used in preemptive scheduling)
    int arrival_time;                          // Simulation timestamp when process enters the READY state
    int priority;                              // Static priority value (lower numerical integer = higher priority)
    ProcessState state;                        // Current execution state in the 5-state lifecycle state machine
    int waiting_time;                          // Total cumulative time spent waiting in READY queue (TAT - BT)
    int turnaround_time;                       // Total elapsed time from arrival to completion (CT - AT)
    int completion_time;                       // Simulation timeline timestamp at which process finishes
    int is_active;                             // Table slot status flag: 1 if active/allocated, 0 if free/unused
} PCB;                                         // Typedef alias for Process Control Block structure

extern PCB process_table[MAX_PROCESSES];       // Global array of Process Control Blocks (the system Process Table)
extern int process_count;                      // Global integer tracking total active processes currently in table

void init_process_table(void);                 // Initializes process table, zeroing all slots and setting is_active=0
int create_process(const char *name, int burst, int priority, int arrival); // Creates PCB, returns PID (>=0) or -1 if full
int kill_process(int pid);                     // Terminates process by PID and reclaims slot; returns 0 on success
void list_processes(void);                     // Prints tabular formatted summary of all resident processes
const char* state_to_string(ProcessState s);   // Maps ProcessState enum value to human-readable string ("READY", etc.)
void reset_scheduling_fields(void);            // Resets remaining_time to burst_time and clears WT, TAT, CT for fair runs

#endif                                         // End of include guard PROCESS_H
