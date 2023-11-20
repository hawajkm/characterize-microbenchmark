/* main.c
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

/* Set features         */
#define _GNU_SOURCE

/* Standard C includes  */
/*  -> Standard Library */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/*  -> Scheduling       */
#include <sched.h>
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

const int SIZE_DATA = 4 * 1024 * 1024;

int main()
{
  /* Set our priority the highest */
  int nice_level = -20;

  printf("Setting up schedulers and affinity:\n");
  printf("  * Setting the niceness level:\n");
  do {
    errno = 0;
    printf("      -> trying niceness level = %d\n", nice_level);
    int ret = nice(nice_level);
  } while (errno != 0 && nice_level++);

  printf("    + Process has niceness level = %d\n", nice_level);

  /* Set scheduling to reduce context switching */
  /*    -> Set scheduling scheme                */
  printf("  * Setting up FIFO scheduling scheme and high priority ... ");
  pid_t pid    = 0;
  int   policy = SCHED_FIFO;
  struct sched_param param;

  param.sched_priority = sched_get_priority_max(policy);
  int res = sched_setscheduler(pid, policy, &param);
  if (res != 0) {
    printf("Failed\n");
  } else {
    printf("Succeeded\n");
  }

  /*    -> Set affinity                         */
  printf("  * Setting up scheduling affinity ... ");
  cpu_set_t cpumask;

  CPU_ZERO(&cpumask);
  CPU_SET(6, &cpumask);

  res = sched_setaffinity(pid, sizeof(cpumask), &cpumask);

  if (res != 0) {
    printf("Failed\n");
  } else {
    printf("Succeeded\n");
  }
  printf("\n");

  /* Statistics */
  __DECLARE_STATS(1024, 3);

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
    for (int j = 0; j < 16; j++) {
      impl_scalar_naive(dest0, src, SIZE_DATA);
    }
    __SET_END_TIME();
    runtimes[i] = __CALC_RUNTIME() / 16;
  }
  printf("Finished\n");

  /* Dump */
  FILE * fp_naive;
  fp_naive = fopen("naive_runtimes.csv", "w");

  for (int i = 0; i < num_runs; i++) {
    if (i != 0) fprintf(fp_naive, ", ");
    fprintf(fp_naive, "%d", runtimes[i]);
  }

  fclose(fp_naive);

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
  uint64_t naive_min     = -1;
  uint64_t naive_max     =  0;

  uint64_t naive_avg     =  0;
  uint64_t naive_avg_n   =  0;

  uint64_t naive_std     =  0;
  uint64_t naive_std_n   =  0;

  int      naive_n_msked =  0;
  int      naive_n_stats =  0;

  for (int i = 0; i < num_runs; i++)
    runtimes_mask[i] = true;

  printf("  * Running statistics:\n");
  do {
    naive_n_stats++;
    printf("    + Starting statistics run number #%d:\n", naive_n_stats);
    naive_avg_n =  0;
    naive_avg   =  0;

    /*   -> Calculate min, max, and avg */
    for (int i = 0; i < num_runs; i++) {
      if (runtimes_mask[i]) {
        if (runtimes[i] < naive_min) {
          naive_min = runtimes[i];
        }
        if (runtimes[i] > naive_max) {
          naive_max = runtimes[i];
        }
        naive_avg += runtimes[i];
        naive_avg_n += 1;
      }
    }
    naive_avg = naive_avg / naive_avg_n;

    /*   -> Calculate standard deviation */
    naive_std   =  0;
    naive_std_n =  0;

    for (int i = 0; i < num_runs; i++) {
      if (runtimes_mask[i]) {
        naive_std   += ((runtimes[i] - naive_avg) *
                        (runtimes[i] - naive_avg));
        naive_std_n += 1;
      }
    }
    naive_std = sqrt(naive_std / naive_std_n);

    /*   -> Calculate outlier-free average (mean) */
    naive_n_msked = 0;
    for (int i = 0; i < num_runs; i++) {
      if (runtimes_mask[i]) {
        if (runtimes[i] > naive_avg) {
          if ((runtimes[i] - naive_avg) > (nstd * naive_std)) {
            runtimes_mask[i] = false;
            naive_n_msked += 1;
          }
        } else {
          if ((naive_avg - runtimes[i]) > (nstd * naive_std)) {
            runtimes_mask[i] = false;
            naive_n_msked += 1;
          }
        }
      }
    }

    printf("      - Standard deviation = %" PRIu64 "\n", naive_std);
    printf("      - Average = %" PRIu64 "\n", naive_avg);
    printf("      - Number of active elements = %d\n", naive_avg_n);
    printf("      - Number of masked-off = %d\n", naive_n_msked);
  } while (naive_n_msked > 0);
  printf("\n");

  /* Optimized algorithm */
  printf("Running Scalar \"Optimized\" implementation:\n");

  printf("  * Invoking the implementation %d times .... ", num_runs);
  sched_yield();
  for (int i = 0; i < num_runs; i++) {
    __SET_START_TIME();
    for (int j = 0; j < 16; j++) {
      impl_scalar_opt(dest1, src, SIZE_DATA);
    }
    __SET_END_TIME();
    runtimes[i] = __CALC_RUNTIME() / 16;
  }
  printf("Finished\n");

  /* Dump */
  FILE * fp_opt;
  fp_opt = fopen("opt_runtimes.csv", "w");

  for (int i = 0; i < num_runs; i++) {
    if (i != 0) fprintf(fp_opt, ", ");
    fprintf(fp_opt, "%d", runtimes[i]);
  }

  fclose(fp_opt);

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
  uint64_t opt_min     = -1;
  uint64_t opt_max     =  0;

  uint64_t opt_avg     =  0;
  uint64_t opt_avg_n   =  0;

  uint64_t opt_std     =  0;
  uint64_t opt_std_n   =  0;

  int      opt_n_msked =  0;
  int      opt_n_stats =  0;

  for (int i = 0; i < num_runs; i++)
    runtimes_mask[i] = true;

  printf("  * Running statistics:\n");
  do {
    opt_n_stats++;
    printf("    + Starting statistics run number #%d:\n", opt_n_stats);
    opt_avg_n =  0;
    opt_avg   =  0;

    /*   -> Calculate min, max, and avg */
    for (int i = 0; i < num_runs; i++) {
      if (runtimes_mask[i]) {
        if (runtimes[i] < opt_min) {
          opt_min = runtimes[i];
        }
        if (runtimes[i] > opt_max) {
          opt_max = runtimes[i];
        }
        opt_avg += runtimes[i];
        opt_avg_n += 1;
      }
    }
    opt_avg = opt_avg / opt_avg_n;

    /*   -> Calculate standard deviation */
    opt_std   =  0;
    opt_std_n =  0;

    for (int i = 0; i < num_runs; i++) {
      if (runtimes_mask[i]) {
        opt_std   += ((runtimes[i] - opt_avg) *
                        (runtimes[i] - opt_avg));
        opt_std_n += 1;
      }
    }
    opt_std = sqrt(opt_std / opt_std_n);

    /*   -> Calculate outlier-free average (mean) */
    opt_n_msked = 0;
    for (int i = 0; i < num_runs; i++) {
      if (runtimes_mask[i]) {
        if (runtimes[i] > opt_avg) {
          if ((runtimes[i] - opt_avg) > (nstd * opt_std)) {
            runtimes_mask[i] = false;
            opt_n_msked += 1;
          }
        } else {
          if ((opt_avg - runtimes[i]) > (nstd * opt_std)) {
            runtimes_mask[i] = false;
            opt_n_msked += 1;
          }
        }
      }
    }

    printf("      - Standard deviation = %" PRIu64 "\n", opt_std);
    printf("      - Average = %" PRIu64 "\n", opt_avg);
    printf("      - Number of active elements = %d\n", opt_avg_n);
    printf("      - Number of masked-off = %d\n", opt_n_msked);
  } while (opt_n_msked > 0);
  printf("\n");

  /* Manage memory */
  free(src);
  free(dest0);
  free(dest1);
  free(ref);

  /* Finished with statistics */
  __DESTROY_STATS();

  /* Display information */
  printf("Runtimes:\n");
  printf("  * Naive      scalar (%s):", __PRINT_MATCH(naive_match));
  printf(" %" PRIu64 " ns\n"          , naive_avg                 );
  printf("  * Optimized  scalar (%s):", __PRINT_MATCH(opt_match)  );
  printf(" %" PRIu64 " ns\n"          , opt_avg                   );
  printf("\n");
  printf("      -> Optimized  speedup = %.2fx\n", ((naive_avg * 1.0f) / opt_avg));
  printf("\n");

  return 0;
}
