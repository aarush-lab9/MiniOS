#ifndef SYNC_H
#define SYNC_H

// Size of the shared ring buffer in the bounded-buffer Producer-Consumer problem
#define BUFFER_SIZE 5

// Maximum number of resource classes in the Banker's deadlock safety algorithm
#define MAX_RESOURCES 4

// Maximum number of concurrent processes evaluated in the Banker's safety matrix
#define MAX_SYNC_PROC 5

// Simulates the classic bounded-buffer Producer-Consumer synchronization problem
// Uses counting semaphores (empty=5, full=0) and a binary semaphore (mutex=1) to prevent race conditions
void run_producer_consumer(void);

// Executes Edsger Dijkstra's Banker's Algorithm for deadlock avoidance
// Verifies whether resource allocation remains in a safe state and calculates the safe sequence
void run_bankers(void);

#endif
