#include "client.h"
#include "metrics.h"
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


#define PORT 8888

typedef struct {
    int length;
    int repetitions;
    int delay;
    struct sockaddr_in server;
    char *sndbuf;
} THREADSTRUCT, *PTHREADSTRUCT;

void newThread();
void* thread_fn(void *structure);
void startMetricsThread();
void* metrics_fn(void *structure);

THREADSTRUCT ts;

int main(int argc, char * argv[]) {
    int i, connections;
    struct hostent *hp;

    if (argc != 6) {
        printf("error c.out <connections> <repetitions> <string length> <delay> <host>\n");
        return 1;
    }
    startMetricsThread();
    connections = atoi(argv[1]);
    ts.repetitions = atoi(argv[2]);
    ts.length = atoi(argv[3]);
    ts.delay = atoi(argv[4]);
    ts.sndbuf = malloc(ts.length);
    memcpy(ts.sndbuf, &ts.length,sizeof(int));
    for (i = 0; i != ts.length; ++i) {
        ts.sndbuf[i + sizeof(int)] = 'a';
    }


    bzero((char *)&ts.server, sizeof(struct sockaddr_in));
    ts.server.sin_family = AF_INET;
    ts.server.sin_port = htons(PORT);
    if ((hp = gethostbyname(argv[5])) == NULL) {
        perror("Unknown server address\n");
        exit(1);
    }
    bcopy(hp->h_addr, (char *)&ts.server.sin_addr, hp->h_length);


    for (i = 0; i != connections; ++i) {
        newThread();
    }
    sleep(ts.delay + 10);
    return 0;
}


void* metrics_fn(void *structure) {
    ++structure;
    metricLoop();
    return 0;
}

void startMetricsThread() {
    int result;
    void *blah = 0;
    //printf("role: %d\n", role);
    pthread_t *thread = malloc(sizeof (pthread_t));
    result = pthread_create(thread, 0, metrics_fn, blah);
    if (result != 0) {
        perror("thread creation error\n");
        exit(1);
    }
}

void newThread() {
    int result;
    void *blah = 0;
    //printf("role: %d\n", role);
    pthread_t *thread = malloc(sizeof (pthread_t));
    result = pthread_create(thread, 0, thread_fn, blah);
    if (result != 0) {
        perror("thread creation error\n");
        exit(1);
    }
}

void sendBuf(int socketFD) {
    int count = send(socketFD, ts.sndbuf, ts.length + sizeof(int), 0);
    if (count != ts.length + sizeof(int)) {
        printf("send mismatch data sent: %d\n", count);
        return;
    }
}

void recBuf(int socketFD) {
    char rcBuffer[ts.length];
    int count = recv(socketFD, rcBuffer, ts.length, 0);
    if (count != ts.length) {
        printf("send mismatch data sent: %d\n", count);
        return;
    }
    setData(count);
}


void* thread_fn(void *structure) {
    int socketFD, i;
    ++structure;
    socketFD = createSocket();
    struct timeval recvBufferTime, sendBufferTime;
    if (connect (socketFD, (struct sockaddr *)&ts.server, sizeof(ts.server)) == -1) {
        perror("connect\n");
        exit(1);
    }
    newConnection();
    for (i = 0; i != ts.repetitions; ++i) {
        //printf("sending\n");

        sendBuf(socketFD);
        gettimeofday(&sendBufferTime,0);

        //printf("recieving\n");
        recBuf(socketFD);
        gettimeofday(&recvBufferTime,0);
        setResponce(delay(sendBufferTime,recvBufferTime));
    }
    sleep(ts.delay);
    printf("thread complete\n");
    close(socketFD);
    return 0;
}
