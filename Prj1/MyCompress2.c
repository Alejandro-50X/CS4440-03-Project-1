#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

/*
    This is doing the same task as MyCompress but the difference in this is that it now takes arguments so we don't have to hard code the source and destination. 
*/
int main(int argc, char *argv[]) {
    FILE *source, *destination;
    int character;

    // Check for the correct number of arguments (program name + source file + destination file)
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_file> <destination_file>\n", argv[0]);
        return 1;
    }

    // argv[1] is the source file name ("source.txt")
    // argv[2] is the destination file name ("compressed.txt")
    source = fopen(argv[1], "r");
    destination = fopen(argv[2], "w");

    if (!source || !destination) {
        perror("Error opening file"); // Using perror to show specific file error
        return 1;
    }

    int count = 1;
    // reads the first character and if it's EOF then the file is empty and closes it 
    int first_char_int = fgetc(source);
    if (first_char_int == EOF) {
        fclose(source);
        fclose(destination); 
        return 0; // empty file
    }
    char currentChar = (char)first_char_int; // sets the currentChar to the first character found in the file

    // Readsa the next character
    character = fgetc(source);
    // keeps the main compression loop going while there are still characters to read
    while (character != EOF) {
        if (character == currentChar) {
            // same run if the new character and current character are the same
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

            // write the space or newline
            fputc(character, destination);

            count = 1;
            currentChar = fgetc(source); // Read the character after the space or newline
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

            // starts a new run
            count = 1;
            currentChar = character;
        }

        character = fgetc(source);
    }

    // flush the final run
    if (currentChar != EOF) { // Ensures that there is actually something to flush
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
