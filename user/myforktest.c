#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc, char** argv){
    int pid = fork();
    if(pid < 0){
        fprintf(2, "fork error.\n");
        exit(1);
    }
    if(pid == 0){
        printf("This is child process.\n");
    }
    else{
        wait(0);
        printf("This is parent process.\n");
    }
    exit(0);
}