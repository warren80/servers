CC=gcc
CFLAGS=-Wall -lpthread

epoll: mainEpoll.o epollServer.o sock.o metrics.o
	$(CC) $(CFLAGS) -o s.out mainEpoll.o epollServer.o sock.o metrics.o
	rm -f *.o
select: mainSelect.o selectServer.o common.o select.o metrics.o sock.o
	$(CC) -o s.out mainSelect.o selectServer.o select.o common.o metrics.o sock.o
	rm -f *.o
process: mainProcess.o process.o processServer.o common.o metrics.o sock.o
	$(CC) -o s.out mainProcess.o process.o processServer.o common.o metrics.o sock.o
	rm -f *.o
client: client.o metrics.o sock.o
	$(CC) $(CFLAGS) -o c.out client.o metrics.o sock.o
	rm -f *.o
clean:
	rm -f *.o
	rm -f *.out
stop:
	ps aux | grep "0[[:space:]]./s.out" | awk '{system("kill " $$2)}'
start:
	./s.out &
common.o:
	$(CC) $(CFLAGS) -o common.o -c common.c
epoll.o:
	$(CC) $(CFLAGS) -o epoll.o -c epoll.c
client.o:
	$(CC) $(CFLAGS) -o client.o -c client.c
select.o:
	$(CC) $(CFLAGS) -o select.o -c select.c
process.o:
	$(CC) $(CFLAGS) -o process.o -c process.c
epollServer.o:
	$(CC) $(CFLAGS) -o epollServer.o -c epollServer.c
processServer.o:
	$(CC) $(CFLAGS) -o processServer.o -c processServer.c
selectServer.o:
	$(CC) $(CFLAGS) -o selectServer.o -c selectServer.c
mainEpoll.o:
	$(CC) $(CFLAGS) -o mainEpoll.o -c mainEpoll.c
mainProcess.o:
	$(CC) $(CFLAGS) -o mainProcess.o -c mainProcess.c
mainSelect.o:
	$(CC) $(CFLAGS) -o mainSelect.o -c mainSelect.c
mainClient.o:
	$(CC) $(CFLAGS) -o mainClient.o -c mainClient.c
sock.o:
	$(CC) $(CFLAGS) -o sock.o -c sock.c
metrics.o:
	$(CC) $(CFLAGS) -o metrics.o -c metrics.c
