/*
gcc main_v1.c -o main_v1.out -lpthread
./main_v1.out 3 10 2.0 1 8 10 12
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>
#include <error.h>
#include <errno.h>

#include "modules/list.c"

struct timespec prevLock;

// #include "modules/thread_pool.c"

pthread_mutex_t mut;
//  = PTHREAD_MUTEX_INITIALIZER;
pthread_t *threads;

int *avail_res, **alloc_res, **req_res;
int thread_cnt, res_cnt, heuristic_id;
int *res_lim;
int *key; // Key for sorting
double interv;

int randint(int lo, int hi)
{
    return rand() % (hi - lo + 1) + lo;
}

double randdoub(double lo, double hi)
{
    return lo + (double)rand() / RAND_MAX * (hi - lo);
}

void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

int check_deadlock(int work[], bool finished[], bool res_deadlocked[])
{
    int completed = 0;
    for (int i = 0; i < res_cnt; i++)
    {
        work[i] = avail_res[i];
    }

    for (int i = 0; i < thread_cnt; i++)
        if (finished[i])
            completed++;

    int i, j;
    for (int k = 0; k < thread_cnt; k++)
    {
        for (i = 0; i < thread_cnt; i++)
        {
            bool isPossible = true;
            if (finished[i])
                continue;
            for (j = 0; j < res_cnt; j++)
            {
                if (req_res[i][j] > work[j])
                {
                    res_deadlocked[j] = true;
                    isPossible = false;
                }
            }

            if (isPossible)
            {
                for (int k = 0; k < res_cnt; k++)
                    work[k] += alloc_res[i][k];
                finished[i] = 1;
                completed++;
            }
        }
    }
    // printf("\nEXIT CHECK LOCK\n");
    return completed;
} // }

void freeResources(int thread_ind)
{
    for (int i = 0; i < res_cnt; i++)
    {
        avail_res[i] += alloc_res[thread_ind][i];
        alloc_res[thread_ind][i] = 0;
    }
}

void unlock_mut(bool *isLocked)
{
    pthread_mutex_unlock(&mut);
    isLocked = false;
    // printf("UNLOCKING\n");
    if (errno)
        printf("Error Num %d,\n %s\n", errno, strerror(errno));
}

void lock_mut(bool *isLocked)
{
    pthread_mutex_lock(&mut);
    *isLocked = true;
    // printf("LOCKING\n");
    if (errno)
        printf("Error Num %d,\n %s\n", errno, strerror(errno));
}
void threadexitfunc(void *null_bool)
{
    bool *isLocked = (bool *)null_bool;

    if (*isLocked)
        unlock_mut(isLocked);

    free(isLocked);
}

void *thread_func(void *ptr_thread_ind)
{
    bool *isLocked = malloc(sizeof(bool));
    *isLocked = false;

    pthread_cleanup_push(threadexitfunc, isLocked);
    int thread_ind = *(int *)ptr_thread_ind;
    free(ptr_thread_ind);

    while (true)
    {
        int cnt_allocated = 0, res_ind;

        lock_mut(isLocked);
        for (int i = 0; i < res_cnt; i++)
        {
            pthread_testcancel();
            req_res[thread_ind][i] = randint(0, res_lim[i] / 2);

            if (!req_res[thread_ind][i])
                cnt_allocated++;
        }
        unlock_mut(isLocked);

        while (cnt_allocated < res_cnt)
        {
            sleep(randdoub(0.3, 1.4));
            res_ind = randint(0, res_cnt - 1); //Index Of resource to be requested

            if (!req_res[thread_ind][res_ind])
                continue;

            lock_mut(isLocked);
            pthread_testcancel();
            if (avail_res[res_ind] >= req_res[thread_ind][res_ind]) //Resources Available
            {
                avail_res[res_ind] -= req_res[thread_ind][res_ind];
                alloc_res[thread_ind][res_ind] += req_res[thread_ind][res_ind];
                req_res[thread_ind][res_ind] = 0;
                cnt_allocated++;
            }
            unlock_mut(isLocked);
        }

        // All Resources Found
        double hold_time = randdoub(0.7 * interv, 1.5 * interv);
        sleep(hold_time);

        //Free All Resources
        printf("THREAD %d COMPLETED\n", thread_ind + 1);

        lock_mut(isLocked);
        freeResources(thread_ind);
        unlock_mut(isLocked);
    }
    pthread_cleanup_pop(1);
}

void activateThread(int thread_ind)
{
    int *temp = malloc(sizeof(int));
    *temp = thread_ind;
    pthread_create(threads + thread_ind, NULL, thread_func, temp);
}

int compare(const void *pa, const void *pb)
{
    const int a = *(const int *)pa;
    const int b = *(const int *)pb;

    if (key[a] != key[b])
        return key[b] - key[a];
    return a - b;
}

void *CheckerThreadFunc(void *nullptr)
{
    bool *isLocked = malloc(sizeof(bool));
    *isLocked = false;

    long long iterSincePrev = 0;

    bool fin[thread_cnt];
    int work[thread_cnt];
    bool res_locked[res_cnt];

    while (true)
    {
        memset(fin, 0, sizeof(fin));
        memset(res_locked, 0, sizeof(res_locked));

        // printf("Check Lock\n");
        lock_mut(isLocked);
        // printf("HELD BY BANKER\n");
        int fin_cnt = check_deadlock(work, fin, res_locked);

        // printf("CNT FIN: %d Total: %d\n", fin_cnt, thread_cnt);
        if (fin_cnt != thread_cnt) // Deadlock Found, No Task Doable
        {
            struct timespec curr_lock_time;
            clock_gettime(CLOCK_MONOTONIC_RAW, &curr_lock_time);

            double delta = (curr_lock_time.tv_sec - prevLock.tv_sec) / 1.0 + (curr_lock_time.tv_nsec - prevLock.tv_nsec) / 1e9;
            printf("\nDeadlock Found!\nTime Since Previous Deadlock: %f, Num Iterations: %lld.\nThreads In Deadlock (Count: %d): \n", delta, iterSincePrev, thread_cnt - fin_cnt);

            int cnt_unfin_ver = 0;
            int cnt_unfin = thread_cnt - fin_cnt;
            printf("Cnt Unfin %d\n", cnt_unfin);
            int unfin_thr[cnt_unfin], ind = 0;

            // printf("Cnt Unfin: %d \n", cnt_unfin);
            for (int i = 0; i < thread_cnt; i++)
            {
                key[i] = 0;

                if (!fin[i])
                {
                    printf("\tThread ID: %d\n", i);

                    unfin_thr[ind] = i;

                    cnt_unfin_ver++;
                    ind++;
                }
            }
            printf("\n");

            switch (heuristic_id)
            {
            case 1: // Heuristic 1 Close All DeadLocked Threads
            {
                for (int i = 0; i < cnt_unfin; i++)
                {
                    ind = unfin_thr[i];
                    pthread_cancel(threads[ind]);
                    // int err = pthread_join(threads[ind], NULL);
                }

                for (int i = 0; i < cnt_unfin; i++)
                {
                    ind = unfin_thr[i];
                    freeResources(ind);
                    activateThread(ind);
                }
                printf("Count Closed Thread: %d.\n", cnt_unfin);
                break;
            }

            case 2: // Heuristic 2 Close Thread with Maximum Resources
            {
                for (int i = 0; i < cnt_unfin; i++)
                {
                    ind = unfin_thr[i];

                    for (int j = 0; j < res_cnt; j++)
                        key[ind] += alloc_res[ind][j];
                }
                qsort(unfin_thr, cnt_unfin, sizeof(unfin_thr[0]), compare);
                // unlock_mut(isLocked);

                int i;
                // printf("BEGIN CLEANING\n");

                for (i = 0; i < cnt_unfin; i++)
                {
                    ind = unfin_thr[i];
                    pthread_cancel(threads[ind]);
                    // int err = pthread_join(threads[ind], NULL);

                    freeResources(ind);
                    fin[ind] = true;
                    fin_cnt = check_deadlock(work, fin, res_locked);

                    if (fin_cnt == thread_cnt)
                    {
                        break;
                    }
                }
                if (i == cnt_unfin)
                    i--;
                printf("Count Closed Thread: %d.\n", i + 1);
                while (i > -1)
                {
                    ind = unfin_thr[i];
                    fin[ind] = false;
                    activateThread(ind);
                    i--;
                }
                // printf("DONE CLEANING\n");
                break;
            }
            case 3:
            {
                for (int i = 0; i < cnt_unfin; i++)
                {
                    ind = unfin_thr[i];

                    for (int j = 0; j < res_cnt; j++)
                        if (res_locked[j])
                            key[ind] += alloc_res[ind][j];
                }
                qsort(unfin_thr, cnt_unfin, sizeof(unfin_thr[0]), compare);

                int i;
                // printf("BEGIN CLEANING\n");

                for (i = 0; i < cnt_unfin; i++)
                {
                    ind = unfin_thr[i];
                    pthread_cancel(threads[ind]);
                    // int err = pthread_join(threads[ind], NULL);

                    freeResources(ind);
                    fin[ind] = true;
                    if (check_deadlock(work, fin, res_locked) == thread_cnt)
                    {
                        break;
                    }
                }

                if (i == cnt_unfin)
                    i--;

                printf("Count Closed Thread: %d.\n", i + 1);

                while (i > -1)
                {
                    ind = unfin_thr[i];
                    fin[ind] = false;
                    activateThread(ind);
                    i--;
                }
                break;
            }

            default:
            {
                printf("Error! Invalid Heuristic ID\n");
                exit(-1);
                break;
            }
            }

            clock_gettime(CLOCK_MONOTONIC_RAW, &prevLock);
            iterSincePrev = 0;
        }
        else
            iterSincePrev++;
        unlock_mut(isLocked);
        // printf("BANKER UNLOCK\n");
        sleep(interv);
    }
}
long stringtolong(char *c)
{
    long num;
    char *ptr;
    num = strtol(c, &ptr, 10);
    if (ptr == c)
        return -1;
    return num;
}

double stringtodouble(char *c)
{
    long num;
    char *ptr;
    num = strtod(c, &ptr);

    if (ptr == c)
        return -1;
    return num;
}

int main(int argc, char **argv)
{
    // Initialize as Recursive Mutex
    pthread_mutexattr_t Attr;

    pthread_mutexattr_init(&Attr);

    pthread_mutexattr_setrobust(&Attr, PTHREAD_MUTEX_ROBUST);
    pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_ERRORCHECK);

    pthread_mutex_init(&mut, &Attr);

    clock_gettime(CLOCK_MONOTONIC_RAW, &prevLock);
    srand(time(NULL));

    bool valid = true;

    if (argc < 5)
        valid = false;
    else
    {
        res_cnt = stringtolong(argv[1]);
        thread_cnt = stringtolong(argv[2]);
        interv = stringtodouble(argv[3]);
        heuristic_id = stringtolong(argv[4]);

        // printf("%d, %d, %d, %f\n", cnt_res, cnt_th, heuristic_id, interv);
        if (res_cnt == -1 || thread_cnt == -1 || heuristic_id == -1 || (argc < res_cnt + 5))
            valid = false;
        else
            avail_res = (int *)malloc(res_cnt * sizeof(int)), res_lim = (int *)malloc(res_cnt * sizeof(int));
    }

    char *c = "USAGE: main.out [Number of Resources] [Number of Threads] [Interval for Execution] [Heuristic ID] Resource Limits [A_lim B_lim ... N_lim].\n Heuristic IDs: 0, 1.\n";

    if (!valid)
    {
        printf("%s\n", c);
        exit(-1);
    }

    for (int i = 0; i < res_cnt; i++)
    {
        res_lim[i] = avail_res[i] = stringtolong(argv[i + 5]);

        if (avail_res[i] == -1)
        {
            valid = false;
            break;
        }
    }

    if (!valid)
    {
        printf("%s\n", c);
        exit(-1);
    }

    threads = malloc(thread_cnt * sizeof(pthread_t));

    req_res = (int **)malloc(thread_cnt * sizeof(int *));
    alloc_res = (int **)malloc(thread_cnt * sizeof(int *));

    for (int i = 0; i < thread_cnt; i++)
    {
        req_res[i] = (int *)malloc(res_cnt * sizeof(int));
        alloc_res[i] = (int *)malloc(res_cnt * sizeof(int));

        for (int j = 0; j < res_cnt; j++)
            req_res[i][j] = alloc_res[i][j] = 0;
    }

    key = (int *)malloc(thread_cnt * sizeof(int));
    for (int i = 0; i < thread_cnt; i++)
    {
        activateThread(i);
    }

    pthread_t deadlock_checker;
    pthread_create(&deadlock_checker, NULL, CheckerThreadFunc, NULL);

    // sleep(10);
    // INFINITE WAIT
    sleep(300);
    // pthread_mutex_t inf_mut = PTHREAD_MUTEX_INITIALIZER;
    // pthread_cond_t inf_cond = PTHREAD_COND_INITIALIZER;
    // pthread_cond_wait(&inf_cond, &inf_mut);
}
