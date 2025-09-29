#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_COUNT_DIGITS 10 // Max digits to 10 so we can prevent buffer overflow

int main() {
    FILE *source, *destination;
    int character;

    // open the compressed file for reading
    source = fopen("compressed.txt", "r");
    // open the destination file for writing the decompressed data
    destination = fopen("decompressed.txt", "w");
    // error message if neither source.txt or compressed.txt are accessed
    if (!source || !destination) {
        printf("Error opening file.\n");
        return 1;
    }

    // read the file character by character
    while ((character = fgetc(source)) != EOF) {
       
        // check for the start of a compressed run code
        if (character == '+' || character == '-') {
            char run_char = (character == '+') ? '1' : '0';
           
            // look for the count number between the delimiters
            char count_str[MAX_COUNT_DIGITS + 1] = {0};
            int i = 0;

            // read the digits of the count
            while ((character = fgetc(source)) != EOF && isdigit(character) && i < MAX_COUNT_DIGITS) {
                count_str[i++] = character;
            }

            // check if the character that followed the digits is the closing delimiter
            if (character == '+' || character == '-') {
                if ((character == '+' && run_char == '1') || (character == '-' && run_char == '0')) {
                    int count = atoi(count_str);

                    // write the decompressed run
                    for (int j = 0; j < count; j++) {
                        fputc(run_char, destination);
                    }

                    // continue the loop to read the next character after the closing delimiter
                    continue;
                }
            }
           
            // if the sequence wasn't a valid run code (e.g., '+123x' or '++')
            // we write the characters we read back out as literals
           
            // first write the initial '+' or '-'
            fputc((run_char == '1' ? '+' : '-'), destination);
           
            // then write the digits that were read
            if (i > 0) {
                fprintf(destination, "%s", count_str);
            }
           
            // lastly write the character that broke the sequence (unless it was EOF)
            if (character != EOF) {
                 fputc(character, destination);
            }
           
            // we now go back to the top of the loop to read the next character
            // since the last character that broke the sequence (character)
            // has already been written to the destination we don't use 'continue' here
        } else {
            // for all other characters (uncompressed 0s, 1s, spaces, newlines)
            // write them directly to the destination file.
            fputc(character, destination);
        }
    }

    // closes the files
    fclose(source);
    fclose(destination);
   
    printf("Decompression complete. Output written to 'decompressed.txt'.\n");
    return 0;
}
