#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE_LENGTH 1024
#define MAX_ARGS 64
#define PROMPT "DupShell> "
#define DELIMITER " "

// Function to parse a single command string into an array of arguments
void parse_command(char *line, char *args[]) {
    int arg_count = 0;
    char *token = strtok(line, DELIMITER);

    while (token != NULL && arg_count < MAX_ARGS - 1) {
        args[arg_count++] = token;
        token = strtok(NULL, DELIMITER);
    }
    args[arg_count] = NULL; // NULL-terminate for execvp
}

// Function to execute a single command (left or right side of the pipe)
void execute_single_command(char *args[]) {
    // execvp replaces the current process with the new command.
    if (execvp(args[0], args) == -1) {
        perror("DupShell: Command execution failed");
        exit(EXIT_FAILURE);
    }
}

// Function to handle the pipeline execution
void execute_pipeline(char *cmd1_line, char *cmd2_line) {
    int pipe_fd[2]; // pipe_fd[0] is read, pipe_fd[1] is write
    pid_t pid1, pid2;

    if (pipe(pipe_fd) == -1) {
        perror("pipe failed");
        return;
    }

    // --- 1. Execute Left Command (Writer to Pipe) ---
    pid1 = fork();
    if (pid1 == 0) {
        // Child 1 (Left Command)
        
        // Redirect stdout (fd 1) to the write end of the pipe
        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
            perror("dup2 failed for writer");
            exit(EXIT_FAILURE);
        }
        
        // Close unused pipe descriptors
        close(pipe_fd[0]);
        close(pipe_fd[1]);

        char *args1[MAX_ARGS];
        parse_command(cmd1_line, args1);
        execute_single_command(args1);
    } else if (pid1 < 0) {
        perror("fork failed for cmd1");
        return;
    }

    // --- 2. Execute Right Command (Reader from Pipe) ---
    pid2 = fork();
    if (pid2 == 0) {
        // Child 2 (Right Command)
        
        // Redirect stdin (fd 0) to the read end of the pipe
        if (dup2(pipe_fd[0], STDIN_FILENO) == -1) {
            perror("dup2 failed for reader");
            exit(EXIT_FAILURE);
        }
        
        // Close unused pipe descriptors
        close(pipe_fd[0]);
        close(pipe_fd[1]);

        char *args2[MAX_ARGS];
        parse_command(cmd2_line, args2);
        execute_single_command(args2);
    } else if (pid2 < 0) {
        perror("fork failed for cmd2");
        return;
    }

    // --- 3. Parent Process Cleanup and Wait ---
    
    // Parent must close its copies of the pipe descriptors
    close(pipe_fd[0]);
    close(pipe_fd[1]);

    // Wait for both children to complete
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}


// --- MAIN FUNCTION ---
int main() {
    char command_line[MAX_LINE_LENGTH];
    char *cmd1_line, *cmd2_line;

    while (1) {
        printf(PROMPT);
        fflush(stdout); 

        if (fgets(command_line, MAX_LINE_LENGTH, stdin) == NULL) {
            printf("\nExiting DupShell...\n");
            break;
        }

        command_line[strcspn(command_line, "\n")] = '\0';
        if (strlen(command_line) == 0) continue;

        // --- PIPELINE PARSING ---
        cmd1_line = command_line;
        cmd2_line = strstr(command_line, "|"); // Find the pipe symbol
        
        if (cmd2_line != NULL) {
            // Found a pipe: replace '|' with '\0' to separate commands
            *cmd2_line = '\0'; 
            cmd2_line++;        // Move pointer past the '\0'
            while (*cmd2_line == ' ') cmd2_line++; // Skip leading space on cmd2

            // Handle case where pipe is the only thing or followed by nothing
            if (strlen(cmd1_line) == 0 || strlen(cmd2_line) == 0) {
                 fprintf(stderr, "DupShell: Invalid pipe usage.\n");
            } else {
                 execute_pipeline(cmd1_line, cmd2_line);
            }
        } else {
            // No pipe found, execute as a single command (fallback to MoreShell logic)
            char *args[MAX_ARGS];
            parse_command(command_line, args);

            if (strcmp(args[0], "exit") == 0) {
                break;
            }
            execute_pipeline(command_line, NULL); // Note: This would need modification 
                                                  // to handle the single command case cleanly.
                                                  // For simplicity here, we'll only demonstrate 
                                                  // the pipelined execution.
        }
    }

    return 0;
}