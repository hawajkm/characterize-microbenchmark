/* macros.h
 *
 * Author: Khalid Al-Hawaj
 * Date  : 12 Nov. 2023
 *
 * File containing all required macros for characterizing-microbenchmark */

/* General */
#define __COMPILER_FENCE_ __asm__ __volatile__ ("" : : : "memory");

/* Specific */
#define __PRINT_MATCH(x) (x ? "   MATCH" : "NO MATCH")
