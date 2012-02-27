#include "metrics.h"

/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		metric.c
--
--	PROGRAM:		Server Architecture Stress Tests: various servers
--
--	FUNCTIONS:              void metricLoop()
--                              void initSems()
--                              void initMetricStructs()
--                              void metricUpdate()
--                              int getRecentConnections()
--                              int getResponces()
--                              long getRecentResponces()
--                              long getRecentData()
-                               void droppedConnection()
--                              void newConnection()
--                              void setData(long data)
--                              void setResponce(long length)
--                              long delay (struct timeval t1, struct timeval t2)
--                              void semOperation(int id, struct sembuf *operation)
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
--	Collects metric data from various threads using threadsafe techniques.
--      Outputs the collected data roughly every second
---------------------------------------------------------------------------------------*/
RESULT oldResult;
RESULT result;

#define SEMDATA         0
#define SEMRESPONCE     1
#define SEMCONNECTION   2
#define METRICKEY       134243

struct sembuf decrementResponce[1];
struct sembuf incrementResponce[1];
struct sembuf decrementData[1];
struct sembuf incrementData[1];
struct sembuf decrementConnections[1];
struct sembuf incrementConnections[1];
int id;


/*---------------------------------------------------------------------------------------
--      FUNCTION: void metricLoop()
--
--      RETURNS: void
--
--      NOTES:
--      Every second this function prints out the activity on the server to a file
---------------------------------------------------------------------------------------*/
void metricLoop() {
    //sigset_t set;
    //int sig;
    //sigemptyset(&set);
    //sigaddset(&set, SIGALRM);
    struct timeval current;
    struct timeval old;
    gettimeofday(&old, 0);
    initSems();
    initMetricStructs();
    FILE * fp = fopen("metrics.csv", "w");
    if (fp == 0) {
        printf("failed to open");
    }
    fprintf(fp,"TimeElapsed,CurrentConnections,RecentResponces/s,AvgDelay,data/s\n");
    while(1) {
        sleep(1);
        gettimeofday(&current, 0);
        fprintf(fp, "%d,%d,%d,%lu,%lu\n", (int) delay(old,current),
                getRecentConnections(), getResponces(), getRecentResponces(), getRecentData());
        fflush(fp);
        metricUpdate();
    }
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: void initSems()
--
--      RETURNS: void
--
--      NOTES:
--      Intializes the semephores used to ensure threadsafe updating of the metric data
---------------------------------------------------------------------------------------*/
void initSems() {
    int i;
    union semun {
        int val;
        struct semid_ds *buf;
        ushort * array;
    } argument;

    id = semget(METRICKEY, 4, 0666 | IPC_CREAT);

    if(id < 0) {
        perror("semget\n");
    }
    argument.val = 1;
    for (i = 0; i != 3; ++i) {
        if( semctl(id,i , SETVAL, argument) < 0) {
            perror("semctl\n");
        }

    }

    decrementResponce[0].sem_num = SEMRESPONCE;
    decrementResponce[0].sem_op = -1;
    decrementResponce[0].sem_flg = 0;

    incrementResponce[0].sem_num = SEMRESPONCE;
    incrementResponce[0].sem_op = 1;
    incrementResponce[0].sem_flg = 0;

    decrementData[0].sem_num = SEMDATA;
    decrementData[0].sem_op = -1;
    decrementData[0].sem_flg = 0;

    incrementData[0].sem_num = SEMDATA;
    incrementData[0].sem_op = 1;
    incrementData[0].sem_flg = 0;

    decrementConnections[0].sem_num = SEMCONNECTION;
    decrementConnections[0].sem_op = -1;
    decrementConnections[0].sem_flg = 0;

    incrementConnections[0].sem_num = SEMCONNECTION;
    incrementConnections[0].sem_op = 1;
    incrementConnections[0].sem_flg = 0;

}

/*---------------------------------------------------------------------------------------
--      FUNCTION: void initMetricStructs()
--
--      RETURNS: void
--
--      NOTES:
--      zeros out the results structures used in metric collection
---------------------------------------------------------------------------------------*/
void initMetricStructs() {
    oldResult.connections = 0;
    oldResult.responces = 0;
    oldResult.totalData = 0;
    result.connections = 0;
    result.totalData = 0;
    result.responces = 0;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: void metricUpdate()
--
--      RETURNS: void
--
--      NOTES:
--      every second the current metric data is moved to old metric data
---------------------------------------------------------------------------------------*/

void metricUpdate() {
    oldResult.responceTotalTime = result.responceTotalTime;
    oldResult.responces = result.responces;
    oldResult.totalData = result.totalData;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: void metricUpdate()
--
--      RETURNS: total currently connected connections
--
--      NOTES: getter
---------------------------------------------------------------------------------------*/
int getRecentConnections() {
    return result.connections;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: int getResponces()
--
--      RETURNS: the results from the last second
--
--      NOTES: getter
---------------------------------------------------------------------------------------*/
int getResponces() {
    return result.responces - oldResult.responces;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: long getRecentResponces()
--
--      RETURNS: the average delay between sending and recieving over the last second
--
--      NOTES: getter
---------------------------------------------------------------------------------------*/
long getRecentResponces() {
    int responces = result.responces - oldResult.responces;
    long avgDelay = result.responceTotalTime - oldResult.responceTotalTime;
    if (responces == 0 || avgDelay == 0) {
        return 0;
    }
    return avgDelay / responces;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: long getRecentData()
--
--      RETURNS: data recieved / transmitted over last second
--
--      NOTES: getter
---------------------------------------------------------------------------------------*/
long getRecentData() {
    return result.totalData - oldResult.totalData;
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: void droppedConnection()
--
--      RETURNS: void
--
--      NOTES: threadsafe setter
---------------------------------------------------------------------------------------*/
void droppedConnection() {
    semOperation(id, decrementConnections);
    --result.connections;
    semOperation(id, incrementConnections);
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: void newConnection()
--
--      RETURNS: void
--
--      NOTES: threadsafe setter
---------------------------------------------------------------------------------------*/
void newConnection() {
    semOperation(id, decrementConnections);
    ++result.connections;
    semOperation(id, incrementConnections);
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: void setData(long data)
--
--      RETURNS: void
--
--      NOTES: threadsafe setter
---------------------------------------------------------------------------------------*/
void setData(long data) {
    semOperation(id, decrementData);
    result.totalData += data;
    semOperation(id, incrementData);
}

/*---------------------------------------------------------------------------------------
--      FUNCTION: void setResponce(long length)
--
--      RETURNS: void
--
--      NOTES: threadsafe setter
---------------------------------------------------------------------------------------*/
void setResponce(long length) {
    semOperation(id, decrementResponce);
    ++result.responces;
    result.responceTotalTime +=  length;
    semOperation(id, incrementResponce);
}





/*---------------------------------------------------------------------------------------
--      FUNCTION: long delay (struct timeval t1, struct timeval t2)
--
--      RETURNS: not used
--
--      AUTHOR Aman Abdulla
--
--      NOTES:
--      calculates the elapsed time in miliseconds between two timeval structures.
---------------------------------------------------------------------------------------*/
long delay (struct timeval t1, struct timeval t2) {
    long d;
    d = (t2.tv_sec - t1.tv_sec) * 1000;
    d += ((t2.tv_usec - t1.tv_usec + 500) / 1000);
    return(d);
}

void semOperation(int id, struct sembuf *operation) {
    if (semop(id, operation, 1) != 0) {
        perror("sem_op\n");
    }
}
