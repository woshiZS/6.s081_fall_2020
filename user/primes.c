#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// parent process push and child process read and print first number.

void childProcess(int* p){
    close(p[1]);
    int prime;
    if(read(p[0], &prime, 4) != 4){
        printf("Process %d read error.\n", getpid());
        exit(1);
    }
    printf("prime %d\n", prime);
    int n;
    if(read(p[0], &n, 4)){
        int newpipe[2];
        pipe(newpipe);
        if(fork() > 0){
            close(newpipe[0]);
            if(n%prime)
                write(newpipe[1], &n, 4);
            while(read(p[0], &n, 4) == 4){
                if(n%prime)
                    write(newpipe[1], &n, 4);
            }
            close(p[0]);
            close(newpipe[1]);
            wait(0);
        }
        else{
            childProcess(newpipe);
        }
    }
}

int main(int argc, char *argv[]){
    // The main theory is print one prime each process
    int p[2];
    pipe(p);
    if(fork() > 0){
        // parent process
        close(p[0]);
        for(int i = 2; i <= 35; ++i){
            if(write(p[1], &i, 4) != 4){
                printf("Process %d write number %d.\n", getpid(), i);
                exit(1);
            }
        }
        close(p[1]);
        wait(0);
    }else{
        childProcess(p);
    }
    exit(0);
}