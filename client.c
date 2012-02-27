#include "client.h"
#include "metrics.h"
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		client.c
--
--	PROGRAM:		Server Architecture Stress Tests: Client
--
--	FUNCTIONS:              int main(int argc, char * argv[])
--                              void* metrics_fn(void *structure)
--                              void startMetricsThread()
--                              void newThread()
--                              void sendBuf(int socketFD)
--                              void recBuf(int socketFD)
--                              void* thread_fn(void *structure)
--
--	DATE:			Febuary 26, 2011
--
--
--	DESIGNERS:		Warren Voelkl
--
--	PROGRAMMERS:		Warren Voelkl
--
--	NOTES:
--	Creates a multithreaded client one thread per socket connection.
--      It connects to a server specified from the commandline then procededs
--      to spawn a user specified amount of threads wich send a user specified
--      length string to the server and then waits for a responce.
--
--      While performing its task the program records the following stats
--          - Average turnaround time in milliseconds
--          - Data recieved per second
--          - Maintained Socket connections
--          - Responces recieved per second
---------------------------------------------------------------------------------------*/


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

/*---------------------------------------------------------------------------------------
--	FUNCTION: int main(int argc, char * argv[])
--
--      RETURNS: 0 on success
--               1 on invalid command line arguments
--
--	NOTES:
--      Entry point into the program parses commandline output
---------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------
--	FUNCTION: void* metrics_fn(void *structure)
--
--      RETURNS: 0
--
--	NOTES:
--      used as a function point in creating the thread which outputs the metric data.
---------------------------------------------------------------------------------------*/
void* metrics_fn(void *structure) {
    ++structure;
    metricLoop();
    return 0;
}

/*---------------------------------------------------------------------------------------
--	FUNCTION: void startMetricsThread()
--
--      RETURNS: 0
--
--	NOTES:
--      creates a new thread which will handle the metric data of the program
---------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------
--	FUNCTION: void newThread()
--
--      RETURNS: 0
--
--	NOTES:
--      creates a new thread which will connect to a server
---------------------------------------------------------------------------------------*/
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

/*---------------------------------------------------------------------------------------
--	FUNCTION: void sendBuf(int socketFD)
--
--      RETURNS: 0
--
--	NOTES:
--      Sends a variable length string as defined by the command line arguments to the
--      server.
---------------------------------------------------------------------------------------*/
void sendBuf(int socketFD) {
    int count = send(socketFD, ts.sndbuf, ts.length + sizeof(int), 0);
    if (count != ts.length + sizeof(int)) {
        printf("send mismatch data sent: %d\n", count);
        return;
    }
}

/*---------------------------------------------------------------------------------------
--	FUNCTION: void recBuf(int socketFD)
--
--      RETURNS: 0
--
--	NOTES:
--      Recieves the echoed string back from the server and updates the metric thread.
---------------------------------------------------------------------------------------*/
void recBuf(int socketFD) {
    char rcBuffer[ts.length];
    int count = recv(socketFD, rcBuffer, ts.length, 0);
    if (count != ts.length) {
        printf("send mismatch data sent: %d\n", count);
        return;
    }
    setData(count);
}

/*---------------------------------------------------------------------------------------
--	FUNCTION: void* thread_fn(void *structure)
--
--      RETURNS: 0
--
--	NOTES:
--      The thread loop which connects to the server and initiates sending and recieving
--      calls from the server.  In addition function updates the metrics on the delay
--      between sending the data request and recieving the reply.
---------------------------------------------------------------------------------------*/
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
