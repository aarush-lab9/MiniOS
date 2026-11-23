/*
 * disk.c - Secondary Storage Disk Scheduling Subsystem
 * 
 * Simulates mechanical read/write head actuator arm movement over a magnetic hard disk:
 * - 200-cylinder physical geometry (tracks 0 through 199)
 * - First-Come, First-Served (FCFS) un-reordered baseline traversal
 * - Shortest Seek Time First (SSTF) greedy nearest-cylinder scheduling
 * - SCAN (Elevator algorithm) sweeping bidirectionally to disk boundary extremes
 * - Circular SCAN (C-SCAN) unidirectional sweep with zero-service return jump
 * - Seek trajectory visualization and total cylinder travel distance calculation
 */

#include <stdio.h>      // Standard I/O library for formatted console printing (printf)
#include <stdlib.h>     // Standard library functions
#include <string.h>     // Memory copy and buffer functions (memcpy)
#include "disk.h"       // Disk scheduling header defining DISK_SIZE, MAX_REQUESTS, prototypes

// ANSI terminal color escape sequence macros for highlighting disk seek output
#define RED     "\033[1;31m"    // Bright Red: indicates long seek movements or errors
#define GREEN   "\033[1;32m"    // Bright Green: highlights optimal seek paths
#define YELLOW  "\033[1;33m"    // Bright Yellow: highlights total cylinder distance calculations
#define CYAN    "\033[1;36m"    // Bright Cyan: highlights trajectory paths and headers
#define MAGENTA "\033[1;35m"    // Bright Magenta: decorative algorithm banners
#define RESET   "\033[0m"       // Color Reset: restores default terminal foreground styling

// Mathematical helper: computes absolute difference |x| for distance calculations
static int abs_val(int x) {
    // If x is negative, negate it; otherwise return x bare
    return (x < 0) ? -x : x;
}

// Formats and displays the step-by-step actuator arm trajectory and total seek distance
static void print_seek_order(int *order, int count, int total_seek) {
    // Print trajectory path header
    printf(CYAN "  Seek Order: " RESET);
    // Iterate through all visited cylinder stops
    for (int i = 0; i < count; i++) {
        // Output cylinder number
        printf("%d", order[i]);
        // Separate consecutive cylinders with directional arrow
        if (i < count - 1) printf(" -> ");
    }
    // Newline break
    printf("\n");
    // Display total accumulated mechanical seek distance in cylinders
    printf(YELLOW "  Total Seek Time: %d cylinders\n\n" RESET, total_seek);
}

/* ==============================================================================
 * FCFS (First-Come, First-Served) Disk Arm Scheduling
 * ============================================================================== */
void disk_fcfs(int *requests, int n, int head) {
    // Print algorithm execution banner
    printf(MAGENTA "\n========== FCFS Disk Scheduling ==========\n" RESET);
    // Print starting actuator arm cylinder position
    printf("  Initial Head Position: %d\n\n", head);
    
    // Array to record the sequence of cylinder stops (initial head + n requests)
    int order[MAX_REQUESTS + 1];
    // Accumulator for total head movement in cylinders
    int total_seek = 0;
    // Current position of the read/write head initialized to starting head
    int current = head;
    
    // Record initial head position as first trajectory node
    order[0] = head;
    // Process cylinder requests in exact order of arrival without reordering
    for (int i = 0; i < n; i++) {
        // Calculate seek travel distance: |target_cylinder - current_cylinder|
        total_seek += abs_val(requests[i] - current);
        // Move actuator head to requested cylinder
        current = requests[i];
        // Record new position in trajectory history
        order[i + 1] = current;
    }
    
    // Display the resulting trajectory and total seek cylinders
    print_seek_order(order, n + 1, total_seek);
}

/* ==============================================================================
 * SSTF (Shortest Seek Time First) Disk Arm Scheduling
 * ============================================================================== */
void disk_sstf(int *requests, int n, int head) {
    // Print algorithm execution banner
    printf(MAGENTA "\n========== SSTF Disk Scheduling ==========\n" RESET);
    // Print starting actuator arm cylinder position
    printf("  Initial Head Position: %d\n\n", head);
    
    // Boolean array tracking which pending cylinder requests have already been serviced
    int visited[MAX_REQUESTS] = {0};
    // Trajectory history array
    int order[MAX_REQUESTS + 1];
    // Accumulator for total head movement
    int total_seek = 0;
    // Current head position tracker
    int current = head;
    
    // Starting head position is first node
    order[0] = head;
    
    // Service exactly n cylinder requests
    for (int i = 0; i < n; i++) {
        // Initialize minimum distance tracking variable to an upper bound larger than disk
        int min_dist = DISK_SIZE + 1;
        // Index of the closest unvisited cylinder request
        int nearest = -1;
        
        // Scan all pending cylinder requests to find the one closest to current head
        for (int j = 0; j < n; j++) {
            // Check only requests that have not yet been serviced
            if (!visited[j]) {
                // Calculate seek distance from current position: |requests[j] - current|
                int dist = abs_val(requests[j] - current);
                // If this cylinder is closer than the current best candidate
                if (dist < min_dist) {
                    // Update shortest seek distance
                    min_dist = dist;
                    // Select request j as closest candidate
                    nearest = j;
                }
            }
        }
        
        // Invariant guard: if no unvisited request was found, terminate loop
        if (nearest == -1) break;
        
        // Mark selected cylinder request as serviced
        visited[nearest] = 1;
        // Add minimal seek distance to total accumulator
        total_seek += min_dist;
        // Move read/write head to the chosen cylinder
        current = requests[nearest];
        // Record cylinder in trajectory sequence
        order[i + 1] = current;
    }
    
    // Display SSTF seek order and total cylinders traversed
    print_seek_order(order, n + 1, total_seek);
}

/* ==============================================================================
 * SCAN (Elevator) Disk Arm Scheduling
 * ============================================================================== */
void disk_scan(int *requests, int n, int head) {
    // Print algorithm execution banner
    printf(MAGENTA "\n========== SCAN (Elevator) Disk Scheduling ==========\n" RESET);
    // Print initial head position and direction of motion (upward sweep)
    printf("  Initial Head Position: %d  (moving towards higher end)\n\n", head);
    
    // Local copy of request queue for sorting
    int sorted[MAX_REQUESTS];
    // Copy input request array into local buffer
    memcpy(sorted, requests, n * sizeof(int));
    // Sort cylinder requests in ascending numerical order using bubble sort
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            // If previous cylinder is greater than next cylinder
            if (sorted[j] > sorted[j + 1]) {
                // Swap elements
                int temp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = temp;
            }
        }
    }
    
    // Trajectory array sizing with extra padding for boundary turnaround cylinder
    int order[MAX_REQUESTS + 10];
    // Trajectory stop counter
    int count = 0;
    // Total seek accumulator
    int total_seek = 0;
    
    // Add initial head position to trajectory
    order[count++] = head;
    
    // Locate the split partition where initial head fits into the sorted cylinders
    int start = 0;
    while (start < n && sorted[start] < head) start++;
    
    // PHASE 1: Sweep upwards towards higher cylinders (servicing requests >= head)
    for (int i = start; i < n; i++) {
        // Append cylinder to trajectory
        order[count++] = sorted[i];
    }
    
    // Reach the extreme boundary end of the disk platter (cylinder 199)
    if (sorted[n - 1] != DISK_SIZE - 1 && start < n) {
        // Actuator sweeps all the way to maximum cylinder (DISK_SIZE - 1 = 199)
        order[count++] = DISK_SIZE - 1;
    }
    
    // PHASE 2: Reverse direction and sweep downwards, servicing lower cylinders
    for (int i = start - 1; i >= 0; i--) {
        // Append lower cylinder requests in descending order
        order[count++] = sorted[i];
    }
    
    // Calculate total accumulated mechanical seek distance across entire trajectory
    int current = head;
    for (int i = 1; i < count; i++) {
        // Accumulate absolute distance between consecutive stops
        total_seek += abs_val(order[i] - current);
        // Move current tracker to next stop
        current = order[i];
    }
    
    // Display SCAN seek trajectory and total seek distance
    print_seek_order(order, count, total_seek);
}

/* ==============================================================================
 * C-SCAN (Circular SCAN) Disk Arm Scheduling
 * ============================================================================== */
void disk_cscan(int *requests, int n, int head) {
    // Print algorithm execution banner
    printf(MAGENTA "\n========== C-SCAN Disk Scheduling ==========\n" RESET);
    // Print initial head position
    printf("  Initial Head Position: %d  (moving towards higher end)\n\n", head);
    
    // Local copy of request queue for sorting
    int sorted[MAX_REQUESTS];
    // Copy requests into local array
    memcpy(sorted, requests, n * sizeof(int));
    // Sort cylinder requests in ascending order
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            // Swap if out of order
            if (sorted[j] > sorted[j + 1]) {
                int temp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = temp;
            }
        }
    }
    
    // Trajectory history array
    int order[MAX_REQUESTS + 10];
    // Trajectory counter
    int count = 0;
    // Total seek distance
    int total_seek = 0;
    
    // Record starting head position
    order[count++] = head;
    
    // Find index of first cylinder greater than or equal to initial head
    int start = 0;
    while (start < n && sorted[start] < head) start++;
    
    // PHASE 1: Sweep upwards servicing higher cylinder requests
    for (int i = start; i < n; i++) {
        order[count++] = sorted[i];
    }
    
    // Sweep to the physical disk boundary limit (cylinder 199)
    order[count++] = DISK_SIZE - 1;
    // Fast return jump: reset actuator arm back to cylinder 0 without servicing requests
    order[count++] = 0;
    
    // PHASE 2: Resume upward sweep from cylinder 0 servicing remaining lower requests
    for (int i = 0; i < start; i++) {
        order[count++] = sorted[i];
    }
    
    // Calculate total accumulated seek travel including the return jump
    int current = head;
    for (int i = 1; i < count; i++) {
        total_seek += abs_val(order[i] - current);
        current = order[i];
    }
    
    // Display C-SCAN seek trajectory and total seek distance
    print_seek_order(order, count, total_seek);
}

/* ==============================================================================
 * Comparative Benchmark Execution (FCFS vs SSTF vs SCAN vs C-SCAN)
 * ============================================================================== */
void disk_compare(int *requests, int n, int head) {
    // Output comparative header banner
    printf(MAGENTA "\n╔══════════════════════════════════════════╗\n");
    printf("║    Comparing Disk Scheduling Algorithms   ║\n");
    printf("╚══════════════════════════════════════════╝\n" RESET);
    
    // Execute FCFS simulation
    disk_fcfs(requests, n, head);
    // Execute SSTF simulation
    disk_sstf(requests, n, head);
    // Execute SCAN simulation
    disk_scan(requests, n, head);
    // Execute C-SCAN simulation
    disk_cscan(requests, n, head);
}
