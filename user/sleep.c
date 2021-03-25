#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    switch (argc)
    {
    case 1:
        printf("Too less command arguments, you should specific a time.\n");
        break;
    case 2:
        {// must specify a postive number
        int sleepTime = atoi(argv[1]);
        sleep(sleepTime);
        }
        break;
    default:
        // too much argument
        printf("There are two much argument, sleep is currently  2 args supported.\n");
        break;
    }

    exit(0);
}