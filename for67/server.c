#include "../info.h"
#include <unistd.h>
#include <fcntl.h>


int main(int argc, char *argv[]) {
    unsigned short echo_serv_port;     /* Server port */
    struct sockaddr_in from_addr;     /* Source address of echo */
    unsigned int from_size = sizeof(from_addr);
    int bytes_rcvd;

    if (argc != 5) {
        fprintf(stderr, "Usage:  %s <Clients number> <Input File Path> <Output File Path> <Server Port>\n", argv[0]);
        exit(1);
    }

    echo_serv_port = atoi(argv[4]);
    int in_file = open(argv[2], O_RDONLY, S_IRWXU);
    int out_file = open(argv[3], O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);
    process_number = atoi(argv[1]);

    createUdpServerSocket(echo_serv_port);
    getAllClients();
    getObserver();

    char message[10000];
    int32_t buffer[MAX_INTS];
    char decoded_part[MAX_INTS + 1];
    char decoded[(MAX_INTS + 1) * process_number];
    int status = 1;
    while (status == 1) {
        int num_of_running = 0;
        for (int i = 0; i < process_number; ++i, ++num_of_running) {
            int size = 0;
            for (; size < MAX_INTS; ++size) {
                status = readInt(in_file, &buffer[size]);
                if (status == -1) {
                    break;
                }
            }
            if (size == 0) {
                break;
            }
            sprintf(message, "Send to decoder client id %d message to decode:\n", i);
            for (int j = 0; j < size; ++j) {
                sprintf(message + strlen(message), "%d ", buffer[j]);
            }
            sprintf(message + strlen(message), "\n");
            printf("%s", message);
            if (sendto(sock, buffer, sizeof(int32_t) * size, 0,
                       (struct sockaddr *) &clnt_adds[i], clnt_len) != sizeof(int32_t) * size) {
                dieWithError("sendto() sent a different number of bytes than expected");
            }
            if (sendto(sock, message, strlen(message), 0,
                       (struct sockaddr *) &obs_addr, obs_len) != strlen(message)) {
                dieWithError("sendto() sent a different number of bytes than expected");
            }
        }
        sleep(2);
        for (int i = 0; i < num_of_running; ++i) {
            if ((bytes_rcvd = recvfrom(sock, decoded_part, MAX_INTS, 0,
                                       (struct sockaddr *) &from_addr, &from_size)) < 0) {
                dieWithError("recvfrom() failed");
            }
            decoded_part[bytes_rcvd] = '\0';
            for (int index = 0; index < process_number; ++index) {
                if (clnt_adds[index].sin_addr.s_addr == from_addr.sin_addr.s_addr &&
                    clnt_adds[index].sin_port == from_addr.sin_port) {
                    sprintf(message, "Received from client id %d message:\n%s\n", index, decoded_part);
                    printf("%s", message);
                    strcpy(&decoded[(MAX_INTS + 1) * index], decoded_part);
                    if (sendto(sock, message, strlen(message), 0,
                               (struct sockaddr *) &obs_addr, obs_len) != strlen(message)) {
                        dieWithError("sendto() sent a different number of bytes than expected");
                    }
                }
            }
        }
        for (int i = 0; i < num_of_running; ++i) {
            int size = strlen(&decoded[(MAX_INTS + 1) * i]);
            if (size == 0) {
                break;
            }
            write(out_file, &decoded[(MAX_INTS + 1) * i], size);
        }
    }
    sprintf(message, "Finishing\n");
    printf("%s", message);
    if (sendto(sock, message, strlen(message), 0,
               (struct sockaddr *) &obs_addr, obs_len) != strlen(message)) {
        dieWithError("sendto() sent a different number of bytes than expected");
    }
    buffer[0] = -1;
    for (int index = 0; index < process_number; ++index) {
        if (sendto(sock, buffer, 4, 0, (struct sockaddr *) &clnt_adds[index], clnt_len) != 4) {
            dieWithError("sendto() sent a different number of bytes than expected");
        }
    }
    message[0] = '\0';
    if (sendto(sock, message, 1, 0, (struct sockaddr *) &obs_addr, obs_len) != 1) {
        dieWithError("sendto() sent a different number of bytes than expected");
    }
    close(in_file);
    close(out_file);
    close(sock);    /* Close socket */
}
