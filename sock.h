#ifndef SOCK_H
#define SOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>

int makeNonBlockingSocket (int socketFD);
int bindSocket(int socketFD, int port, const char *ip);
int setSocketReuse(int socketFD);
int listenSocket(int socketFD, int sizeOfQueue);
int createSocket();
int acceptSocket(int socketFD, struct sockaddr_in* cliAddr);
int connectSocket(int socketFD, int port, const char* ip);
int setupServerTCPSocket(int port, int workers, const char* ip);

#endif
