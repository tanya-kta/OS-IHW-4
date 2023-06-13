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


void dieWithError(char *error_message) {
    perror(error_message);
    exit(1);
}

char getCodedLetter(int *decoder, int code) {
    for (int i = 0; i < 26; ++i) {
        if (decoder[i] == code) {
            return 'a' + i;
        }
    }
    return 0;
}
