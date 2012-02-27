#include "sock.h"

int setupServerTCPSocket(int port) {
    int socketFD;
    if ((socketFD = createSocket()) == -1) {return -1;}
    if (setSocketReuse(socketFD) == -1) {return -1;}
    if (bindSocket(socketFD, port) == -1) {return -1;}
    if (listenSocket(socketFD, 5) == -1) {return -1;}

    return socketFD;
}

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

int setSocketReuse(int socketFD) {
    int reuse = 1;
    if(setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR,
                  (char *)&reuse, sizeof(reuse)) != 0) {
        perror("Setting SO_REUSEADDR error\n");
        return -1;
    }
    return 1;
}

int createSocket() {
    int socketFD;
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFD == -1) {
        perror("error opening socket\n");
        return -1;
    }
    return socketFD;
}

int listenSocket(int socketFD, int sizeOfQueue) {
    if ((listen(socketFD, sizeOfQueue)) != 0) {
        perror("error listening to socket\n");
        return -1;
    }
    return 1;
}

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


