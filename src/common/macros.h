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

#endif //__COMMON_MACROS_H_
