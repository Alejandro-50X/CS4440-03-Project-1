#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE_LENGTH 1024
#define MAX_ARGS 64
#define PROMPT "MoreShell> "

/*
     Moresheel is a basic terminal just like MinShell that reads commands from the user and take the command and its argument and execute the command. 
 */
void execute_command(char *args[]) {
    pid_t pid;
    int status;

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    } 
    
    if (pid == 0) {
        // the child process
        
        // execute the command execvp searches the PATH environment variable for the command
        // the args array must be NULL terminated for execvp to work correctly
        if (execvp(args[0], args) == -1) {
            perror("MoreShell: Command execution failed");
            exit(EXIT_FAILURE);
        }
    } else {
        // parent process
        
        // waiting for the child
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
        }
    }
}

int main() {
    char command_line[MAX_LINE_LENGTH];
    char *args[MAX_ARGS]; // array to hold the command and its arguments
    char *token;
    int arg_count;

    while (1) {
        printf(PROMPT);
        fflush(stdout); 

        if (fgets(command_line, MAX_LINE_LENGTH, stdin) == NULL) {
            // Handle Ctrl+D (EOF)
            printf("\nExiting MoreShell...\n");
            break;
        }

        // remove the trailing newline character
        command_line[strcspn(command_line, "\n")] = '\0';
        
        // ignore empty commands
        if (strlen(command_line) == 0) {
            continue;
        }
        
        // this is argument parsing
        arg_count = 0;
        
        // use strtok to get the first token which is the command
        token = strtok(command_line, " ");
        
        while (token != NULL && arg_count < MAX_ARGS - 1) {
            args[arg_count++] = token;
            // Get the next token which will be the argument
            token = strtok(NULL, " ");
        }
        
        // the argument array must be NULL terminated for execvp
        args[arg_count] = NULL; 

        // check for the exit command 
        if (strcmp(args[0], "exit") == 0) {
            break;
        }
        
        // execute the parsed command and its arguments
        execute_command(args);
    }

    return 0;
}
