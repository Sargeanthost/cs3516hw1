/*
** showip.c -- show IP addresses for a host given on the command line
*/
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <regex.h>

#define MAX_LINE_LEN 4096
#define MAX_ARGS 4

//Writes a request to the given socket, saves the response to a file, and returns the rtt in microseconds
uint32_t socketResponse2File(FILE *fp, int sd, char response_buffer[], size_t line_len) {
    struct tcp_info info;
    socklen_t tcp_info_length = sizeof(info);
    if (fp == NULL) {
        printf("Error opening file\n");
        exit(1);
    }

    //we dont care about -1 returning, just that the recieved and sent are different
    if ((size_t) write(sd, response_buffer, line_len) != line_len) {
        puts("write to socket did not complete. Please rerun the program");
        exit(1);
    }
    if(getsockopt(sd, SOL_TCP, TCP_INFO, &info, &tcp_info_length) == -1){
        puts("RTT could not be calculated. Something is wrong with the socket");
        return 0;
    }

    ssize_t n;
    memset(response_buffer, 0, MAX_LINE_LEN);
    puts("Writing response to file \"index.html\"");
    while ((n = read(sd, response_buffer, MAX_LINE_LEN - 1)) > 0) {
        fprintf(fp, "%s", response_buffer);
        memset(response_buffer, 0, MAX_LINE_LEN);
    }
    fclose(fp);
    if (n < 0) {
        puts("socket read error");
        exit(1);
    }
    close(sd);

    return info.tcpi_rtt;
}

//converts a domain to an ipv4 ip address
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

        // convert the IP to a string
        inet_ntop(temp->ai_family, addr, ip_string, sizeof(char) * INET_ADDRSTRLEN);
    }

    freeaddrinfo(results);
}

//split string in half base on delimiter. delimiter stays with second half
void split(char *input, char delimiter, char **first, char **second){
    char* delimiter_pos = strchr(input, delimiter);
    if (delimiter_pos == NULL) {
        *first = strdup(input);
        *second = NULL;
    } else {
        size_t delimiter_index = delimiter_pos - input;
        *first = strndup(input, delimiter_index);
        *second = strdup(input + delimiter_index);
    }
}

int is_ip_address(const char *domain) {
    regex_t regex;
    //found regex on the internet, tested on https://regexr.com/
    int ret = regcomp(&regex, "^((25[0-5]|(2[0-4]|1\\d|[1-9]|)\\d)\\.?\\b){4}$", REG_EXTENDED);
    if (ret != 0) {
        perror("Failed to compile regular expression\n");
        return 0;
    }

    int is_ip = 0;
    ret = regexec(&regex, domain, 0, NULL, 0);
    if (ret != REG_NOMATCH) {
        is_ip = 1;
    }

    regfree(&regex);
    return is_ip;
}

int main(int argc, char *argv[]) {
    char *url, *url_domain, *url_path, *port;
    int will_time = (argc == 4);

    if (argc < 3 || argc > 4) {
        fprintf(stderr, "usage: ./http_client [-temp] server_url port_number\n");
        return 1;
    }

    url = argv[MAX_ARGS - (MAX_ARGS - argc) - 2];
    port = argv[MAX_ARGS - (MAX_ARGS - argc) - 1];

    char temp_url[strlen(url)];
    strcpy(temp_url, url);
    split(temp_url, '/', &url_domain, &url_path);
    url_path = (url_path == NULL) ? "/" : url_path;

    char ip_string[INET_ADDRSTRLEN];

    if (is_ip_address(url_domain)) {
        strcpy(url_domain, ip_string);
    } else {
        domain2ip(url_domain, port, ip_string);
    }
    int socket_descriptor;

    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        puts("socket call error");
        return 3;
    }
    //connect socket to port
    int temp_connection_port = atoi(port);
    struct sockaddr_in website_sockaddr;

    memset(&website_sockaddr, 0, sizeof website_sockaddr);
    if (temp_connection_port == 0) {
        puts("Port conversion failed");
        return 4;
    }
    website_sockaddr.sin_port = htons(temp_connection_port);
    website_sockaddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip_string, &website_sockaddr.sin_addr) < 0) {
        puts("inet_pton fail");
        return 10;
    }

    if (connect(socket_descriptor, (struct sockaddr *) &website_sockaddr, sizeof(website_sockaddr)) != 0) {
        puts("connection failed");
        return 5;
    }

//    write to socket
    char send_line[MAX_LINE_LEN];
    size_t send_line_len;
    sprintf(send_line, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", url_path, url_domain);
    send_line_len = strlen(send_line);
    FILE *output_file;
    output_file = fopen("index.html", "w");

    printf("GETTING contents from %s...\n", url_domain);
    if (will_time) {
        printf("\nRTT is %u milliseconds\n",
               socketResponse2File(output_file, socket_descriptor, send_line, send_line_len) / 1000);
    } else {
        socketResponse2File(output_file, socket_descriptor, send_line, send_line_len);
    }
    return 0;
}
