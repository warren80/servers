#ifndef EPOLLSERVER_H
#define EPOLLSERVER_H

//#include "common.h"

#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>


#include "sock.h"
#include "metrics.h"

#define TRUE        1
#define FALSE       0
#define MAXEVENTS   64
#define BINDER      0
#define WORKER      1
#define METRIC      2
#define KEY         (996687)
#define WORKERS     3
#define MAXBUFFSIZE 2048
#define PORT        8888

typedef void* (*fnPtr)(void *);


typedef struct {
    int threadType;
    void * payload;
    int socketFD;
    int epollFD;
} THREADSETUPSTRUCT, *PTHREADSETUPSTRUCT;

typedef struct{
    int id;
    int jobcount;
    int jobs;
    int activeJobs;
    int epollFD;
    int socketFD;
    struct epoll_event *events;
    struct sembuf decrementGetJob[1];
    struct sembuf incrementGetJob[1];
    struct sembuf decrementProtectJobNum[1];
    struct sembuf incrementProtectJobNum[1];
    struct sembuf lockEpoll[1];
    struct sembuf unLockEpoll[1];
} WORKERSTRUCT, *PWORKERSTRUCT;


void workerLoop(PWORKERSTRUCT semStruct);
void blockSignals();






/**
 *Starts the server from running
 *@author Warren Voelkl
 */
void startServer();
/**
 * Any tasks that may want to be performed upon new client connecting.
 * currently empty.
 * @author Warren Voelkl
 */
void newConnectionTasks(int socketFD);
/**
 * Reads all data currently on socket and then sends it to the fnpointer
 * TODO limit to reading length of packet.
 * @author Warren Voelkl
 * @return 1 on success don't believe it can do anything else may as well return void
 */
int readDataFromSocket(int socketFD);
/**
 * Makes the socket Non blocking
 * @author Warren Voelkl
 */
//void makeNonBlockingSocket (int socketDescriptor);  //now in sock.c
/**
 * Used to fill the addrinfo struct and validate success of call.
 * @author Warren Voelkl
 * @return -1 on failure 0 on success
 */
int getAddressResult(int port, struct addrinfo **result);
/**
 * Binds socket and checks for success of operation
 * @author Warren Voelkl
 * @return -1 on failure the socket file descriptor on success
 */
int createAndBind(int port);
/**
 * Accepts all the incoming new sockets and sets them as nonblocking.
 * @author Warren Voelkl
 */
void processIncomingNewSocket(int socketFD, int epollFD);

#ifndef MUTLIREAD
/**
 * Main loop for this server.  Blocks until activity on a monitored socket
 * then it delets/adds and reads from sockets.  Upon a request to read from a socket
 * this function calls the functionPointer that was passed to it to process data.
 * @author Warren Voelkl
 */
void eventLoop(PWORKERSTRUCT semStruct);
#endif
/**
 * Checks to see if socket was bound properly
 * @author Warren Voelkl
 * @return -1 on failure or the socket file descriptor on success
 */
int validateSocket(int port);
/**
 * binds the server socket and sets it to be listened to.
 * @author Warren Voelkl
 */
void bindandListenSocket(int socketFD);
/**
 * sets the events to monitor on the epoll socket
 * @author Warren Voelkl
 */
void setEPollSocket(int epollFD, int socketFD, struct epoll_event **pevents);
/**
 * creates an epoll file descriptor
 * @author Warren Voelkl
 * @return only on failure.
 */
int createEPoll();

#endif
