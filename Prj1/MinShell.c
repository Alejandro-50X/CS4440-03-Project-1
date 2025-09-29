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

    // fork a new process
    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    } 
    
    if (pid == 0) {
        // this is the child process
        
        // This is the array of arguments for execvp
        // since we are handling argument-less commands 
        // the command itself is the first element (argv[0]) 
        // and the list is terminated by a NULL pointer
        char *args[] = {command, NULL};

        // Execute the command 
        // execvp searches the PATH environment variable for the command
        // it replaces the childs process image with the new program like ls
        if (execvp(command, args) == -1) {
            // if execvp returns an error occurred like command not found
            perror("MinShell: Command execution failed");
            exit(EXIT_FAILURE);
        }
    } else {
        // this is the parent process
        
        // the parent waits for the child process to finish
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
        }
    }
}

int main() {
    char command_line[MAX_COMMAND_LENGTH];

    while (1) {
        // print the shell prompt
        printf(PROMPT);
        fflush(stdout); // ensure the prompt is immediately displayed

        // read the command from standard input
        if (fgets(command_line, MAX_COMMAND_LENGTH, stdin) == NULL) {
            printf("\nExiting MinShell...\n");
            break;
        }

        // remove the trailing newline character added by fgets
        command_line[strcspn(command_line, "\n")] = '\0';
        
        // check for the 'exit' command to quit the shell
        if (strcmp(command_line, "exit") == 0) {
            break;
        }

        // ignore empty commands like when you hit enter
        if (strlen(command_line) == 0) {
            continue;
        }

        // execute the command in a new process
        execute_command(command_line);
    }

    return 0;
}
