#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    pid_t pid;
    int status;
   
    // 1. Check for correct command-line arguments for THIS program
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_file> <destination_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 2. Create a new process using fork()
    pid = fork();

    if (pid < 0) {
        // fork() failed
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // This is the CHILD process
        printf("Child process (PID: %d) is starting compression...\n", getpid());
       
        // 3. Execute the MyCompress program using execl()
        // execl replaces the child process's image with the new program (MyCompress)
        // Arguments to execl:
        // 1. Path to the executable: "./MyCompress" assumes it's in the current directory.
        // 2. Arg0 (program name): "MyCompress"
        // 3. Arg1 (source file): argv[1]
        // 4. Arg2 (destination file): argv[2]
        // 5. NULL (terminator for argument list)
       
        execl("./MyCompress2", "MyCompress2", argv[1], argv[2], (char *)NULL);

        // If execl returns, it must have failed (error)
        perror("execl failed");
        exit(EXIT_FAILURE); // Exit the child process if execl fails
    } else {
        // This is the PARENT process
        printf("Parent process (PID: %d) forked child (PID: %d). Waiting...\n", getpid(), pid);

        // 4. Wait for the child process to finish using wait()
        if (wait(&status) == -1) {
            perror("wait failed");
            exit(EXIT_FAILURE);
        }

        // 5. Check the child's exit status
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