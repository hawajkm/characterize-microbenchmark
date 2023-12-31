/* mach_pthread_compatibility.h
 *
 * Author: Khalid Al-Hawaj
 * Date  : 30 Dec. 2023
 * 
 * This file contains compatability layer for POSIX pthreads
 * with Darwin. So far, there is one function that requires
 * implementation:
 *
 *   pthread_setaffinity_np
 *
 * There are multiple CPU SET functions and data type that
 * requires implementation and declarations:
 *
 *   datatype: cpu_set_t
 *   functions: CPU_ZERO, CPU_SET, CPU_ISSET
*/

#if defined(__APPLE__)
#include <mach/thread_policy.h>
#include <mach/thread_act.h>
#include <pthread.h>
#endif

#ifndef __COMMON_MACH_PTHREAD_COMPATIBILITY_H_
#define __COMMON_MACH_PTHREAD_COMPATIBILITY_H_

/* CPU SET */
typedef struct cpu_set {
  uint32_t    count;
} cpu_set_t;

static inline void
CPU_ZERO(cpu_set_t *cs) { cs->count = 0; }

static inline void
CPU_SET(int num, cpu_set_t *cs) { cs->count |= (1 << num); }

static inline int
CPU_ISSET(int num, cpu_set_t *cs) { return (cs->count & (1 << num)); }

/* pthreads functions */
int pthread_setaffinity_np(pthread_t thread, size_t cpu_size,
                           cpu_set_t *cpu_set)
{
  thread_port_t mach_thread;
  int core = 0;

  for (core = 0; core < 8 * cpu_size; core++) {
    if (CPU_ISSET(core, cpu_set)) break;
  }
  //printf("binding to core %d\n", core);
  thread_affinity_policy_data_t policy = { core };
  mach_thread = pthread_mach_thread_np(thread);
  thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY,
                    (thread_policy_t)&policy, 1);
  return 0;
}

#endif //__COMMON_MACH_PTHREAD_COMPATIBILITY_H_
