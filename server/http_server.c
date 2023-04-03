//#include <signal.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <sys/socket.h>
//
//int sd;
//
//void sigterm_handler(int signum) {
//    printf("Received SIGTERM signal\n");
//
//    // Close any open sockets
//    if (sd != -1) {
//        close(sd);
//        sd = -1;
//    }
//
//    // Perform any other necessary cleanup here
//
//    // Exit the program
//    exit(EXIT_SUCCESS);
//}
//
//int main() {
//    // Register the signal handler function for SIGTERM
//    signal(SIGTERM, sigterm_handler);
//
//    // Open a socket (just an example)
//    sd = socket(AF_INET, SOCK_STREAM, 0);
//    if (sd == -1) {
//        perror("socket");
//        exit(EXIT_FAILURE);
//    }
//
//    // Do some other work here
//
//    // Wait for the SIGTERM signal
//    while (1) {
//        sleep(1);
//    }
//
//    return 0;
//}

//================================================
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define _GNU_SOURCE
#define BUFFER_SIZE 1024

void handle_request(int client_sock, char *request_buffer, char *root_dir) {
    // Parse the HTTP request
    char method[BUFFER_SIZE], path[BUFFER_SIZE], protocol[BUFFER_SIZE];
    //GET / HTTP/1.1
    sscanf(request_buffer, "%s %s %s", method, path, protocol);

    if (strcasecmp(method, "GET") != 0) {
        dprintf(client_sock, "HTTP/1.1 405 Method Not Allowed\r\n\r\n");
        return;
    }

    // Build the path to the requested file. Root dir should be followed by an os file separate
    char full_path[BUFFER_SIZE];
    sprintf(full_path, "%s%s", root_dir, path);

    // Try to open the requested file
    int fd = open(full_path, O_RDONLY);
    if (fd == -1) {
        dprintf(client_sock, "HTTP/1.1 404 Not Found\r\n\r\n");
        return;
    }

    // Send the contents of the file to the client
    dprintf(client_sock, "HTTP/1.1 200 OK\r\n\r\n");
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        write(client_sock, buffer, bytes_read);
    }

    // Close the file and the socket
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

    // Get the current working directory to host on the port
    char root_dir[BUFFER_SIZE];
    if (getcwd(root_dir, BUFFER_SIZE) == NULL) {
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
        //handle sigterm
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
        }

        // Handle the HTTP request
        handle_request(client_sock, request_buffer, root_dir);
    }

// Close the server socket
    close(server_sock);

    return 0;
}