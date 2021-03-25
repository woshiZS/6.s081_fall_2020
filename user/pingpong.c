#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    // swap a byte between parent and child, 0 for read and 1 for write.
    int p[2];
    pipe(p);
    char send = '1';
    char receive;
    if(fork() == 0){
        // read data and then feed back
        int readLen = read(p[0], &receive, 1);
        if(readLen != 1){
            printf("Child process read content failed\n");
        }
        int pid = getpid();
        printf("%d: received ping\n", pid);
        int writeLen = write(p[1], &send, 1);
        if(writeLen != 1){
            printf("Child process write error.\n");
        }
    }else{
        // parent process
        int writeLen = write(p[1], &send, 1);
        if(writeLen != 1){
            printf("Parent process write error.\n");
        }
        wait(0);
        int readLen = read(p[0], &send, 1);
        if(readLen != 1){
            printf("Parent read error.\n");
        }
        int pid = getpid();
        // need add space
        printf("%d: received pong\n", pid);
    }
    close(p[0]);
    close(p[1]);
    exit(0);
}