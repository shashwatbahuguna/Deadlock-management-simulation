#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include "modules/list.c"
#include <sys/time.h>

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

void exitfunc()
{
    printf("EXIT\n");
}

void *thread(void *pr)
{
    pthread_cleanup_push(exitfunc, NULL);
    printf("TRY LOCK\n");
    pthread_mutex_lock(&mut);
    printf("GOT LOCK\n");
    sleep(3);
    pthread_mutex_unlock(&mut);
    printf("EXIT\n");
    pthread_cleanup_pop(0);
}

int main()
{
    // pthread_cleanup_push(exitfunc, NULL);
    pthread_mutex_lock(&mut);
    pthread_t p;
    pthread_create(&p, NULL, thread, NULL);
    sleep(2);
    pthread_cancel(p);

    printf("UNLOCK\n");
    pthread_mutex_unlock(&mut);
    sleep(10);
    // pthread_cancel(p);
    printf("Done\n");
    // pthread_cleanup_pop(0);
}