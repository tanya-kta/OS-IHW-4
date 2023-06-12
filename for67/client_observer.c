#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include "../clientInfo.h"


int main(int argc, char *argv[])
{
    int sock;                          /* Socket descriptor */
    struct sockaddr_in echo_serv_addr; /* Echo server address */
    unsigned short echo_serv_port;     /* Echo server port */
    char *serv_ip;                     /* Server IP address (dotted quad) */
    struct sockaddr_in from_addr;     /* Source address of echo */
    unsigned int from_size = sizeof(from_addr);
    char buffer[10010];
    int bytes_rcvd;                    /* Bytes read in single recv() */

    if ((argc < 3) || (argc > 4)) {
        fprintf(stderr, "Usage: %s <Server IP> [<Echo Port>]\n", argv[0]);
        exit(1);
    }

    serv_ip = argv[1];

    if (argc == 3) {
        echo_serv_port = atoi(argv[2]); /* Use given port, if any */
    } else {
        echo_serv_port = 7; /* 7 is the well-known port for the echo service */
    }

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        dieWithError("socket() failed");
    }

    /* Construct the server address structure */
    memset(&echo_serv_addr, 0, sizeof(echo_serv_addr));     /* Zero out structure */
    echo_serv_addr.sin_family      = AF_INET;             /* Internet address family */
    echo_serv_addr.sin_addr.s_addr = inet_addr(serv_ip);   /* Server IP address */
    echo_serv_addr.sin_port        = htons(echo_serv_port); /* Server port */
    char *st = "Connected observer";
    if (sendto(sock, st, strlen(st), 0, (struct sockaddr *)
               &echo_serv_addr, sizeof(echo_serv_addr)) != strlen(st)) {
        dieWithError("sendto() sent a different number of bytes than expected");
    }

    if ((bytes_rcvd = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                               (struct sockaddr *) &from_addr, &from_size)) < 0) {
        dieWithError("recvfrom() failed");
    }
    if (echo_serv_addr.sin_addr.s_addr != from_addr.sin_addr.s_addr) {
        dieWithError("Error: received a packet from unknown source.\n");
    }

    for(; bytes_rcvd > 0;) {
        if (bytes_rcvd == 1 && buffer[0] == '\0') {
            break;
        }
        printf("Received: ");
        buffer[bytes_rcvd] = '\0';
        printf("%s\n", buffer);
        sleep(1);
        if ((bytes_rcvd = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                                   (struct sockaddr *) &from_addr, &from_size)) < 0) {
            dieWithError("recvfrom() failed");
        }
        if (echo_serv_addr.sin_addr.s_addr != from_addr.sin_addr.s_addr) {
            dieWithError("Error: received a packet from unknown source.\n");
        }
    }

    close(sock);
    exit(0);
}
