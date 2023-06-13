#include "../info.h"
#include <unistd.h>
#include <fcntl.h>


int main(int argc, char *argv[]) {
    unsigned short echo_serv_port;     /* Server port */
    struct sockaddr_in from_addr;     /* Source address of echo */
    unsigned int from_size = sizeof(from_addr);
    int bytes_rcvd;

    if (argc != 6) {
        fprintf(stderr, "Usage:  %s <Clients number> <Decoder File Path> <Input File Path> <Output File Path> <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }

    echo_serv_port = atoi(argv[5]);
    process_number = atoi(argv[1]);
    int in_file = open(argv[3], O_RDONLY, S_IRWXU);
    int out_file = open(argv[4], O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);

    createUdpServerSocket(echo_serv_port);
    getAllClients(argv[2]);

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
            printf("Send to client id %d message:\n", i);
            for (int j = 0; j < size; ++j) {
                printf("%d ", buffer[j]);
            }
            printf("\n");
            if (sendto(sock, buffer, sizeof(int32_t) * size, 0,
                       (struct sockaddr *) &clnt_adds[i], clnt_len) != sizeof(int32_t) * size) {
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
                    printf("Received from client id %d message:\n%s\n", index, decoded_part);
                    strcpy(&decoded[(MAX_INTS + 1) * index], decoded_part);
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
    buffer[0] = -1;
    for (int index = 0; index < process_number; ++index) {
        if (sendto(sock, buffer, 4, 0, (struct sockaddr *) &clnt_adds[index], clnt_len) != 4) {
            dieWithError("sendto() sent a different number of bytes than expected");
        }
    }
    printf("Finishing\n");
    close(in_file);
    close(out_file);
    close(sock);
}
