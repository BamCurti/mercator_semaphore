#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <semaphore.h>

#define NPROCS 4
#define SERIES_MEMBER_COUNT 200000

double *sums;
double x = 1.0;
int *proc_count;
int *start_all;
int *completed;
double *res;

sem_t mutex;
sem_t all_started;

double get_member(int n, double x) {
    int i;
    double numerator = 1;

    for(i = 0; i < n; i++) {
        numerator *= x;
    }

    return (n % 2 == 0) ? 
        -numerator / n :
        numerator / n;
}

void proc(int proc_num) {
    int i;

    sem_wait(&all_started);

    sums[proc_num] = 0;

    for(i = proc_num; i < SERIES_MEMBER_COUNT; i += NPROCS) {
        sums[proc_num] += get_member(i + 1, x);
    }

    sem_wait(&mutex);
    (*proc_count)++;
    sem_post(&mutex);

    // Incrementa el contador de procesos completados
    sem_wait(&mutex);
    (*completed)++;
    sem_post(&mutex);

    exit(0);
}

void master_proc() {
    int i;
    sleep(1);
    sem_post(&all_started);

    while (*completed != NPROCS) {
        // Espera hasta que todos los procesos esclavos hayan terminado
        usleep(10000); // Agrega una pequeña pausa para liberar el CPU
    }

    *res = 0;

    for(i = 0; i < NPROCS; i++) {
        *res += sums[i];
    }

    exit(0);
}

int main() {
    int *threadIdPtr;
    long long start_ts;
    long long stop_ts;
    long long elapsed_time;
    long lElapsedTime;

    struct timeval ts;
    int i;
    int p;
    int shmid;
    void *shmstart;

    size_t size_shmid = NPROCS * sizeof(double) + 4 * sizeof(int);

    shmid = shmget(0x1234, size_shmid, 0666 | IPC_CREAT);
    shmstart = shmat(shmid, NULL, 0);
    sums = shmstart;

    proc_count = (int *)(sums + NPROCS);
    start_all = (int *)(proc_count + 1);
    completed = (int *)(start_all + 1);
    res = (double *)(completed + 1);

    *proc_count = 0;
    *start_all = 0;
    *completed = 0;

    sem_init(&mutex, 1, 1);
    sem_init(&all_started, 1, 0);

    gettimeofday(&ts, NULL);
    start_ts = ts.tv_sec; // Tiempo inicial

    for(i = 0; i < NPROCS; i++) {
        p = fork();
        if(p == 0) {
            proc(i);
        }
    }

    p = fork();

    if(p == 0) {
        master_proc();
    }

    printf("El recuento de ln(1 + x) miembros de la serie de Mercator es %d\n", SERIES_MEMBER_COUNT);
    printf("El valor del argumento x es %f\n", (double)x);

    for(int i = 0; i < NPROCS + 1; i++) {
        wait(NULL);
    }

    gettimeofday(&ts, NULL);

    stop_ts = ts.tv_sec; // Tiempo final
    elapsed_time = stop_ts - start_ts;

    printf("Tiempo = %lld segundos\n", elapsed_time);
    printf("El resultado es %10.8f\n", *res);
    printf("Llamando a la función ln(1 + %f) = %10.8f\n", x, log(1+x));

    sem_destroy(&mutex);
    sem_destroy(&all_started);

    shmdt(shmstart);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
