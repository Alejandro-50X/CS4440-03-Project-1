#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE_LENGTH 1024
#define MAX_ARGS 64
#define PROMPT "MoreShell> "

/**
 * Executes a command in a child process, passing an array of arguments.
 * @param args An array of strings where args[0] is the command and the rest are arguments.
 */
void execute_command(char *args[]) {
    pid_t pid;
    int status;

    // 1. Fork a new process
    pid = fork();

    if (pid < 0) {
        // Fork failed
        perror("fork failed");
        return;
    } 
    
    if (pid == 0) {
        // --- CHILD PROCESS ---
        
        // Execute the command: execvp searches the PATH environment variable for the command.
        // The args array must be NULL-terminated for execvp to work correctly.
        if (execvp(args[0], args) == -1) {
            // If execvp returns, an error occurred (e.g., command not found)
            perror("MoreShell: Command execution failed");
            exit(EXIT_FAILURE);
        }
    } else {
        // --- PARENT PROCESS ---
        
        // The parent waits for the child process to finish.
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
        }
    }
}

int main() {
    char command_line[MAX_LINE_LENGTH];
    char *args[MAX_ARGS]; // Array to hold the command and its arguments
    char *token;
    int arg_count;

    while (1) {
        printf(PROMPT);
        fflush(stdout); 

        // Read the command from standard input
        if (fgets(command_line, MAX_LINE_LENGTH, stdin) == NULL) {
            // Handle Ctrl+D (EOF)
            printf("\nExiting MoreShell...\n");
            break;
        }

        // Remove the trailing newline character
        command_line[strcspn(command_line, "\n")] = '\0';
        
        // Ignore empty commands
        if (strlen(command_line) == 0) {
            continue;
        }
        
        // --- ARGUMENT PARSING (TOKENIZATION) ---
        arg_count = 0;
        
        // Use strtok to get the first token (the command)
        token = strtok(command_line, " ");
        
        while (token != NULL && arg_count < MAX_ARGS - 1) {
            args[arg_count++] = token;
            // Get the next token (argument)
            token = strtok(NULL, " ");
        }
        
        // The argument array MUST be NULL-terminated for execvp
        args[arg_count] = NULL; 

        // Check for the 'exit' command (must be done AFTER parsing)
        if (strcmp(args[0], "exit") == 0) {
            break;
        }
        
        // Execute the parsed command and its arguments
        execute_command(args);
    }

    return 0;
}