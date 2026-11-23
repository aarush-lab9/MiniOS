#include <stdio.h>                                 // Standard I/O library for formatted console printing (printf, snprintf)
#include <string.h>                                // String and memory manipulation functions (memcpy, strlen)
#include <limits.h>                                // System integer limits (INT_MAX for minimum search algorithms)
#include "../include/process.h"                               // Process management header defining PCB, ProcessState, process_table
#include "../include/scheduler.h"                             // Scheduler prototypes and Gantt chart rendering declarations

#define RED     "\033[1;31m"                       // Bright Red: highlights errors and preemptions
#define GREEN   "\033[1;32m"                       // Bright Green: highlights optimal results and completions
#define YELLOW  "\033[1;33m"                       // Bright Yellow: highlights calculated summary averages
#define CYAN    "\033[1;36m"                       // Bright Cyan: styles tabular borders and Gantt headers
#define MAGENTA "\033[1;35m"                       // Bright Magenta: styles algorithm banner borders
#define RESET   "\033[0m"                          // Color Reset: restores default terminal foreground styling

static int get_active_indices(int *indices) {      // Scans process_table and populates array with active indices
    int count = 0;                                 // Initialize active process counter to zero
    for (int i = 0; i < MAX_PROCESSES; i++) {      // Iterate through all slots in the static Process Table
        if (process_table[i].is_active) {          // Check if slot i contains an active, resident process
            indices[count++] = i;                  // Record table slot index i into output array
        }
    }
    return count;                                  // Return total count of active processes found
}

static void print_results(int *indices, int n, const char *algo_name) { // Formats post-execution scheduling metrics table
    float total_wt = 0;                            // Accumulator for total waiting time across all processes
    float total_tat = 0;                           // Accumulator for total turnaround time across all processes
    
    printf(CYAN "\n--- %s Results ---\n" RESET, algo_name); // Print algorithm execution results table header
    printf("%-8s %-8s %-10s %-8s %-12s %-12s %-12s\n", // Column headers for scheduling metrics
           "Name", "Arrival", "Burst", "Priority", "Completion", "Turnaround", "Waiting");
    printf("────────────────────────────────────────────────────────────────────────\n"); // Dividing line
    
    for (int i = 0; i < n; i++) {                  // Iterate over all scheduled processes in the index array
        int idx = indices[i];                      // Resolve original table index of process i
        printf("%-8s %-8d %-10d %-8d %-12d %-12d %-12d\n", // Output row displaying metrics
               process_table[idx].name,            // Process Tag (e.g. "P1")
               process_table[idx].arrival_time,    // Submission instant (AT)
               process_table[idx].burst_time,      // CPU burst service demand (BT)
               process_table[idx].priority,        // Priority rating
               process_table[idx].completion_time, // Timeline finish instant (CT)
               process_table[idx].turnaround_time, // Turnaround Time: TAT = CT - AT
               process_table[idx].waiting_time);   // Waiting Time: WT = TAT - BT
        total_wt += (float)process_table[idx].waiting_time; // Accumulate individual waiting time into total
        total_tat += (float)process_table[idx].turnaround_time; // Accumulate individual turnaround time into total
    }
    
    printf("────────────────────────────────────────────────────────────────────────\n"); // Dividing line
    printf(YELLOW "Average Turnaround Time: %.2f\n" RESET, total_tat / (float)n); // Print Mean System Residence Time
    printf(YELLOW "Average Waiting Time   : %.2f\n\n" RESET, total_wt / (float)n); // Print Mean Ready Queue Delay
}

void print_gantt_chart(int *order, int *start_times, int *end_times, int count) { // Renders horizontal ASCII Gantt chart
    printf(CYAN "\nGantt Chart:\n" RESET);         // Gantt chart header
    
    printf(" ");                                   // Space indent for top border
    for (int i = 0; i < count; i++) {              // Draw horizontal segments proportional to durations
        int width = (end_times[i] - start_times[i]) * 3; // Visual width based on slice duration (duration * 3)
        if (width < 4) width = 4;                  // Enforce minimum visual column width of 4 characters
        for (int j = 0; j < width; j++) printf("─"); // Draw top line segment
        printf(" ");                               // Space between adjacent intervals
    }
    printf("\n|");                                 // Newline and starting left border
    
    for (int i = 0; i < count; i++) {              // Center process names or 'idle' within each interval
        int width = (end_times[i] - start_times[i]) * 3; // Calculate cell width
        if (width < 4) width = 4;                  // Minimum cell width
        int idx = order[i];                        // Process index (-1 represents CPU idle time)
        char label[16];                            // Label buffer
        if (idx == -1)                             // If idx is -1, CPU had no ready jobs
            snprintf(label, sizeof(label), "idle"); // Label as idle
        else                                       // Otherwise format process name tag
            snprintf(label, sizeof(label), "%s", process_table[idx].name); // Format process tag
        
        int pad = width - (int)strlen(label);      // Compute padding spaces to horizontally center label
        int left_pad = pad / 2;                    // Left padding
        int right_pad = pad - left_pad;            // Right padding
        for (int j = 0; j < left_pad; j++) printf(" "); // Print left whitespace padding
        printf("%s", label);                       // Print process label
        for (int j = 0; j < right_pad; j++) printf(" "); // Print right whitespace padding
        printf("|");                               // Print cell separator boundary
    }
    printf("\n ");                                 // Newline and indent for bottom border
    
    for (int i = 0; i < count; i++) {              // Render bottom border
        int width = (end_times[i] - start_times[i]) * 3; // Cell width
        if (width < 4) width = 4;                  // Minimum width
        for (int j = 0; j < width; j++) printf("─"); // Bottom line segment
        printf(" ");                               // Space
    }
    printf("\n");                                  // Newline for timeline ticks
    
    printf("%-3d", start_times[0]);                // Print origin time tick (e.g. 0)
    for (int i = 0; i < count; i++) {              // Output timeline time ticks aligned under boundaries
        int width = (end_times[i] - start_times[i]) * 3; // Cell width
        if (width < 4) width = 4;                  // Minimum width
        char time_str[8];                          // Buffer for tick number
        snprintf(time_str, sizeof(time_str), "%d", end_times[i]); // Format end time integer into string
        int pad = width + 1 - (int)strlen(time_str); // Calculate offset padding so number aligns with pipe
        for (int j = 0; j < pad; j++) printf(" "); // Print padding
        printf("%s", time_str);                    // Print time string
    }
    printf("\n\n");                                // Newline break
}

void schedule_fcfs(void) {                         // First-Come, First-Served (FCFS) CPU scheduling
    int indices[MAX_PROCESSES];                    // Array to hold active process indices
    int n = get_active_indices(indices);           // Retrieve count of active jobs
    if (n == 0) return;                            // Guard: abort if no processes exist
    
    reset_scheduling_fields();                     // Reset process remaining times and statistics
    
    printf(MAGENTA "\n========== FCFS Scheduling ==========\n" RESET); // Algorithm execution banner
    
    for (int i = 0; i < n - 1; i++) {              // Sort processes in ascending order of arrival time
        for (int j = 0; j < n - i - 1; j++) {      // Inner bubble sort pass
            if (process_table[indices[j]].arrival_time > process_table[indices[j + 1]].arrival_time) { // Compare arrivals
                int temp = indices[j];             // Swap indices to order by arrival
                indices[j] = indices[j + 1];       // Move earlier arrival forward
                indices[j + 1] = temp;             // Move later arrival backward
            }
        }
    }
    
    int gantt_order[100], gantt_start[100], gantt_end[100]; // Gantt chart recording buffers
    int gantt_count = 0;                           // Number of recorded timeline intervals
    int current_time = 0;                          // Master simulation timeline clock (in ms)
    
    for (int i = 0; i < n; i++) {                  // Dispatch processes in arrival order non-preemptively
        int idx = indices[i];                      // Target process table index
        
        if (current_time < process_table[idx].arrival_time) { // IDLE DETECTION: process has not arrived yet
            gantt_order[gantt_count] = -1;         // Record idle block
            gantt_start[gantt_count] = current_time; // Idle start
            gantt_end[gantt_count] = process_table[idx].arrival_time; // Idle end at arrival
            gantt_count++;                         // Increment Gantt count
            current_time = process_table[idx].arrival_time; // Advance clock to arrival
        }
        
        gantt_order[gantt_count] = idx;            // Record executing PID
        gantt_start[gantt_count] = current_time;   // Slice start time
        gantt_end[gantt_count] = current_time + process_table[idx].burst_time; // Slice end time
        gantt_count++;                             // Increment Gantt count
        
        current_time += process_table[idx].burst_time; // Advance clock by total burst time
        process_table[idx].completion_time = current_time; // CT equals current timeline clock
        process_table[idx].turnaround_time = current_time - process_table[idx].arrival_time; // TAT = CT - AT
        process_table[idx].waiting_time = process_table[idx].turnaround_time - process_table[idx].burst_time; // WT = TAT - BT
        process_table[idx].state = STATE_TERMINATED; // Transition process state to TERMINATED
    }
    
    print_gantt_chart(gantt_order, gantt_start, gantt_end, gantt_count); // Render Gantt chart
    print_results(indices, n, "FCFS");             // Output numerical results table
}

void schedule_sjf(void) {                          // Shortest Job First (SJF - Non-Preemptive) scheduling
    int indices[MAX_PROCESSES];                    // Active process table indices array
    int n = get_active_indices(indices);           // Retrieve active process count
    if (n == 0) return;                            // Abort if no processes exist
    
    reset_scheduling_fields();                     // Reset scheduling fields
    
    printf(MAGENTA "\n========== SJF Scheduling (Non-Preemptive) ==========\n" RESET); // Execution banner
    
    int completed[MAX_PROCESSES] = {0};            // Boolean array tracking completed jobs
    int done = 0;                                  // Completed processes counter
    int current_time = 0;                          // Master simulation timeline clock
    
    int gantt_order[100], gantt_start[100], gantt_end[100]; // Gantt chart recording buffers
    int gantt_count = 0;                           // Gantt interval counter
    
    while (done < n) {                             // Continues until all n processes terminate
        int shortest = -1;                         // Index of shortest arrived job
        int min_burst = INT_MAX;                   // Initialize minimum burst tracker to infinity
        
        for (int i = 0; i < n; i++) {              // Search among arrived, uncompleted processes for min burst
            int idx = indices[i];                  // Process index
            if (!completed[i] && process_table[idx].arrival_time <= current_time) { // Arrived and uncompleted
                if (process_table[idx].burst_time < min_burst) { // Strictly shorter burst time
                    min_burst = process_table[idx].burst_time; // Update candidate burst
                    shortest = i;                  // Select job i
                }
            }
        }
        
        if (shortest == -1) {                      // If no arrived job is ready, advance clock
            current_time++;                        // Advance time by 1 ms
            continue;                              // Re-evaluate ready jobs
        }
        
        int idx = indices[shortest];               // Dispatch shortest job non-preemptively
        gantt_order[gantt_count] = idx;            // Record PID
        gantt_start[gantt_count] = current_time;   // Start time
        gantt_end[gantt_count] = current_time + process_table[idx].burst_time; // End time
        gantt_count++;                             // Increment Gantt count
        
        current_time += process_table[idx].burst_time; // Advance simulation clock by burst duration
        process_table[idx].completion_time = current_time; // Record CT
        process_table[idx].turnaround_time = current_time - process_table[idx].arrival_time; // TAT = CT - AT
        process_table[idx].waiting_time = process_table[idx].turnaround_time - process_table[idx].burst_time; // WT = TAT - BT
        process_table[idx].state = STATE_TERMINATED; // Transition state to TERMINATED
        completed[shortest] = 1;                   // Mark job as completed
        done++;                                    // Increment completed count
    }
    
    print_gantt_chart(gantt_order, gantt_start, gantt_end, gantt_count); // Render Gantt chart
    print_results(indices, n, "SJF (Non-Preemptive)"); // Output numerical results table
}

void schedule_rr(int quantum) {                    // Round Robin (RR - Preemptive) scheduling
    int indices[MAX_PROCESSES];                    // Active process indices array
    int n = get_active_indices(indices);           // Retrieve active process count
    if (n == 0) return;                            // Abort if no processes exist
    
    reset_scheduling_fields();                     // Reset scheduling fields
    
    printf(MAGENTA "\n========== Round Robin (quantum=%d) ==========\n" RESET, quantum); // Execution banner
    
    for (int i = 0; i < n - 1; i++) {              // Initial sort by arrival time
        for (int j = 0; j < n - i - 1; j++) {      // Bubble sort pass
            if (process_table[indices[j]].arrival_time > process_table[indices[j + 1]].arrival_time) { // Compare arrivals
                int temp = indices[j];             // Swap
                indices[j] = indices[j + 1];       // Swap
                indices[j + 1] = temp;             // Swap
            }
        }
    }
    
    int queue[100], front = 0, rear = 0;           // Circular FIFO Ready Queue structures
    int in_queue[MAX_PROCESSES] = {0};             // Bitmask array tracking whether process is queued
    int remaining[MAX_PROCESSES];                  // Dynamic remaining burst time tracking
    
    for (int i = 0; i < n; i++) {                  // Initialize remaining burst array from PCB burst times
        remaining[i] = process_table[indices[i]].burst_time; // Copy burst time
    }
    
    int gantt_order[200], gantt_start[200], gantt_end[200]; // Gantt chart recording buffers
    int gantt_count = 0;                           // Gantt slice counter
    int current_time = 0;                          // Master simulation clock
    int done = 0;                                  // Completed process counter
    
    for (int i = 0; i < n; i++) {                  // Enqueue initial processes arriving at or before time 0
        if (process_table[indices[i]].arrival_time <= 0) { // Arrived at or before 0
            queue[rear++] = i;                     // Enqueue job i at rear
            in_queue[i] = 1;                       // Mark job i as enqueued
        }
    }
    
    while (done < n) {                             // Round Robin dispatching loop
        if (front == rear) {                       // Queue empty check: if no jobs in ready queue, CPU idle
            current_time++;                        // Advance clock to next arrival
            for (int i = 0; i < n; i++) {          // Check if new processes have arrived at updated time
                if (!in_queue[i] && remaining[i] > 0 &&
                    process_table[indices[i]].arrival_time <= current_time) { // Newly arrived job
                    queue[rear++] = i;             // Enqueue arriving job
                    in_queue[i] = 1;               // Mark enqueued
                }
            }
            continue;                              // Re-evaluate queue
        }
        
        int qi = queue[front++];                   // Dequeue process at front of circular ready queue
        int idx = indices[qi];                     // Table index
        
        int exec_time = (remaining[qi] < quantum) ? remaining[qi] : quantum; // Slice = min(remaining, quantum)
        
        gantt_order[gantt_count] = idx;            // Record PID
        gantt_start[gantt_count] = current_time;   // Slice start time
        gantt_end[gantt_count] = current_time + exec_time; // Slice end time
        gantt_count++;                             // Increment Gantt count
        
        current_time += exec_time;                 // Advance master clock by slice duration
        remaining[qi] -= exec_time;                // Decrement remaining burst time by slice duration
        
        for (int i = 0; i < n; i++) {              // CRITICAL: enqueue processes arriving DURING this slice
            if (!in_queue[i] && remaining[i] > 0 &&
                process_table[indices[i]].arrival_time <= current_time) { // Newly arrived job
                queue[rear++] = i;                 // Newly arrived jobs enter before preempted job
                in_queue[i] = 1;                   // Mark enqueued
            }
        }
        
        if (remaining[qi] == 0) {                  // Check if process finished its complete CPU demand
            process_table[idx].completion_time = current_time; // Record CT
            process_table[idx].turnaround_time = current_time - process_table[idx].arrival_time; // TAT = CT - AT
            process_table[idx].waiting_time = process_table[idx].turnaround_time - process_table[idx].burst_time; // WT = TAT - BT
            process_table[idx].state = STATE_TERMINATED; // Transition state to TERMINATED
            done++;                                // Increment completed counter
        } else {                                   // PREEMPTION: job still has remaining burst; re-queue at rear
            queue[rear++] = qi;                    // Re-insert at rear of ready queue
        }
    }
    
    print_gantt_chart(gantt_order, gantt_start, gantt_end, gantt_count); // Render Gantt chart
    print_results(indices, n, "Round Robin");      // Output numerical results table
}

void schedule_priority(void) {                     // Priority (Non-Preemptive) CPU Scheduling
    int indices[MAX_PROCESSES];                    // Active process indices array
    int n = get_active_indices(indices);           // Retrieve active process count
    if (n == 0) return;                            // Abort if no processes exist
    
    reset_scheduling_fields();                     // Reset scheduling fields
    
    printf(MAGENTA "\n========== Priority Scheduling (Non-Preemptive) ==========\n" RESET); // Execution banner
    
    int completed[MAX_PROCESSES] = {0};            // Completion tracking bitmask
    int done = 0;                                  // Completed counter
    int current_time = 0;                          // Simulation clock
    
    int gantt_order[100], gantt_start[100], gantt_end[100]; // Gantt chart recording buffers
    int gantt_count = 0;                           // Gantt slice counter
    
    while (done < n) {                             // Continues until all processes finish
        int best = -1;                             // Highest priority candidate index
        int best_prio = INT_MAX;                   // Initialize priority threshold to worst possible
        
        for (int i = 0; i < n; i++) {              // Search for arrived job with HIGHEST priority (lowest int)
            int idx = indices[i];                  // Process index
            if (!completed[i] && process_table[idx].arrival_time <= current_time) { // Arrived and uncompleted
                if (process_table[idx].priority < best_prio) { // Lower integer = higher priority
                    best_prio = process_table[idx].priority; // Update best priority
                    best = i;                      // Select candidate
                }
            }
        }
        
        if (best == -1) {                          // If no arrived job is ready, advance time
            current_time++;                        // Advance time
            continue;                              // Re-evaluate
        }
        
        int idx = indices[best];                   // Dispatch chosen highest-priority process non-preemptively
        gantt_order[gantt_count] = idx;            // Record PID
        gantt_start[gantt_count] = current_time;   // Start time
        gantt_end[gantt_count] = current_time + process_table[idx].burst_time; // End time
        gantt_count++;                             // Increment Gantt count
        
        current_time += process_table[idx].burst_time; // Advance clock by burst time
        process_table[idx].completion_time = current_time; // Record CT
        process_table[idx].turnaround_time = current_time - process_table[idx].arrival_time; // TAT = CT - AT
        process_table[idx].waiting_time = process_table[idx].turnaround_time - process_table[idx].burst_time; // WT = TAT - BT
        process_table[idx].state = STATE_TERMINATED; // Transition state to TERMINATED
        completed[best] = 1;                       // Mark completed
        done++;                                    // Increment completed counter
    }
    
    print_gantt_chart(gantt_order, gantt_start, gantt_end, gantt_count); // Render Gantt chart
    print_results(indices, n, "Priority (Non-Preemptive)"); // Output numerical results table
}

void schedule_mlfq(void) {                         // Multilevel Feedback Queue (3-Tier MLFQ) scheduling
    int indices[MAX_PROCESSES];                    // Active process indices array
    int n = get_active_indices(indices);           // Retrieve active process count
    if (n == 0) return;                            // Abort if no processes exist
    
    reset_scheduling_fields();                     // Reset scheduling fields
    
    printf(MAGENTA "\n========== Multilevel Feedback Queue (3 levels) ==========\n" RESET); // Execution banner
    printf("Queue 0: RR q=1 | Queue 1: RR q=2 | Queue 2: FCFS\n\n"); // Queue tier configuration description
    
    for (int i = 0; i < n - 1; i++) {              // Sort processes by arrival time
        for (int j = 0; j < n - i - 1; j++) {      // Bubble sort pass
            if (process_table[indices[j]].arrival_time > process_table[indices[j + 1]].arrival_time) { // Compare arrivals
                int temp = indices[j];             // Swap
                indices[j] = indices[j + 1];       // Swap
                indices[j + 1] = temp;             // Swap
            }
        }
    }
    
    int q0[50], q0_front = 0, q0_rear = 0;         // Highest priority tier: Round Robin with quantum = 1 ms
    int q1[50], q1_front = 0, q1_rear = 0;         // Medium priority tier: Round Robin with quantum = 2 ms
    int q2[50], q2_front = 0, q2_rear = 0;         // Lowest priority tier: FCFS non-preemptive queue
    
    int remaining[MAX_PROCESSES];                  // Dynamic remaining burst time tracking
    int level[MAX_PROCESSES];                      // Tracks current priority queue tier (0, 1, or 2)
    int arrived[MAX_PROCESSES] = {0};              // Bitmask tracking job arrival admission
    
    for (int i = 0; i < n; i++) {                  // Initialize burst times and assign jobs to start in Queue 0
        remaining[i] = process_table[indices[i]].burst_time; // Copy burst
        level[i] = 0;                              // All jobs begin with highest priority (Tier 0)
    }
    
    int gantt_order[200], gantt_start[200], gantt_end[200]; // Gantt chart recording buffers
    int gantt_count = 0;                           // Gantt slice counter
    int current_time = 0;                          // Simulation clock
    int done = 0;                                  // Completed process counter
    int quantum[] = {1, 2, 999};                   // Quantum for each tier: Q0=1ms, Q1=2ms, Q2=large (FCFS)
    
    while (done < n) {                             // MLFQ execution loop
        for (int i = 0; i < n; i++) {              // Check for new job arrivals and admit directly into Queue 0
            if (!arrived[i] && process_table[indices[i]].arrival_time <= current_time) { // Newly arrived job
                q0[q0_rear++] = i;                 // Enqueue newly arrived process in Queue 0
                arrived[i] = 1;                    // Mark admitted
            }
        }
        
        int qi = -1;                               // Process index to dispatch
        int q_level = -1;                          // Queue level of dispatched process
        
        if (q0_front < q0_rear) {                  // Tier 0 has absolute priority over Tier 1 and Tier 2
            qi = q0[q0_front++];                   // Dequeue from Tier 0
            q_level = 0;                           // Set level 0
        } else if (q1_front < q1_rear) {           // Tier 1 executes only when Tier 0 is empty
            qi = q1[q1_front++];                   // Dequeue from Tier 1
            q_level = 1;                           // Set level 1
        } else if (q2_front < q2_rear) {           // Tier 2 executes only when both Tier 0 and Tier 1 are empty
            qi = q2[q2_front++];                   // Dequeue from Tier 2
            q_level = 2;                           // Set level 2
        }
        
        if (qi == -1) {                            // If all queues are empty, advance clock
            current_time++;                        // Advance time
            continue;                              // Re-evaluate
        }
        
        int idx = indices[qi];                     // Table index
        int exec_time = (remaining[qi] < quantum[q_level]) ? remaining[qi] : quantum[q_level]; // Slice duration
        
        gantt_order[gantt_count] = idx;            // Record PID
        gantt_start[gantt_count] = current_time;   // Start time
        gantt_end[gantt_count] = current_time + exec_time; // End time
        gantt_count++;                             // Increment Gantt count
        
        current_time += exec_time;                 // Advance clock by slice
        remaining[qi] -= exec_time;                // Decrement remaining burst
        
        for (int i = 0; i < n; i++) {              // Enqueue processes arriving during slice directly into Queue 0
            if (!arrived[i] && process_table[indices[i]].arrival_time <= current_time) { // Arrived during slice
                q0[q0_rear++] = i;                 // Enqueue at rear of Queue 0
                arrived[i] = 1;                    // Mark admitted
            }
        }
        
        if (remaining[qi] == 0) {                  // Check if process finished execution
            process_table[idx].completion_time = current_time; // Record CT
            process_table[idx].turnaround_time = current_time - process_table[idx].arrival_time; // TAT = CT - AT
            process_table[idx].waiting_time = process_table[idx].turnaround_time - process_table[idx].burst_time; // WT = TAT - BT
            process_table[idx].state = STATE_TERMINATED; // Transition state to TERMINATED
            done++;                                // Increment completed counter
        } else {                                   // FEEDBACK DEMOTION: penalize to lower priority queue tier
            if (q_level == 0) {                    // Demote from Queue 0 to Queue 1
                level[qi] = 1;                     // Set level 1
                q1[q1_rear++] = qi;                // Insert into Queue 1 rear
            } else if (q_level == 1) {             // Demote from Queue 1 to Queue 2 (background FCFS)
                level[qi] = 2;                     // Set level 2
                q2[q2_rear++] = qi;                // Insert into Queue 2 rear
            } else {                               // Already at lowest tier (Queue 2); remain in Queue 2
                q2[q2_rear++] = qi;                // Re-insert into Queue 2 rear
            }
        }
    }
    
    print_gantt_chart(gantt_order, gantt_start, gantt_end, gantt_count); // Render Gantt chart
    print_results(indices, n, "MLFQ");             // Output numerical results table
}

void schedule_compare(void) {                      // Benchmarks all 5 schedulers side-by-side on identical input
    printf(MAGENTA "\n╔══════════════════════════════════════════╗\n"); // Comparative benchmark banner
    printf("║     Comparing All Scheduling Algorithms   ║\n"); // Banner title
    printf("╚══════════════════════════════════════════╝\n" RESET); // Banner bottom border
    
    PCB backup[MAX_PROCESSES];                     // Save original state of Process Table for fair comparison
    memcpy(backup, process_table, sizeof(process_table)); // Backup process table
    
    float avg_wt[5], avg_tat[5];                   // Arrays to store average WT and TAT for each policy
    const char *names[] = {"FCFS", "SJF", "RR(q=2)", "Priority", "MLFQ"}; // Algorithm names
    int indices[MAX_PROCESSES];                    // Active process indices array
    int n;                                         // Active process count
    
    memcpy(process_table, backup, sizeof(process_table)); // Benchmark 1: FCFS - restore input state
    schedule_fcfs();                               // Run FCFS
    n = get_active_indices(indices);               // Retrieve process count
    {                                              // Calculate metrics block
        float tw = 0.0f, tt = 0.0f;                // Accumulators
        for (int i = 0; i < n; i++) {              // Sum metrics
            tw += (float)process_table[indices[i]].waiting_time; // Sum WT
            tt += (float)process_table[indices[i]].turnaround_time; // Sum TAT
        }
        avg_wt[0] = tw / (float)n; avg_tat[0] = tt / (float)n; // Compute averages
    }
    
    memcpy(process_table, backup, sizeof(process_table)); // Benchmark 2: SJF - restore input state
    schedule_sjf();                                // Run SJF
    {                                              // Calculate metrics block
        float tw = 0.0f, tt = 0.0f;                // Accumulators
        for (int i = 0; i < n; i++) {              // Sum metrics
            tw += (float)process_table[indices[i]].waiting_time; // Sum WT
            tt += (float)process_table[indices[i]].turnaround_time; // Sum TAT
        }
        avg_wt[1] = tw / (float)n; avg_tat[1] = tt / (float)n; // Compute averages
    }
    
    memcpy(process_table, backup, sizeof(process_table)); // Benchmark 3: Round Robin - restore input state
    schedule_rr(2);                                // Run RR with quantum = 2
    {                                              // Calculate metrics block
        float tw = 0.0f, tt = 0.0f;                // Accumulators
        for (int i = 0; i < n; i++) {              // Sum metrics
            tw += (float)process_table[indices[i]].waiting_time; // Sum WT
            tt += (float)process_table[indices[i]].turnaround_time; // Sum TAT
        }
        avg_wt[2] = tw / (float)n; avg_tat[2] = tt / (float)n; // Compute averages
    }
    
    memcpy(process_table, backup, sizeof(process_table)); // Benchmark 4: Priority - restore input state
    schedule_priority();                           // Run Priority
    {                                              // Calculate metrics block
        float tw = 0.0f, tt = 0.0f;                // Accumulators
        for (int i = 0; i < n; i++) {              // Sum metrics
            tw += (float)process_table[indices[i]].waiting_time; // Sum WT
            tt += (float)process_table[indices[i]].turnaround_time; // Sum TAT
        }
        avg_wt[3] = tw / (float)n; avg_tat[3] = tt / (float)n; // Compute averages
    }
    
    memcpy(process_table, backup, sizeof(process_table)); // Benchmark 5: MLFQ - restore input state
    schedule_mlfq();                               // Run MLFQ
    {                                              // Calculate metrics block
        float tw = 0.0f, tt = 0.0f;                // Accumulators
        for (int i = 0; i < n; i++) {              // Sum metrics
            tw += (float)process_table[indices[i]].waiting_time; // Sum WT
            tt += (float)process_table[indices[i]].turnaround_time; // Sum TAT
        }
        avg_wt[4] = tw / (float)n; avg_tat[4] = tt / (float)n; // Compute averages
    }
    
    printf(CYAN "\n╔═══════════════════════════════════════════════════════╗\n"); // Summary table top border
    printf("║              Algorithm Comparison Summary              ║\n"); // Table title
    printf("╠═══════════════╦═══════════════════╦═══════════════════╣\n"); // Divider
    printf("║  Algorithm    ║  Avg Wait Time    ║  Avg Turnaround   ║\n"); // Column headers
    printf("╠═══════════════╬═══════════════════╬═══════════════════╣\n" RESET); // Divider
    
    float best_wt = avg_wt[0];                     // Identify algorithm achieving minimum Average Waiting Time
    int best_wt_idx = 0;                           // Index of optimal algorithm
    for (int i = 1; i < 5; i++) {                  // Scan all 5 algorithms
        if (avg_wt[i] < best_wt) {                 // Lower average waiting time found
            best_wt = avg_wt[i];                   // Update best WT
            best_wt_idx = i;                       // Update index
        }
    }
    
    for (int i = 0; i < 5; i++) {                  // Print each algorithm row, highlighting optimal in green
        if (i == best_wt_idx)                      // If this is the optimal algorithm
            printf(GREEN);                         // Set green highlight
        printf("║  %-12s ║      %8.2f     ║      %8.2f     ║\n", // Output formatted row
               names[i], avg_wt[i], avg_tat[i]);
        if (i == best_wt_idx)                      // Reset highlight
            printf(RESET);                         // Reset color
    }
    printf(CYAN "╚═══════════════╩═══════════════════╩═══════════════════╝\n" RESET); // Summary bottom border
    printf(GREEN "\nBest Average Waiting Time: %s (%.2f)\n\n" RESET, names[best_wt_idx], best_wt); // Announce optimal
    
    memcpy(process_table, backup, sizeof(process_table)); // Restore original Process Table state from backup
}
