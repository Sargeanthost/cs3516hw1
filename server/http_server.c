#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024

void handle_request(int client_sock, char *request_buffer, char *root_dir) {
    // Parse the HTTP request. will have "/" by default. However, for the homework this will be "/TMDG.html"
    char method[BUFFER_SIZE], path[BUFFER_SIZE], protocol[BUFFER_SIZE];
    //GET /TMDG.html HTTP/1.1
    sscanf(request_buffer, "%s %s %s", method, path, protocol);

    if (strcasecmp(method, "GET") != 0) {
        dprintf(client_sock, "HTTP/1.1 405 Method Not Allowed\r\n\r\n");
        return;
    }

    // Build the path to the requested file.
    char full_path[BUFFER_SIZE];
    sprintf(full_path, "%s%s", root_dir, path);
    printf("Requested file is: %s\n", full_path);

    // Try to open the requested file
    int fd = open(full_path, O_RDONLY);
    if (fd == -1) {
        dprintf(client_sock, "HTTP/1.1 404 Not Found\r\n\r\n");
        return;
    }
    struct stat file_stat;
    fstat(fd, &file_stat);

    // Num bytes of file
    __off_t size = file_stat.st_size;

    // Send the contents of the file to the client
    dprintf(client_sock, "HTTP/1.1 200 OK\r\nContent-length: %ld\r\n\r\n",size);
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        write(client_sock, buffer, bytes_read);
    }

    // Close the file and the socket. This ends the connection and the client wont hang.
    close(fd);
    close(client_sock);
}

int main(int argc, char *argv[]) {
    // Parse the command line arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);

    // Get the current working directory to host on the port for the socket
    char root_dir[BUFFER_SIZE];
    if (getcwd(root_dir, BUFFER_SIZE-2) == NULL) {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr.s_addr = INADDR_ANY
    };
    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // 5 max connections
    if (listen(server_sock, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Accept and handle incoming connections
    while (1) {
        //sigterm handles the closing of sockets automatically
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_sock == -1) {
            perror("accept");
            continue;
        }

        // should only be getting a simple get request in the header
        char request_buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(client_sock, request_buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("read");
            close(client_sock);
            continue;
        } else {
            printf("Request received is: %s\n", request_buffer);
        }

        handle_request(client_sock, request_buffer, root_dir);
    }
}