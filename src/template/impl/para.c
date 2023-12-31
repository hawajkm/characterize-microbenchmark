/* para.c
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

/* If we are on Darwin, include the compatibility header */
#if defined(__APPLE__)
#include "common/mach_pthread_compatibility.h"
#endif

/* Include application-specific headers */
#include "include/types.h"

/* Alternative Implementation */
void* impl_parallel(void* args)
{
  return NULL;
}
