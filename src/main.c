/* naive.c
 *
 * Author: Khalid Al-Hawaj
 * Date  : 12 Nov. 2023
 *
 * This file is structured to call different implementation of the same
 * algorithm/microbenchmark. The file will allocate 3 output arrays one
 * for: scalar naive impl, scalar opt impl, vectorized impl. As it stands
 * the file will allocate and initialize with random data one input array
 * of type 'byte'. To check correctness, the file allocate a 'ref' array;
 * to calculate this 'ref' array, the file will invoke a ref_impl, which
 * is supposed to be functionally correct and act as a reference for
 * the functionality. The file also adds a guard word at the end of the
 * output arrays to check for buffer overruns.
 *
 * The file will invoke each implementation n number of times. It will
 * record the runtime of _each_ invocation through the following Linux
 * API:
 *    clock_gettime(), with the clk_id set to CLOCK_MONOTONIC
 * Then, the file will calculate the standard deviation and calculate
 * an outlier-free average by excluding runtimes that are larger than
 * 2 standard deviation of the original average.
 */

/* Standard C includes  */
/*  -> Standard Library */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/*  -> Types            */
#include <stdbool.h>
#include <inttypes.h>
/*  -> Runtimes         */
#include <time.h>
#include <unistd.h>
#include <errno.h>

/* Include all implementations declarations */
#include "impl/ref.h"
#include "impl/naive.h"
#include "impl/opt.h"

/* Include macros.h */
#include "common/types.h"
#include "common/macros.h"

const int SIZE_DATA = 16 * 1024 * 1024;

int main()
{
  /* Set our priority the highest */
  int nice_level = -20;

  printf("* Setting the niceness level:\n");

  do {
    errno = 0;
    printf("  -> trying niceness level = %d\n", nice_level);
    int ret = nice(nice_level);
  } while (errno != 0 && nice_level++);

  printf("  Process has niceness level = %d\n", nice_level);
  printf("\n\n");

  /* Statistics */
  __DECLARE_STATS(200, 2);

  /* Initialize Rand */
  srand(0xdeadbeef);

  /* Datasets */
  /* Allocation and initialization */
  byte* src   = __ALLOC_INIT_DATA(byte, SIZE_DATA + 0);
  byte* ref   = __ALLOC_INIT_DATA(byte, SIZE_DATA + 4);
  byte* dest0 = __ALLOC_DATA     (byte, SIZE_DATA + 4);
  byte* dest1 = __ALLOC_DATA     (byte, SIZE_DATA + 4);

  /* Setting a guards, which is 0xdeadcafe.
     The guard should not change or be touched. */
  __SET_GUARD(ref  , SIZE_DATA);
  __SET_GUARD(dest0, SIZE_DATA);
  __SET_GUARD(dest1, SIZE_DATA);

  /* Generate ref data */
  impl_ref(ref, src, SIZE_DATA);

  /* Naive algorithm */
  printf("Running Scalar \"Naive\" implementation:\n");

  printf("  * Invoking the implementation %d times .... ", num_runs);
  for (int i = 0; i < num_runs; i++) {
    __SET_START_TIME();
    impl_scalar_naive(dest0, src, SIZE_DATA);
    __SET_END_TIME();

    runtimes[i] = __CALC_RUNTIME();
  }
  printf("Finished\n");

  /* Verfication */
  printf("  * Verifying results .... ");
  bool naive_match = __CHECK_MATCH(ref, dest0, SIZE_DATA);
  bool naive_guard = __CHECK_GUARD(     dest0, SIZE_DATA);
  if (naive_match && naive_guard) {
    printf("Success\n");
  } else if (!naive_match && naive_guard) {
    printf("Fail, but no buffer overruns\n");
  } else if (naive_match && !naive_guard) {
    printf("Success, but failed buffer overruns check\n");
  } else if(!naive_match && !naive_guard) {
    printf("Failed, and failed buffer overruns check\n");
  }

  /* Running analytics */
  /*   -> Calculate min, max, and avg */
  uint64_t naive_min = -1;
  uint64_t naive_max =  0;
  uint64_t naive_avg =  0;
  for (int i = 0; i < num_runs; i++) {
    if (runtimes[i] < naive_min) {
      naive_min = runtimes[i];
    }
    if (runtimes[i] > naive_max) {
      naive_max = runtimes[i];
    }
    naive_avg += runtimes[i];
  }
  naive_avg = naive_avg / num_runs;

  /*   -> Calculate standard deviation */
  uint64_t naive_std =  0;
  for (int i = 0; i < num_runs; i++) {
    naive_std += (runtimes[i] - naive_avg) * (runtimes[i] - naive_avg);
  }
  naive_std = naive_std / num_runs;

  /*   -> Calculate outlier-free average (mean) */
  uint64_t naive_mean   = 0;
  uint64_t naive_mean_n = 0;
  for (int i = 0; i < num_runs; i++) {
    if (runtimes[i] > naive_avg) {
      if ((runtimes[i] - naive_avg) <= (nstd * naive_std)) {
        naive_mean += runtimes[i];
        naive_mean_n++;
      }
    } else {
      if ((naive_avg - runtimes[i]) <= (nstd * naive_std)) {
        naive_mean += runtimes[i];
        naive_mean_n++;
      }
    }
  }
  naive_mean = naive_mean / naive_mean_n;
  printf("\n");

  /* Optimized algorithm */
  printf("Running Scalar \"Optimized\" implementation:\n");

  printf("  * Invoking the implementation %d times .... ", num_runs);
  for (int i = 0; i < num_runs; i++) {
    __SET_START_TIME();
    impl_scalar_opt(dest1, src, SIZE_DATA);
    __SET_END_TIME();

    runtimes[i] = __CALC_RUNTIME();
  }
  printf("Finished\n");

  /* Verification */
  printf("  * Verifying results .... ");
  bool opt_match = __CHECK_MATCH(ref, dest1, SIZE_DATA);
  bool opt_guard = __CHECK_GUARD(     dest1, SIZE_DATA);
  if (opt_match && opt_guard) {
    printf("Success\n");
  } else if (!opt_match && opt_guard) {
    printf("Fail, but no buffer overruns\n");
  } else if (opt_match && !opt_guard) {
    printf("Success, but failed buffer overruns check\n");
  } else if(!opt_match && !opt_guard) {
    printf("Failed, and failed buffer overruns check\n");
  }

  /* Running analytics */
  /*   -> Calculate min, max, and avg */
  uint64_t opt_min = -1;
  uint64_t opt_max =  0;
  uint64_t opt_avg =  0;
  for (int i = 0; i < num_runs; i++) {
    if (runtimes[i] < opt_min) {
      opt_min = runtimes[i];
    }
    if (runtimes[i] > opt_max) {
      opt_max = runtimes[i];
    }
    opt_avg += runtimes[i];
  }
  opt_avg = opt_avg / num_runs;

  /*   -> Calculate standard deviation */
  uint64_t opt_std =  0;
  for (int i = 0; i < num_runs; i++) {
    opt_std += (runtimes[i] - opt_avg) * (runtimes[i] - opt_avg);
  }
  opt_std = sqrt(opt_std / num_runs);

  /*   -> Calculate outlier-free average (mean) */
  uint64_t opt_mean   = 0;
  uint64_t opt_mean_n = 0;
  for (int i = 0; i < num_runs; i++) {
    if (runtimes[i] > opt_avg) {
      if ((runtimes[i] - opt_avg) <= (nstd * opt_std)) {
        opt_mean += runtimes[i];
        opt_mean_n++;
      }
    } else {
      if ((opt_avg - runtimes[i]) <= (nstd * opt_std)) {
        opt_mean += runtimes[i];
        opt_mean_n++;
      }
    }
  }
  opt_mean = opt_mean / opt_mean_n;

  /* Display information */
  printf("\n\n");
  printf("Runtimes:\n");
  printf("  * Basic scalar (%s):", __PRINT_MATCH(naive_match));
  printf(" %" PRIu64 " ns\n"     , naive_mean                );
  printf("  * Opt   scalar (%s):", __PRINT_MATCH(opt_match)  );
  printf(" %" PRIu64 " ns\n"     , opt_mean                  );
  printf("\n");
  printf("      -> Speedup = %.2fx\n", ((naive_mean * 1.0f) / opt_mean));
  printf("\n");

  return 0;
}
