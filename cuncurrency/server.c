#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/syslimits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <aio.h>
#include <errno.h>

#define MAX_CLIENTS 150

enum operation_type
{
    READ,
    WRITE
};
struct disk_operation
{
    int connection_fd;
    char *file_buffer;
    struct aiocb *aiocb;
    enum operation_type type;
};

struct disk_operation *io_state_init(int connection_fd, struct aiocb *aiocb, char *buffer, enum operation_type type)
{
    struct disk_operation *state = malloc(sizeof(struct disk_operation));
    state->connection_fd = connection_fd;
    state->aiocb = aiocb;
    state->file_buffer = buffer;
    state->type = type;
    return state;
}

int socket_fd;
struct disk_operation *ongoing_disk_operations[MAX_CLIENTS];
fd_set fds, readfds;
int fdmax;

void add_disk_operation(struct disk_operation *op)
{
    int i;
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (ongoing_disk_operations[i] == NULL)
        {
            ongoing_disk_operations[i] = op;
            return;
        }
    }
}

void remove_disk_operation(struct disk_operation *op)
{
    int i;
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (ongoing_disk_operations[i] != NULL && ongoing_disk_operations[i] == op)
        {
            struct disk_operation *temp = ongoing_disk_operations[i];
            ongoing_disk_operations[i] = NULL;
            free((void *)temp->aiocb->aio_buf);
            free(temp->aiocb);
            free(temp);
            return;
        }
    }
}

void add_fd_to_event_loop(int fd)
{
    FD_SET(fd, &fds);
    if (fd > fdmax)
        fdmax = fd;
}
void error(const char *msg)
{
    perror(msg);
    exit(1);
}
char *ltrim(char *s)
{
    while (isspace(*s))
        s++;
    return s;
}

char *rtrim(char *s)
{
    char *back = s + strlen(s);
    while (isspace(*--back))
        ;
    *(back + 1) = '\0';
    return s;
}

char *trim(char *s)
{
    return rtrim(ltrim(s));
}

int getServerFileDescriptor(char *fileName)
{
    char *path;
    long size;
    int fd;
    size = pathconf(".", _PC_PATH_MAX);
    path = (char *)malloc(size);
    bzero(path, size);
    getcwd(path, (size_t)size);
    strcat(path, "/public_files/");
    strcat(path, fileName);
    path = trim(path);
    fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        perror("File open failed");
        return -1;
    }
    return fd;
}

char *cleanPath(char *path)
{
    char *newPath = (char *)malloc(strlen(path) + 1);
    path = trim(path);
    int i = 0;
    for (i = 0; i < strlen(path); i++)
    {
        if (path[i] == '.')
        {
            if (path[i + 1] == '.' && path[i + 2] == '/')
            {
                i = i + 2;
            }
            else if (path[i + 1] == '/')
            {
                i = i + 1;
            }
            else
            {
                newPath[i] = path[i];
            }
        }
        else
        {
            newPath[i] = path[i];
        }
    }
    newPath[i] = '\0';
    return newPath;
}

int isRequestForNewConnection(int fd)
{
    return fd == socket_fd;
}

int areIncomingMessages(int fd, fd_set *read_fds)
{
    return FD_ISSET(fd, read_fds);
}

int bind_socket(int port)
{
    int sockfd;
    struct sockaddr_in serv_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    return sockfd;
}

int accept_connection(int listener)
{
    int newsockfd;
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    newsockfd = accept(listener, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    return newsockfd;
}

struct aiocb *init_aiocb(int fd, volatile void *file_buffer, size_t file_size)
{
    struct aiocb *aiocbp = malloc(sizeof(struct aiocb));
    aiocbp->aio_fildes = fd;
    aiocbp->aio_offset = 0;
    aiocbp->aio_buf = file_buffer;
    aiocbp->aio_nbytes = file_size;
    return aiocbp;
}

int process_my_request(char *buffer, int connection_fd)
{
    int fd = getServerFileDescriptor(cleanPath(buffer));
    if (fd < 0)
        return -1;
    struct stat st;
    fstat(fd, &st);
    char *file_buffer = malloc(st.st_size);
    struct aiocb *aiocbp = init_aiocb(fd, file_buffer, st.st_size);
    aio_read(aiocbp);

    add_disk_operation(io_state_init(connection_fd, aiocbp, file_buffer, READ));
    return 0;
}

void handle_request(char *buffer, int fd)
{
    int status = process_my_request(buffer, fd);
    if (status == -1)
        write(fd, "404", 3);
}

void handle_new_connection(int fd)
{
    int newsockfd = accept_connection(fd);
    add_fd_to_event_loop(newsockfd);
}

void client_disconnected(int fd)
{
    printf("Client disconnected\n");
    FD_CLR(fd, &fds);
    close(fd);
}

void handle_incoming_messages(int fd)
{
    char buffer[256];
    bzero(buffer, 256);
    int read_status = read(fd, buffer, 255);
    if (read_status < 0)
        error("ERROR reading from socket");
    if (read_status == 0)
        client_disconnected(fd);
    else
        handle_request(buffer, fd);
}

void process_poll(int fd)
{
    if (isRequestForNewConnection(fd))
        handle_new_connection(fd);
    else
        handle_incoming_messages(fd);
}

void process_op_completed(struct disk_operation *d_op, int index)
{
    write(d_op->connection_fd, d_op->file_buffer, d_op->aiocb->aio_nbytes);
    remove_disk_operation(d_op);
}

void process_completed_disk_operations()
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (ongoing_disk_operations[i] != NULL)
        {
            int op_stat = aio_error(ongoing_disk_operations[i]->aiocb);
            if (op_stat == EINPROGRESS)
                continue;
            if (aio_return(ongoing_disk_operations[i]->aiocb) == -1)
            {
                perror("Error while reading or writing to disk");
                remove_disk_operation(ongoing_disk_operations[i]);
            }
            else
            {
                process_op_completed(ongoing_disk_operations[i], i);
            }
        }
    }
}

int poll_loop(int socket_file_descriptor)
{

    struct sockaddr_storage;

    fdmax = socket_file_descriptor;

    FD_ZERO(&fds);
    FD_SET(socket_file_descriptor, &fds);
    struct timeval tv = {0, 100};
    while (1)
    {
        readfds = fds;
        if (select(fdmax + 1, &readfds, NULL, NULL, &tv) == -1)
            error("select");

        for (int fd = 0; fd <= (fdmax + 1); fd++)
        {
            if (areIncomingMessages(fd, &readfds))
                process_poll(fd);
        }
        process_completed_disk_operations();
    }
}

int server_init(int port_number)
{
    socket_fd = bind_socket(port_number);
    listen(socket_fd, 5);
    poll_loop(socket_fd);
    close(socket_fd);
    return 0;
}

void sigint_handler(int sig)
{
    printf("\nCaught signal %d\n", sig);
    close(socket_fd);
    exit(0);
}

void catch_signals()
{
    signal(SIGINT, sigint_handler);
}

int main(int argc, char **argv)
{
    int port_number = atoi(argv[1]);
    catch_signals();
    server_init(port_number);
    return 0;
}