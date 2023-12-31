/* opt.c
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

/* Alternative Implementation */
#pragma GCC push_options
#pragma GCC optimize ("O1")
__attribute__ ((optimize(1)))
void* impl_scalar_opt(void* args)
{
  /* Get the argument struct */
  args_t* parsed_args = (args_t*)args;

  /* Get all the arguments */
  register       int*   dest = (      int*)(parsed_args->output);
  register const int*   src0 = (const int*)(parsed_args->input0);
  register const int*   src1 = (const int*)(parsed_args->input1);
  register       size_t size =              parsed_args->size / 4;

  register       size_t sz_8 = size / 8;

  switch (size % 8) {
    case 7:  *(dest++) = *(src0++) + *(src1++);
    case 6:  *(dest++) = *(src0++) + *(src1++);
    case 5:  *(dest++) = *(src0++) + *(src1++);
    case 4:  *(dest++) = *(src0++) + *(src1++);
    case 3:  *(dest++) = *(src0++) + *(src1++);
    case 2:  *(dest++) = *(src0++) + *(src1++);
    case 1:  *(dest++) = *(src0++) + *(src1++);
    case 0:  break;
  }


  while (sz_8 > 0) {
    dest[0] = src0[0] + src1[0];
    dest[1] = src0[1] + src1[1];
    dest[2] = src0[2] + src1[2];
    dest[3] = src0[3] + src1[3];
    dest[4] = src0[4] + src1[4];
    dest[5] = src0[5] + src1[5];
    dest[6] = src0[6] + src1[6];
    dest[7] = src0[7] + src1[7];

    dest += 8;
    src0 += 8;
    src1 += 8;

    --sz_8;
  }

  /* Done */
  return NULL;
}
#pragma GCC pop_options
