#ifndef METRICS_H
#define METRICS_H

#include <sys/time.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    int connections;
    int responces;
    long responceTotalTime;
    long totalData;
} RESULT, *PRESULT;


void initMetricStructs();
void initSems();

void setResponce(long);
void setData(long);
void droppedConnection();
void newConnection();

long getRecentResponces();
int getRecentConnections();
long getRecentData();


void metricLoop();
void metricUpdate();

long delay(struct timeval t1, struct timeval t2);
void semOperation(int id, struct sembuf *operation);

#endif
