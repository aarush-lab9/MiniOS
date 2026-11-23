#ifndef PROCESS_H
#define PROCESS_H

// Maximum number of concurrent processes supported in the Process Table
#define MAX_PROCESSES 20

// Maximum length of a process tag/name string including null terminator
#define MAX_NAME_LEN 16

// Enumeration of the standard 5 operating system process lifecycle states
typedef enum {
    STATE_NEW,        // Initial instantiation: PCB allocated but not yet admitted to ready queue
    STATE_READY,      // Resident in memory and prepared for CPU core dispatching
    STATE_RUNNING,    // Currently executing instructions on the simulated processor
    STATE_WAITING,    // Blocked awaiting an external synchronization event or I/O completion
    STATE_TERMINATED  // Execution finished; PCB retained temporarily until reaped
} ProcessState;

// Process Control Block (PCB): Core operating system metadata structure for an active job
typedef struct {
    int pid;                    // Unique Process Identifier assigned by the kernel (0 to MAX_PROCESSES - 1)
    char name[MAX_NAME_LEN];    // Human-readable process tag (e.g., "P1", "P2")
    int burst_time;             // Total processor execution demand required by the process (in ms)
    int remaining_time;         // Remaining CPU time required to finish (decremented during preemptive scheduling)
    int arrival_time;           // Timeline instant at which the process enters the ready queue
    int priority;               // Static scheduling priority integer (lower numerical value = higher priority)
    ProcessState state;         // Current lifecycle state of the process in the 5-state machine
    int waiting_time;           // Cumulative duration spent sitting in the READY state awaiting dispatch
    int turnaround_time;        // Total elapsed time from process arrival to final completion (CT - AT)
    int completion_time;        // Simulation timeline instant at which process completes execution
    int is_active;              // Allocation flag: 1 if this process table slot is occupied, 0 if free
} PCB;

// Global process table array holding all resident Process Control Blocks
extern PCB process_table[MAX_PROCESSES];

// Counter tracking the current number of active processes in the system
extern int process_count;

// Initializes the process table by clearing memory and marking all slots inactive
void init_process_table(void);

// Instantiates a new process, allocates a PCB slot, and initializes attributes
// Returns the assigned PID (>= 0) on success, or -1 if the table is saturated
int create_process(const char *name, int burst, int priority, int arrival);

// Terminates an active process by PID, frees its PCB slot, and updates process count
// Returns 0 on successful termination, or -1 if PID is invalid or inactive
int kill_process(int pid);

// Formats and displays the active Process Table in tabular ANSI format
void list_processes(void);

// Converts a ProcessState enum value to a human-readable string representation
const char* state_to_string(ProcessState s);

// Restores remaining_time to burst_time and zeros all scheduling statistics (WT, TAT, CT)
// Essential for benchmarking multiple scheduling algorithms under identical initial conditions
void reset_scheduling_fields(void);

#endif
