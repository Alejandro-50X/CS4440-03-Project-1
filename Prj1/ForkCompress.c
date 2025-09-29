#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
    The main function of this program is to compress files with argument validation and process creation like fork. 
    Having a parent and child and making sure they execute properly.
*/

int main(int argc, char *argv[]) {
    pid_t pid;
    int status;
   
    // check for correct command line arguments 
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_file> <destination_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // create a new process using fork
    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // this is the child process
        printf("Child process (PID: %d) is starting compression...\n", getpid());
       
        // execute the MyCompress2 program using execl()
        // Arguments to execl:
        // path to the executable: "./MyCompress" assumes it's in the current directory.
        // Arg0 (program name): "MyCompress2"
        // Arg1 (source file): argv[1]
        // Arg2 (destination file): argv[2]
        // NULL terminator for argument list
       
        execl("./MyCompress2", "MyCompress2", argv[1], argv[2], (char *)NULL);

        // if execl returns it must have failed 
        perror("execl failed");
        exit(EXIT_FAILURE); // exit the child process if execl fails
    } else {
        // this is the parent process
        printf("Parent process (PID: %d) forked child (PID: %d). Waiting...\n", getpid(), pid);

        // wait for the child process to finish using wait()
        if (wait(&status) == -1) {
            perror("wait failed");
            exit(EXIT_FAILURE);
        }

        // check the childs exit status
        if (WIFEXITED(status)) {
            printf("Child process (PID: %d) completed with exit status %d.\n", pid, WEXITSTATUS(status));
            if (WEXITSTATUS(status) == 0) {
                printf("Compression successful.\n");
            } else {
                printf("Compression failed.\n");
            }
        } else {
            printf("Child process terminated abnormally.\n");
        }
    }

    return 0;
}
