#include "selectServer.h"

/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		selectServer.c
--
--	PROGRAM:		Server Architecture Stress Tests: Select Test
--
--	FUNCTIONS:              int main()
--                              int sendBuf(int socketFD, char *buf[MAXBUFFSIZE], int length)
--                              int recvBuf(int socketFD, char **buf)
--                              void* thread_fn(void *sockid)
--                              void newThread(int socketFD)
--                              void* metrics_fn(void *structure)
--                              void startMetricsThread()
--                              int processJob(int socketFD)
--                              void selectLoop(int listen_sd)
--
--
--	DATE:			Febuary 26, 2011
--
--
--	DESIGNERS:		Warren Voelkl
--                              Aman Abdulla
--
--	PROGRAMMERS:		Warren Voelkl
--                              Aman Abdulla
--
--	NOTES:
--      Server program which uses a single thread and the select system call to monitor
--      activity on the server.
--
--      While performing its task the program records the following stats
--          - Average turnaround time in milliseconds
--          - Data recieved per second
--          - Maintained Socket connections
--          - Responces recieved per second
---------------------------------------------------------------------------------------*/

void selectLoop(int);
void* metrics_fn(void *structure);
void startMetricsThread();

#define PORT        8888
#define MAXBUFFSIZE 2048

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
    startMetricsThread();
    socketFD = setupServerTCPSocket(PORT);
    selectLoop(socketFD);
    return 1;
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
--      FUNCTION: int processJob(int socketFD)
--
--      RETURNS: void
--
--      NOTES:
--      updates metrics and calls the recieve and send functions on the socket
---------------------------------------------------------------------------------------*/
int processJob(int socketFD) {
    int length;
    struct timeval recvBufferTime, sendBufferTime;
    char *buf = malloc(MAXBUFFSIZE);
    length = recvBuf(socketFD, &buf);
    if (length == -1) {
        return 0;
    }
    gettimeofday(&sendBufferTime,0);
    if(sendBuf(socketFD, &buf, length) == -1) {
        return 0;
    }
    gettimeofday(&recvBufferTime,0);
    setResponce(delay(sendBufferTime,recvBufferTime));
    return 1;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION:   void selectLoop(int listen_sd)
--
--      RETURNS:    void
--
--      AUTHOR:     AMAN ABDULLA
--
--      REVISION    Feb 26, 2011 by Warren Voelkl
--                  Moved the read and write functions into their own functions
--
--      NOTES:
--      loops on the select function call
---------------------------------------------------------------------------------------*/
void selectLoop(int listen_sd) {
    fd_set rset, allset;
    int new_sd, sockfd, nready, maxfd, client[FD_SETSIZE], i, maxi;
    struct sockaddr_in client_addr;
    size_t n;

    maxfd	= listen_sd;	// initialize
    maxi	= -1;		// index into client[] array

    for (i = 0; i < FD_SETSIZE; i++) {
            client[i] = -1;             // -1 indicates available entry
    }
    FD_ZERO(&allset);
    FD_SET(listen_sd, &allset);

    while (1) {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listen_sd, &rset)) {
            new_sd = acceptSocket(listen_sd, &client_addr);
            for (i = 0; i < FD_SETSIZE; i++)
                if (client[i] < 0) {
                    client[i] = new_sd;
                    break;
                }
            if (i == FD_SETSIZE) {
                perror("Too many clients\n");
                exit(1);
            }
            newConnection();

            FD_SET (new_sd, &allset);
            if (new_sd > maxfd) {
                maxfd = new_sd;
            }

            if (i > maxi) {
                maxi = i;
            }

            if (--nready <= 0) {
                continue;
            }
        }

        for (i = 0; i <= maxi; i++) {
            if ((sockfd = client[i]) < 0) {
                continue;
            }

            if (FD_ISSET(sockfd, &rset)) {
                n = processJob(sockfd);

                if (n == 0) {
                    droppedConnection();
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                }

                if (--nready <= 0) {
                    break;        // no more readable descriptors
                }
            }
        }
    }
    return;
}
