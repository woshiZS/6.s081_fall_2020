#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){
	// check arg amount
	if (argc != 1){
		fprintf(2, "usage: uptime.\n");
		exit(1);
	}
	printf("Uptime: %d\n", uptime());
	
	exit(0);
}
