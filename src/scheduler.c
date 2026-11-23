/*
 * scheduler.c - CPU Scheduling Subsystem
 * 
 * Implements core operating system processor dispatching policies:
 * 1. First-Come, First-Served (FCFS) - Non-preemptive arrival-ordered scheduling
 * 2. Shortest Job First (SJF) - Non-preemptive burst-minimizing policy
 * 3. Round Robin (RR) - Preemptive time-sliced circular ready queue scheduling
 * 4. Priority Scheduling - Non-preemptive static priority dispatching
 * 5. Multilevel Feedback Queue (MLFQ) - 3-tier dynamic priority demotion system
 * 
 * Includes automated ASCII horizontal Gantt chart rendering and side-by-side
 * comparative metric evaluations (Turnaround Time, Waiting Time, Completion Time).
 */

#include <stdio.h>      // Standard I/O library for formatted console printing (printf, snprintf)
#include <string.h>     // String and memory manipulation functions (memcpy, strlen)
#include <limits.h>     // System integer limits (INT_MAX for minimum search algorithms)
#include "process.h"    // Process management header defining PCB, ProcessState, process_table
#include "scheduler.h"  // Scheduler prototypes and Gantt chart rendering declarations

// ANSI terminal color escape sequence macros for structured, highlighted console output
#define RED     "\033[1;31m"    // Bright Red: highlights errors and preemptions
#define GREEN   "\033[1;32m"    // Bright Green: highlights optimal results and completions
#define YELLOW  "\033[1;33m"    // Bright Yellow: highlights calculated summary averages
#define CYAN    "\033[1;36m"    // Bright Cyan: styles tabular borders and Gantt headers
#define MAGENTA "\033[1;35m"    // Bright Magenta: styles algorithm banner borders
#define RESET   "\033[0m"       // Color Reset: restores default terminal foreground styling

// Helper function: scans process_table and populates an array with indices of all active processes
static int get_active_indices(int *indices) {
    int count = 0; // Initialize active process counter to zero
    // Iterate through all slots in the static Process Table
    for (int i = 0; i < MAX_PROCESSES; i++) {
        // Check if slot i contains an active, resident process
        if (process_table[i].is_active) {
            // Record table slot index i into the output array
            indices[count++] = i;
        }
    }
    // Return total count of active processes found
    return count;
}

// Formats and displays the post-execution scheduling metrics table
static void print_results(int *indices, int n, const char *algo_name) {
    float total_wt = 0;   // Accumulator for total waiting time across all processes
    float total_tat = 0;  // Accumulator for total turnaround time across all processes
    
    // Print algorithm execution results table header
    printf(CYAN "\n--- %s Results ---\n" RESET, algo_name);
    // Column headers for all operating system scheduling metrics
    printf("%-8s %-8s %-10s %-8s %-12s %-12s %-12s\n",
           "Name", "Arrival", "Burst", "Priority", "Completion", "Turnaround", "Waiting");
    // Horizontal dividing line
    printf("────────────────────────────────────────────────────────────────────────\n");
    
    // Iterate over all scheduled processes in the index array
    for (int i = 0; i < n; i++) {
        // Resolve original table index of process i
        int idx = indices[i];
        // Output row displaying process identity, workload parameters, and measured metrics
        printf("%-8s %-8d %-10d %-8d %-12d %-12d %-12d\n",
               process_table[idx].name,             // Process Tag (e.g. "P1")
               process_table[idx].arrival_time,     // Submission instant (AT)
               process_table[idx].burst_time,       // CPU burst service demand (BT)
               process_table[idx].priority,         // Priority rating
               process_table[idx].completion_time,  // Timeline finish instant (CT)
               process_table[idx].turnaround_time,  // Turnaround Time: TAT = CT - AT
               process_table[idx].waiting_time);    // Waiting Time: WT = TAT - BT
        // Accumulate individual waiting time into total
        total_wt += process_table[idx].waiting_time;
        // Accumulate individual turnaround time into total
        total_tat += process_table[idx].turnaround_time;
    }
    
    // Dividing line
    printf("────────────────────────────────────────────────────────────────────────\n");
    // Calculate and print Average Turnaround Time (Mean System Residence Time)
    printf(YELLOW "Average Turnaround Time: %.2f\n" RESET, total_tat / n);
    // Calculate and print Average Waiting Time (Mean Ready Queue Delay)
    printf(YELLOW "Average Waiting Time   : %.2f\n\n" RESET, total_wt / n);
}

// Renders an authentic ASCII horizontal Gantt chart representing CPU allocation timelines
void print_gantt_chart(int *order, int *start_times, int *end_times, int count) {
    // Gantt chart header
    printf(CYAN "\nGantt Chart:\n" RESET);
    
    // RENDER TOP BORDER: Draw horizontal dashed segments proportional to time durations
    printf(" ");
    for (int i = 0; i < count; i++) {
        // Compute visual width based on slice duration: duration * 3 characters
        int width = (end_times[i] - start_times[i]) * 3;
        // Enforce minimum visual column width of 4 characters for readability
        if (width < 4) width = 4;
        // Draw top line segment
        for (int j = 0; j < width; j++) printf("─");
        // Space between adjacent intervals
        printf(" ");
    }
    // Newline and starting left border
    printf("\n|");
    
    // RENDER PROCESS LABELS: Center process names or 'idle' within each interval
    for (int i = 0; i < count; i++) {
        // Calculate cell width
        int width = (end_times[i] - start_times[i]) * 3;
        if (width < 4) width = 4;
        int idx = order[i]; // Process index (-1 represents CPU idle time)
        char label[16];
        // If idx is -1, CPU had no ready jobs during this timeline window
        if (idx == -1)
            snprintf(label, sizeof(label), "idle");
        else
            // Otherwise format process name tag
            snprintf(label, sizeof(label), "%s", process_table[idx].name);
        
        // Compute padding spaces to horizontally center the label
        int pad = width - (int)strlen(label);
        int left_pad = pad / 2;
        int right_pad = pad - left_pad;
        // Print left whitespace padding
        for (int j = 0; j < left_pad; j++) printf(" ");
        // Print process label
        printf("%s", label);
        // Print right whitespace padding
        for (int j = 0; j < right_pad; j++) printf(" ");
        // Print cell separator boundary
        printf("|");
    }
    printf("\n ");
    
    // RENDER BOTTOM BORDER
    for (int i = 0; i < count; i++) {
        int width = (end_times[i] - start_times[i]) * 3;
        if (width < 4) width = 4;
        for (int j = 0; j < width; j++) printf("─");
        printf(" ");
    }
    printf("\n");
    
    // RENDER TIMELINE TICKS: Output timeline time instant numbers aligned under cell boundaries
    printf("%-3d", start_times[0]); // Print origin time tick (e.g. 0)
    for (int i = 0; i < count; i++) {
        int width = (end_times[i] - start_times[i]) * 3;
        if (width < 4) width = 4;
        char time_str[8];
        // Format end time integer into string
        snprintf(time_str, sizeof(time_str), "%d", end_times[i]);
        // Calculate offset padding so number aligns with the right pipe boundary
        int pad = width + 1 - (int)strlen(time_str);
        for (int j = 0; j < pad; j++) printf(" ");
        printf("%s", time_str);
    }
    printf("\n\n");
}

/* ==============================================================================
 * FCFS (First-Come, First-Served) CPU Scheduling Algorithm
 * ============================================================================== */
void schedule_fcfs(void) {
    int indices[MAX_PROCESSES]; // Array to hold active process indices
    int n = get_active_indices(indices); // Retrieve count of active jobs
    if (n == 0) return; // Guard: abort if no processes exist
    
    // Reset process remaining times and statistics prior to execution
    reset_scheduling_fields();
    
    // Print algorithm execution banner
    printf(MAGENTA "\n========== FCFS Scheduling ==========\n" RESET);
    
    // Sort processes in ascending order of arrival time using bubble sort
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            // Compare arrival times of adjacent jobs
            if (process_table[indices[j]].arrival_time > process_table[indices[j + 1]].arrival_time) {
                // Swap indices to order by arrival
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }
    
    // Gantt chart recording buffers
    int gantt_order[100], gantt_start[100], gantt_end[100];
    int gantt_count = 0;    // Number of recorded timeline intervals
    int current_time = 0;   // Master simulation timeline clock (in ms)
    
    // Dispatch processes in arrival order non-preemptively
    for (int i = 0; i < n; i++) {
        int idx = indices[i]; // Target process table index
        
        // IDLE DETECTION: if process has not arrived yet, CPU remains idle until arrival
        if (current_time < process_table[idx].arrival_time) {
            gantt_order[gantt_count] = -1;                                // Record idle block
            gantt_start[gantt_count] = current_time;                      // Idle start
            gantt_end[gantt_count] = process_table[idx].arrival_time;    // Idle end at arrival
            gantt_count++;                                                // Increment Gantt count
            current_time = process_table[idx].arrival_time;              // Advance clock to arrival
        }
        
        // DISPATCH PROCESS: allocate CPU to process idx non-preemptively
        gantt_order[gantt_count] = idx;                                   // Record executing PID
        gantt_start[gantt_count] = current_time;                         // Slice start time
        gantt_end[gantt_count] = current_time + process_table[idx].burst_time; // Slice end time
        gantt_count++;                                                    // Increment Gantt count
        
        // Advance master simulation timeline clock by total burst time
        current_time += process_table[idx].burst_time;
        // Invariant: Completion Time (CT) equals current timeline clock
        process_table[idx].completion_time = current_time;
        // Invariant: Turnaround Time (TAT) = Completion Time - Arrival Time
        process_table[idx].turnaround_time = current_time - process_table[idx].arrival_time;
        // Invariant: Waiting Time (WT) = Turnaround Time - Burst Time
        process_table[idx].waiting_time = process_table[idx].turnaround_time - process_table[idx].burst_time;
        // Transition process state to TERMINATED
        process_table[idx].state = STATE_TERMINATED;
    }
    
    // Render Gantt chart and output numerical results table
    print_gantt_chart(gantt_order, gantt_start, gantt_end, gantt_count);
    print_results(indices, n, "FCFS");
}

/* ==============================================================================
 * SJF (Shortest Job First - Non-Preemptive) CPU Scheduling Algorithm
 * ============================================================================== */
void schedule_sjf(void) {
    int indices[MAX_PROCESSES];
    int n = get_active_indices(indices);
    if (n == 0) return;
    
    // Reset scheduling fields
    reset_scheduling_fields();
    
    // Print algorithm execution banner
    printf(MAGENTA "\n========== SJF Scheduling (Non-Preemptive) ==========\n" RESET);
    
    // Boolean array tracking completed jobs
    int completed[MAX_PROCESSES] = {0};
    int done = 0;           // Completed processes counter
    int current_time = 0;   // Master simulation timeline clock
    
    // Gantt chart recording buffers
    int gantt_order[100], gantt_start[100], gantt_end[100];
    int gantt_count = 0;
    
    // Main scheduling loop: continues until all n processes terminate
    while (done < n) {
        // Search among all arrived, uncompleted processes for the one with MINIMUM burst time
        int shortest = -1;
        int min_burst = INT_MAX; // Initialize minimum burst tracker to infinity
        
        // Iterate through all active jobs
        for (int i = 0; i < n; i++) {
            int idx = indices[i];
            // Condition: job must NOT be completed AND job must have already arrived
            if (!completed[i] && process_table[idx].arrival_time <= current_time) {
                // If this job's burst time is strictly shorter than current candidate
                if (process_table[idx].burst_time < min_burst) {
                    min_burst = process_table[idx].burst_time; // Update candidate burst
                    shortest = i;                              // Select job i
                }
            }
        }
        
        // If no arrived job is ready, advance simulation clock to wait for next arrival
        if (shortest == -1) {
            current_time++; // Advance time by 1 ms
            continue;       // Re-evaluate ready jobs
        }
        
        // Dispatch the shortest job non-preemptively
        int idx = indices[shortest];
        gantt_order[gantt_count] = idx;
        gantt_start[gantt_count] = current_time;
        gantt_end[gantt_count] = current_time + process_table[idx].burst_time;
        gantt_count++;
        
        // Advance simulation clock by process burst duration
        current_time += process_table[idx].burst_time;
        // Invariant calculations: CT, TAT, and WT
        process_table[idx].completion_time = current_time;
        process_table[idx].turnaround_time = current_time - process_table[idx].arrival_time;
        process_table[idx].waiting_time = process_table[idx].turnaround_time - process_table[idx].burst_time;
        process_table[idx].state = STATE_TERMINATED;
        // Mark job as completed
        completed[shortest] = 1;
        // Increment completed count
        done++;
    }
    
    // Render Gantt chart and output numerical results table
    print_gantt_chart(gantt_order, gantt_start, gantt_end, gantt_count);
    print_results(indices, n, "SJF (Non-Preemptive)");
}

/* ==============================================================================
 * Round Robin (RR - Preemptive) CPU Scheduling Algorithm
 * ============================================================================== */
void schedule_rr(int quantum) {
    int indices[MAX_PROCESSES];
    int n = get_active_indices(indices);
    if (n == 0) return;
    
    // Reset scheduling fields
    reset_scheduling_fields();
    
    // Print algorithm execution banner with configured time quantum
    printf(MAGENTA "\n========== Round Robin (quantum=%d) ==========\n" RESET, quantum);
    
    // Initial sort by arrival time
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (process_table[indices[j]].arrival_time > process_table[indices[j + 1]].arrival_time) {
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }
    
    // Circular FIFO Ready Queue structures
    int queue[100], front = 0, rear = 0;
    // Bitmask array tracking whether a process is currently queued
    int in_queue[MAX_PROCESSES] = {0};
    // Array tracking dynamic remaining burst time for each job
    int remaining[MAX_PROCESSES];
    
    // Initialize remaining burst array from PCB burst times
    for (int i = 0; i < n; i++) {
        remaining[i] = process_table[indices[i]].burst_time;
    }
    
    // Gantt chart recording buffers
    int gantt_order[200], gantt_start[200], gantt_end[200];
    int gantt_count = 0;
    int current_time = 0;   // Master simulation clock
    int done = 0;           // Completed process counter
    
    // Enqueue initial processes that arrive at or before time 0
    for (int i = 0; i < n; i++) {
        if (process_table[indices[i]].arrival_time <= 0) {
            queue[rear++] = i; // Enqueue job i at rear
            in_queue[i] = 1;   // Mark job i as enqueued
        }
    }
    
    // Round Robin dispatching loop
    while (done < n) {
        // Queue empty check: if no jobs are in ready queue, CPU is idle
        if (front == rear) {
            current_time++; // Advance clock to next arrival
            // Check if any new processes have arrived at the updated time
            for (int i = 0; i < n; i++) {
                if (!in_queue[i] && remaining[i] > 0 &&
                    process_table[indices[i]].arrival_time <= current_time) {
                    queue[rear++] = i; // Enqueue arriving job
                    in_queue[i] = 1;
                }
            }
            continue; // Re-evaluate queue
        }
        
        // Dequeue process at front of the circular ready queue
        int qi = queue[front++];
        int idx = indices[qi];
        
        // Calculate execution time slice: exec = min(remaining, quantum)
        int exec_time = (remaining[qi] < quantum) ? remaining[qi] : quantum;
        
        // Record interval into Gantt chart structures
        gantt_order[gantt_count] = idx;
        gantt_start[gantt_count] = current_time;
        gantt_end[gantt_count] = current_time + exec_time;
        gantt_count++;
        
        // Advance master clock by time slice duration
        current_time += exec_time;
        // Decrement process remaining burst time by slice duration
        remaining[qi] -= exec_time;
        
        // CRITICAL INVARIANT: check and enqueue any new processes arriving DURING this slice
        for (int i = 0; i < n; i++) {
            if (!in_queue[i] && remaining[i] > 0 &&
                process_table[indices[i]].arrival_time <= current_time) {
                queue[rear++] = i; // Newly arrived jobs enter before preempted job
                in_queue[i] = 1;
            }
        }
        
        // Check if process has finished its complete CPU demand
        if (remaining[qi] == 0) {
            // Process terminated: record completion, turnaround, and waiting times
            process_table[idx].completion_time = current_time;
            process_table[idx].turnaround_time = current_time - process_table[idx].arrival_time;
            process_table[idx].waiting_time = process_table[idx].turnaround_time - process_table[idx].burst_time;
            process_table[idx].state = STATE_TERMINATED;
            done++; // Increment completed counter
        } else {
            // PREEMPTION: job still has remaining burst; re-queue at the rear of ready queue
            queue[rear++] = qi;
        }
    }
    
    // Render Gantt chart and output numerical results table
    print_gantt_chart(gantt_order, gantt_start, gantt_end, gantt_count);
    print_results(indices, n, "Round Robin");
}

/* ==============================================================================
 * Priority (Non-Preemptive) CPU Scheduling Algorithm
 * ============================================================================== */
void schedule_priority(void) {
    int indices[MAX_PROCESSES];
    int n = get_active_indices(indices);
    if (n == 0) return;
    
    // Reset scheduling fields
    reset_scheduling_fields();
    
    // Print algorithm execution banner
    printf(MAGENTA "\n========== Priority Scheduling (Non-Preemptive) ==========\n" RESET);
    
    int completed[MAX_PROCESSES] = {0};
    int done = 0;
    int current_time = 0;
    
    // Gantt chart recording buffers
    int gantt_order[100], gantt_start[100], gantt_end[100];
    int gantt_count = 0;
    
    while (done < n) {
        // Search for arrived process with HIGHEST priority (lowest integer value)
        int best = -1;
        int best_prio = INT_MAX; // Initialize priority threshold to worst possible
        
        for (int i = 0; i < n; i++) {
            int idx = indices[i];
            // Process must be uncompleted AND have arrived
            if (!completed[i] && process_table[idx].arrival_time <= current_time) {
                // If priority integer is strictly smaller (higher priority)
                if (process_table[idx].priority < best_prio) {
                    best_prio = process_table[idx].priority;
                    best = i;
                }
            }
        }
        
        // If no arrived job is ready, advance time
        if (best == -1) {
            current_time++;
            continue;
        }
        
        // Dispatch chosen highest-priority process non-preemptively
        int idx = indices[best];
        gantt_order[gantt_count] = idx;
        gantt_start[gantt_count] = current_time;
        gantt_end[gantt_count] = current_time + process_table[idx].burst_time;
        gantt_count++;
        
        // Advance clock and calculate metrics
        current_time += process_table[idx].burst_time;
        process_table[idx].completion_time = current_time;
        process_table[idx].turnaround_time = current_time - process_table[idx].arrival_time;
        process_table[idx].waiting_time = process_table[idx].turnaround_time - process_table[idx].burst_time;
        process_table[idx].state = STATE_TERMINATED;
        completed[best] = 1;
        done++;
    }
    
    // Render Gantt chart and output numerical results table
    print_gantt_chart(gantt_order, gantt_start, gantt_end, gantt_count);
    print_results(indices, n, "Priority (Non-Preemptive)");
}

/* ==============================================================================
 * MLFQ (Multilevel Feedback Queue - 3 Tiers) CPU Scheduling Algorithm
 * ============================================================================== */
void schedule_mlfq(void) {
    int indices[MAX_PROCESSES];
    int n = get_active_indices(indices);
    if (n == 0) return;
    
    // Reset scheduling fields
    reset_scheduling_fields();
    
    // Print algorithm execution banner showing queue tier configuration
    printf(MAGENTA "\n========== Multilevel Feedback Queue (3 levels) ==========\n" RESET);
    printf("Queue 0: RR q=1 | Queue 1: RR q=2 | Queue 2: FCFS\n\n");
    
    // Sort processes by arrival time
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (process_table[indices[j]].arrival_time > process_table[indices[j + 1]].arrival_time) {
                int temp = indices[j];
                indices[j] = indices[j + 1];
                indices[j + 1] = temp;
            }
        }
    }
    
    // Three discrete priority queue arrays
    int q0[50], q0_front = 0, q0_rear = 0;  // Highest priority tier: Round Robin with quantum = 1 ms
    int q1[50], q1_front = 0, q1_rear = 0;  // Medium priority tier: Round Robin with quantum = 2 ms
    int q2[50], q2_front = 0, q2_rear = 0;  // Lowest priority tier: FCFS non-preemptive queue
    
    int remaining[MAX_PROCESSES];           // Dynamic remaining burst time tracking
    int level[MAX_PROCESSES];               // Tracks current priority queue tier (0, 1, or 2)
    int arrived[MAX_PROCESSES] = {0};       // Bitmask tracking job arrival admission
    
    // Initialize burst times and assign all processes to start at highest tier (Queue 0)
    for (int i = 0; i < n; i++) {
        remaining[i] = process_table[indices[i]].burst_time;
        level[i] = 0; // All jobs begin with highest priority
    }
    
    // Gantt chart recording buffers
    int gantt_order[200], gantt_start[200], gantt_end[200];
    int gantt_count = 0;
    int current_time = 0;
    int done = 0;
    // Quantum assigned to each queue tier: Q0=1ms, Q1=2ms, Q2=large (FCFS)
    int quantum[] = {1, 2, 999};
    
    // MLFQ execution loop
    while (done < n) {
        // Check for new job arrivals and admit them into highest-priority Queue 0
        for (int i = 0; i < n; i++) {
            if (!arrived[i] && process_table[indices[i]].arrival_time <= current_time) {
                q0[q0_rear++] = i; // Enqueue newly arrived process in Queue 0
                arrived[i] = 1;
            }
        }
        
        // PRIORITY DISPATCHING: Select job from highest-priority non-empty queue tier
        int qi = -1;
        int q_level = -1;
        
        // Tier 0 has absolute priority over Tier 1 and Tier 2
        if (q0_front < q0_rear) {
            qi = q0[q0_front++];
            q_level = 0;
        } 
        // Tier 1 executes only when Tier 0 is empty
        else if (q1_front < q1_rear) {
            qi = q1[q1_front++];
            q_level = 1;
        } 
        // Tier 2 executes only when both Tier 0 and Tier 1 are empty
        else if (q2_front < q2_rear) {
            qi = q2[q2_front++];
            q_level = 2;
        }
        
        // If all queues are empty, advance clock
        if (qi == -1) {
            current_time++;
            continue;
        }
        
        int idx = indices[qi];
        // Calculate execution slice governed by the quantum of current queue tier
        int exec_time = (remaining[qi] < quantum[q_level]) ? remaining[qi] : quantum[q_level];
        
        // Record into Gantt chart
        gantt_order[gantt_count] = idx;
        gantt_start[gantt_count] = current_time;
        gantt_end[gantt_count] = current_time + exec_time;
        gantt_count++;
        
        // Advance clock and decrement remaining burst
        current_time += exec_time;
        remaining[qi] -= exec_time;
        
        // Enqueue any new processes arriving during this slice directly into Queue 0
        for (int i = 0; i < n; i++) {
            if (!arrived[i] && process_table[indices[i]].arrival_time <= current_time) {
                q0[q0_rear++] = i;
                arrived[i] = 1;
            }
        }
        
        // Check completion status
        if (remaining[qi] == 0) {
            // Process finished execution
            process_table[idx].completion_time = current_time;
            process_table[idx].turnaround_time = current_time - process_table[idx].arrival_time;
            process_table[idx].waiting_time = process_table[idx].turnaround_time - process_table[idx].burst_time;
            process_table[idx].state = STATE_TERMINATED;
            done++;
        } else {
            // FEEDBACK DEMOTION: process exhausted its quantum without finishing; penalize to lower queue
            if (q_level == 0) {
                level[qi] = 1;          // Demote from Queue 0 to Queue 1
                q1[q1_rear++] = qi;     // Insert into Queue 1 rear
            } else if (q_level == 1) {
                level[qi] = 2;          // Demote from Queue 1 to Queue 2 (background FCFS)
                q2[q2_rear++] = qi;     // Insert into Queue 2 rear
            } else {
                // Already at lowest tier (Queue 2); remain in Queue 2
                q2[q2_rear++] = qi;
            }
        }
    }
    
    // Render Gantt chart and output numerical results table
    print_gantt_chart(gantt_order, gantt_start, gantt_end, gantt_count);
    print_results(indices, n, "MLFQ");
}

/* ==============================================================================
 * Comparative Benchmark Execution (All 5 Schedulers Side-by-Side)
 * ============================================================================== */
void schedule_compare(void) {
    // Print comparative benchmark banner
    printf(MAGENTA "\n╔══════════════════════════════════════════╗\n");
    printf("║     Comparing All Scheduling Algorithms   ║\n");
    printf("╚══════════════════════════════════════════╝\n" RESET);
    
    // Save original state of the Process Table so each algorithm runs on identical inputs
    PCB backup[MAX_PROCESSES];
    memcpy(backup, process_table, sizeof(process_table));
    
    // Arrays to store measured average waiting time and turnaround time for each policy
    float avg_wt[5], avg_tat[5];
    const char *names[] = {"FCFS", "SJF", "RR(q=2)", "Priority", "MLFQ"};
    int indices[MAX_PROCESSES];
    int n;
    
    // Benchmark 1: FCFS
    memcpy(process_table, backup, sizeof(process_table));
    schedule_fcfs();
    n = get_active_indices(indices);
    {
        float tw = 0, tt = 0;
        for (int i = 0; i < n; i++) {
            tw += process_table[indices[i]].waiting_time;
            tt += process_table[indices[i]].turnaround_time;
        }
        avg_wt[0] = tw / n; avg_tat[0] = tt / n;
    }
    
    // Benchmark 2: SJF
    memcpy(process_table, backup, sizeof(process_table));
    schedule_sjf();
    {
        float tw = 0, tt = 0;
        for (int i = 0; i < n; i++) {
            tw += process_table[indices[i]].waiting_time;
            tt += process_table[indices[i]].turnaround_time;
        }
        avg_wt[1] = tw / n; avg_tat[1] = tt / n;
    }
    
    // Benchmark 3: Round Robin (quantum = 2)
    memcpy(process_table, backup, sizeof(process_table));
    schedule_rr(2);
    {
        float tw = 0, tt = 0;
        for (int i = 0; i < n; i++) {
            tw += process_table[indices[i]].waiting_time;
            tt += process_table[indices[i]].turnaround_time;
        }
        avg_wt[2] = tw / n; avg_tat[2] = tt / n;
    }
    
    // Benchmark 4: Priority
    memcpy(process_table, backup, sizeof(process_table));
    schedule_priority();
    {
        float tw = 0, tt = 0;
        for (int i = 0; i < n; i++) {
            tw += process_table[indices[i]].waiting_time;
            tt += process_table[indices[i]].turnaround_time;
        }
        avg_wt[3] = tw / n; avg_tat[3] = tt / n;
    }
    
    // Benchmark 5: MLFQ
    memcpy(process_table, backup, sizeof(process_table));
    schedule_mlfq();
    {
        float tw = 0, tt = 0;
        for (int i = 0; i < n; i++) {
            tw += process_table[indices[i]].waiting_time;
            tt += process_table[indices[i]].turnaround_time;
        }
        avg_wt[4] = tw / n; avg_tat[4] = tt / n;
    }
    
    // Output comparative summary table
    printf(CYAN "\n╔═══════════════════════════════════════════════════════╗\n");
    printf("║              Algorithm Comparison Summary              ║\n");
    printf("╠═══════════════╦═══════════════════╦═══════════════════╣\n");
    printf("║  Algorithm    ║  Avg Wait Time    ║  Avg Turnaround   ║\n");
    printf("╠═══════════════╬═══════════════════╬═══════════════════╣\n" RESET);
    
    // Identify algorithm achieving the minimum Average Waiting Time (optimal policy)
    float best_wt = avg_wt[0];
    int best_wt_idx = 0;
    for (int i = 1; i < 5; i++) {
        if (avg_wt[i] < best_wt) { 
            best_wt = avg_wt[i]; 
            best_wt_idx = i; 
        }
    }
    
    // Print each algorithm record, highlighting the optimal algorithm in green
    for (int i = 0; i < 5; i++) {
        if (i == best_wt_idx)
            printf(GREEN);
        printf("║  %-12s ║      %8.2f     ║      %8.2f     ║\n",
               names[i], avg_wt[i], avg_tat[i]);
        if (i == best_wt_idx)
            printf(RESET);
    }
    printf(CYAN "╚═══════════════╩═══════════════════╩═══════════════════╝\n" RESET);
    // Print concluding statement confirming the optimal scheduler
    printf(GREEN "\nBest Average Waiting Time: %s (%.2f)\n\n" RESET, names[best_wt_idx], best_wt);
    
    // Restore original Process Table state from backup
    memcpy(process_table, backup, sizeof(process_table));
}
