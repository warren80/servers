#include "common.h":
#include <signal.h>



void semOperation(int id, struct sembuf *operation) {
    if (semop(id, operation, 1) != 0) {
        perror("sem_op\n");
    }
}

PWORKERSTRUCT getSemStruct() {
    PWORKERSTRUCT ss = (PWORKERSTRUCT) malloc(sizeof(WORKERSTRUCT));

    ss->id = semget(KEY, 4, 0666 | IPC_CREAT);
    if(id < 0) {
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
    printf("role: %d\n", role);
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
