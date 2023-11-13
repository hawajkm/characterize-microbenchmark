#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

/* Include all implementations declarations */
#include "impl/naive.h"
#include "impl/opt.h"

/* Include macros.h */
#include "tools/macros.h"

const int SIZE_DATA = 16 * 1024 * 1024;

int main()
{
  // Set our priority the highest
  int nice_level = -20;

  printf("* Setting the niceness level:\n");

  do {
    errno = 0;
    printf("  -> trying niceness level = %d\n", nice_level);
    int ret = nice(nice_level);
  } while (errno != 0 && nice_level++);

  printf("  Process has niceness level = %d\n", nice_level);
  printf("\n\n");

  /* Datasets */
  unsigned char *src, *dest0, *dest1;

  /* Time keeping */
  struct timespec ts;
  struct timespec te;

  /* Iterate and average runtimes */
  unsigned int num_runs = 200;
  unsigned long long* runtimes;

  runtimes = (unsigned long long*)calloc(num_runs, sizeof(unsigned long long));

  /* Constants for statistical analysis */
  const unsigned int nstd = 2;

  /* Initialize Rand */
  srand(0xdeadbeef);

  /* Allocation */
  src   = (unsigned char*)calloc(SIZE_DATA + 0, 1);
  dest0 = (unsigned char*)calloc(SIZE_DATA + 4, 1);
  dest1 = (unsigned char*)calloc(SIZE_DATA + 4, 1);

  printf("Allocation data:\n");
  printf("    src   address is %p\n", src  );
  printf("    dest0 address is %p\n", dest0);
  printf("    dest1 address is %p\n", dest1);
  printf("\n");
  printf("\n");

  /* Generate data */
  for(int i = 0; i < SIZE_DATA; i++) {
    src[i] = rand() % 256;
  }

  /* Setting a guards, which is 0xdeadcafe.
     The guard should not change or be touched. */
  dest0[SIZE_DATA + 0] = 0xfe;
  dest0[SIZE_DATA + 1] = 0xca;
  dest0[SIZE_DATA + 2] = 0xad;
  dest0[SIZE_DATA + 3] = 0xde;

  dest1[SIZE_DATA + 0] = 0xfe;
  dest1[SIZE_DATA + 1] = 0xca;
  dest1[SIZE_DATA + 2] = 0xad;
  dest1[SIZE_DATA + 3] = 0xde;

  /* Make sure all PTEs are created. */
  printf("Running Scalar \"Naive\" implementation:\n");
  printf("  * Warming up all PTEs .... ");
  for(int i = 0; i < SIZE_DATA; i++) {
    dest0[i] = 0x00;
  }
  printf("Finished\n");

  /* Basic algorithm */
  printf("  * Invoking the implementation %d times .... ", num_runs);
  for (int i = 0; i < num_runs; i++) {
    __COMPILER_FENCE_;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
      printf("\n\n    ERROR: getting time failed!\n\n\n");
      exit(-1);
    }
    impl_scalar_naive(dest0, src, SIZE_DATA);
    if (clock_gettime(CLOCK_MONOTONIC, &te) != 0) {
      printf("\n\n    ERROR: getting time failed!\n\n\n");
      exit(-1);
    }
    __COMPILER_FENCE_;

    runtimes[i] = (te.tv_sec  - ts.tv_sec ) * 1e9 +
                  (te.tv_nsec - ts.tv_nsec)       ;
  }
  printf("Finished\n");

  /* Verfication */
  bool naive_match = true;

  naive_match = naive_match && (dest0[SIZE_DATA + 0] == 0xfe);
  naive_match = naive_match && (dest0[SIZE_DATA + 1] == 0xca);
  naive_match = naive_match && (dest0[SIZE_DATA + 2] == 0xad);
  naive_match = naive_match && (dest0[SIZE_DATA + 3] == 0xde);

  for(int i = 0; (i < SIZE_DATA) && naive_match; i++) {
    naive_match = naive_match && (src[i] == dest0[i]);
  }

  /* Running analytics */
  unsigned long long naive_min = -1;
  unsigned long long naive_max =  0;
  unsigned long long naive_avg =  0;
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

  unsigned long long naive_std =  0;
  for (int i = 0; i < num_runs; i++) {
    naive_std += (runtimes[i] - naive_avg) * (runtimes[i] - naive_avg);
  }
  naive_std = naive_std / num_runs;

  unsigned long long naive_mean   = 0;
  unsigned long long naive_mean_n = 0;
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

  /* Make sure all PTEs are created. */
  printf("\n");
  printf("Running Scalar \"Optimized\" implementation:\n");
  printf("  * Warming up all PTEs .... ");
  for(int i = 0; i < SIZE_DATA; i++) {
    dest1[i] = 0x00;
  }
  printf("Finished\n");

  printf("  * Invoking the implementation %d times .... ", num_runs);
  for (int i = 0; i < num_runs; i++) {
    __COMPILER_FENCE_;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
      printf("\n\n    ERROR: getting time failed!\n\n\n");
      exit(-1);
    }
    impl_scalar_opt(dest1, src, SIZE_DATA);
    if (clock_gettime(CLOCK_MONOTONIC, &te) == -1) {
      printf("\n\n    ERROR: getting time failed!\n\n\n");
      exit(-1);
    }
    __COMPILER_FENCE_;

    runtimes[i] = (te.tv_sec  - ts.tv_sec ) * 1e9 +
                  (te.tv_nsec - ts.tv_nsec)       ;
  }
  printf("Finished\n");

  /* Verification */
  bool opt_match = true;

  /* Guards */
  opt_match = opt_match && (dest1[SIZE_DATA + 0] == 0xfe);
  opt_match = opt_match && (dest1[SIZE_DATA + 1] == 0xca);
  opt_match = opt_match && (dest1[SIZE_DATA + 2] == 0xad);
  opt_match = opt_match && (dest1[SIZE_DATA + 3] == 0xde);

  for(int i = 0; (i < SIZE_DATA) && opt_match; i++) {
    opt_match = opt_match && (src[i] == dest1[i]);
  }

  /* Running analytics */
  unsigned long long opt_min = -1;
  unsigned long long opt_max =  0;
  unsigned long long opt_avg =  0;
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

  unsigned long long opt_std =  0;
  for (int i = 0; i < num_runs; i++) {
    opt_std += (runtimes[i] - opt_avg) * (runtimes[i] - opt_avg);
  }
  opt_std = sqrt(opt_std / num_runs);

  unsigned long long opt_mean   = 0;
  unsigned long long opt_mean_n = 0;
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
  printf("  * Basic scalar (%s): %lld ns\n", __PRINT_MATCH(naive_match), naive_mean);
  printf("  * Opt   scalar (%s): %lld ns\n", __PRINT_MATCH(opt_match)  , opt_mean  );
  printf("\n");
  printf("      -> Speedup = %.2fx\n", ((naive_mean * 1.0f) / opt_mean));
  printf("\n");

  return 0;
}
