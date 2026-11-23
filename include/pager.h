#ifndef PAGER_H
#define PAGER_H

// Maximum capacity of the simulated page reference string
#define MAX_REF_STRING 50

// Maximum number of physical page frame slots in simulated main memory
#define MAX_FRAMES 10

// Simulates demand paging page replacement using the First-In, First-Out (FIFO) queue policy
void page_replace_fifo(int frames, int *ref_string, int ref_len);

// Simulates demand paging page replacement using the Least Recently Used (LRU) policy via temporal locality
void page_replace_lru(int frames, int *ref_string, int ref_len);

// Simulates demand paging page replacement using Bélády's Optimal foresight policy (minimum theoretical faults)
void page_replace_optimal(int frames, int *ref_string, int ref_len);

// Executes FIFO, LRU, and Optimal algorithms on the same reference string and outputs a comparative table
void page_replace_compare(int frames, int *ref_string, int ref_len);

#endif
