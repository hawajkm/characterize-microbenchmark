/* para.c
 *
 * Author:
 * Date  :
 *
 *  Description
 */

/* Standard C includes */
#include <stdlib.h>
#include <pthread.h>

/* Include common headers */
#include "common/macros.h"
#include "common/types.h"

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
  register       size_t size =              p_args->size / 4;

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
  pthread_t th[nthreads];
  args_t    th_args[nthreads];

  for (int i = 0; i < nthreads; i++) {
    /* Initialize the argument structure */
    th_args[i].size     = size / nthreads;
    th_args[i].input0   = (byte*)(src0 + (i * (size / nthreads)));
    th_args[i].input1   = (byte*)(src1 + (i * (size / nthreads)));
    th_args[i].output   = (byte*)(dest + (i * (size / nthreads)));

    th_args[i].cpu      = (cpu + i) % nthreads;
    th_args[i].nthreads = nthreads;

    int res = pthread_create(&th[i], NULL, worker, (void*)&th_args[i]);
  }

  /* Perform trailing elements */
  int remaining = size % nthreads;
  for (int i = size - remaining; i < size; i++) {
    dest[i] = src0[i] + src1[i];
  }

  /* Wait for all threads to finish execution */
  for (int i = 0; i < nthreads; i++) {
    pthread_join(th[i], NULL);
  }

  /* Done */
  return 0;
}
