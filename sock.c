#include "sock.h"

/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		sock.c
--
--	PROGRAM:		Server Architecture Stress Tests: Socket wrappers
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
--
--	PROGRAMMERS:		Warren Voelkl
--
--	NOTES:
--      various wrappers for common socket system calls
---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------
--      FUNCTION: int setupServerTCPSocket(int port)
--
--      RETURNS: 1 on success
--               -1 on failure
--
--      NOTES:
--      Wrapper for wrapper functions used in creating a tcp server socket
---------------------------------------------------------------------------------------*/
int setupServerTCPSocket(int port) {
    int socketFD;
    if ((socketFD = createSocket()) == -1) {return -1;}
    if (setSocketReuse(socketFD) == -1) {return -1;}
    if (bindSocket(socketFD, port) == -1) {return -1;}
    if (listenSocket(socketFD, 5) == -1) {return -1;}

    return socketFD;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: int makeNonBlockingSocket (int socketFD)
--
--      RETURNS: 1 on success
--               -1 on failure
--
--      NOTES:
--      Wrapper for fnctl to make sockets not blocking
---------------------------------------------------------------------------------------*/
int makeNonBlockingSocket (int socketFD) {
    int flags, result;
    flags = fcntl (socketFD, F_GETFL, 0);
    if (flags == -1) {
        perror ("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    result = fcntl (socketFD, F_SETFL, flags);
    if (result == -1) {
        perror ("fcntl");
        return -1;
    }
    return 1;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: int bindSocket(int socketFD, int port)
--
--      RETURNS: 1 on success
--               -1 on failure
--
--      NOTES:
--      Wrapper for the bind call
---------------------------------------------------------------------------------------*/
int bindSocket(int socketFD, int port) {
    struct sockaddr_in sin;
    memset((char *) &sin, 0, sizeof(sin));
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr =  INADDR_ANY;
    sin.sin_family = AF_INET;
    if(bind(socketFD, (struct sockaddr *)&sin,
            sizeof(struct sockaddr_in) ) != 0) {
        printf("error binding socket\n");
        return -1;
    }
    return 1;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: int setSocketReuse(int socketFD)
--
--      RETURNS: 1 on success
--               -1 on failure
--
--      NOTES:
--      Wrapper for the bind call
---------------------------------------------------------------------------------------*/
int setSocketReuse(int socketFD) {
    int reuse = 1;
    if(setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR,
                  (char *)&reuse, sizeof(reuse)) != 0) {
        perror("Setting SO_REUSEADDR error\n");
        return -1;
    }
    return 1;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: int createSocket()
--
--      RETURNS: socket descriptor on success
--               -1 on failure
--
--      NOTES:
--      Wrapper for the socket call
---------------------------------------------------------------------------------------*/
int createSocket() {
    int socketFD;
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFD == -1) {
        perror("error opening socket\n");
        return -1;
    }
    return socketFD;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: int listenSocket(int socketFD, int sizeOfQueue)
--
--      RETURNS: 1 on success
--               -1 on failure
--
--      NOTES:
--      Wrapper for the listen call
---------------------------------------------------------------------------------------*/
int listenSocket(int socketFD, int sizeOfQueue) {
    if ((listen(socketFD, sizeOfQueue)) != 0) {
        perror("error listening to socket\n");
        return -1;
    }
    return 1;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: int acceptSocket(int socketFD, struct sockaddr_in* cliAddr)
--
--      RETURNS: socket descripter on success
--               exits program on failure.
--
--      NOTES:
--      Wrapper for the accept call
---------------------------------------------------------------------------------------*/
int acceptSocket(int socketFD, struct sockaddr_in* cliAddr) {
    int newSock;
    socklen_t cliLen = sizeof(struct sockaddr_in);
    if ((newSock = accept(socketFD,
                          (struct sockaddr *) &cliAddr, &cliLen)) < 0) {
        perror("Error on accept\n");
        exit(1);
    }
    return newSock;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: int connectSocket(int socketFD, int port, const char* ip)
--
--      RETURNS: socket descriptor on success
--               -1 on failure
--
--      NOTES:
--      Wrapper for the socket call
---------------------------------------------------------------------------------------*/
int connectSocket(int socketFD, int port, const char* ip) {
    struct sockaddr_in sin;
    memset((char *) &sin, 0, sizeof(sin));
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = 0;
    sin.sin_addr.s_addr =  inet_addr(ip);
    sin.sin_family = AF_INET;
    if (connect(socketFD, (struct sockaddr *) &sin,
                sizeof(sin)) < 0) {
        perror("Error on connect");
        return -1;
    }
    return 1;
}


