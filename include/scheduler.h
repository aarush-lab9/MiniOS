#ifndef SCHEDULER_H                            // Include guard: prevents multiple inclusion of scheduler interface
#define SCHEDULER_H                            // Define SCHEDULER_H symbol

void schedule_fcfs(void);                      // Simulates non-preemptive First-Come First-Served CPU scheduling
void schedule_sjf(void);                       // Simulates non-preemptive Shortest Job First scheduling (minimal WT)
void schedule_rr(int quantum);                 // Simulates preemptive Round Robin scheduling with time quantum slice
void schedule_priority(void);                  // Simulates non-preemptive static Priority scheduling
void schedule_mlfq(void);                      // Simulates 3-level Multilevel Feedback Queue scheduling with demotion
void schedule_compare(void);                   // Benchmarks all 5 scheduling algorithms and prints comparison matrix
void print_gantt_chart(int *order, int *start_times, int *end_times, int count); // Renders horizontal ASCII Gantt timeline

#endif                                         // End of include guard SCHEDULER_H
