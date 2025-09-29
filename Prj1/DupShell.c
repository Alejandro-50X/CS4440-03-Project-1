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

/*
    The dupShell program will allow you to enter a shell environemnt and use commands like ls -l | wc and the exit command. It allows for the execution of single commands 
    with using some pipelining. 
*/

// function to parse a single command string into an array of arguments
void parse_command(char *line, char *args[]) {
    int arg_count = 0;
    char *token = strtok(line, DELIMITER);

    while (token != NULL && arg_count < MAX_ARGS - 1) {
        args[arg_count++] = token;
        token = strtok(NULL, DELIMITER);
    }
    args[arg_count] = NULL; // NULL terminate for execvp
}

// function to execute a single command 
void execute_single_command(char *args[]) {
    // execvp replaces the current process with the new command
    if (execvp(args[0], args) == -1) {
        perror("DupShell: Command execution failed");
        exit(EXIT_FAILURE);
    }
}

// function to handle the pipeline execution
void execute_pipeline(char *cmd1_line, char *cmd2_line) {
    int pipe_fd[2]; 
    pid_t pid1, pid2;

    if (pipe(pipe_fd) == -1) {
        perror("pipe failed");
        return;
    }

    // execute left command writer to the pipe
    pid1 = fork();
    if (pid1 == 0) {
        
        
        
        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
            perror("dup2 failed for writer");
            exit(EXIT_FAILURE);
        }
        
        // close unused pipe descriptors
        close(pipe_fd[0]);
        close(pipe_fd[1]);

        char *args1[MAX_ARGS];
        parse_command(cmd1_line, args1);
        execute_single_command(args1);
    } else if (pid1 < 0) {
        perror("fork failed for cmd1");
        return;
    }

    // execute right command 
    pid2 = fork();
    if (pid2 == 0) {
        
        if (dup2(pipe_fd[0], STDIN_FILENO) == -1) {
            perror("dup2 failed for reader");
            exit(EXIT_FAILURE);
        }
        
        close(pipe_fd[0]);
        close(pipe_fd[1]);

        char *args2[MAX_ARGS];
        parse_command(cmd2_line, args2);
        execute_single_command(args2);
    } else if (pid2 < 0) {
        perror("fork failed for cmd2");
        return;
    }

    // parent process and waits
    
    // parent must close its copies of the pipe descriptors
    close(pipe_fd[0]);
    close(pipe_fd[1]);

    // wait for both children to complete
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}


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

        // this is the pipeline parsing
        cmd1_line = command_line;
        cmd2_line = strstr(command_line, "|"); // find the pipe symbol
        
        if (cmd2_line != NULL) {
            *cmd2_line = '\0'; 
            cmd2_line++;       
            while (*cmd2_line == ' ') cmd2_line++; 

            // handle case where pipe is the only thing or followed by nothing
            if (strlen(cmd1_line) == 0 || strlen(cmd2_line) == 0) {
                 fprintf(stderr, "DupShell: Invalid pipe usage.\n");
            } else {
                 execute_pipeline(cmd1_line, cmd2_line);
            }
        } else {
            // no pipe found execute as a single command 
            char *args[MAX_ARGS];
            parse_command(command_line, args);

            if (strcmp(args[0], "exit") == 0) {
                break;
            }
            execute_pipeline(command_line, NULL); 
                                                  
        }
    }

    return 0;
}
