#include "process.h"

void acceptLoop(int socketFD);
int sendBuf(int socketFD, char **buf, int length);
int recvBuf(int socketFD, char **buf);
void* thread_fn(void *sockid);
void newThread(int socketFD);
void* metrics_fn(void *structure);
void startMetricsThread();

int main() {
    int socketFD;
    socketFD = setupServerTCPSocket(PORT);
    startMetricsThread();
    acceptLoop(socketFD);
    return 0;
}

void acceptLoop(int socketFD) {
    struct sockaddr_in cli_addr;
    int cliSocket;
    while(1) {
        cliSocket = acceptSocket(socketFD, &cli_addr);
        printf("cliSocket %d\n", cliSocket);
        newThread(cliSocket);
    }
}

int sendBuf(int socketFD, char *buf[MAXBUFFSIZE], int length) {
    printf("e\n");
    int count = send(socketFD, *buf, length, 0);
    if (count != length) {
        printf("f\n");
        return -1;
    }
    return 1;
}

int recvBuf(int socketFD, char *buf[MAXBUFFSIZE]) {
    ssize_t length = 0;
    ssize_t count = 0;
    //count = recv(socketFD, (char *) &length, sizeof(int), 0);
    //count = recv(socketFD, (char *) &readLength, sizeof(int), 0);
    //if (count != sizeof(int)) {
    //    printf("a\n");
    //    return -1;
    //}
    //printf("sizeof int: %d\n", sizeof(int));
    //printf("count: %d, length: %d\n", count);
    count = recv(socketFD, *buf, 5, 0);
    printf("count %d\n", count);
    if (count != 5) {
        printf("c\n");
        return -1;
    }
    printf("d\n");
    return length;

}

void* thread_fn(void *sockid) {
    int socketFD = (int) sockid;
    printf("cliSocket %d\n", socketFD);
    struct timeval recvBufferTime, sendBufferTime;
    char buf[MAXBUFFSIZE];
    newConnection();
    int length;
    while(1) {
        length = recvBuf(socketFD, &buf);
        if (length == -1) {
            droppedConnection();
            close(socketFD);
            return 0;
        }
        gettimeofday(&sendBufferTime,0);

        //printf("recieving\n");
        if(sendBuf(socketFD, &buf, length) == -1) {
            droppedConnection();
            close(socketFD);
            return 0;
        }
        gettimeofday(&recvBufferTime,0);
        setResponce(delay(sendBufferTime,recvBufferTime));
    }

    return 0;
}

void newThread(int socketFD) {
    int result;
    pthread_t *thread = malloc(sizeof (pthread_t));
    result = pthread_create(thread, 0, thread_fn, (void *) socketFD);
    if (result != 0) {
        perror("thread creation error\n");
        exit(1);
    }
}

void* metrics_fn(void *structure) {
    ++structure;
    metricLoop();
    return 0;
}

void startMetricsThread() {
    int result;
    void *blah = 0;
    pthread_t *thread = malloc(sizeof (pthread_t));
    result = pthread_create(thread, 0, metrics_fn, blah);
    if (result != 0) {
        perror("thread creation error\n");
        exit(1);
    }
}
