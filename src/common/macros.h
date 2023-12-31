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
#define __PRINT_MATCH(x) (x ? "MATCHING" : "MISMATCH")

/* Testing and Statistics Macros */
#ifndef __APPLE__
#define __ALLOC_DATA(type, nelems) ({                  \
  unsigned int nbytes = (nelems) * sizeof(type);       \
  type* temp = (type*)aligned_alloc(512 / 8, nbytes);  \
                                                       \
  if (temp == NULL) {                                  \
    printf("\n");                                      \
    printf("  ERROR: Cannot allocate memory!");        \
    printf("\n");                                      \
    printf("\n");                                      \
    exit(-2);                                          \
  }                                                    \
                                                       \
  temp;                                                \
})
#else
#define __ALLOC_DATA(type, nelems) ({                  \
  unsigned int nbytes = (nelems) * sizeof(type);       \
  nbytes = ((nbytes + 63) / 64) * 64;                  \
  type* temp = (type*)aligned_alloc(512 / 8, nbytes);  \
                                                       \
  if (temp == NULL) {                                  \
    printf("\n");                                      \
    printf("  ERROR: Cannot allocate memory!");        \
    printf("\n");                                      \
    printf("\n");                                      \
    exit(-2);                                          \
  }                                                    \
                                                       \
  temp;                                                \
})
#endif

#ifndef __APPLE__
#define __ALLOC_INIT_DATA(type, nelems) ({             \
  unsigned int nbytes = (nelems) * sizeof(type);       \
  type* temp = (type*)aligned_alloc(512 / 8, nbytes);  \
                                                       \
  if (temp == NULL) {                                  \
    printf("\n");                                      \
    printf("  ERROR: Cannot allocate memory!");        \
    printf("\n");                                      \
    printf("\n");                                      \
    exit(-2);                                          \
  }                                                    \
                                                       \
  /* Generate data */                                  \
  for(int i = 0; i < nelems; i++) {                    \
    temp[i] = rand() % (0x1llu << (sizeof(type) * 8)); \
  }                                                    \
  temp;                                                \
})
#else
#define __ALLOC_INIT_DATA(type, nelems) ({             \
  unsigned int nbytes = (nelems) * sizeof(type);       \
  nbytes = ((nbytes + 63) / 64) * 64;                  \
  type* temp = (type*)aligned_alloc(512 / 8, nbytes);  \
                                                       \
  if (temp == NULL) {                                  \
    printf("\n");                                      \
    printf("  ERROR: Cannot allocate memory!");        \
    printf("\n");                                      \
    printf("\n");                                      \
    exit(-2);                                          \
  }                                                    \
                                                       \
  /* Generate data */                                  \
  for(int i = 0; i < nelems; i++) {                    \
    temp[i] = rand() % (0x1llu << (sizeof(type) * 8)); \
  }                                                    \
  temp;                                                \
})
#endif

#define __SET_GUARD(array, sz) {                       \
  ((byte*)array)[sz + 0] = 0xfe;                       \
  ((byte*)array)[sz + 1] = 0xca;                       \
  ((byte*)array)[sz + 2] = 0xad;                       \
  ((byte*)array)[sz + 3] = 0xde;                       \
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

#define __CHECK_FLOAT_MATCH(ref, array, sz, delta) ({  \
  bool __tmp = true;                                   \
                                                       \
  for(int i = 0; (i < sz) && __tmp; i++) {             \
    __tmp = __tmp && (fabs(ref[i] - array[i]) < delta);\
  }                                                    \
                                                       \
  __tmp;                                               \
})

#define __CHECK_GUARD(array, sz) ({                    \
  bool match = true;                                   \
                                                       \
  match = match && (((byte*)array)[sz + 0] == 0xfe);   \
  match = match && (((byte*)array)[sz + 1] == 0xca);   \
  match = match && (((byte*)array)[sz + 2] == 0xad);   \
  match = match && (((byte*)array)[sz + 3] == 0xde);   \
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
  bool* runtimes_mask;                                 \
                                                       \
  runtimes = (uint64_t*)calloc(num_runs,               \
                                 sizeof(uint64_t));    \
                                                       \
  runtimes_mask = (bool*)calloc(num_runs,              \
                                     sizeof(bool));    \
                                                       \
  /* Constants for statistical analysis */             \
  const unsigned int nstd = _num_stdev;

#define __DESTROY_STATS()                              \
  free(runtimes);                                      \
  free(runtimes_mask);

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
