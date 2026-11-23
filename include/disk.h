#ifndef DISK_H                                 // Include guard: prevents multiple inclusion of disk scheduling header
#define DISK_H                                 // Define DISK_H symbol

#define MAX_REQUESTS 30                        // Maximum pending I/O track requests stored in disk cylinder queue
#define DISK_SIZE 200                          // Total track cylinder span of simulated hard drive (cylinders 0-199)

void disk_fcfs(int *requests, int n, int head);      // Simulates First-Come First-Served disk arm track scheduling
void disk_sstf(int *requests, int n, int head);      // Simulates Shortest Seek Time First greedy cylinder scheduling
void disk_scan(int *requests, int n, int head);      // Simulates SCAN (Elevator) bidirectional sweep disk scheduling
void disk_cscan(int *requests, int n, int head);     // Simulates Circular SCAN (C-SCAN) unidirectional sweep scheduling
void disk_compare(int *requests, int n, int head);   // Evaluates and compares total head seek distance across algorithms

#endif                                         // End of include guard DISK_H
