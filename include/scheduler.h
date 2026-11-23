#ifndef SCHEDULER_H
#define SCHEDULER_H

// Simulates First-Come, First-Served (FCFS) non-preemptive CPU scheduling
void schedule_fcfs(void);

// Simulates Shortest Job First (SJF) non-preemptive CPU scheduling (minimizes average waiting time)
void schedule_sjf(void);

// Simulates Round Robin (RR) preemptive CPU scheduling with a configurable time quantum
void schedule_rr(int quantum);

// Simulates non-preemptive Priority CPU scheduling (lower integer value = higher priority)
void schedule_priority(void);

// Simulates 3-Level Multilevel Feedback Queue (MLFQ) scheduling with feedback-based priority demotion
void schedule_mlfq(void);

// Executes all five CPU scheduling algorithms on the active workload and prints a comparative summary table
void schedule_compare(void);

// Helper function to render a formatted ASCII horizontal Gantt chart showing process execution intervals
// Parameters:
//   order: array of process identifiers in execution sequence
//   start_times: array of timeline start instants for each interval
//   end_times: array of timeline end instants for each interval
//   count: number of discrete timeline intervals in the schedule
void print_gantt_chart(int *order, int *start_times, int *end_times, int count);

#endif
