#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAX_BUF
#define MAX_BUF 200
#endif
//a simple program to get the stats of a file or directory
char *join_relative_path(const char *relative_path)
{
    char *absolute_path = malloc(MAX_BUF);
    bzero(absolute_path, MAX_BUF);
    getcwd(absolute_path, MAX_BUF);
    strcat(absolute_path, "/");
    strcat(absolute_path, relative_path);
    return absolute_path;
}
int main(int argc, char **arg)
{
    if (argc != 2)
    {
        printf("Usage: stat <file>\n");
        return 1;
    }
    char *absolute_path = join_relative_path(arg[1]);
    struct stat s;
    //join the path of the file with the current directory

    if (stat(absolute_path, &s) == -1)
    {
        perror("stat");
        return 1;
    }
    printf("%s\n", arg[1]);
    printf("Size: %lld\n", s.st_size);
    printf("Blocks: %lld\n", s.st_blocks);
    printf("Links: %d\n", s.st_nlink);
    printf("User: %d\n", s.st_uid);
    printf("Group: %d\n", s.st_gid);
    printf("Device: %d\n", s.st_dev);
    printf("Inode: %llu\n", s.st_ino);
    printf("Access: %s\n", ctime(&s.st_atime));
    printf("Modify: %s\n", ctime(&s.st_mtime));
    printf("Change: %s\n", ctime(&s.st_ctime));
    return 0;
}