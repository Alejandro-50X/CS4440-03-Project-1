#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

// Function containing the compression logic
void compress_data(FILE *source_file, int write_fd);
// Function containing the write-to-file logic
void write_data(int read_fd, const char *dest_filename);

// --- MAIN FUNCTION ---
int main(int argc, char *argv[]) {
    int pipe_fd[2]; // pipe_fd[0] is for reading, pipe_fd[1] is for writing
    pid_t pid;
    
    // 1. Check for correct command-line arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_file> <destination_file>\n", argv[0]);
        return 1;
    }

    const char *source_filename = argv[1];
    const char *dest_filename = argv[2];

    // 2. Create the pipe
    if (pipe(pipe_fd) == -1) {
        perror("pipe failed");
        return 1;
    }

    // 3. Fork a new process
    pid = fork();

    if (pid < 0) {
        // Fork failed
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // --- CHILD PROCESS (The Compressor/Pipe Writer) ---
        
        // Close the read end of the pipe (we only write)
        close(pipe_fd[0]);
        
        // Open the source file
        FILE *source = fopen(source_filename, "r");
        if (!source) {
            perror("Child: Error opening source file");
            close(pipe_fd[1]); // Close the write end before exiting
            exit(1);
        }

        // Run the compression logic, writing compressed data to pipe_fd[1]
        compress_data(source, pipe_fd[1]);

        // Clean up
        fclose(source);
        close(pipe_fd[1]); // Close the write end to signal EOF to the reader (parent)
        exit(0);
    } else {
        // --- PARENT PROCESS (The Pipe Reader/Destination Writer) ---
        
        // Close the write end of the pipe (we only read)
        close(pipe_fd[1]);

        // Run the write logic, reading compressed data from pipe_fd[0]
        write_data(pipe_fd[0], dest_filename);

        // Clean up
        close(pipe_fd[0]); // Close the read end
        
        // Wait for the child process to finish
        wait(NULL); 
        printf("Compression complete: '%s' -> '%s' via pipe.\n", source_filename, dest_filename);
    }

    return 0;
}

// --- COMPRESSION LOGIC (Modified to write to a File Descriptor) ---
void compress_data(FILE *source_file, int write_fd) {
    int count = 1;
    char currentChar;
    int character_int;
    
    // Get the first character
    character_int = fgetc(source_file);
    if (character_int == EOF) {
        return; // Empty file
    }
    currentChar = (char)character_int;

    // Get the second character
    while ((character_int = fgetc(source_file)) != EOF) {
        char character = (char)character_int;
        
        if (character == currentChar) {
            // Same run
            count++;
        }
        else {
            // New character, flush previous run
            
            // --- Flush Logic ---
            if (count >= 16 && currentChar == '1') {
                char buffer[32];
                // Use snprintf to format the string
                int len = snprintf(buffer, sizeof(buffer), "+%d+", count);
                // Write the string to the pipe
                write(write_fd, buffer, len); 
            } else if (count >= 16 && currentChar == '0') {
                char buffer[32];
                int len = snprintf(buffer, sizeof(buffer), "-%d-", count);
                write(write_fd, buffer, len);
            } else {
                // Write uncompressed run
                for (int i = 0; i < count; i++) {
                    write(write_fd, &currentChar, 1);
                }
            }

            // --- Handle Run Breaker ---
            if (character == ' ' || character == '\n') {
                // Write the space/newline delimiter to the pipe
                write(write_fd, &character, 1);
                
                // Reset to read the character after the delimiter
                character_int = fgetc(source_file);
                if (character_int == EOF) break; 
                currentChar = (char)character_int;
                count = 1;
            } else {
                // Start new run with the new character
                currentChar = character;
                count = 1;
            }
        }
    }

    // Flush the final run (End of File reached)
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


// --- PIPE READER/FILE WRITER LOGIC ---
void write_data(int read_fd, const char *dest_filename) {
    FILE *destination = fopen(dest_filename, "w");
    if (!destination) {
        perror("Parent: Error opening destination file");
        return;
    }

    char buffer[1024];
    ssize_t bytes_read;
    
    // Read data from the pipe until EOF (when child closes write_fd)
    while ((bytes_read = read(read_fd, buffer, sizeof(buffer))) > 0) {
        // Write the data received from the pipe directly to the destination file
        fwrite(buffer, 1, bytes_read, destination);
    }

    fclose(destination);
}