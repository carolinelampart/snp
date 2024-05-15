#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "snp_error_handler.h"
#include "tcp_client.h"

/*
Diese Funktion erstellt und verbindet einen Socket zum Server und gibt den Socket zurück.
Verwenden Sie folgende Funktionen:
- getAddrInfo mit folgenden hints
	hints.ai_family = AF_INET;       // Nur IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP Stream Sockets
- socket 
- connect
*/
int client_connect(const char* ServerName, const char* PortNumber) {
    struct addrinfo hints, *res, *p;
    int sockfd;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // Only IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
 
    int status;
    if ((status = getaddrinfo(ServerName, PortNumber, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return -1;
    }
 
    // Loop through all the results and connect to the first we can
    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
 
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
 
        break; // if we get here, we must have connected successfully
    }
 
    if (p == NULL) {
        // Loop through all results; couldn't connect to any
        return -2;
    }
 
    freeaddrinfo(res); // all done with this structure
    return sockfd;
}


/*
Diese Funktion sendet einen Request an den Server.
buffer: in buffer steht der Inhalt
len: gibt die Anzahl zu sendende Bytes
*/
int receiveResponse(int communicationSocket, char* buffer, int len) {
    int total = 0;        // how many bytes we've received
    int bytesleft = len;  // how many we have left to receive
    int n;
 
    while(total < len) {
        n = recv(communicationSocket, buffer + total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
 
    if (n == -1) {
        perror("recv error");
        // handle errors appropriately
    }
    return (n == -1) ? -1 : total; // return -1 on failure, total on success
}

/*
Diese Funktion empfängt genau len Anzahl Bytes und speichert diese in buffer.
*/
void sendRequest(int communicationSocket, char* buffer, int len) {
    int total = 0;        // how many bytes we've sent
    int bytesleft = len;  // how many we have left to send
    int n;
 
    while(total < len) {
        n = send(communicationSocket, buffer + total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
 
    if (n == -1) {
        perror("send error");
        // handle errors appropriately
    }
}

