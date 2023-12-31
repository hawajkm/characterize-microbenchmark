/* para.c
 *
 * Author:
 * Date  :
 *
 *  Description
 */

#define _GNU_SOURCE

/* Standard C includes */
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <assert.h>

/* Include common headers */
#include "common/macros.h"
#include "common/types.h"

/* If we are on Darwin, include the compatibility header */
#if defined(__APPLE__)
#include "common/mach_pthread_compatibility.h"
#endif

/* Include application-specific headers */
#include "include/types.h"

/* Alternative Implementation */
void* worker(void* args) {
  /* Parse the arguments structure */
  args_t *p_args = (args_t*)args;

  /* Get all the arguments */
  register       int*   dest = (      int*)(p_args->output);
  register const int*   src0 = (const int*)(p_args->input0);
  register const int*   src1 = (const int*)(p_args->input1);
  register       size_t size =              p_args->size;

  for (int i = 0; i < size; i++) {
    dest[i] = src0[i] + src1[i];
  }

  return NULL;
}

void* impl_parallel(void* args)
{
  /* Get the argument struct */
  args_t* p_args = (args_t*)args;

  /* Get all the arguments */
  register       int*   dest = (      int*)(p_args->output);
  register const int*   src0 = (const int*)(p_args->input0);
  register const int*   src1 = (const int*)(p_args->input1);
  register       size_t size =              p_args->size / 4;

  register       size_t nthreads = p_args->nthreads;
  register       size_t cpu      = p_args->cpu;

  /* Create all threads */
  pthread_t tid[nthreads];
  args_t    targs[nthreads];
  cpu_set_t cpuset[nthreads];

  /* Amount of work per thread */
  size_t size_per_thread = size / nthreads;
  size_t remaining = size % nthreads;

  for (int i = 0; i < nthreads; i++) {
    /* Initialize the argument structure */
    targs[i].size     = size_per_thread;
    targs[i].output   = (byte*)dest;
    targs[i].input0   = (byte*)src0;
    targs[i].input1   = (byte*)src1;

    dest += targs[i].size;
    src0 += targs[i].size;
    src1 += targs[i].size;

    targs[i].cpu      = (cpu + i) % nthreads;
    targs[i].nthreads = nthreads;

    /* Affinity */
    CPU_ZERO(&(cpuset[i]));
    CPU_SET(targs[i].cpu, &(cpuset[i]));

    /* Set affinity */
    if (i == 0) {
      tid[i] = pthread_self();
    } else {
      int __attribute__((unused)) res = \
                         pthread_create(&tid[i], NULL, worker, (void*)&targs[i]);
    }

    int __attribute__((unused)) res_affinity = pthread_setaffinity_np(tid[i],
                                                sizeof(cpuset[i]), &(cpuset[i]));
  }

  if (nthreads > 0) {
    /* Perform one portion of the work */
    for (int i = 0; i < targs[0].size; i++) {
      ((int*)targs[0].output)[i] =                            \
                          ((const int*)targs[0].input0)[i] +  \
                              ((const int*)targs[0].input1)[i];
    }

    /* Perform trailing elements */
    for (int i = size - remaining; i < size; i++) {
      ((int*)targs[0].output)[i] =                            \
                          ((const int*)targs[0].input0)[i] +  \
                              ((const int*)targs[0].input1)[i];
    }
  }

  /* Wait for all threads to finish execution */
  for (int i = 0; i < nthreads; i++) {
    pthread_join(tid[i], NULL);
  }

  /* Done */
  return NULL;
}
