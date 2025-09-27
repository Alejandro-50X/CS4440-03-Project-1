#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *source, *destination;
    int character;

    source = fopen("source.txt", "r");
    destination = fopen("compressed.txt", "w");

    if (!source || !destination) {
        printf("Error opening file.\n");
        return 1;
    }

    int count = 1;
    char currentChar = fgetc(source);   // first char
    if (currentChar == EOF) {
        fclose(source);
        fclose(destination);
        return 0; // empty file
    }

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

            // reset
            count = 1;
            currentChar = fgetc(source);
            character = fgetc(source);
            continue;
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
    if (count >= 16 && currentChar == '1') {
        fprintf(destination, "+%d+", count);
    } else if (count >= 16 && currentChar == '0') {
        fprintf(destination, "-%d-", count);
    } else {
        for (int i = 0; i < count; i++) {
            fputc(currentChar, destination);
        }
    }

    fclose(source);
    fclose(destination);
    return 0;
}