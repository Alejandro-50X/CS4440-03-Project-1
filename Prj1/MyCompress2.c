#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

// Note: This is your original compression logic wrapped in a main function
// that accepts command-line arguments (argc and argv)
int main(int argc, char *argv[]) {
    FILE *source, *destination;
    int character;

    // Check for the correct number of arguments (program name + source file + destination file)
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_file> <destination_file>\n", argv[0]);
        return 1;
    }

    // argv[1] is the source file name (e.g., "source.txt")
    // argv[2] is the destination file name (e.g., "compressed.txt")
    source = fopen(argv[1], "r");
    destination = fopen(argv[2], "w");

    if (!source || !destination) {
        perror("Error opening file"); // Use perror to show specific file error
        return 1;
    }

    int count = 1;
    // Use fgetc for initialization to handle the first character correctly
    int first_char_int = fgetc(source);
    if (first_char_int == EOF) {
        fclose(source);
        fclose(destination);
        return 0; // empty file
    }
    char currentChar = (char)first_char_int;

    // Read the next character
    character = fgetc(source);

    while (character != EOF) {
        if (character == currentChar) {
            // same run
            count++;
        }
        else if (character == ' ' || character == '\n') {
            // flush previous run
            if (count >= 16 && currentChar == '1') {
                fprintf(destination, "+%d+", count);
            } else if (count >= 16 && currentChar == '0') {
                fprintf(destination, "-%d-", count);
            } else {
                for (int i = 0; i < count; i++) {
                    fputc(currentChar, destination);
                }
            }

            // write the space/newline
            fputc(character, destination);

            // The original logic had complex reading here. Simpler to just reset:
            // reset
            count = 1;
            currentChar = fgetc(source); // Read the character after the space/newline
            if (currentChar == EOF) break; // Check if we hit EOF right after the delimiter
            character = fgetc(source); // Read the character *after* the new currentChar
            continue; // Go back to the while loop start
        }
        else {
            // flush previous run
            if (count >= 16 && currentChar == '1') {
                fprintf(destination, "+%d+", count);
            } else if (count >= 16 && currentChar == '0') {
                fprintf(destination, "-%d-", count);
            } else {
                for (int i = 0; i < count; i++) {
                    fputc(currentChar, destination);
                }
            }

            // start new run
            count = 1;
            currentChar = character;
        }

        character = fgetc(source);
    }

    // flush the final run
    if (currentChar != EOF) { // Ensure there was actually something to flush
        if (count >= 16 && currentChar == '1') {
            fprintf(destination, "+%d+", count);
        } else if (count >= 16 && currentChar == '0') {
            fprintf(destination, "-%d-", count);
        } else {
            for (int i = 0; i < count; i++) {
                fputc(currentChar, destination);
            }
        }
    }

    fclose(source);
    fclose(destination);
   
    // Indicate successful compression
    return 0;
}