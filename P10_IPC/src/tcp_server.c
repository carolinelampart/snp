#include "tcp_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>
#include <unistd.h>


#include "snp_error_handler.h"
#include "stdbool.h"


static int listeningSocket = 0;
static int connectedSocket = 0; 

void server_init(char* portNumber) {
    struct addrinfo hints, *res;
    int status;
 
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
 
    if ((status = getaddrinfo(NULL, portNumber, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
 
    listeningSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (listeningSocket == -1) {
        perror("socket");
        exit(1);
    }
 
    int yes = 1;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
 
    if (bind(listeningSocket, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        exit(1);
    }
 
    if (listen(listeningSocket, 10) == -1) { // can queue up to 10 clients
        perror("listen");
        exit(1);
    }
 
    freeaddrinfo(res); // free the linked list
}

int getRequest(char* requestBuffer, int max_len) {
    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);
    int new_fd = accept(listeningSocket, (struct sockaddr *)&their_addr, &addr_size);
    if (new_fd == -1) {
        perror("accept");
        return -1; // return -1 on accept failure
    }
 
    int numbytes = recv(new_fd, requestBuffer, max_len, 0);
    if (numbytes == -1) {
        perror("recv");
        close(new_fd);
        return -1;
    }
 
    // Close the new socket after receiving the request
    close(new_fd);
    return numbytes; // return the number of bytes received
}

void sendResponse(char* response, int resp_len) {
    int total = 0;
    while (total < resp_len) {
        int bytes_sent = send(connectedSocket, response + total, resp_len - total, 0);
        if (bytes_sent == -1) {
            perror("send");
            break;
        }
        total += bytes_sent;
    }
}

void server_close_connection(void) {
    if (listeningSocket) {
        close(listeningSocket);
        listeningSocket = 0;
    }
}

