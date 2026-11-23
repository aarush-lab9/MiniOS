#ifndef SYNC_H                                 // Include guard: prevents multiple inclusion of synchronization header
#define SYNC_H                                 // Define SYNC_H symbol

#define BUFFER_SIZE 5                          // Capacity of circular ring buffer in Producer-Consumer simulation
#define MAX_RESOURCES 4                        // Maximum distinct resource types modeled in Banker's safety matrix
#define MAX_SYNC_PROC 5                        // Maximum processes evaluated simultaneously in Banker's algorithm

void run_producer_consumer(void);              // Simulates bounded buffer synchronization using counting & mutex semaphores
void run_bankers(void);                        // Simulates Banker's deadlock avoidance safety algorithm and safe sequence

#endif                                         // End of include guard SYNC_H
