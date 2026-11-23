#include <stdio.h>                                 // Standard I/O library for formatted console printing (printf)
#include <stdlib.h>                                // Standard library functions
#include <string.h>                                // Memory copy and buffer functions (memcpy)
#include "disk.h"                                  // Disk scheduling header defining DISK_SIZE, MAX_REQUESTS

#define RED     "\033[1;31m"                       // Bright Red: indicates long seek movements or errors
#define GREEN   "\033[1;32m"                       // Bright Green: highlights optimal seek paths
#define YELLOW  "\033[1;33m"                       // Bright Yellow: highlights total cylinder distance calculations
#define CYAN    "\033[1;36m"                       // Bright Cyan: highlights trajectory paths and headers
#define MAGENTA "\033[1;35m"                       // Bright Magenta: decorative algorithm banners
#define RESET   "\033[0m"                          // Color Reset: restores default terminal foreground styling

static int abs_val(int x) {                        // Mathematical helper: computes absolute difference |x|
    return (x < 0) ? -x : x;                       // If x is negative, negate it; otherwise return x bare
}

static void print_seek_order(int *order, int count, int total_seek) { // Displays actuator trajectory and seek distance
    printf(CYAN "  Seek Order: " RESET);           // Print trajectory path header
    for (int i = 0; i < count; i++) {              // Iterate through all visited cylinder stops
        printf("%d", order[i]);                    // Output cylinder number
        if (i < count - 1) printf(" -> ");         // Separate consecutive cylinders with directional arrow
    }
    printf("\n");                                  // Newline break
    printf(YELLOW "  Total Seek Time: %d cylinders\n\n" RESET, total_seek); // Display total accumulated seek cylinders
}

void disk_fcfs(int *requests, int n, int head) {   // First-Come, First-Served (FCFS) disk arm scheduling
    printf(MAGENTA "\n========== FCFS Disk Scheduling ==========\n" RESET); // Print execution banner
    printf("  Initial Head Position: %d\n\n", head); // Print starting actuator arm cylinder position
    
    int order[MAX_REQUESTS + 1];                   // Sequence of cylinder stops (initial head + n requests)
    int total_seek = 0;                            // Accumulator for total head movement in cylinders
    int current = head;                            // Current position of read/write head initialized to head
    
    order[0] = head;                               // Record initial head position as first trajectory node
    for (int i = 0; i < n; i++) {                  // Process cylinder requests in exact order of arrival
        total_seek += abs_val(requests[i] - current); // Calculate seek travel: |target_cylinder - current_cylinder|
        current = requests[i];                     // Move actuator head to requested cylinder
        order[i + 1] = current;                    // Record new position in trajectory history
    }
    
    print_seek_order(order, n + 1, total_seek);    // Display resulting trajectory and total seek cylinders
}

void disk_sstf(int *requests, int n, int head) {   // Shortest Seek Time First (SSTF) disk arm scheduling
    printf(MAGENTA "\n========== SSTF Disk Scheduling ==========\n" RESET); // Print execution banner
    printf("  Initial Head Position: %d\n\n", head); // Print starting actuator arm cylinder position
    
    int visited[MAX_REQUESTS] = {0};               // Boolean array tracking which cylinder requests were serviced
    int order[MAX_REQUESTS + 1];                   // Trajectory history array
    int total_seek = 0;                            // Accumulator for total head movement
    int current = head;                            // Current head position tracker
    
    order[0] = head;                               // Starting head position is first node
    
    for (int i = 0; i < n; i++) {                  // Service exactly n cylinder requests
        int min_dist = DISK_SIZE + 1;              // Initialize minimum distance tracking variable to upper bound
        int nearest = -1;                          // Index of closest unvisited cylinder request
        
        for (int j = 0; j < n; j++) {              // Scan all pending cylinder requests to find closest
            if (!visited[j]) {                     // Check only requests that have not yet been serviced
                int dist = abs_val(requests[j] - current); // Calculate seek distance: |requests[j] - current|
                if (dist < min_dist) {             // If this cylinder is closer than current best candidate
                    min_dist = dist;               // Update shortest seek distance
                    nearest = j;                   // Select request j as closest candidate
                }
            }
        }
        
        if (nearest == -1) break;                  // Invariant guard: if no unvisited request found, terminate loop
        
        visited[nearest] = 1;                      // Mark selected cylinder request as serviced
        total_seek += min_dist;                    // Add minimal seek distance to total accumulator
        current = requests[nearest];               // Move read/write head to chosen cylinder
        order[i + 1] = current;                    // Record cylinder in trajectory sequence
    }
    
    print_seek_order(order, n + 1, total_seek);    // Display SSTF seek order and total cylinders traversed
}

void disk_scan(int *requests, int n, int head) {   // SCAN (Elevator) disk arm scheduling
    printf(MAGENTA "\n========== SCAN (Elevator) Disk Scheduling ==========\n" RESET); // Print execution banner
    printf("  Initial Head Position: %d  (moving towards higher end)\n\n", head); // Print initial head and direction
    
    int sorted[MAX_REQUESTS];                      // Local copy of request queue for sorting
    memcpy(sorted, requests, n * sizeof(int));     // Copy input request array into local buffer
    for (int i = 0; i < n - 1; i++) {              // Sort cylinder requests in ascending numerical order
        for (int j = 0; j < n - i - 1; j++) {      // Inner bubble sort loop
            if (sorted[j] > sorted[j + 1]) {       // If previous cylinder is greater than next cylinder
                int temp = sorted[j];              // Swap elements
                sorted[j] = sorted[j + 1];         // Move smaller forward
                sorted[j + 1] = temp;              // Move larger backward
            }
        }
    }
    
    int order[MAX_REQUESTS + 10];                  // Trajectory array with padding for boundary turnaround cylinder
    int count = 0;                                 // Trajectory stop counter
    int total_seek = 0;                            // Total seek accumulator
    
    order[count++] = head;                         // Add initial head position to trajectory
    
    int start = 0;                                 // Locate split partition where head fits into sorted cylinders
    while (start < n && sorted[start] < head) start++; // Advance start until sorted[start] >= head
    
    for (int i = start; i < n; i++) {              // PHASE 1: Sweep upwards towards higher cylinders (>= head)
        order[count++] = sorted[i];                // Append cylinder to trajectory
    }
    
    if (sorted[n - 1] != DISK_SIZE - 1 && start < n) { // If requests do not reach end of platter
        order[count++] = DISK_SIZE - 1;            // Actuator sweeps all the way to maximum cylinder (199)
    }
    
    for (int i = start - 1; i >= 0; i--) {          // PHASE 2: Reverse direction and sweep downwards
        order[count++] = sorted[i];                // Append lower cylinder requests in descending order
    }
    
    int current = head;                            // Calculate total accumulated mechanical seek distance
    for (int i = 1; i < count; i++) {              // Iterate through consecutive trajectory stops
        total_seek += abs_val(order[i] - current); // Accumulate absolute distance between consecutive stops
        current = order[i];                        // Move current tracker to next stop
    }
    
    print_seek_order(order, count, total_seek);    // Display SCAN seek trajectory and total seek distance
}

void disk_cscan(int *requests, int n, int head) {  // Circular SCAN (C-SCAN) disk arm scheduling
    printf(MAGENTA "\n========== C-SCAN Disk Scheduling ==========\n" RESET); // Print execution banner
    printf("  Initial Head Position: %d  (moving towards higher end)\n\n", head); // Print initial head and direction
    
    int sorted[MAX_REQUESTS];                      // Local copy of request queue for sorting
    memcpy(sorted, requests, n * sizeof(int));     // Copy requests into local array
    for (int i = 0; i < n - 1; i++) {              // Sort cylinder requests in ascending order
        for (int j = 0; j < n - i - 1; j++) {      // Bubble sort pass
            if (sorted[j] > sorted[j + 1]) {       // Swap if out of order
                int temp = sorted[j];              // Temp variable
                sorted[j] = sorted[j + 1];         // Swap
                sorted[j + 1] = temp;              // Swap
            }
        }
    }
    
    int order[MAX_REQUESTS + 10];                  // Trajectory history array
    int count = 0;                                 // Trajectory counter
    int total_seek = 0;                            // Total seek distance
    
    order[count++] = head;                         // Record starting head position
    
    int start = 0;                                 // Find index of first cylinder >= initial head
    while (start < n && sorted[start] < head) start++; // Advance start
    
    for (int i = start; i < n; i++) {              // PHASE 1: Sweep upwards servicing higher cylinder requests
        order[count++] = sorted[i];                // Append cylinder to trajectory
    }
    
    order[count++] = DISK_SIZE - 1;                // Sweep to physical disk boundary limit (cylinder 199)
    order[count++] = 0;                            // Fast return jump: reset actuator arm to cylinder 0 without service
    
    for (int i = 0; i < start; i++) {              // PHASE 2: Resume upward sweep from cylinder 0 for lower requests
        order[count++] = sorted[i];                // Append lower cylinder to trajectory
    }
    
    int current = head;                            // Calculate total accumulated seek travel including return jump
    for (int i = 1; i < count; i++) {              // Iterate through consecutive trajectory stops
        total_seek += abs_val(order[i] - current); // Accumulate seek distance
        current = order[i];                        // Advance current tracker
    }
    
    print_seek_order(order, count, total_seek);    // Display C-SCAN seek trajectory and total seek distance
}

void disk_compare(int *requests, int n, int head) { // Evaluates and compares all four disk scheduling policies
    printf(MAGENTA "\n╔══════════════════════════════════════════╗\n"); // Output comparative header banner
    printf("║    Comparing Disk Scheduling Algorithms   ║\n"); // Banner title
    printf("╚══════════════════════════════════════════╝\n" RESET); // Banner bottom border
    
    disk_fcfs(requests, n, head);                  // Execute FCFS simulation
    disk_sstf(requests, n, head);                  // Execute SSTF simulation
    disk_scan(requests, n, head);                  // Execute SCAN simulation
    disk_cscan(requests, n, head);                 // Execute C-SCAN simulation
}
