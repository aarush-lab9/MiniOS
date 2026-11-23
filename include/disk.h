#ifndef DISK_H
#define DISK_H

// Maximum number of pending I/O cylinder requests in the disk scheduling queue
#define MAX_REQUESTS 30

// Physical geometry size of the simulated magnetic disk: 200 cylinders labeled 0 through 199
#define DISK_SIZE 200

// Simulates First-Come, First-Served (FCFS) disk arm scheduling in arrival order
void disk_fcfs(int *requests, int n, int head);

// Simulates Shortest Seek Time First (SSTF) disk scheduling by greedily servicing the closest cylinder
void disk_sstf(int *requests, int n, int head);

// Simulates SCAN (Elevator algorithm) disk scheduling by sweeping bidirectionally across disk bounds
void disk_scan(int *requests, int n, int head);

// Simulates Circular SCAN (C-SCAN) disk scheduling by sweeping in one direction with uniform return jumps
void disk_cscan(int *requests, int n, int head);

// Executes all four disk scheduling policies on the same request queue and outputs total head seek travel
void disk_compare(int *requests, int n, int head);

#endif
