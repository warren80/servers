#include "metrics.h"

RESULT oldResult;
RESULT result;

/*
typedef struct {
    int connections;
    int responces;
    int requests;
    long totalData;
} RESULT, *PRESULT;*/

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
    while(1) {

        //if (sigwait(&set, &sig) != 0) {
        //    perror("sigwait error");
       //}


        sleep(1);

        gettimeofday(&current, 0);
        fprintf(fp, "%d,%d,%lu,%lu\n", (int) delay(old,current),
                getRecentConnections(), getRecentResponces(), getRecentData());
        fflush(fp);
        //printf("%d,%d,%lu,%lu\n", (int) delay(old,current),
        //        getRecentConnections(), getRecentResponces(), getRecentData());
        metricUpdate();
        //printf(
        //just printing so don't realy care about threadsafe;

        //TODO METRICS STUFF
        //printf("metrics happening here: %d\n", (int) pthread_self());
    }
}

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

void initMetricStructs() {
    oldResult.connections = 0;
    oldResult.responces = 0;
    oldResult.totalData = 0;
    result.connections = 0;
    result.totalData = 0;
    result.responces = 0;
}

void metricUpdate() {
    oldResult.responceTotalTime = result.responceTotalTime;
    oldResult.responces = result.responces;
    oldResult.totalData = result.totalData;
}

int getRecentConnections() {
    return result.connections;
}

long getRecentResponces() {
    int responces = result.responces - oldResult.responces;
    long avgDelay = result.responceTotalTime - oldResult.responceTotalTime;
    if (responces == 0 || avgDelay == 0) {
        return 0;
    }
    return avgDelay / responces;
}

long getRecentData() {
    return result.totalData - oldResult.totalData;
}

void droppedConnection() {
    semOperation(id, decrementConnections);
    --result.connections;
    semOperation(id, incrementConnections);
}

void newConnection() {
    semOperation(id, decrementConnections);
    ++result.connections;
    semOperation(id, incrementConnections);
}

void setData(long data) {
    semOperation(id, decrementData);
    result.totalData += data;
    semOperation(id, incrementData);
}

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
