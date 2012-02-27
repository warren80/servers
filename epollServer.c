#include "epollServer.h"

void semOperation(int id, struct sembuf *operation);
PWORKERSTRUCT getSemStruct();
void setupSemaphoreStruct(PWORKERSTRUCT ss);
void newThread(void *role);
void* thread_fn(void *structure);
void processJob(int jobId, int epollFD, int socketFD);

struct epoll_event *events; //made global protected via semaphores



PWORKERSTRUCT getSemStruct() {
    PWORKERSTRUCT ss = (PWORKERSTRUCT) malloc(sizeof(WORKERSTRUCT));

    ss->id = semget(KEY, 4, 0666 | IPC_CREAT);
    if(ss->id < 0) {
        perror("semget\n");
    }

    //sem 0 is counting semephore
    ss->decrementGetJob[0].sem_num = 0;
    ss->decrementGetJob[0].sem_op = -1;
    ss->decrementGetJob[0].sem_flg = 0;

    //sem_op needs to be modified every usage
    ss->incrementGetJob[0].sem_num = 0;
    ss->incrementGetJob[0].sem_op = 0;
    ss->incrementGetJob[0].sem_flg = 0;

    //these are all binary semaphores (mutexes)
    ss->decrementProtectJobNum[0].sem_num = 1;
    ss->decrementProtectJobNum[0].sem_op = -1;
    ss->decrementProtectJobNum[0].sem_flg = 0;

    ss->incrementProtectJobNum[0].sem_num = 1;
    ss->incrementProtectJobNum[0].sem_op = 1;
    ss->incrementProtectJobNum[0].sem_flg = 0;


    ss->lockEpoll[0].sem_num = 2;
    ss->lockEpoll[0].sem_op = -1;
    ss->lockEpoll[0].sem_flg = 0;

    ss->unLockEpoll[0].sem_num = 2;
    ss->unLockEpoll[0].sem_op = 1;
    ss->unLockEpoll[0].sem_flg = 0;

    ss->jobcount = 0;
    return ss;
}

void setupSemaphoreStruct(PWORKERSTRUCT ss) {
    union semun {
        int val;
        struct semid_ds *buf;
        ushort * array;
    } argument;
    argument.val = 0;
    if( semctl(ss->id, 0, SETVAL, argument) < 0) {
        perror("semctl\n");
    }
    argument.val = 1;
    if(semctl(ss->id,1, SETVAL, argument) < 0) {
        perror("semctl\n");
    }
    if(semctl(ss->id,2, SETVAL, argument) < 0) {
        perror("semctl\n");
    }
}

void newThread(void *role) {
    int result;
    //printf("role: %d\n", role);
    pthread_t *thread = malloc(sizeof (pthread_t));
    result = pthread_create(thread, 0, thread_fn, (void *) role);
    if (result != 0) {
        perror("thread creation error\n");
        exit(1);
    }
}

void blockSignals() {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0) {
        perror("pthread_sigmask error\n");
    }
}






void* thread_fn(void *structure) {
    PTHREADSETUPSTRUCT tss = (PTHREADSETUPSTRUCT) structure;
    //printf("role: %d\n", tss->threadType);
    switch (tss->threadType) {
    case WORKER:
        workerLoop(tss->payload);
        //printf("worker thread: %d\n", (int) pthread_self());
        break;
    case METRIC:
        //printf("metric thread: %d\n", (int) pthread_self());
        metricLoop(tss->payload);
        break;
    }

    return 0;
}

void processJob(int jobId, int epollFD, int socketFD) {
    if ((events[jobId].events & EPOLLERR) ||
            (events[jobId].events & EPOLLHUP) ||
            (!(events[jobId].events & EPOLLIN))) {
        close (events[jobId].data.fd);
        droppedConnection();
        return;
    } else if (socketFD == events[jobId].data.fd) {
        processIncomingNewSocket(socketFD, epollFD);
        return;
    } else {
        readDataFromSocket(events[jobId].data.fd);

    }
}

void eventLoop(PWORKERSTRUCT semStruct) {
    int jobQueue;
    while (TRUE) {
        semOperation(semStruct->id, semStruct->lockEpoll);
        jobQueue = epoll_wait (semStruct->epollFD, events, MAXEVENTS, -1);
        semStruct->incrementGetJob[0].sem_op = jobQueue;
        semStruct->jobs = jobQueue;
        semStruct->jobcount = 0;
        semOperation(semStruct->id, semStruct->incrementGetJob); //starts the workerthreads
    }
    free (events);//never reached
}

void workerLoop(PWORKERSTRUCT semStruct) {
    int socketFD = semStruct->socketFD;
    int epollFD = semStruct->epollFD;
    int jobcount;
    int id = semStruct->id;
    //static int counter = 0;
    while (1) {
        semOperation(id, semStruct->decrementGetJob);
        semOperation(id, semStruct->decrementProtectJobNum);
        jobcount = semStruct->jobcount++;
        ++semStruct->activeJobs;
        semOperation(id, semStruct->incrementProtectJobNum);
        //printf("counter: %d, jobcount: %d\n", ++counter, jobcount);

        processJob(jobcount, epollFD, socketFD);
        semOperation(id, semStruct->decrementProtectJobNum);
        --semStruct->activeJobs;
        if (semStruct->jobcount == semStruct->jobs && semStruct->activeJobs == 0) {
            semOperation(id, semStruct->unLockEpoll);
        }
        semOperation(id, semStruct->incrementProtectJobNum);
    }
}


void startThreads(PWORKERSTRUCT semStruct) {
    int i;
    PTHREADSETUPSTRUCT ptss = (PTHREADSETUPSTRUCT) malloc(sizeof(THREADSETUPSTRUCT));
    ptss->threadType = METRIC;
    ptss->payload = semStruct;
    newThread(ptss);
    ptss = (PTHREADSETUPSTRUCT) malloc(sizeof(THREADSETUPSTRUCT));
    ptss->payload = semStruct;
    ptss->threadType = WORKER;
    for(i = 0; i != WORKERS; ++i) {
        newThread(ptss);
    }
}

int parseClientRequest(int socketFD, char * buffer, int length) {
    return 0;
}






//probably done
void startServer() {
    PWORKERSTRUCT semStruct = getSemStruct();
    setupSemaphoreStruct(semStruct);
    //struct epoll_event *events;
    int socketFD;
    int epollFD;
    socketFD = validateSocket(PORT);
    bindandListenSocket(socketFD);
    epollFD = createEPoll();
    setEPollSocket(epollFD, socketFD, &events);

    semStruct->socketFD = socketFD;
    semStruct->epollFD = epollFD;

    startThreads(semStruct);
    eventLoop(semStruct);
    close(socketFD);
}


int readDataFromSocket(int socketFD) {
    ssize_t length = 0;
    char buf[MAXBUFFSIZE];
    struct timeval recvBufferTime, sendBufferTime;
    if (buf == NULL) {
        perror("MALLOC");
        abort();
    }
    ssize_t count;
    int readLength;
    gettimeofday(&recvBufferTime,0);
    count = recv(socketFD, (char *) &readLength, sizeof(int), 0);
    printf("readLength: %d\n", readLength);
    if (count != sizeof(int)) {
        //perror("recv\n");
        droppedConnection();
        return 0;
    }
    while (1) {
        count = recv(socketFD, &buf[length], readLength - length, 0);
        length += count;
        if   (length == readLength) {
            break;
        }
        if (length > readLength) {
            perror("buffer overflow\n");
        }
        switch (count) {
        case -1:
            if (errno != EAGAIN) {
                perror ("read");
                return 1;
            }
            if (length != 0) {
                //call send function here not done yet
                break;
            }
        case 0:
            return 1;
        default:
            continue;
        }
    }
    //may want to wrap this in a loop to ensure full sending
    count = send(socketFD, buf, readLength, 0);
    if (readLength != count) {
        perror("send");
    }
    gettimeofday(&sendBufferTime,0);
    setResponce(delay(sendBufferTime,recvBufferTime));
    setData(count);
    return 0;
}

int getAddressResult(int port, struct addrinfo **result) {
    struct addrinfo hints;
    int returnValue;
    char sPort[6];
    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    sprintf(sPort, "%d",port);
    returnValue = getaddrinfo (NULL, sPort, &hints, result);
    if (returnValue != 0) {
        fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (returnValue));
        return -1;
    }
    return 0;
}

int createAndBind(int port) {

    struct addrinfo *result, *rp;
    int socketFD;

    if (getAddressResult(port, &result) == -1) { return -1; }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        socketFD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (socketFD == -1) {
            continue;
        }
        if (bind (socketFD, rp->ai_addr, rp->ai_addrlen) == 0) {
            /* We managed to bind successfully! */
            break;
        }
        close (socketFD);
    }

    if (rp == NULL) {
        fprintf (stderr, "Could not bind\n");
        return -1;
    }
    freeaddrinfo (result);
    return socketFD;
}

void processIncomingNewSocket(int socketFD, int epollFD) {
    while (1) {
        struct epoll_event event;
        struct sockaddr in_addr;
        socklen_t in_len;
        int infd, epollSuccess;//, connectResult;
        //char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        in_len = sizeof(in_addr);
        infd = accept (socketFD, &in_addr, &in_len);
        if (infd == -1) {
            if ((errno == EAGAIN) ||
                    (errno == EWOULDBLOCK)) {
                /* We have processed all incoming
                   connections. */
                break;
            } else {
                perror ("accept");
                break;
            }
        }
        /*
        connectResult = getnameinfo (&in_addr, in_len,
                         hbuf, sizeof hbuf,
                         sbuf, sizeof sbuf,
                         NI_NUMERICHOST | NI_NUMERICSERV);
        if (connectResult == 0) {
            printf("Accepted connection on descriptor %d "
                   "(host=%s, port=%s)\n", infd, hbuf, sbuf);
            //newConnectionTasks(infd); //not used right now
        }
        */
        makeNonBlockingSocket (infd);

        event.data.fd = infd;
        event.events = EPOLLIN | EPOLLET;
        epollSuccess = epoll_ctl (epollFD, EPOLL_CTL_ADD, infd, &event);
        if (epollSuccess == -1) {
            perror ("epoll_ctl");
            abort ();
        }
        newConnection();
    }
}

int validateSocket(int port) {
    int socketFD = createAndBind (port);
    if (socketFD == -1) {
        abort ();
    }
    return socketFD;
}

void bindandListenSocket(int socketFD) {
    int result;
    makeNonBlockingSocket (socketFD);

    result = listen (socketFD, SOMAXCONN);
    if (result == -1) {
        perror ("listen");
        abort ();
    }
}

int createEPoll() {
    int epollFD = epoll_create1(0);
    if (epollFD == -1) {
        perror ("epoll_create");
        abort ();
    }
    return epollFD;
}

void setEPollSocket(int epollFD, int socketFD, struct epoll_event **pevents) {
    struct epoll_event event;
    int result;
    event.data.fd = socketFD;
    event.events = EPOLLIN | EPOLLET;
    result = epoll_ctl (epollFD, EPOLL_CTL_ADD, socketFD, &event);
    if (result == -1) {
        perror ("epoll_ctl");
        abort ();
    }
    *pevents = calloc (MAXEVENTS, sizeof event);
}
