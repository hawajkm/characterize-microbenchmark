/* vec.c
 *
 * Author:
 * Date  :
 *
 *  Description
 */

/* Standard C includes  */
#include <stdlib.h>
#include <math.h>
/*  -> SIMD header file  */
#if defined(__amd64__) || defined(__x86_64__)
#include <immintrin.h>
#elif defined(__aarch__) || defined(__aarch64__) || defined(__arm64__)
#include <arm_neon.h>
#endif

/* Include common headers */
#include "common/macros.h"
#include "common/types.h"
#include "common/vmath.h"

/* Include application-specific headers */
#include "include/types.h"

/* Alternative Implementation */
void* impl_vector(void* args)
{
#if defined(__amd64__) || defined(__x86_64__)
  /* Get the argument struct */
  args_t* parsed_args = (args_t*)args;

  /* Get all the arguments */
  register       int*   dest = (      int*)(parsed_args->output);
  register const int*   src0 = (const int*)(parsed_args->input0);
  register const int*   src1 = (const int*)(parsed_args->input1);
  register       size_t size =              parsed_args->size / 4;

  __m256i vm = _mm256_set1_epi32(0x80000000);
  const int max_vlen = 32 / sizeof(int);

  for (register size_t hw_vlen, i = 0; i < size; i += hw_vlen) {

    register int rem = size - i;
    hw_vlen = rem < max_vlen ? rem : max_vlen;        /* num of elems      */
    if (hw_vlen < max_vlen) {
      unsigned int m[max_vlen];
      for (size_t j = 0; j < max_vlen; j++)
        m[j] = (j < hw_vlen) ? 0x80000000 : 0x00000000;
      vm = _mm256_setr_epi32(m[0], m[1], m[2], m[3],
                             m[4], m[5], m[6], m[7]);
    }

    __m256i vec0 = _mm256_maskload_epi32(src0, vm);   /* Load vectors from */
    __m256i vec1 = _mm256_maskload_epi32(src1, vm);   /* src0 and src1     */

    __m256i res  = _mm256_add_epi32(vec0, vec1);      /* Do the compute    */

    _mm256_maskstore_epi32(dest, vm, res);            /* Store output      */

    src0 += hw_vlen;                                  /* -\                */
    src1 += hw_vlen;                                  /*   |-> ptr arith   */
    dest += hw_vlen;                                  /* -/                */
  }

  /* Done */
  return NULL;
#elif defined(__aarch__) || defined(__aarch64__) || defined(__arm64__)
#endif
}
