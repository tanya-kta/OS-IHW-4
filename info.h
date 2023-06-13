#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <fcntl.h>
#include <sys/mman.h>

#define MAX_INTS        30    // максимальная длина текстового сообщения


int process_number;
int sock;
struct sockaddr_in echo_serv_addr; /* Local address */
struct sockaddr_in obs_addr; /* Local address */
unsigned int clnt_len = sizeof(struct sockaddr_in);
unsigned int obs_len = sizeof(struct sockaddr_in);
struct sockaddr_in clnt_adds[10];

void dieWithError(char *error_message) {
    perror(error_message);
    exit(1);
}

void createUdpServerSocket(unsigned short port) {
    /* Create socket for incoming connections */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        dieWithError("socket() failed");
    }

    /* Construct local address structure */
    memset(&echo_serv_addr, 0, sizeof(echo_serv_addr));   /* Zero out structure */
    echo_serv_addr.sin_family = AF_INET;                /* Internet address family */
    echo_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echo_serv_addr.sin_port = htons(port);              /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &echo_serv_addr, sizeof(echo_serv_addr)) < 0) {
        dieWithError("bind() failed");
    }
}

void getDecoder(int *decoder, char *filename) {
    int file = open(filename, O_RDONLY, S_IRWXU);
    char letter;
    char line[13];
    int code;
    for (int i = 0; i < 26; ++i) {
        for (int ind = 0; ind < 12; ++ind) {
            int num = read(file, &line[ind], sizeof(char));
            if (num == 0 || line[ind] == '\n' || line[ind] == '\0') {
                line[ind] = '\0';
                break;
            }
        }
        sscanf(line, "%c %d", &letter, &code);
        decoder[letter - 'a'] = code;
    }
}

void getAllClients(char *filename) {
    int decoder[26];
    getDecoder(decoder, filename);
    for (int i = 0; i < process_number; ++i) {
        int msg_size;
        char buffer[30];
        if ((msg_size = recvfrom(sock, buffer, 29, 0,
                                 (struct sockaddr *) &clnt_adds[i], &clnt_len)) < 0) {
            dieWithError("recvfrom() failed");
        }
        if (sendto(sock, decoder, sizeof(decoder), 0,
                   (struct sockaddr *) &clnt_adds[i], clnt_len) != sizeof(decoder)) {
            dieWithError("sendto() sent a different number of bytes than expected");
        }
        buffer[msg_size] = '\0';
        printf("Handling decoder client %s with message %s\n", inet_ntoa(clnt_adds[i].sin_addr), buffer);
    }
}

void getObserver() {
    int msg_size;
    char buffer[30];
    if ((msg_size = recvfrom(sock, buffer, 29, 0,
                             (struct sockaddr *) &obs_addr, &obs_len)) < 0) {
        dieWithError("recvfrom() failed");
    }
    buffer[msg_size] = '\0';
    printf("Handling observer client %s with message %s\n", inet_ntoa(obs_addr.sin_addr), buffer);
}

int readInt(int file, int *p) {
    char line[10];
    for (int ind = 0; ind < 10; ++ind) {
        int num = read(file, &line[ind], sizeof(char));
        if (num == 0 || line[ind] == '\n' || line[ind] == ' ') {
            line[ind] = '\0';
            break;
        }
    }
    if (strlen(line) == 0) {
        return -1;
    }
    sscanf(line, "%d", p);
    return 1;
}
