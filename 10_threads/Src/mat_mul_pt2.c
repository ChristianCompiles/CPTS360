#include <stdlib.h> // for malloc()
#include <time.h>
#include <assert.h>
#include <pthread.h>

#include "a2d.h"
#include "tspec_diff.h"
#include "mat_mul_pt2.h"

/*
 *  Good practice: This structure contains all of the "globals" each
 *  thread can access.
 *
 *  A pointer to a shared instance of this one struct is shared by all
 *  threads used for a single matrix multiply, so any values that
 *  might be modified by more than one thread should not be changed by
 *  any thread without being protected by some kind of mutex, a la
 *  "nextRow".
 */
typedef struct {
    /*
     * `nextRow` is modified by every thread, so it needs a mutex.
     */
    int nextRow; /* the next row to be done *and* # of rows already done */
    pthread_mutex_t nextRowMutex; /* restricts access to `nextRow` */

    /*
     * As far as a thread is concerned, these are constants, so no
     * mutex is necessary.
     */
    int nThreads;
    int n, m, p; /* matrix dimensions */
    const double *_a, *_b; /* input (1D) matrices */

    /*
     * Each row of this matrix is modified by only one thread, so
     * again no mutex is necessary. (Note how this decision, which is
     * critical to good threaded performance, requires knowledge of
     * the algorithm.)
     */
    double *_c; /* output (1D) matrix */
} ThreadGlobals;


static void multiplyRow(double *_c, const int i, const int n, const int m,
                        const double *_a, const int p, const double *_b)
{
#define c(i,j) _c[I2D(i, n, j, m)]
#define a(i,j) _a[I2D(i, n, j, p)]
#define b(i,j) _b[I2D(i, p, j, m)]
    int j, k;
    double sum;

    sum = 0.0;
    for (j = 0; j < m; j++) {
        sum = 0.0;
        for (k = 0; k < p; k++)
            sum += a(i,k) * b(k,j);
        c(i,j) = sum;
    }
#undef a
#undef b
#undef c
}


// inThread -- function for each thread
static void *inThread(void *threadGlobals_)
{
    /*
     * ASSIGNMENT
     *
     * Implement the following pseudocode:
     *
     * allocate a MatMulThreadStats instance `matMulThreadStats`
     *  (hint: malloc())
     * set `matMulThreadStats->nRowsDone` to 0
     * get this thread's cpu clock id (hint: pthread_self() and
     *  pthread_getcpuclockid())
     * get the thread clock id's start cpu time (hint: clock_gettime())
     * loop the following indefinitely,
     *     if there are multiple threads,
     *         lock the "nextRow" mutex (hint: pthread_mutex_lock())
     *     set `i` to the current value of `threadGlobals->nextRow`
     *     increment `threadGlobals->nextRow`
     *     if there are multiple threads,
     *         unlock the "nextRow" thread mutex (hint: pthread_mutex_unlock())
     *     if `i` >= `n` (the number of rows),
     *         break out of the loop
     *     multiply row `i` (hint: multiplyRow())
     *     increment `matMulThreadStats->nRowsDone`
     * get the thread clock id's end cpu time (hint: clock_gettime())
     * set `matMulThreadStats->cpuTime` to the difference of thread start and
     *  stop cpu times (hint: tspecDiff())
     * return `matMulThreadStats`
     */

    ThreadGlobals * threadGlobals = (ThreadGlobals*)threadGlobals_; // cast void ptr

    MatMulThreadStats* matMulThreadStats = (MatMulThreadStats*) malloc(sizeof(MatMulThreadStats));
    matMulThreadStats->nRowsDone = 0;

    clockid_t cid;
    pthread_getcpuclockid(pthread_self(), &cid);

    struct timespec threadcputimestart;
    clock_gettime(cid, &threadcputimestart);

    while(1)
    {
        if (threadGlobals->nThreads > 1)
        {
            pthread_mutex_lock(&threadGlobals->nextRowMutex);
        }
        int i = threadGlobals->nextRow;
        threadGlobals->nextRow++;

        if (threadGlobals->nThreads > 1)
        {
            pthread_mutex_unlock(&threadGlobals->nextRowMutex);
        }

        if(i >= threadGlobals->n)
        {
            break;
        }
        multiplyRow(threadGlobals->_c, i, threadGlobals->n, threadGlobals->m, threadGlobals->_a, threadGlobals->p, threadGlobals->_b);
        matMulThreadStats->nRowsDone++;
    }
    struct timespec threadcputimeend;
    clock_gettime(cid, &threadcputimeend);
    matMulThreadStats->cpuTime = tspecDiff(threadcputimestart, threadcputimeend);
    return matMulThreadStats;
}


void mat_mul(double *_c, const int n, const int m,
             const double *_a, const int p, const double *_b,
             const int nThreads, MatMulThreadStats allMatMulThreadStats[])
{
    /*
     * ASSIGNMENT
     *
     * Implement the following pseudocode:
     *
     * declare a ThreadGlobals struct and set all fields to the
     *  corresponding parameters and `nextRow` to 0. (No rows have
     *  been done yet.)
     * if `nThreads` > 0,
     *     malloc() an array of `nThreads` pthread_t's
     *     initialize the `nextRowMutex` thread mutex (hint:
     *      pthread_mutex_init())
     *     for each of `nThreads` threads,
     *         create a thread calling inThread() and pass it a
     *          pointer to `threadGlobals` (hint: pthread_create())
     *     for each thread `i` of `nThreads` threads,
     *         wait for the thread to complete (hint: pthread_join())
     *         copy its MatMulThreadStats return to `allMatMulThreadStats[i]`
     *         free the returned struct
     *     destroy the thread mutex (hint: pthread_mutex_destroy())
     *     free the pthread_t array
     * otherwise
     *     call the thread function directly (hint: inThread())
     *     copy its MatMulThreadStats return to `allMatMulThreadStats[0]`
     *     free the returned struct
     */

    ThreadGlobals threadGlobals;
    threadGlobals._c = _c;
    threadGlobals.n = n;
    threadGlobals.m = m;
    threadGlobals._a = _a;
    threadGlobals.p = p;
    threadGlobals._b = _b;
    threadGlobals.nextRow = 0;

    if(nThreads > 0)
    {
        pthread_t* array_pthreads = (pthread_t*) malloc(nThreads * sizeof (pthread_t));
        pthread_mutex_init(&threadGlobals.nextRowMutex, NULL);

        for(int i = 0; i < nThreads; i++)
        {
            pthread_create(&array_pthreads[i], NULL, inThread, &threadGlobals);
        }

        for(int i = 0; i < nThreads; i++)
        {
            MatMulThreadStats* threadfunctionreturnval;
            pthread_join(array_pthreads[i], (void*)&threadfunctionreturnval);
            allMatMulThreadStats[i].nRowsDone = threadfunctionreturnval->nRowsDone;
            allMatMulThreadStats[i].cpuTime = threadfunctionreturnval->cpuTime;
            free(threadfunctionreturnval);
        }

        pthread_mutex_destroy(&threadGlobals.nextRowMutex);

        free(array_pthreads);
    }
    else
    {
        MatMulThreadStats* inthreadreturn = inThread(&threadGlobals);
        allMatMulThreadStats[0].nRowsDone = inthreadreturn->nRowsDone;
        allMatMulThreadStats[0].cpuTime = inthreadreturn->cpuTime;
        free(inthreadreturn);
    }

}
