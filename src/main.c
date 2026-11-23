/*
 * MiniOS - Operating System Simulator
 * Module: Main Shell & System Call Command Dispatcher (main.c)
 *
 * Description:
 *   This source file serves as the interactive entry point, user-space shell,
 *   and system call dispatcher for MiniOS. In real operating systems (such as
 *   UNIX/Linux or Windows), the shell (e.g., bash, sh, zsh) acts as a command-line
 *   interpreter that interfaces user requests with the underlying kernel services.
 *   MiniOS implements a Read-Eval-Print Loop (REPL) that reads user commands from
 *   standard input, parses arguments, and dispatches requests to the appropriate
 *   kernel subsystems:
 *     1. Process Management Subsystem (create, list, kill, clear_all)
 *     2. CPU Scheduling Subsystem (FCFS, SJF, RR, Priority, MLFQ, compare)
 *     3. Memory Management Subsystem (First-Fit, Best-Fit, Worst-Fit allocation/free)
 *     4. Process Synchronization & Concurrency (Producer-Consumer, Banker's Algorithm)
 *     5. Virtual Memory Page Replacement (FIFO, LRU, Optimal)
 *     6. Secondary Storage Disk Head Scheduling (FCFS, SSTF, SCAN, C-SCAN)
 *
 * Line-by-Line Educational Commentary:
 *   Every line below contains detailed comments explaining C standard library
 *   operations, operating system concepts, unbuffered I/O streams, string tokenization,
 *   subsystem initialization, validation, and defensive programming practices.
 */

/* Standard C library header for formatted input and output functions (printf, fgets, sscanf, setvbuf) */
#include <stdio.h>
/* Standard C library header for general utility functions (atoi, system, exit, dynamic memory) */
#include <stdlib.h>
/* Standard C library header for string manipulation routines (strcmp, strncmp, strtok, strlen, strcspn) */
#include <string.h>

/* MiniOS process management header declaring PCB structures, states, and process control APIs */
#include "process.h"
/* MiniOS CPU scheduling header declaring algorithms, Gantt chart generation, and metrics calculation */
#include "scheduler.h"
/* MiniOS memory management header declaring doubly-linked heap blocks, allocation policies, and memory APIs */
#include "memory.h"
/* MiniOS synchronization header declaring semaphores, producer-consumer queues, and Banker's algorithm */
#include "sync.h"
/* MiniOS virtual memory paging header declaring page frame limits and replacement policies (FIFO/LRU/Optimal) */
#include "pager.h"
/* MiniOS secondary storage header declaring disk cylinder limits and head movement scheduling algorithms */
#include "disk.h"

/* ANSI Escape Sequence: Text color Red for error alerts and diagnostic messages */
#define COLOR_RED     "\033[1;31m"
/* ANSI Escape Sequence: Text color Green for success notifications and command categories */
#define COLOR_GREEN   "\033[1;32m"
/* ANSI Escape Sequence: Text color Yellow for section headers and warning prompts */
#define COLOR_YELLOW  "\033[1;33m"
/* ANSI Escape Sequence: Text color Blue for the interactive shell prompt (minios>) */
#define COLOR_BLUE    "\033[1;34m"
/* ANSI Escape Sequence: Text color Cyan for the introductory MiniOS graphical banner */
#define COLOR_CYAN    "\033[1;36m"
/* ANSI Escape Sequence: Text attribute Reset to restore standard terminal rendering */
#define COLOR_RESET   "\033[0m"

/*
 * Function: clear_screen
 * ----------------------
 * Clears the terminal display in a platform-agnostic manner.
 * In a real OS, a terminal driver provides control codes; here, we invoke the
 * host operating system's standard terminal clear command via system call.
 */
void clear_screen(void) {
    /* Preprocessor conditional compilation directive: inspect if compiling on Windows */
    #ifdef _WIN32
        /* Execute the DOS/Windows shell built-in command 'cls' to clear the active console window */
        system("cls");
    /* Conditional compilation fallback: target POSIX-compliant systems (Linux, macOS, Unix) */
    #else
        /* Execute the POSIX terminal command 'clear' to reset terminal scrollback and buffer */
        system("clear");
    /* Terminate the preprocessor conditional compilation block */
    #endif
}

/*
 * Function: print_banner
 * ----------------------
 * Renders the introductory ASCII graphic banner on shell startup or screen refresh.
 * Provides clear visual feedback establishing the MiniOS simulation environment.
 */
void print_banner(void) {
    /* Emit the cyan ANSI escape sequence to color the banner borders and title text */
    printf(COLOR_CYAN);
    /* Print top border box line using UTF-8 box-drawing characters */
    printf("╔══════════════════════════════════════════╗\n");
    /* Print the primary application name and version string */
    printf("║          MiniOS v1.0 Simulator           ║\n");
    /* Print the academic course attribution line */
    printf("║     Operating System Course Project      ║\n");
    /* Print bottom border box line enclosing the banner */
    printf("╚══════════════════════════════════════════╝\n");
    /* Reset terminal foreground color back to system default */
    printf(COLOR_RESET);
    /* Instruct the user on how to display available commands */
    printf("Type 'help' for list of commands.\n\n");
}

/*
 * Function: print_help
 * --------------------
 * Outputs a categorized reference manual of all supported shell commands.
 * Documents syntax, required/optional parameters, and behavioral descriptions
 * across all six core operating system subsystems.
 */
void print_help(void) {
    /* Print yellow header line delineating the command reference section */
    printf(COLOR_YELLOW "\n=== MiniOS Commands ===" COLOR_RESET "\n\n");
    
    /* Section 1: Process Management Subsystem commands */
    printf(COLOR_GREEN "Process Management:\n" COLOR_RESET);
    /* Document command to instantiate a new Process Control Block (PCB) in the process table */
    printf("  create <name> <burst> <priority> [arrival]  - Create a new process\n");
    /* Document command to display all active processes with their attributes and states */
    printf("  list                                        - List all processes\n");
    /* Document command to terminate an active process by unique Process Identifier (PID) */
    printf("  kill <pid>                                  - Terminate a process\n");
    /* Document command to flush all entries from the process table */
    printf("  clear_all                                   - Remove all processes\n\n");
    
    /* Section 2: CPU Scheduling Subsystem commands */
    printf(COLOR_GREEN "CPU Scheduling:\n" COLOR_RESET);
    /* Document First-Come, First-Served non-preemptive scheduling invocation */
    printf("  schedule fcfs                 - First Come First Serve\n");
    /* Document Shortest Job First non-preemptive scheduling invocation */
    printf("  schedule sjf                  - Shortest Job First\n");
    /* Document Round Robin preemptive time-sliced scheduling invocation with quantum */
    printf("  schedule rr <quantum>         - Round Robin\n");
    /* Document static priority-based scheduling invocation */
    printf("  schedule priority             - Priority Scheduling\n");
    /* Document 3-tier Multilevel Feedback Queue scheduling invocation */
    printf("  schedule mlfq                 - Multilevel Feedback Queue\n");
    /* Document comparative benchmark running all scheduling algorithms simultaneously */
    printf("  schedule compare              - Compare all algorithms\n\n");
    
    /* Section 3: Contiguous Memory Management Subsystem commands */
    printf(COLOR_GREEN "Memory Management:\n" COLOR_RESET);
    /* Document dynamic variable partition allocation command supporting FF, BF, and WF policies */
    printf("  mem_alloc <name> <size> <ff|bf|wf>  - Allocate memory\n");
    /* Document dynamic memory deallocation and adjacent hole coalescing command */
    printf("  mem_free <name>                     - Free memory\n");
    /* Document command to render the visual heap memory map and fragmentation statistics */
    printf("  mem_status                          - Show memory map\n");
    /* Document command to recycle the heap back to a single unallocated 1024 MB partition */
    printf("  mem_reset                           - Reset memory\n\n");
    
    /* Section 4: Process Synchronization & Concurrency Control commands */
    printf(COLOR_GREEN "Synchronization:\n" COLOR_RESET);
    /* Document bounded-buffer producer-consumer multi-threaded simulation command */
    printf("  producer_consumer             - Run Producer-Consumer simulation\n");
    /* Document Banker's deadlock detection and avoidance safety algorithm command */
    printf("  bankers                       - Run Banker's Algorithm\n\n");
    
    /* Section 5: Virtual Memory Page Replacement Subsystem commands */
    printf(COLOR_GREEN "Page Replacement:\n" COLOR_RESET);
    /* Document page replacement simulation command syntax accepting algorithm, frames, and reference string */
    printf("  page <fifo|lru|optimal|compare> <frames> <ref_string>\n");
    /* Provide concrete example showing 3 physical frames and standard page reference sequence */
    printf("  Example: page lru 3 7,0,1,2,0,3,0,4,2,3\n\n");
    
    /* Section 6: Secondary Storage Disk Arm Scheduling Subsystem commands */
    printf(COLOR_GREEN "Disk Scheduling:\n" COLOR_RESET);
    /* Document disk head scheduling simulation command syntax accepting algorithm, head pos, and cylinder list */
    printf("  disk <fcfs|sstf|scan|cscan|compare> <head> <requests>\n");
    /* Provide concrete example showing initial head at cylinder 53 and incoming I/O requests */
    printf("  Example: disk sstf 53 98,183,37,122,14,124,65,67\n\n");
    
    /* Section 7: General Utility and Shell Control commands */
    printf(COLOR_GREEN "Other:\n" COLOR_RESET);
    /* Document command to re-display this help manual */
    printf("  help                          - Show this help\n");
    /* Document command to clear the console display and re-render the banner */
    printf("  clear                         - Clear screen\n");
    /* Document command to cleanly terminate MiniOS and release all allocated resources */
    printf("  exit                          - Exit MiniOS\n\n");
}

/*
 * Function: parse_int_list
 * ------------------------
 * Tokenizes a comma-delimited string of integer values (e.g., "7,0,1,2,0,3")
 * and populates a caller-provided integer buffer array.
 *
 * Parameters:
 *   str       - Constant pointer to the raw comma-separated ASCII string.
 *   arr       - Destination integer array to store parsed numeric values.
 *   max_count - Maximum allowable elements to parse to prevent buffer overflows.
 *
 * Returns:
 *   Total count of successfully parsed integer elements.
 */
int parse_int_list(const char *str, int *arr, int max_count) {
    /* Initialize parsed integer counter to zero */
    int count = 0;
    /* Allocate temporary local buffer to store a mutable copy of the input string */
    char buffer[256];
    /* Safely copy at most sizeof(buffer)-1 bytes from the input string into buffer */
    strncpy(buffer, str, sizeof(buffer) - 1);
    /* Explicitly terminate string with null byte to guarantee safety against missing null terminators */
    buffer[sizeof(buffer) - 1] = '\0';
    
    /* Initialize standard library tokenizer using comma character as the delimiter */
    char *token = strtok(buffer, ",");
    /* Loop while valid delimited tokens exist and capacity remains in destination array */
    while (token != NULL && count < max_count) {
        /* Convert ASCII string token to integer via atoi and store in destination array slot */
        arr[count++] = atoi(token);
        /* Advance strtok by passing NULL to retrieve the next comma-delimited token */
        token = strtok(NULL, ",");
    }
    /* Return the total number of integer tokens extracted */
    return count;
}

/*
 * Function: handle_schedule
 * -------------------------
 * Command handler for the 'schedule' family of shell commands.
 * Validates process table prerequisites and dispatches execution to the
 * chosen CPU scheduling algorithm.
 *
 * Parameters:
 *   args - Pointer to the argument substring following the 'schedule' command token.
 */
void handle_schedule(char *args) {
    /* Validate that at least one process exists in the process table */
    if (process_count == 0) {
        /* Output error notification if user attempts scheduling with an empty table */
        printf(COLOR_RED "Error: No processes created. Use 'create' first.\n" COLOR_RESET);
        /* Return early to shell loop */
        return;
    }
    
    /* Validate that algorithm arguments were supplied */
    if (args == NULL) {
        /* Output syntax usage hint for the schedule command */
        printf(COLOR_RED "Usage: schedule <fcfs|sjf|rr|priority|mlfq|compare>\n" COLOR_RESET);
        /* Return early to shell loop */
        return;
    }
    
    /* Check if requested algorithm is First-Come, First-Served */
    if (strcmp(args, "fcfs") == 0) {
        /* Invoke non-preemptive FCFS scheduler implementation */
        schedule_fcfs();
    /* Check if requested algorithm is Shortest Job First */
    } else if (strcmp(args, "sjf") == 0) {
        /* Invoke non-preemptive SJF scheduler implementation */
        schedule_sjf();
    /* Check if requested algorithm begins with 'rr' for Round Robin */
    } else if (strncmp(args, "rr", 2) == 0) {
        /* Define default time quantum of 2 milliseconds if unspecified */
        int quantum = 2;
        /* Attempt to tokenize optional quantum argument following 'rr' prefix */
        char *q = strtok(args + 2, " ");
        /* If quantum argument token was parsed, convert string to integer */
        if (q != NULL) quantum = atoi(q);
        /* Guard against non-positive quantum values by resetting to default 2 */
        if (quantum <= 0) quantum = 2;
        /* Invoke preemptive Round Robin scheduler with validated time slice */
        schedule_rr(quantum);
    /* Check if requested algorithm is non-preemptive static priority scheduling */
    } else if (strcmp(args, "priority") == 0) {
        /* Invoke Priority scheduler implementation */
        schedule_priority();
    /* Check if requested algorithm is Multilevel Feedback Queue */
    } else if (strcmp(args, "mlfq") == 0) {
        /* Invoke 3-level feedback queue scheduler with dynamic queue demotion */
        schedule_mlfq();
    /* Check if requested command is comparative performance benchmark */
    } else if (strcmp(args, "compare") == 0) {
        /* Execute all schedulers across the workload and print side-by-side metrics table */
        schedule_compare();
    /* Handle unknown or invalid scheduling algorithm identifier */
    } else {
        /* Print error alerting the user to unrecognized algorithm */
        printf(COLOR_RED "Unknown algorithm: %s\n" COLOR_RESET, args);
    }
}

/*
 * Function: handle_page
 * --------------------
 * Command handler for virtual memory page replacement simulations.
 * Parses replacement algorithm name, physical frame count, and reference string.
 *
 * Parameters:
 *   args - Pointer to the argument substring following the 'page' command token.
 */
void handle_page(char *args) {
    /* Validate that argument string is non-null */
    if (args == NULL) {
        /* Print syntax usage message */
        printf(COLOR_RED "Usage: page <fifo|lru|optimal|compare> <frames> <ref_string>\n" COLOR_RESET);
        /* Return early to shell loop */
        return;
    }
    
    /* Declare buffer to store algorithm name token */
    char algo[20];
    /* Declare integer variable for physical page frame capacity */
    int frames;
    /* Declare buffer to store the raw comma-separated page reference string */
    char ref_str[256];
    
    /* Parse exactly three formatted arguments: algorithm string, frame integer, and reference string */
    if (sscanf(args, "%s %d %s", algo, &frames, ref_str) != 3) {
        /* Print usage error if required parameters could not be parsed */
        printf(COLOR_RED "Usage: page <fifo|lru|optimal|compare> <frames> <ref_string>\n" COLOR_RESET);
        /* Return early to shell loop */
        return;
    }
    
    /* Validate that physical frame count is within allowable system bounds [1, MAX_FRAMES] */
    if (frames <= 0 || frames > MAX_FRAMES) {
        /* Output boundary violation diagnostic */
        printf(COLOR_RED "Error: frames must be between 1 and %d\n" COLOR_RESET, MAX_FRAMES);
        /* Return early to shell loop */
        return;
    }
    
    /* Allocate integer array to store parsed page reference sequence */
    int ref[MAX_REF_STRING];
    /* Parse comma-separated page reference string into integer array */
    int ref_len = parse_int_list(ref_str, ref, MAX_REF_STRING);
    
    /* Validate that at least one page reference was parsed */
    if (ref_len == 0) {
        /* Output error alerting user to empty page reference string */
        printf(COLOR_RED "Error: empty reference string\n" COLOR_RESET);
        /* Return early to shell loop */
        return;
    }
    
    /* Dispatch to First-In, First-Out page replacement algorithm */
    if (strcmp(algo, "fifo") == 0) {
        /* Execute FIFO page replacement simulation */
        page_replace_fifo(frames, ref, ref_len);
    /* Dispatch to Least Recently Used page replacement algorithm */
    } else if (strcmp(algo, "lru") == 0) {
        /* Execute LRU page replacement simulation */
        page_replace_lru(frames, ref, ref_len);
    /* Dispatch to Bélády's Optimal theoretical page replacement algorithm */
    } else if (strcmp(algo, "optimal") == 0) {
        /* Execute Optimal page replacement simulation */
        page_replace_optimal(frames, ref, ref_len);
    /* Dispatch to comparative paging benchmark */
    } else if (strcmp(algo, "compare") == 0) {
        /* Execute all page replacement algorithms on reference sequence and print summary */
        page_replace_compare(frames, ref, ref_len);
    /* Handle unknown page replacement algorithm */
    } else {
        /* Print error alerting the user to invalid algorithm choice */
        printf(COLOR_RED "Unknown algorithm: %s\n" COLOR_RESET, algo);
    }
}

/*
 * Function: handle_disk
 * --------------------
 * Command handler for secondary storage disk arm scheduling simulations.
 * Parses scheduling algorithm, initial read/write head position, and cylinder request list.
 *
 * Parameters:
 *   args - Pointer to the argument substring following the 'disk' command token.
 */
void handle_disk(char *args) {
    /* Validate that argument string is non-null */
    if (args == NULL) {
        /* Print syntax usage message */
        printf(COLOR_RED "Usage: disk <fcfs|sstf|scan|cscan|compare> <head> <requests>\n" COLOR_RESET);
        /* Return early to shell loop */
        return;
    }
    
    /* Declare buffer to store algorithm name token */
    char algo[20];
    /* Declare integer variable to hold initial disk head position */
    int head;
    /* Declare buffer to hold raw comma-separated cylinder request numbers */
    char req_str[256];
    
    /* Parse exactly three formatted arguments: algorithm string, head integer, and requests string */
    if (sscanf(args, "%s %d %s", algo, &head, req_str) != 3) {
        /* Print syntax usage message if argument parsing fails */
        printf(COLOR_RED "Usage: disk <fcfs|sstf|scan|cscan|compare> <head> <requests>\n" COLOR_RESET);
        /* Return early to shell loop */
        return;
    }
    
    /* Allocate integer array to store parsed track cylinder requests */
    int requests[MAX_REQUESTS];
    /* Parse comma-separated cylinder requests into integer array */
    int n = parse_int_list(req_str, requests, MAX_REQUESTS);
    
    /* Validate that at least one cylinder request was successfully parsed */
    if (n == 0) {
        /* Output error alerting user to empty cylinder request list */
        printf(COLOR_RED "Error: empty request list\n" COLOR_RESET);
        /* Return early to shell loop */
        return;
    }
    
    /* Dispatch to First-Come, First-Served disk arm scheduling */
    if (strcmp(algo, "fcfs") == 0) {
        /* Execute FCFS disk scheduling simulation */
        disk_fcfs(requests, n, head);
    /* Dispatch to Shortest Seek Time First greedy disk scheduling */
    } else if (strcmp(algo, "sstf") == 0) {
        /* Execute SSTF disk scheduling simulation */
        disk_sstf(requests, n, head);
    /* Dispatch to SCAN elevator bidirectional disk scheduling */
    } else if (strcmp(algo, "scan") == 0) {
        /* Execute SCAN disk scheduling simulation */
        disk_scan(requests, n, head);
    /* Dispatch to Circular SCAN unidirectional disk scheduling */
    } else if (strcmp(algo, "cscan") == 0) {
        /* Execute C-SCAN disk scheduling simulation */
        disk_cscan(requests, n, head);
    /* Dispatch to comparative disk benchmark */
    } else if (strcmp(algo, "compare") == 0) {
        /* Execute all disk scheduling algorithms and compare total seek distances */
        disk_compare(requests, n, head);
    /* Handle unknown disk scheduling algorithm */
    } else {
        /* Print error alerting the user to invalid algorithm choice */
        printf(COLOR_RED "Unknown algorithm: %s\n" COLOR_RESET, algo);
    }
}

/*
 * Function: main
 * --------------
 * Kernel simulator entry point and primary interactive command shell loop.
 * Initializes core kernel data structures, disables stream buffering for real-time
 * communication, and executes the Read-Eval-Print Loop (REPL).
 */
int main(void) {
    /* Allocate 512-byte character array on stack to store incoming raw shell command lines */
    char input[512];
    
    /*
     * Disable standard output stream buffering (_IONBF = I/O No Buffer).
     * By default in C, stdout is block-buffered when piped or line-buffered when attached to a tty.
     * In an operating system simulation or GUI-backend architecture, disabling output buffering
     * guarantees that diagnostic prints, Gantt charts, and memory maps flush immediately to stdout
     * without requiring an explicit fflush(stdout) call after every print operation.
     */
    setvbuf(stdout, NULL, _IONBF, 0);
    
    /* Initialize the process subsystem: clear PCB array, reset PID counter and metrics */
    init_process_table();
    /* Initialize the contiguous memory subsystem: create root 1024 MB free memory partition */
    init_memory();
    
    /* Clear terminal window for clean visual presentation */
    clear_screen();
    /* Display the welcome banner and introductory instructions */
    print_banner();
    
    /*
     * Shell Read-Eval-Print Loop (REPL):
     * Runs continuously until user enters 'exit'/'quit' or EOF is encountered on stdin.
     */
    while (1) {
        /* Display the shell prompt in bright blue */
        printf(COLOR_BLUE "minios> " COLOR_RESET);
        
        /* Read a complete command line from standard input into input buffer */
        if (fgets(input, sizeof(input), stdin) == NULL) {
            /* Break loop and exit cleanly if EOF (End-Of-File / Ctrl+D) is detected */
            break;
        }
        
        /* Find and replace trailing newline character '\n' with null terminator '\0' */
        input[strcspn(input, "\n")] = '\0';
        
        /* If user pressed Enter without typing any command characters, skip evaluation */
        if (strlen(input) == 0) continue;
        
        /* Extract primary command name by tokenizing up to the first space delimiter */
        char *cmd = strtok(input, " ");
        /* Compute initial raw pointer to arguments located immediately after the command token */
        char *args = cmd + strlen(cmd) + 1;
        /* Validate pointer boundary against original input buffer bounds */
        if (args >= input + strlen(input) + 1) args = NULL;
        /* Block scope for precise argument pointer sanitization and whitespace trimming */
        {
            /* Check if argument pointer lies within the valid 512-byte allocated buffer boundary */
            if (cmd + strlen(cmd) < input + 511) {
                /* Point args to the character immediately following the null byte written by strtok */
                args = cmd + strlen(cmd) + 1;
                /* Advance pointer past any leading whitespace characters */
                while (args < input + 512 && *args == ' ') args++;
                /* If argument string is empty after trimming spaces, set pointer to NULL */
                if (*args == '\0') args = NULL;
            /* If command token exhausted the entire buffer, no arguments exist */
            } else {
                /* Set argument pointer to NULL */
                args = NULL;
            }
        }
        
        /*
         * Command Evaluation and Dispatch Phase:
         * Compare parsed command string against registered MiniOS system commands.
         */
        
        /* Check for shell termination commands: 'exit' or 'quit' */
        if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
            /* Print graceful shutdown message */
            printf("Shutting down MiniOS...\n");
            /* Release all dynamically allocated memory partitions in the heap block list */
            cleanup_memory();
            /* Break out of the infinite REPL loop to exit main */
            break;
        }
        /* Check for 'help' command to display command manual */
        else if (strcmp(cmd, "help") == 0) {
            /* Invoke help documentation printer */
            print_help();
        }
        /* Check for 'clear' command to refresh the console display */
        else if (strcmp(cmd, "clear") == 0) {
            /* Clear the terminal screen */
            clear_screen();
            /* Re-render the introductory MiniOS graphical banner */
            print_banner();
        }
        /* Check for 'create' command to instantiate a new process */
        else if (strcmp(cmd, "create") == 0) {
            /* Validate that required process creation arguments were supplied */
            if (args == NULL) {
                /* Print usage reminder */
                printf(COLOR_RED "Usage: create <name> <burst_time> <priority> [arrival_time]\n" COLOR_RESET);
                /* Continue to next REPL cycle */
                continue;
            }
            /* Declare local buffer to hold process name string */
            char name[MAX_NAME_LEN];
            /* Declare variables for CPU burst time, static priority, and optional arrival time */
            int burst, prio, arrival = 0;
            /* Parse name, burst, priority, and optional arrival time using sscanf */
            int parsed = sscanf(args, "%s %d %d %d", name, &burst, &prio, &arrival);
            /* Ensure at least the three mandatory fields (name, burst, priority) were parsed */
            if (parsed < 3) {
                /* Print usage error if required fields are missing */
                printf(COLOR_RED "Usage: create <name> <burst_time> <priority> [arrival_time]\n" COLOR_RESET);
                /* Continue to next REPL cycle */
                continue;
            }
            /* Invoke kernel process creation API to allocate PCB in process table */
            int pid = create_process(name, burst, prio, arrival);
            /* If process creation succeeded, announce PID to user */
            if (pid >= 0) {
                /* Print green confirmation message showing assigned PID */
                printf(COLOR_GREEN "Process '%s' created with PID %d\n" COLOR_RESET, name, pid);
            }
        }
        /* Check for 'list' command to display all processes in process table */
        else if (strcmp(cmd, "list") == 0) {
            /* Invoke process table listing routine */
            list_processes();
        }
        /* Check for 'kill' command to terminate a running or ready process */
        else if (strcmp(cmd, "kill") == 0) {
            /* Validate that PID argument was supplied */
            if (args == NULL) {
                /* Print usage reminder */
                printf(COLOR_RED "Usage: kill <pid>\n" COLOR_RESET);
                /* Continue to next REPL cycle */
                continue;
            }
            /* Convert PID string argument to integer */
            int pid = atoi(args);
            /* Invoke kernel kill process API; returns 0 on success */
            if (kill_process(pid) == 0) {
                /* Print green termination confirmation message */
                printf(COLOR_GREEN "Process %d terminated.\n" COLOR_RESET, pid);
            }
        }
        /* Check for 'clear_all' command to wipe and reinitialize the entire process table */
        else if (strcmp(cmd, "clear_all") == 0) {
            /* Reset the process table, zeroing process count and reclaiming all slots */
            init_process_table();
            /* Print green confirmation message */
            printf(COLOR_GREEN "All processes cleared.\n" COLOR_RESET);
        }
        /* Check for 'schedule' command to invoke CPU scheduling */
        else if (strcmp(cmd, "schedule") == 0) {
            /* Delegate argument parsing and execution to handle_schedule */
            handle_schedule(args);
        }
        /* Check for 'mem_alloc' command to dynamically allocate a contiguous memory block */
        else if (strcmp(cmd, "mem_alloc") == 0) {
            /* Validate that memory allocation arguments were supplied */
            if (args == NULL) {
                /* Print usage reminder */
                printf(COLOR_RED "Usage: mem_alloc <name> <size> <ff|bf|wf>\n" COLOR_RESET);
                /* Continue to next REPL cycle */
                continue;
            }
            /* Declare local buffer to hold process name for memory partition ownership */
            char name[MAX_NAME_LEN];
            /* Declare integer for requested partition size in MB */
            int size;
            /* Declare string buffer to store allocation placement policy flag */
            char policy_str[4];
            /* Parse process name, allocation size, and policy string from arguments */
            if (sscanf(args, "%s %d %s", name, &size, policy_str) != 3) {
                /* Print error if arguments could not be parsed */
                printf(COLOR_RED "Usage: mem_alloc <name> <size> <ff|bf|wf>\n" COLOR_RESET);
                /* Continue to next REPL cycle */
                continue;
            }
            /* Declare enumeration variable for allocation policy */
            AllocPolicy pol;
            /* Map 'ff' string flag to First-Fit enumeration value */
            if (strcmp(policy_str, "ff") == 0) pol = FIRST_FIT;
            /* Map 'bf' string flag to Best-Fit enumeration value */
            else if (strcmp(policy_str, "bf") == 0) pol = BEST_FIT;
            /* Map 'wf' string flag to Worst-Fit enumeration value */
            else if (strcmp(policy_str, "wf") == 0) pol = WORST_FIT;
            /* Handle invalid policy identifier */
            else {
                /* Print error informing user of supported allocation policies */
                printf(COLOR_RED "Policy must be ff (First Fit), bf (Best Fit), or wf (Worst Fit)\n" COLOR_RESET);
                /* Continue to next REPL cycle */
                continue;
            }
            /* Call kernel memory allocation routine; returns 0 on success */
            if (mem_allocate(name, size, pol) == 0) {
                /* Print green confirmation showing allocated size and owner name */
                printf(COLOR_GREEN "Allocated %d MB to '%s'\n" COLOR_RESET, size, name);
            }
        }
        /* Check for 'mem_free' command to deallocate a process's memory partition */
        else if (strcmp(cmd, "mem_free") == 0) {
            /* Validate that process name argument was supplied */
            if (args == NULL) {
                /* Print usage reminder */
                printf(COLOR_RED "Usage: mem_free <name>\n" COLOR_RESET);
                /* Continue to next REPL cycle */
                continue;
            }
            /* Declare local buffer to hold process name */
            char name[MAX_NAME_LEN];
            /* Parse process name from arguments */
            sscanf(args, "%s", name);
            /* Invoke kernel memory deallocation routine; coalesces adjacent free holes */
            if (mem_free(name) == 0) {
                /* Print green confirmation of memory partition release */
                printf(COLOR_GREEN "Freed memory for '%s'\n" COLOR_RESET, name);
            }
        }
        /* Check for 'mem_status' command to display partition layout and fragmentation statistics */
        else if (strcmp(cmd, "mem_status") == 0) {
            /* Invoke memory status dump function */
            mem_status();
        }
        /* Check for 'mem_reset' command to release all partitions and reset heap to 1024 MB */
        else if (strcmp(cmd, "mem_reset") == 0) {
            /* Deallocate all existing partition nodes in doubly-linked heap list */
            cleanup_memory();
            /* Reinitialize heap with a single pristine 1024 MB unallocated block */
            init_memory();
            /* Print green confirmation of memory reset */
            printf(COLOR_GREEN "Memory reset.\n" COLOR_RESET);
        }
        /* Check for 'producer_consumer' command to run semaphore-synchronized concurrency simulation */
        else if (strcmp(cmd, "producer_consumer") == 0) {
            /* Execute bounded-buffer Producer-Consumer simulation */
            run_producer_consumer();
        }
        /* Check for 'bankers' command to run deadlock avoidance safety algorithm */
        else if (strcmp(cmd, "bankers") == 0) {
            /* Execute Banker's algorithm matrix calculation and safe sequence detection */
            run_bankers();
        }
        /* Check for 'page' command to invoke virtual memory page replacement */
        else if (strcmp(cmd, "page") == 0) {
            /* Delegate argument parsing and execution to handle_page */
            handle_page(args);
        }
        /* Check for 'disk' command to invoke secondary storage disk arm scheduling */
        else if (strcmp(cmd, "disk") == 0) {
            /* Delegate argument parsing and execution to handle_disk */
            handle_disk(args);
        }
        /* Handle unrecognized command input */
        else {
            /* Print error indicating command not found and suggesting 'help' */
            printf(COLOR_RED "Unknown command: '%s'. Type 'help' for list of commands.\n" COLOR_RESET, cmd);
        }
    }
    
    /* Return exit code 0 indicating successful and normal program termination */
    return 0;
}
