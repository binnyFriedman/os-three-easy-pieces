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

int socket_fd;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

char *process_my_request(char *buffer, int fd)
{
    char *response = malloc(sizeof(char) * 512);
    char number_fd[10];
    struct timeval tv;
    sprintf(number_fd, "%d", fd);
    strcpy(response, "Coming from :");
    strcat(response, number_fd);
    strcat(response, "\r\n at: ");
    gettimeofday(&tv, NULL);
    strcat(response, (char *)ctime(&tv.tv_sec));
    strcat(response, "\r\n");
    strcat(response, buffer);
    return response;
}

int isRequestForNewConnection(int fd, int listener)
{
    return fd == listener;
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

void handle_request(char *buffer, int fd, int *fdmax)
{
    char *response = process_my_request(buffer, fd);
    for (int i = 0; i < *fdmax + 1; i++)
        if (i != fd)
            write(i, response, strlen(response));
    free(response);
}

void handle_new_connection(int fd, int *fdmax, fd_set *fds)
{
    int newsockfd = accept_connection(fd);
    FD_SET(newsockfd, fds);
    if (newsockfd > *fdmax)
    {
        *fdmax = newsockfd;
    }
}

void client_disconnected(int fd, fd_set *fds)
{
    printf("Client disconnected\n");
    FD_CLR(fd, fds);
    close(fd);
}

void handle_incoming_messages(int fd, int *fdmax, fd_set *fds)
{
    char buffer[256];
    bzero(buffer, 256);
    int read_status = read(fd, buffer, 255);
    if (read_status < 0)
        error("ERROR reading from socket");
    if (read_status == 0)
        client_disconnected(fd, fds);
    else
        handle_request(buffer, fd, fdmax);
}

void process_poll(int fd, int sockfd, int *fdmax, fd_set *read_fds, fd_set *fds)
{
    if (isRequestForNewConnection(fd, sockfd))
        handle_new_connection(fd, fdmax, fds);
    else
        handle_incoming_messages(fd, fdmax, fds);
}

int poll_loop(int socket_file_descriptor)
{
    fd_set fds, readfds;
    struct sockaddr_storage;
    int fdmax;
    fdmax = socket_file_descriptor;

    FD_ZERO(&fds);
    FD_SET(socket_file_descriptor, &fds);
    while (1)
    {
        readfds = fds;
        if (select(fdmax + 1, &readfds, NULL, NULL, NULL) == -1)
            error("select");

        for (int fd = 0; fd <= (fdmax + 1); fd++)
        {
            if (areIncomingMessages(fd, &readfds))
                process_poll(fd, socket_file_descriptor, &fdmax, &readfds, &fds);
        }
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