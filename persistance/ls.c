#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>

#ifndef MAX_BUF
#define MAX_BUF 200
#endif
//a program that lists all the files in a directory.
//options are: -l prints full information about the file
char *join_relative_path(const char *relative_path)
{
    char *absolute_path = malloc(MAX_BUF);
    bzero(absolute_path, MAX_BUF);
    getcwd(absolute_path, MAX_BUF);
    strcat(absolute_path, "/");
    strcat(absolute_path, relative_path);
    return absolute_path;
}

char *getUser(uid_t uid)
{
    struct passwd *pws;
    pws = getpwuid(uid);
    return pws->pw_name;
}

char *getGroup(gid_t gid)
{
    struct group *grp;
    grp = getgrgid(gid);
    return grp->gr_name;
}

void print_file_info(struct stat *s)
{

    printf("%llu\t%d\t", s->st_ino, s->st_dev);
    printf("%s", (s->st_mode & S_IRUSR) ? "r" : "-");
    printf("%s", (s->st_mode & S_IWUSR) ? "w" : "-");
    printf("%s", (s->st_mode & S_IXUSR) ? "x" : "-");
    printf("%s", (s->st_mode & S_IRGRP) ? "r" : "-");
    printf("%s", (s->st_mode & S_IWGRP) ? "w" : "-");
    printf("%s", (s->st_mode & S_IXGRP) ? "x" : "-");
    printf("%s", (s->st_mode & S_IROTH) ? "r" : "-");
    printf("%s", (s->st_mode & S_IWOTH) ? "w" : "-");
    printf("%s\t", (s->st_mode & S_IXOTH) ? "x" : "-");
    printf("%llu\t", s->st_size);
    printf("%llu\t", s->st_blocks);
    printf("%hu\t", s->st_nlink);
    printf("%s\t", s->st_uid == -1 ? "?" : getUser(s->st_uid));
    printf("%s\t", s->st_gid == -1 ? "?" : getGroup(s->st_gid));
    char a_time[50], m_time[50], c_time[50];
    strftime(a_time, 50, "%Y-%m-%d %H:%M:%S", localtime(&s->st_atime));
    strftime(m_time, 50, "%Y-%m-%d %H:%M:%S", localtime(&s->st_mtime));
    printf("%s\t", a_time);
    printf("%s\t", m_time);
}

int main(int argc, char **argv)
{
    int i, print_file_info_op = -1;
    char *dirname = NULL;
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-l") == 0)
        {
            print_file_info_op = 1;
        }
        else
        {
            dirname = argv[i];
        }
    }
    if (dirname == NULL)
        dirname = ".";
    else
        dirname = join_relative_path(dirname);

    DIR *dir = opendir(dirname);

    if (dir == NULL)
    {
        perror("opendir");
        return 1;
    }
    struct dirent *entry;
    // read each file in the directory
    while ((entry = readdir(dir)) != NULL)
    {
        if (print_file_info_op == 1)
        {
            struct stat file_info;
            char file_name[MAX_BUF];
            strcpy(file_name, dirname);
            strcat(file_name, "/");
            strcat(file_name, entry->d_name);
            if (stat(file_name, &file_info) == -1)
            {
                perror("stat");
                return 1;
            }
            print_file_info(&file_info);
            printf("%s\n", entry->d_name);
        }
        else
        {
            printf("%s\n", entry->d_name);
        }
    }
}