#include "process.h"

/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		process.c
--
--	PROGRAM:		Server Architecture Stress Tests: Process Test
--
--	FUNCTIONS:              int main()
--                              void acceptLoop(int socketFD)
--                              int sendBuf(int socketFD, char *buf[MAXBUFFSIZE], int length)
--                              int recvBuf(int socketFD, char **buf)
--                              void* thread_fn(void *sockid)
--                              void newThread(int socketFD)
--                              void* metrics_fn(void *structure)
--                              void startMetricsThread()
--
--
--	DATE:			Febuary 26, 2011
--
--
--	DESIGNERS:		Warren Voelkl
--
--	PROGRAMMERS:		Warren Voelkl
--
--	NOTES:
--      Server program which creates a new thread for each connected client
--
--      While performing its task the program records the following stats
--          - Average turnaround time in milliseconds
--          - Data recieved per second
--          - Maintained Socket connections
--          - Responces recieved per second
---------------------------------------------------------------------------------------*/

void acceptLoop(int socketFD);
int sendBuf(int socketFD, char **buf, int length);
int recvBuf(int socketFD, char **buf);
void* thread_fn(void *sockid);
void newThread(int socketFD);
void* metrics_fn(void *structure);
void startMetricsThread();

/*---------------------------------------------------------------------------------------
--      FUNCTION: int main()
--
--      RETURNS: void
--
--      NOTES:
--      Entry point of the program creastes the server socket and starts the different
--      threads
---------------------------------------------------------------------------------------*/
int main() {
    int socketFD;
    socketFD = setupServerTCPSocket(PORT);
    startMetricsThread();
    acceptLoop(socketFD);
    return 0;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: void acceptLoop(int socketFD)
--
--      RETURNS: void
--
--      NOTES:
--      loops the accept call
---------------------------------------------------------------------------------------*/
void acceptLoop(int socketFD) {
    struct sockaddr_in cli_addr;
    int cliSocket;
    while(1) {
        cliSocket = acceptSocket(socketFD, &cli_addr);
        newThread(cliSocket);
    }
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: int sendBuf(int socketFD, char *buf[MAXBUFFSIZE], int length)
--
--      RETURNS: void
--
--      NOTES:
--      sends over the socket
---------------------------------------------------------------------------------------*/
int sendBuf(int socketFD, char *buf[MAXBUFFSIZE], int length) {
    int count = send(socketFD, *buf, length, 0);
    if (count != length) {
        return -1;
    }
    return 1;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: int recvBuf(int socketFD, char **buf)
--
--      RETURNS: void
--
--      NOTES:
--      recieves data from socket
---------------------------------------------------------------------------------------*/
int recvBuf(int socketFD, char **buf) {
    int length = 0;
    ssize_t count = 0;
    count = recv(socketFD, (char *) &length, sizeof(int), 0);
    //count = recv(socketFD, (char *) &readLength, sizeof(int), 0);
    if (count != sizeof(int)) {
        return -1;
    }
    count = recv(socketFD, *buf, length, 0);
    if (count != length) {
        return -1;
    }
    return length;

}

/*---------------------------------------------------------------------------------------
--      FUNCTION: void* thread_fn(void *sockid)
--
--      RETURNS: void
--
--      NOTES:
--      loops through sending and receiving on a buffer
---------------------------------------------------------------------------------------*/
void* thread_fn(void *sockid) {
    int socketFD = (int) sockid;
    struct timeval recvBufferTime, sendBufferTime;
    char *buf = malloc(MAXBUFFSIZE);
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

/*---------------------------------------------------------------------------------------
--      FUNCTION: void newThread(int socketFD)
--
--      RETURNS: void
--
--      NOTES:
--      wrapper for pthread_create
---------------------------------------------------------------------------------------*/
void newThread(int socketFD) {
    int result;
    pthread_t *thread = malloc(sizeof (pthread_t));
    result = pthread_create(thread, 0, thread_fn, (void *) socketFD);
    if (result != 0) {
        perror("thread creation error\n");
        exit(1);
    }
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: void* metrics_fn(void *structure)
--
--      RETURNS: void
--
--      NOTES:
--      wrapper for metrics loop to make it compatible with pthread_create
---------------------------------------------------------------------------------------*/
void* metrics_fn(void *structure) {
    ++structure;
    metricLoop();
    return 0;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: void startMetricsThread()
--
--      RETURNS: void
--
--      NOTES:
--      wrapper to start the metrics thread
---------------------------------------------------------------------------------------*/
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
