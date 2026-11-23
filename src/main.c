#include <stdio.h>                                 // Standard C library header for formatted I/O (printf, fgets, sscanf, setvbuf)
#include <stdlib.h>                                // Standard C library header for general utilities (atoi, system, exit)
#include <string.h>                                // Standard C library header for string routines (strcmp, strtok, strlen)
#include "../include/process.h"                    // Process management header declaring PCB structures, states, and APIs
#include "../include/scheduler.h"                  // CPU scheduling header declaring algorithms, Gantt chart, and metrics
#include "../include/memory.h"                     // Memory management header declaring doubly-linked heap blocks and policies
#include "../include/sync.h"                       // Synchronization header declaring semaphores and Banker's algorithm
#include "../include/pager.h"                      // Virtual memory paging header declaring page replacement policies
#include "../include/disk.h"                       // Secondary storage header declaring disk head scheduling algorithms

#define COLOR_RED     "\033[1;31m"                 // ANSI Escape: Text color Red for error alerts and diagnostic messages
#define COLOR_GREEN   "\033[1;32m"                 // ANSI Escape: Text color Green for success notifications and categories
#define COLOR_YELLOW  "\033[1;33m"                 // ANSI Escape: Text color Yellow for section headers and prompts
#define COLOR_BLUE    "\033[1;34m"                 // ANSI Escape: Text color Blue for interactive shell prompt (minios>)
#define COLOR_CYAN    "\033[1;36m"                 // ANSI Escape: Text color Cyan for introductory graphical banner
#define COLOR_RESET   "\033[0m"                    // ANSI Escape: Text attribute Reset to restore standard terminal styling

void clear_screen(void) {                          // Clears the terminal display in a platform-agnostic manner
    #ifdef _WIN32                                  // Preprocessor check: inspect if compiling on Windows
        system("cls");                             // Execute DOS/Windows shell built-in command 'cls'
    #else                                          // Conditional compilation fallback: POSIX (Linux, macOS)
        system("clear");                           // Execute POSIX terminal command 'clear'
    #endif                                         // Terminate conditional compilation block
}

void print_banner(void) {                          // Renders introductory ASCII graphic banner on startup or refresh
    printf(COLOR_CYAN);                            // Emit cyan ANSI escape sequence to color banner borders
    printf("╔══════════════════════════════════════════╗\n"); // Print top border box line
    printf("║          MiniOS v1.0 Simulator           ║\n"); // Print application name and version string
    printf("║     Operating System Course Project      ║\n"); // Print academic course attribution line
    printf("╚══════════════════════════════════════════╝\n"); // Print bottom border box line
    printf(COLOR_RESET);                           // Reset terminal foreground color back to system default
    printf("Type 'help' for list of commands.\n\n"); // Instruct user how to display available commands
}

void print_help(void) {                            // Outputs categorized reference manual of all supported commands
    printf(COLOR_YELLOW "\n=== MiniOS Commands ===" COLOR_RESET "\n\n"); // Header delineating command reference
    
    printf(COLOR_GREEN "Process Management:\n" COLOR_RESET); // Section 1: Process Management Subsystem commands
    printf("  create <name> <burst> <priority> [arrival]  - Create a new process\n"); // Process creation syntax
    printf("  list                                        - List all processes\n");   // Process listing syntax
    printf("  kill <pid>                                  - Terminate a process\n");  // Process termination syntax
    printf("  clear_all                                   - Remove all processes\n\n");// Process table reset syntax
    
    printf(COLOR_GREEN "CPU Scheduling:\n" COLOR_RESET);    // Section 2: CPU Scheduling Subsystem commands
    printf("  schedule fcfs                 - First Come First Serve\n");            // FCFS scheduling syntax
    printf("  schedule sjf                  - Shortest Job First\n");                // SJF scheduling syntax
    printf("  schedule rr <quantum>         - Round Robin\n");                       // Round Robin scheduling syntax
    printf("  schedule priority             - Priority Scheduling\n");               // Priority scheduling syntax
    printf("  schedule mlfq                 - Multilevel Feedback Queue\n");         // MLFQ scheduling syntax
    printf("  schedule compare              - Compare all algorithms\n\n");          // Scheduling benchmark syntax
    
    printf(COLOR_GREEN "Memory Management:\n" COLOR_RESET); // Section 3: Contiguous Memory Management commands
    printf("  mem_alloc <name> <size> <ff|bf|wf>  - Allocate memory\n");             // Memory allocation syntax
    printf("  mem_free <name>                     - Free memory\n");                 // Memory deallocation syntax
    printf("  mem_status                          - Show memory map\n");             // Memory visualization syntax
    printf("  mem_reset                           - Reset memory\n\n");              // Memory reset syntax
    
    printf(COLOR_GREEN "Synchronization:\n" COLOR_RESET);   // Section 4: Process Synchronization commands
    printf("  producer_consumer             - Run Producer-Consumer simulation\n");  // Producer-Consumer syntax
    printf("  bankers                       - Run Banker's Algorithm\n\n");          // Banker's algorithm syntax
    
    printf(COLOR_GREEN "Page Replacement:\n" COLOR_RESET);  // Section 5: Virtual Memory Page Replacement commands
    printf("  page <fifo|lru|optimal|compare> <frames> <ref_string>\n");             // Page replacement syntax
    printf("  Example: page lru 3 7,0,1,2,0,3,0,4,2,3\n\n");                         // Paging concrete example
    
    printf(COLOR_GREEN "Disk Scheduling:\n" COLOR_RESET);   // Section 6: Secondary Storage Disk Scheduling commands
    printf("  disk <fcfs|sstf|scan|cscan|compare> <head> <requests>\n");             // Disk scheduling syntax
    printf("  Example: disk sstf 53 98,183,37,122,14,124,65,67\n\n");                // Disk scheduling concrete example
    
    printf(COLOR_GREEN "Other:\n" COLOR_RESET);             // Section 7: General Utility and Shell Control commands
    printf("  help                          - Show this help\n");                    // Help command syntax
    printf("  clear                         - Clear screen\n");                      // Clear screen command syntax
    printf("  exit                          - Exit MiniOS\n\n");                     // Exit command syntax
}

int parse_int_list(const char *str, int *arr, int max_count) { // Tokenizes comma-delimited integers into array
    int count = 0;                                 // Initialize parsed integer counter to zero
    char buffer[256];                              // Allocate local buffer for mutable copy of input string
    strncpy(buffer, str, sizeof(buffer) - 1);      // Safely copy at most sizeof(buffer)-1 bytes from input string
    buffer[sizeof(buffer) - 1] = '\0';             // Guarantee null-termination against missing null terminators
    
    char *token = strtok(buffer, ",");             // Initialize tokenizer using comma character as delimiter
    while (token != NULL && count < max_count) {   // Loop while valid tokens exist and buffer capacity remains
        arr[count++] = atoi(token);                // Convert ASCII token to integer and store in destination array
        token = strtok(NULL, ",");                 // Advance tokenizer to retrieve next comma-delimited token
    }
    return count;                                  // Return total number of integer tokens extracted
}

void handle_schedule(char *args) {                 // Command handler for 'schedule' family of shell commands
    if (process_count == 0) {                      // Validate that at least one process exists in process table
        printf(COLOR_RED "Error: No processes created. Use 'create' first.\n" COLOR_RESET); // Alert empty table
        return;                                    // Return early to shell loop
    }
    
    if (args == NULL) {                            // Validate that algorithm arguments were supplied
        printf(COLOR_RED "Usage: schedule <fcfs|sjf|rr|priority|mlfq|compare>\n" COLOR_RESET); // Syntax usage hint
        return;                                    // Return early to shell loop
    }
    
    if (strcmp(args, "fcfs") == 0) {               // Check if requested algorithm is First-Come, First-Served
        schedule_fcfs();                           // Invoke non-preemptive FCFS scheduler
    } else if (strcmp(args, "sjf") == 0) {         // Check if requested algorithm is Shortest Job First
        schedule_sjf();                            // Invoke non-preemptive SJF scheduler
    } else if (strncmp(args, "rr", 2) == 0) {      // Check if requested algorithm begins with 'rr'
        int quantum = 2;                           // Default time quantum of 2 ms if unspecified
        char *q = strtok(args + 2, " ");           // Tokenize optional quantum argument following 'rr' prefix
        if (q != NULL) quantum = atoi(q);          // Convert parsed quantum string to integer
        if (quantum <= 0) quantum = 2;             // Guard against non-positive quantum by resetting to default 2
        schedule_rr(quantum);                      // Invoke preemptive Round Robin scheduler with time slice
    } else if (strcmp(args, "priority") == 0) {    // Check if requested algorithm is static priority scheduling
        schedule_priority();                       // Invoke Priority scheduler implementation
    } else if (strcmp(args, "mlfq") == 0) {        // Check if requested algorithm is Multilevel Feedback Queue
        schedule_mlfq();                           // Invoke 3-level feedback queue scheduler with dynamic demotion
    } else if (strcmp(args, "compare") == 0) {     // Check if requested command is comparative performance benchmark
        schedule_compare();                        // Execute all schedulers and print side-by-side metrics table
    } else {                                       // Handle unknown or invalid scheduling algorithm identifier
        printf(COLOR_RED "Unknown algorithm: %s\n" COLOR_RESET, args); // Alert user to unrecognized algorithm
    }
}

void handle_page(char *args) {                     // Command handler for virtual memory page replacement simulations
    if (args == NULL) {                            // Validate that argument string is non-null
        printf(COLOR_RED "Usage: page <fifo|lru|optimal|compare> <frames> <ref_string>\n" COLOR_RESET); // Usage error
        return;                                    // Return early to shell loop
    }
    
    char algo[20];                                 // Declare buffer to store algorithm name token
    int frames;                                    // Declare integer variable for physical page frame capacity
    char ref_str[256];                             // Declare buffer to store raw comma-separated page references
    
    if (sscanf(args, "%s %d %s", algo, &frames, ref_str) != 3) { // Parse formatted algorithm, frames, and ref_string
        printf(COLOR_RED "Usage: page <fifo|lru|optimal|compare> <frames> <ref_string>\n" COLOR_RESET); // Usage error
        return;                                    // Return early to shell loop
    }
    
    if (frames <= 0 || frames > MAX_FRAMES) {      // Validate physical frame count within [1, MAX_FRAMES]
        printf(COLOR_RED "Error: frames must be between 1 and %d\n" COLOR_RESET, MAX_FRAMES); // Boundary error
        return;                                    // Return early to shell loop
    }
    
    int ref[MAX_REF_STRING];                       // Allocate integer array to store parsed page reference sequence
    int ref_len = parse_int_list(ref_str, ref, MAX_REF_STRING); // Parse comma-separated page reference string
    
    if (ref_len == 0) {                            // Validate that at least one page reference was parsed
        printf(COLOR_RED "Error: empty reference string\n" COLOR_RESET); // Alert user to empty reference string
        return;                                    // Return early to shell loop
    }
    
    if (strcmp(algo, "fifo") == 0) {               // Dispatch to First-In, First-Out page replacement
        page_replace_fifo(frames, ref, ref_len);   // Execute FIFO page replacement simulation
    } else if (strcmp(algo, "lru") == 0) {         // Dispatch to Least Recently Used page replacement
        page_replace_lru(frames, ref, ref_len);    // Execute LRU page replacement simulation
    } else if (strcmp(algo, "optimal") == 0) {     // Dispatch to Bélády's Optimal theoretical page replacement
        page_replace_optimal(frames, ref, ref_len);// Execute Optimal page replacement simulation
    } else if (strcmp(algo, "compare") == 0) {     // Dispatch to comparative paging benchmark
        page_replace_compare(frames, ref, ref_len);// Execute all page replacement algorithms and print summary
    } else {                                       // Handle unknown page replacement algorithm
        printf(COLOR_RED "Unknown algorithm: %s\n" COLOR_RESET, algo); // Alert user to invalid algorithm choice
    }
}

void handle_disk(char *args) {                     // Command handler for secondary storage disk arm scheduling
    if (args == NULL) {                            // Validate that argument string is non-null
        printf(COLOR_RED "Usage: disk <fcfs|sstf|scan|cscan|compare> <head> <requests>\n" COLOR_RESET); // Usage error
        return;                                    // Return early to shell loop
    }
    
    char algo[20];                                 // Declare buffer to store algorithm name token
    int head;                                      // Declare integer variable to hold initial disk head position
    char req_str[256];                             // Declare buffer to hold raw comma-separated cylinder requests
    
    if (sscanf(args, "%s %d %s", algo, &head, req_str) != 3) { // Parse formatted algorithm, head, and requests string
        printf(COLOR_RED "Usage: disk <fcfs|sstf|scan|cscan|compare> <head> <requests>\n" COLOR_RESET); // Usage error
        return;                                    // Return early to shell loop
    }
    
    int requests[MAX_REQUESTS];                    // Allocate integer array to store parsed track cylinder requests
    int n = parse_int_list(req_str, requests, MAX_REQUESTS); // Parse comma-separated cylinder requests into array
    
    if (n == 0) {                                  // Validate that at least one cylinder request was parsed
        printf(COLOR_RED "Error: empty request list\n" COLOR_RESET); // Alert user to empty cylinder request list
        return;                                    // Return early to shell loop
    }
    
    if (strcmp(algo, "fcfs") == 0) {               // Dispatch to First-Come, First-Served disk arm scheduling
        disk_fcfs(requests, n, head);              // Execute FCFS disk scheduling simulation
    } else if (strcmp(algo, "sstf") == 0) {        // Dispatch to Shortest Seek Time First greedy disk scheduling
        disk_sstf(requests, n, head);              // Execute SSTF disk scheduling simulation
    } else if (strcmp(algo, "scan") == 0) {        // Dispatch to SCAN elevator bidirectional disk scheduling
        disk_scan(requests, n, head);              // Execute SCAN disk scheduling simulation
    } else if (strcmp(algo, "cscan") == 0) {       // Dispatch to Circular SCAN unidirectional disk scheduling
        disk_cscan(requests, n, head);             // Execute C-SCAN disk scheduling simulation
    } else if (strcmp(algo, "compare") == 0) {     // Dispatch to comparative disk benchmark
        disk_compare(requests, n, head);           // Execute all disk scheduling algorithms and compare total seeks
    } else {                                       // Handle unknown disk scheduling algorithm
        printf(COLOR_RED "Unknown algorithm: %s\n" COLOR_RESET, algo); // Alert user to invalid algorithm choice
    }
}

int main(void) {                                   // Kernel simulator entry point and interactive shell REPL
    char input[512];                               // Allocate 512-byte character array on stack to store input lines
    
    setvbuf(stdout, NULL, _IONBF, 0);              // Disable stream buffering (_IONBF) so output flushes immediately
    
    init_process_table();                          // Initialize process subsystem: clear PCB array and reset counters
    init_memory();                                 // Initialize memory subsystem: create 1024 MB unallocated partition
    
    clear_screen();                                // Clear terminal window for clean visual presentation
    print_banner();                                // Display welcome banner and introductory instructions
    
    while (1) {                                    // Shell Read-Eval-Print Loop (REPL): runs until exit or EOF
        printf(COLOR_BLUE "minios> " COLOR_RESET); // Display interactive shell prompt in bright blue
        
        if (fgets(input, sizeof(input), stdin) == NULL) { // Read a complete command line from standard input
            break;                                 // Break loop and exit cleanly if EOF (Ctrl+D) is detected
        }
        
        input[strcspn(input, "\n")] = '\0';        // Replace trailing newline character '\n' with null terminator '\0'
        
        if (strlen(input) == 0) continue;          // If user pressed Enter without typing commands, skip evaluation
        
        char *cmd = strtok(input, " ");            // Extract primary command name by tokenizing up to first space
        char *args = cmd + strlen(cmd) + 1;        // Compute raw pointer to arguments located after command token
        if (args >= input + strlen(input) + 1) args = NULL; // Validate pointer boundary against input buffer bounds
        {                                          // Argument pointer sanitization and whitespace trimming block
            if (cmd + strlen(cmd) < input + 511) { // Check if pointer lies within valid 512-byte buffer boundary
                args = cmd + strlen(cmd) + 1;      // Point args to character following null byte from strtok
                while (args < input + 512 && *args == ' ') args++; // Advance pointer past leading whitespace spaces
                if (*args == '\0') args = NULL;    // If argument string is empty after trimming, set pointer to NULL
            } else {                               // If command token exhausted entire buffer
                args = NULL;                       // Set argument pointer to NULL
            }
        }
        
        if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) { // Check for shell termination: 'exit' or 'quit'
            printf("Shutting down MiniOS...\n");   // Print graceful shutdown message
            cleanup_memory();                      // Release all dynamically allocated heap memory partition nodes
            break;                                 // Break out of infinite REPL loop to terminate main
        } else if (strcmp(cmd, "help") == 0) {     // Check for 'help' command to display command manual
            print_help();                          // Invoke help documentation printer
        } else if (strcmp(cmd, "clear") == 0) {    // Check for 'clear' command to refresh console display
            clear_screen();                        // Clear terminal screen
            print_banner();                        // Re-render introductory MiniOS graphical banner
        } else if (strcmp(cmd, "create") == 0) {   // Check for 'create' command to instantiate a new process
            if (args == NULL) {                    // Validate that required process creation arguments were supplied
                printf(COLOR_RED "Usage: create <name> <burst_time> <priority> [arrival_time]\n" COLOR_RESET); // Usage
                continue;                          // Continue to next REPL cycle
            }
            char name[MAX_NAME_LEN];               // Local buffer to hold process name string
            int burst, prio, arrival = 0;          // Variables for CPU burst time, static priority, arrival time
            int parsed = sscanf(args, "%s %d %d %d", name, &burst, &prio, &arrival); // Parse creation parameters
            if (parsed < 3) {                      // Ensure at least 3 mandatory fields were parsed
                printf(COLOR_RED "Usage: create <name> <burst_time> <priority> [arrival_time]\n" COLOR_RESET); // Usage
                continue;                          // Continue to next REPL cycle
            }
            int pid = create_process(name, burst, prio, arrival); // Invoke kernel process creation API
            if (pid >= 0) {                        // If process creation succeeded, announce PID to user
                printf(COLOR_GREEN "Process '%s' created with PID %d\n" COLOR_RESET, name, pid); // Confirmation
            }
        } else if (strcmp(cmd, "list") == 0) {     // Check for 'list' command to display all processes
            list_processes();                      // Invoke process table listing routine
        } else if (strcmp(cmd, "kill") == 0) {     // Check for 'kill' command to terminate a process
            if (args == NULL) {                    // Validate that PID argument was supplied
                printf(COLOR_RED "Usage: kill <pid>\n" COLOR_RESET); // Usage reminder
                continue;                          // Continue to next REPL cycle
            }
            int pid = atoi(args);                  // Convert PID string argument to integer
            if (kill_process(pid) == 0) {          // Invoke kernel kill process API; returns 0 on success
                printf(COLOR_GREEN "Process %d terminated.\n" COLOR_RESET, pid); // Termination confirmation
            }
        } else if (strcmp(cmd, "clear_all") == 0) {// Check for 'clear_all' command to wipe the entire process table
            init_process_table();                  // Reset process table, zeroing count and reclaiming all slots
            printf(COLOR_GREEN "All processes cleared.\n" COLOR_RESET); // Confirmation message
        } else if (strcmp(cmd, "schedule") == 0) { // Check for 'schedule' command to invoke CPU scheduling
            handle_schedule(args);                 // Delegate argument parsing and execution to handle_schedule
        } else if (strcmp(cmd, "mem_alloc") == 0) {// Check for 'mem_alloc' to dynamically allocate memory
            if (args == NULL) {                    // Validate that memory allocation arguments were supplied
                printf(COLOR_RED "Usage: mem_alloc <name> <size> <ff|bf|wf>\n" COLOR_RESET); // Usage reminder
                continue;                          // Continue to next REPL cycle
            }
            char name[MAX_NAME_LEN];               // Local buffer to hold process name for memory partition ownership
            int size;                              // Integer for requested partition size in MB
            char policy_str[4];                    // String buffer to store allocation placement policy flag
            if (sscanf(args, "%s %d %s", name, &size, policy_str) != 3) { // Parse parameters
                printf(COLOR_RED "Usage: mem_alloc <name> <size> <ff|bf|wf>\n" COLOR_RESET); // Usage error
                continue;                          // Continue to next REPL cycle
            }
            AllocPolicy pol;                       // Enumeration variable for allocation policy
            if (strcmp(policy_str, "ff") == 0) pol = FIRST_FIT;       // Map 'ff' string flag to FIRST_FIT
            else if (strcmp(policy_str, "bf") == 0) pol = BEST_FIT;   // Map 'bf' string flag to BEST_FIT
            else if (strcmp(policy_str, "wf") == 0) pol = WORST_FIT;  // Map 'wf' string flag to WORST_FIT
            else {                                 // Handle invalid policy identifier
                printf(COLOR_RED "Policy must be ff (First Fit), bf (Best Fit), or wf (Worst Fit)\n" COLOR_RESET); // Error
                continue;                          // Continue to next REPL cycle
            }
            if (mem_allocate(name, size, pol) == 0) { // Call kernel memory allocation routine
                printf(COLOR_GREEN "Allocated %d MB to '%s'\n" COLOR_RESET, size, name); // Confirmation
            }
        } else if (strcmp(cmd, "mem_free") == 0) { // Check for 'mem_free' to deallocate process memory
            if (args == NULL) {                    // Validate that process name argument was supplied
                printf(COLOR_RED "Usage: mem_free <name>\n" COLOR_RESET); // Usage reminder
                continue;                          // Continue to next REPL cycle
            }
            char name[MAX_NAME_LEN];               // Local buffer to hold process name
            sscanf(args, "%s", name);              // Parse process name from arguments
            if (mem_free(name) == 0) {             // Invoke kernel memory deallocation routine (coalesces holes)
                printf(COLOR_GREEN "Freed memory for '%s'\n" COLOR_RESET, name); // Confirmation
            }
        } else if (strcmp(cmd, "mem_status") == 0){// Check for 'mem_status' to display partition layout
            mem_status();                          // Invoke memory status dump function
        } else if (strcmp(cmd, "mem_reset") == 0) {// Check for 'mem_reset' to release all partitions
            cleanup_memory();                      // Deallocate all existing partition nodes in doubly-linked list
            init_memory();                         // Reinitialize heap with single 1024 MB unallocated block
            printf(COLOR_GREEN "Memory reset.\n" COLOR_RESET); // Confirmation message
        } else if (strcmp(cmd, "producer_consumer") == 0) { // Check for 'producer_consumer' simulation command
            run_producer_consumer();               // Execute bounded-buffer Producer-Consumer simulation
        } else if (strcmp(cmd, "bankers") == 0) {  // Check for 'bankers' deadlock avoidance algorithm command
            run_bankers();                         // Execute Banker's algorithm matrix calculation and safe sequence
        } else if (strcmp(cmd, "page") == 0) {     // Check for 'page' virtual memory replacement command
            handle_page(args);                     // Delegate argument parsing and execution to handle_page
        } else if (strcmp(cmd, "disk") == 0) {     // Check for 'disk' secondary storage scheduling command
            handle_disk(args);                     // Delegate argument parsing and execution to handle_disk
        } else {                                   // Handle unrecognized command input
            printf(COLOR_RED "Unknown command: '%s'. Type 'help' for list of commands.\n" COLOR_RESET, cmd); // Alert
        }
    }
    
    return 0;                                      // Return exit code 0 indicating successful program termination
}
