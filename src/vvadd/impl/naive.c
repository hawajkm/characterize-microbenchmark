/* naive.c
 *
 * Author:
 * Date  :
 *
 *  Description
 */

/* Standard C includes */
#include <stdlib.h>

/* Include common headers */
#include "common/macros.h"
#include "common/types.h"

/* Include application-specific headers */
#include "include/types.h"

/* Naive Implementation */
#pragma GCC push_options
#pragma GCC optimize ("O1")
__attribute__ ((optimize(1)))
void* impl_scalar_naive(void* args)
{
  /* Get the argument struct */
  args_t* parsed_args = (args_t*)args;

  /* Get all the arguments */
  register       int*   dest = (      int*)(parsed_args->output);
  register const int*   src0 = (const int*)(parsed_args->input0);
  register const int*   src1 = (const int*)(parsed_args->input1);
  register       size_t size =              parsed_args->size / 4;

  for (register int i = 0; i < size; i++) {
    dest[i] = src0[i] + src1[i];
  }

  /* Done */
  return NULL;
}
#pragma GCC pop_options
