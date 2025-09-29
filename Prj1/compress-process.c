/*
This code will help compress a textfile using process. The main process helps create
the process. The logic helps to compress the lines of 1s and 0s in each chunk. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(){
    //helps to open the source file
    FILE *source = fopen("source.txt", "r"); 
    fseeko(source, 0, SEEK_END); 
    off_t size = ftello(source); 
    fclose(source);

    //helps create the chunks and set the number of processes
    int n = 4; 
    off_t chunk = (size + n - 1) / n; 
    pid_t p[n];

    //helps to create the processes
    for(int i = 0; i < n; i++){
        off_t off = i *chunk; 
        siz0e_t len = (i == n - 1) ? size - off : chunk;
        char tmp[16]; 
        sprintf(tmp,"t%d",i);

        if((p[i] = fork()) == 0){
            FILE *in = fopen("source.txt","r"), *out=fopen(tmp, "w"); 
            fseeko(in, off, SEEK_SET);
            int c = fgetc(in), x, count = 1;
            size_t done = 1;
            while(done < len && (x = fgetc(in)) != EOF){
                done++;
                if(x == c)
                {
                    count++;
                }
                //contains the logic to help compress the chunk
                else{

                    if(count >= 16 && c == '1')
                        fprintf(out, "+%d+", count);
                    else if(count >= 16 && c == '0')
                        fprintf(out, "-%d-", count);
                    else while(count--)
                        fputc(c,out);
                    if(x == ' ' || x == '\n')
                    {
                        fputc(x,out);
                        c = fgetc(in);
                        done++;
                        count = 1;
                    }
                    else{c=x; count = 1;}
                }
            }
            if(count >= 16 && c == '1')
                fprintf(out, "+%d+" , count);
            else if(count >= 16 && c == '0')
                fprintf(out, "-%d-", count);
            else while(count--)
                fputc(c, out);
            fclose(in);
            fclose(out);
            exit(0);
        }
    }

    //helps wait for the process to end
    for(int i = 0; i < n; i++)
        waitpid(p[i], 0, 0);

    //writes to the new compressed file
    FILE *out = fopen("compressed.txt", "w"), *t; 
    char buf[4096]; 
    size_t r;

    for(int i = 0; i < n; i++)
    {
        char tmp[16];sprintf(tmp,"t%d",i);
        t = fopen(tmp, "r");
        while((r = fread(buf, 1, sizeof buf, t)) > 0)
            fwrite(buf,1,r,out);
            fclose(t);
            remove(tmp);
    }
    fclose(out);
}