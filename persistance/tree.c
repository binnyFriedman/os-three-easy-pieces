
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>

#ifndef MAX_BUF
#define MAX_BUF 1024
#endif

char *get_directory_name(int argc, char **argv)
{
    char *directory_name = malloc(MAX_BUF);
    bzero(directory_name, MAX_BUF);
    if (argc == 1)
    {
        strcpy(directory_name, ".");
    }
    else
    {
        strcpy(directory_name, argv[1]);
    }
    return directory_name;
}

void print_before_file_entry(int num_entries, int file_position, int indentation, char *name)
{

    for (int i = indentation; i >= 0; i--)
    {
        if (i == 0)
        {
            //this is the case where we need to print the tree entries.
            if (file_position == num_entries)
            {
                //we are at the end of the branch
                //print the last entry.
                if (indentation == 0)
                {
                    printf("└─── ");
                }
                else
                {
                    printf("  └─── ");
                }
            }
            else
            {
                //we are not at the end of the file.
                //print the entry.
                if (indentation == 0)
                {
                    printf("├─── ");
                }
                else
                {
                    printf("  ├─── ");
                }
            }
        }
        else if (i == indentation)
        {
            printf("│   ");
        }
        else
        {
            printf("     ");
        }
    }
    printf("%s\n", name);
}

void print_directory_contents(char *directory_name, int indentation)
{
    DIR *dir;
    struct dirent *ent;
    int num_entries = 0;
    // struct dirent *entries[] = {NULL};

    //loop through the directory to collect info first
    // create an array of entries and sort them.
    dir = opendir(directory_name);
    if (dir == NULL)
    {
        perror("opendir");
        exit(1);
    }
    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        num_entries++;
    }
    closedir(dir);

    if ((dir = opendir(directory_name)) != NULL)
    {
        int count = 0;
        while ((ent = readdir(dir)) != NULL)
        {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;

            count++;
            print_before_file_entry(num_entries, count, indentation, ent->d_name);

            if (ent->d_type == DT_DIR && strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
            {
                char *new_directory_name = malloc(MAX_BUF);
                bzero(new_directory_name, MAX_BUF);
                strcpy(new_directory_name, directory_name);
                strcat(new_directory_name, "/");
                strcat(new_directory_name, ent->d_name);
                print_directory_contents(new_directory_name, indentation + 1);
                free(new_directory_name);
            }
        }
        closedir(dir);
    }
    else
    {
        perror("");
        exit(EXIT_FAILURE);
    }
}
//a program to print the contents of the given directory recursively.
int main(int argc, char *argv[])
{
    char *dir_name = get_directory_name(argc, argv);
    printf("%s\n", dir_name);
    DIR *dir = opendir(dir_name);
    struct dirent *entry;
    if (dir == NULL)
    {
        perror("opendir");
        exit(1);
    }
    print_directory_contents(dir_name, 0);
    free(dir_name);
    return 0;
}