/* macros.h
 *
 * Author: Khalid Al-Hawaj
 * Date  : 13 Nov. 2023
 * 
 * This file contains all macro decalarations.
*/

#ifndef __COMMON_MACROS_H_
#define __COMMON_MACROS_H_

/* General */
#define __COMPILER_FENCE_ __asm__ __volatile__ ("" : : : "memory");

/* Specific */
#define __PRINT_MATCH(x) (x ? "   MATCH" : "NO MATCH")

/* Testing and Statistics Macros */
#define __ALLOC_DATA(type, num_elems) ({               \
  type* temp = (type*)calloc(num_elems, sizeof(type)); \
  temp;                                                \
})

#define __ALLOC_INIT_DATA(type, num_elems) ({          \
  type* temp = (type*)calloc(num_elems, sizeof(type)); \
  /* Generate data */                                  \
  for(int i = 0; i < num_elems; i++) {                 \
    temp[i] = rand() % (0x1llu << (sizeof(type) * 8)); \
  }                                                    \
  temp;                                                \
})

#define __SET_GUARD(array, sz) {                       \
  array[sz + 0] = 0xfe;                                \
  array[sz + 1] = 0xca;                                \
  array[sz + 2] = 0xad;                                \
  array[sz + 3] = 0xde;                                \
}

#define __CHECK_MATCH(ref, array, sz) ({               \
  bool __tmp = true;                                   \
                                                       \
  for(int i = 0; (i < sz) && __tmp; i++) {             \
    __tmp = __tmp && (ref[i] == array[i]);             \
  }                                                    \
                                                       \
  __tmp;                                               \
})

#define __CHECK_GUARD(array, sz) ({                    \
  bool match = true;                                   \
                                                       \
  match = match && (array[sz + 0] == 0xfe);            \
  match = match && (array[sz + 1] == 0xca);            \
  match = match && (array[sz + 2] == 0xad);            \
  match = match && (array[sz + 3] == 0xde);            \
                                                       \
  match;                                               \
})

#define __DECLARE_STATS(_num_runs, _num_stdev)         \
  /* Time keeping */                                   \
  struct timespec ts;                                  \
  struct timespec te;                                  \
                                                       \
  /* Iterate and average runtimes */                   \
  uint32_t num_runs = _num_runs;                       \
  uint64_t* runtimes;                                  \
                                                       \
  runtimes = (uint64_t*)calloc(num_runs,               \
                                 sizeof(uint64_t));    \
                                                       \
  /* Constants for statistical analysis */             \
  const unsigned int nstd = _num_stdev;

#define __SET_START_TIME() {                           \
  __COMPILER_FENCE_;                                   \
  if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {     \
    printf("\n\n    ERROR: getting time failed!\n\n"); \
    exit(-1);                                          \
  }                                                    \
}

#define __SET_END_TIME() {                             \
  __COMPILER_FENCE_;                                   \
  if (clock_gettime(CLOCK_MONOTONIC, &te) == -1) {     \
    printf("\n\n    ERROR: getting time failed!\n\n"); \
    exit(-1);                                          \
  }                                                    \
}

#define __CALC_RUNTIME() ({                            \
    (((te.tv_sec  - ts.tv_sec ) * 1e9) +               \
      (te.tv_nsec - ts.tv_nsec)      ) ;               \
})

#endif //__COMMON_MACROS_H_
