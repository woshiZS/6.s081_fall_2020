#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAXN 1024

int main(int argc, char *argv[]){
     if(argc < 2){
         fprintf(2, "xargs: at least 2 args\n");
         exit(1);
     }

     // read former args, real_args can contain at most MAXARGS parameters
     char * real_args[MAXARG];
     for(int i = 1; i < argc; ++i){
         real_args[i - 1] = argv[i];
     }
     // need to read from standard input 
     char buf[MAXN];
     int status = 1; // represent read state from the standard input
     while(status){
         int real_pos =  argc - 1;
         int cur_arg_start = 0;
         int buf_pos = 0;
         char c;
         while(1){
             if(read(0, &c, 1) == 0){
                // end of the buffer
                exit(0);
             }
             if(c == ' ' || c == '\n'){
                 buf[buf_pos++] = 0;
                 real_args[real_pos++] = &buf[cur_arg_start];
                 cur_arg_start = buf_pos;
                 if(c == '\n'){
                     break;
                 }
             }
             else{
                 buf[buf_pos++] = c;
             }
         }
         // need to execute
         real_args[real_pos] = 0; // exec need to judge 0 as the end of cmd.
         if(fork() == 0){
             exec(real_args[0], real_args);
         }
         else{
             wait(0);
         }
     }
     exit(0);
}