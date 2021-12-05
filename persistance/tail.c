#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
//a simple program to print the last couple of lines of a file
int main(int argc, char **argv)
{
    //check for correct number of arguments
    if (argc < 2)
    {
        printf("Usage: tail -n <num_lines> <file>\n");
        return 1;
    }
    //get the number of lines to print
    int lines = 10;
    char *pathname = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-n") == 0)
        {
            lines = atoi(argv[i + 1]);
            i++;
        }
        else
        {
            pathname = argv[i];
        }
    }
    if (pathname == NULL)
    {
        printf("Usage: tail -n <num_lines> <file>\n");
        return 1;
    }

    //open the file
    int fd = open(pathname, O_RDONLY);
    if (fd == -1)
    {
        perror("Error: could not open file:");
        return 1;
    }

    //check if the file is empty
    if (lseek(fd, 0, SEEK_END) == 0)
    {
        printf("File is empty\n");
        return 0;
    }
    //go through the file backwords and collect the lines characters
    int line_size = 80;
    lseek(fd, line_size * (-1), SEEK_END);
    char line[line_size];
    int line_count = 0;
    while (line_count < lines)
    {

        read(fd, line, sizeof(char) * line_size);
        for (int i = 0; i < line_size; i++)
        {
            if (line[i] == '\n')
            {
                line_count++;
                if (line_count == lines)
                {
                    lseek(fd, i + 1, SEEK_CUR);
                    break;
                }
            }
        }
        if (line_count < lines)
        {
            lseek(fd, line_size * (-1), SEEK_CUR);
        }
    }
    //print the lines
    int rc = 1;
    while (rc > 0)
    {
        rc = read(fd, line, sizeof(char) * line_size);
        printf("%s", line);
    }
    close(fd);

    return 0;
}