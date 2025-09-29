/*
This code will help compress a textfile using threads. The code contains a function named my_compress
what contains the logic to compress the lines of 1s and 0s. The main function helps to 
create the threads.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

char *infile = "source.txt";
off_t size;
int numberOfProcesses = 4;

//contains code that helps compress 1s and 0s
void* my_compress(void *arg) {
    int id = *(int*)arg;
    //helps create the size of the chunk
    off_t chunk = (size + numberOfProcesses - 1) / numberOfProcesses;
    off_t off = id * chunk;
    size_t len = (id == numberOfProcesses - 1) ? size - off : chunk;

    char tmp[16]; 
    sprintf(tmp, "t%d", id);
    FILE * in = fopen(infile, "r"), *out = fopen(tmp, "w");
    fseeko(in, off, SEEK_SET);

    int c = fgetc(in), x, count = 1; 
    size_t done = 1;

    //goes throught the chunk
    while(done < len && (x = fgetc(in)) != EOF){ 
        done++;
        //helps to count the occurances
        if(x == c)
        {
            count++;
        }
        //handles if it changes to different characters such as and spaces and enter key
        else {
            if(count >= 16 && c == '1')
            {
            fprintf(out, "+%d+", count);

            }
            else if(count >= 16 && c == '0')
            {
                fprintf(out, "-%d-", count);
            }
            else while(count--)
            {
                fputc(c,out);
            }
            if(x == ' '|| x == '\n')
            { 
                fputc(x, out); 
                c = fgetc(in); 
                done++; 
                count = 1; 
            }
            else 
            { 
                c = x; 
                count = 1; 
            }
        }
    }


    if(count >= 16 && c == '1')
    {
        fprintf(out, "+%d+", count);
    }

    else if(count >= 16 && c == '0')
    {
        fprintf(out, "-%d-", count);
    }

    else while(count--)
    {
        fputc(c,out);
    }

    //close file
    fclose(in);
    fclose(out);
    return NULL;
}

int main() {
    //reads from input file
    FILE *f = fopen(infile, "r");
    fseeko(f, 0, SEEK_END); 
    size = ftello(f); 
    fclose(f);

    //helps create process and also their ids
    pthread_t T[numberOfProcesses];
    int ids[numberOfProcesses];

    for(int i = 0; i < numberOfProcesses; i++){
        ids[i] = i;
        int rc = pthread_create(&T[i], NULL, my_compress, &ids[i]);
        if(rc)
        { 
            printf("ERROR creating thread %d\n", i); exit(-1); 
        }
    }

    //join the processes
    for(int i = 0; i < numberOfProcesses; i++)
    {
        pthread_join(T[i], NULL);
    }

    //results get merged with new file
    FILE *out = fopen("compressed.txt","w"), *t; 
    char buf[4096]; 
    size_t r;

    for(int i = 0; i < numberOfProcesses; i++)
    {
        char tmp[16]; 
        sprintf(tmp, "t%d", i);
        t = fopen(tmp, "r");
        while((r = fread(buf,1,sizeof buf, t)) > 0)
        {
        fwrite(buf, 1, r, out);
        }

        fclose(t); 
        remove(tmp);
    }

    fclose(out);
}