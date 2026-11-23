#ifndef PAGER_H                                // Include guard: prevents duplicate inclusion of virtual memory header
#define PAGER_H                                // Define PAGER_H symbol

#define MAX_REF_STRING 50                      // Maximum length of virtual page reference address sequence
#define MAX_FRAMES 10                          // Maximum physical page frame slots in simulated main memory

void page_replace_fifo(int frames, int *ref_string, int ref_len);    // Simulates First-In First-Out page replacement
void page_replace_lru(int frames, int *ref_string, int ref_len);     // Simulates Least Recently Used page replacement
void page_replace_optimal(int frames, int *ref_string, int ref_len); // Simulates Bélády's Optimal page replacement
void page_replace_compare(int frames, int *ref_string, int ref_len); // Runs and compares all page replacement algorithms

#endif                                         // End of include guard PAGER_H
