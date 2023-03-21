/*
** showip.c -- show IP addresses for a host given on the command line
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define MAX_LINE_LEN 4096

//creates a file called index.html. and writes the response of the get request to the supplied ip.
//void to_file(char** response){
//    //open file
//    //write contents
//    //make sure it ends with </body></html>, as this is what they check
//}
//
void domain2ip(char *domain, char *port, char ip_string[]) {
    struct addrinfo hints, *results, *temp;
    int status;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(domain, port, &hints, &results)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }

    for (temp = results; temp != NULL; temp = temp->ai_next) {
        void *addr;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (temp->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *) temp->ai_addr;
            addr = &(ipv4->sin_addr);
        }

        // convert the IP to a string and print it:
        inet_ntop(temp->ai_family, addr, ip_string, sizeof(char) * INET_ADDRSTRLEN);
    }

    freeaddrinfo(results); // fre
}

int main(int argc, char *argv[]) {
    char *website_name;
    char *connection_port;
    short will_time = 0;
    //if website_name matchs:
    // /^(?=.*[^\.]$)((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.?){4}$/
    //then you have ip address
//   ./http_client -temp google.com 80
    if (argc == 3) {
        website_name = argv[1];
        connection_port = argv[2];
    } else if (argc == 4) {
        website_name = argv[2];
        connection_port = argv[3];
        will_time = 1;
    } else {
        fprintf(stderr, "usage: ./http_client [-temp] server_url port_number\n");
        return 1;
    }

    char ip_string[INET_ADDRSTRLEN];

    domain2ip(website_name, connection_port, ip_string);

    int socket_descriptor;
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        puts("socket call error");
        return 3;
    }
    //connect socket to port
    int temp_connection_port = atoi(connection_port);
    struct sockaddr_in website_sockaddr;

    memset(&website_sockaddr, 0, sizeof website_sockaddr);
    if (temp_connection_port == 0) {
        puts("Port conversion failed");
        return 4;
    }
    website_sockaddr.sin_port = htons(temp_connection_port);
    website_sockaddr.sin_family = AF_INET;
    if(inet_pton(AF_INET,ip_string,&website_sockaddr.sin_addr) < 0){
        puts("inet_pton");
        return 99;
    }

    if (connect(socket_descriptor, (struct sockaddr *) &website_sockaddr, sizeof(website_sockaddr)) != 0) {
        puts("connection failed");
        return 5;
    }

//    write to socket
    char send_line[MAX_LINE_LEN];
    size_t send_line_len;
    char receive_line[MAX_LINE_LEN];
    sprintf(send_line, "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", website_name);
    send_line_len = strlen(send_line);

    time_t timer = clock();
    if (write(socket_descriptor, send_line, send_line_len) != send_line_len) {
        puts("write did not complete");
        return 6;
    }

    //receive from socket
    memset(receive_line, 0, MAX_LINE_LEN);

    ssize_t n;
    //freezing on google, https://www.ibm.com/docs/en/zos/2.1.0?topic=functions-read-read-from-file-socket
    while ((n = read(socket_descriptor, receive_line, MAX_LINE_LEN - 1)) > 0) {
        printf("%s", receive_line);
        memset(receive_line, 0, MAX_LINE_LEN);
    }
    if (n < 0) {
        puts("socket read error");
        return 7;
    }

    if(will_time){
        printf("RTT is %l\n", clock() - timer);
    }
    close(socket_descriptor);
    exit(0);
    //TODO gettimeofday() for rtt
}