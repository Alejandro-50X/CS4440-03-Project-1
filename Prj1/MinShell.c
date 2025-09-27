#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 1024
#define PROMPT "MinShell> "

void execute_command(char *command) {
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
        
        // This is the array of arguments for execvp.
        // Since we are handling "argument-less" commands, 
        // the command itself is the first element (argv[0]), 
        // and the list is terminated by a NULL pointer.
        char *args[] = {command, NULL};

        // Execute the command: 
        // execvp searches the PATH environment variable for the command.
        // It replaces the child's process image with the new program (e.g., /bin/ls).
        if (execvp(command, args) == -1) {
            // If execvp returns, an error occurred (e.g., command not found)
            perror("MinShell: Command execution failed");
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
    char command_line[MAX_COMMAND_LENGTH];

    while (1) {
        // Print the shell prompt
        printf(PROMPT);
        fflush(stdout); // Ensure the prompt is immediately displayed

        // Read the command from standard input
        if (fgets(command_line, MAX_COMMAND_LENGTH, stdin) == NULL) {
            // Handle Ctrl+D (EOF)
            printf("\nExiting MinShell...\n");
            break;
        }

        // Remove the trailing newline character added by fgets
        command_line[strcspn(command_line, "\n")] = '\0';
        
        // Check for the 'exit' command to quit the shell
        if (strcmp(command_line, "exit") == 0) {
            break;
        }

        // Ignore empty commands (just hitting Enter)
        if (strlen(command_line) == 0) {
            continue;
        }

        // Execute the command in a new process
        execute_command(command_line);
    }

    return 0;
}