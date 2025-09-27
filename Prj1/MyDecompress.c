#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_COUNT_DIGITS 10 // Max digits for an int count (e.g., 2 billion is 10 digits)

int main() {
    FILE *source, *destination;
    int character;

    // Open the compressed file for reading
    source = fopen("compressed.txt", "r");
    // Open the destination file for writing the decompressed data
    destination = fopen("decompressed.txt", "w");

    if (!source || !destination) {
        printf("Error opening file.\n");
        return 1;
    }

    // Read the file character by character
    while ((character = fgetc(source)) != EOF) {
       
        // 1. Check for the start of a compressed run code
        if (character == '+' || character == '-') {
            char run_char = (character == '+') ? '1' : '0';
           
            // Look for the count number between the delimiters
            char count_str[MAX_COUNT_DIGITS + 1] = {0};
            int i = 0;

            // Read the digits of the count
            while ((character = fgetc(source)) != EOF && isdigit(character) && i < MAX_COUNT_DIGITS) {
                count_str[i++] = character;
            }

            // Check if the character that followed the digits is the closing delimiter
            if (character == '+' || character == '-') {
                if ((character == '+' && run_char == '1') || (character == '-' && run_char == '0')) {
                    // Successfully read a valid run code: +<count>+ or -<count>-
                    int count = atoi(count_str);

                    // Write the decompressed run
                    for (int j = 0; j < count; j++) {
                        fputc(run_char, destination);
                    }

                    // Continue the loop to read the next character after the closing delimiter
                    continue;
                }
            }
           
            // 2. If the sequence wasn't a valid run code (e.g., '+123x' or '++'),
            //    we must write the characters we read back out as literals.
           
            // First, write the initial '+' or '-'
            fputc((run_char == '1' ? '+' : '-'), destination);
           
            // Then, write the digits that were read
            if (i > 0) {
                fprintf(destination, "%s", count_str);
            }
           
            // Finally, write the character that broke the sequence (unless it was EOF)
            if (character != EOF) {
                 fputc(character, destination);
            }
           
            // We now go back to the top of the loop to read the next character.
            // Since the last character that broke the sequence (character)
            // has already been written to the destination, we don't use 'continue' here.
        } else {
            // 3. For all other characters (uncompressed 0s, 1s, spaces, newlines, etc.),
            //    write them directly to the destination file.
            fputc(character, destination);
        }
    }

    // Close the files
    fclose(source);
    fclose(destination);
   
    printf("Decompression complete. Output written to 'decompressed.txt'.\n");
    return 0;
}