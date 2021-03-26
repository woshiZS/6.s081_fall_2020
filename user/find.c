#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *cur_dir, char *file_name){
    // read all file
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(cur_dir, 0)) < 0){
        fprintf(2, "find: command can not open current directory.\n");
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat current directory.\n");
        return;
    }

    if(strlen(cur_dir) + 1 + DIRSIZ + 1 > sizeof(buf)){
        printf("Current directory is too long.\n");
        return;
    }
    strcpy(buf, cur_dir);
    p = buf + strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0){
            printf("find cannot stat %s.\n", buf);
            continue;
        }
        switch (st.type)
        {
        case T_FILE:
            // constract tail file name 
            if(strcmp(de.name, file_name) == 0){
                printf("%s\n", buf);
            }
            break;
        case T_DIR:
            find(buf, file_name);
            break;

        default:
            break;
        }
    }
}


int main(int argc, char *argv[]){
    if(argc < 3){
        fprintf(2, "You should specify at least 3 args!\n");
        exit(1);
    }
    for(int i = 2; i < argc; ++i){
        find(argv[1] , argv[i]);
    }
    exit(0);
}