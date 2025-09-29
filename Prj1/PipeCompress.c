#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// function containing the compression logic
void compress_data(FILE *source_file, int write_fd);
// function containing the write to file logic
void write_data(int read_fd, const char *dest_filename);

// the main process
int main(int argc, char *argv[]) {
    int pipe_fd[2]; // pipe_fd[0] is for reading pipe_fd[1] is for writing
    pid_t pid;
    
    // check for correct command line arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_file> <destination_file>\n", argv[0]);
        return 1;
    }

    const char *source_filename = argv[1];
    const char *dest_filename = argv[2];

    // create the pipe
    if (pipe(pipe_fd) == -1) {
        perror("pipe failed");
        return 1;
    }

    // fork a new process
    pid = fork();

    if (pid < 0) {
        // fork failed
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // the child process
        
        // close the read end of the pipe 
        close(pipe_fd[0]);
        
        // open the source file
        FILE *source = fopen(source_filename, "r");
        if (!source) {
            perror("Child: Error opening source file");
            close(pipe_fd[1]); // close the write end before exiting
            exit(1);
        }

        // run the compression logic writing compressed data to pipe_fd[1]
        compress_data(source, pipe_fd[1]);

        // clean up
        fclose(source);
        close(pipe_fd[1]); // close the write end to signal EOF to the reader (parent)
        exit(0);
    } else {
        // the parent process
        
        // close the write end of the pipe 
        close(pipe_fd[1]);

        // run the write logic reading compressed data from pipe_fd[0]
        write_data(pipe_fd[0], dest_filename);

        close(pipe_fd[0]); // close the read end
        
        // waits for the child process to finish
        wait(NULL); 
        printf("Compression complete: '%s' -> '%s' via pipe.\n", source_filename, dest_filename);
    }

    return 0;
}

// the compression logic
void compress_data(FILE *source_file, int write_fd) {
    int count = 1;
    char currentChar;
    int character_int;
    
    // gets the first character
    character_int = fgetc(source_file);
    if (character_int == EOF) {
        return; // empty file
    }
    currentChar = (char)character_int;

    // get the second character
    while ((character_int = fgetc(source_file)) != EOF) {
        char character = (char)character_int;
        
        if (character == currentChar) {
            
            count++;
        }
        else {
            // new character flush previous run
            
            // the flush logic
            if (count >= 16 && currentChar == '1') {
                char buffer[32];
                int len = snprintf(buffer, sizeof(buffer), "+%d+", count);
                // write the string to the pipe
                write(write_fd, buffer, len); 
            } else if (count >= 16 && currentChar == '0') {
                char buffer[32];
                int len = snprintf(buffer, sizeof(buffer), "-%d-", count);
                write(write_fd, buffer, len);
            } else {
                // write uncompressed run
                for (int i = 0; i < count; i++) {
                    write(write_fd, &currentChar, 1);
                }
            }

            // handle run
            if (character == ' ' || character == '\n') {
                // write the space or newline delimiter to the pipe
                write(write_fd, &character, 1);
                
                // Reset to read the character after the delimiter
                character_int = fgetc(source_file);
                if (character_int == EOF) break; 
                currentChar = (char)character_int;
                count = 1;
            } else {
                // start new run with the new character
                currentChar = character;
                count = 1;
            }
        }
    }

    // flush the final run 
    // the end of file is reached
    if (count > 0) {
        if (count >= 16 && currentChar == '1') {
            char buffer[32];
            int len = snprintf(buffer, sizeof(buffer), "+%d+", count);
            write(write_fd, buffer, len);
        } else if (count >= 16 && currentChar == '0') {
            char buffer[32];
            int len = snprintf(buffer, sizeof(buffer), "-%d-", count);
            write(write_fd, buffer, len);
        } else {
            for (int i = 0; i < count; i++) {
                write(write_fd, &currentChar, 1);
            }
        }
    }
}


// pipe reader and write file logic
void write_data(int read_fd, const char *dest_filename) {
    FILE *destination = fopen(dest_filename, "w");
    if (!destination) {
        perror("Parent: Error opening destination file");
        return;
    }

    char buffer[1024];
    ssize_t bytes_read;
    
    // read data from the pipe until EOF when child closes 
    while ((bytes_read = read(read_fd, buffer, sizeof(buffer))) > 0) {
        // write the data received from the pipe directly to the destination file
        fwrite(buffer, 1, bytes_read, destination);
    }

    fclose(destination);
}
