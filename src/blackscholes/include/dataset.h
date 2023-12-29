/* dataset.h
 *
 * Author: Khalid Al-Hawaj
 * Date  : 28 Dec. 2023
 *
 * This file helps with generation of different datasets.
 * The defined function will take as an input the size of the dataset.
 * Then, the function will replicate a set of pre-calculated dataset in
 * optionData.txt over and over again.
 */

#ifndef __INCLUDE_DATASET_H_
#define __INCLUDE_DATASET_H_

#define __dataset_name(x) ((x == 0? "test"  : \
                           (x == 1? "dev"   : \
                           (x == 2? "small" : \
                           (x == 3? "medium": \
                           (x == 4? "large" : \
                           (x == 5? "native": \
                                    "unknown" )))))))

/* Struct for the optionData.txt dataset
 * ref: PARSEC v3.0
 */
typedef struct _optionData_t {
  float sptPrice;
  float strike;
  float rate;
  float divq;
  float volatility;
  float otime;

  char  otype;
  float divs;
  float price;
} optionData_t;

optionData_t refDataSet[] = {
  #include "dataset/optionData.txt"
};

const int REF_DATASET_SIZE = sizeof(refDataSet) / sizeof(optionData_t);

void genDataset(args_t* args) {
  /* Get all needed pointers */
  size_t num_stocks = args->num_stocks;

  float* sptPrice   = args->sptPrice  ;
  float* strike     = args->strike    ;
  float* rate       = args->rate      ;
  float* volatility = args->volatility;
  float* otime      = args->otime     ;
  char * otype      = args->otype     ;
  float* ref        = args->output    ;

  /* Copy the data from the reference dataset */
  for (size_t i = 0; i < num_stocks; i++) {
    size_t ref_i = i % REF_DATASET_SIZE;

    sptPrice[i]   = refDataSet[ref_i].sptPrice;
    strike[i]     = refDataSet[ref_i].strike;
    rate[i]       = refDataSet[ref_i].rate;
    volatility[i] = refDataSet[ref_i].volatility;
    otime[i]      = refDataSet[ref_i].otime;
    otype[i]      = refDataSet[ref_i].otype;

    ref[i]        = refDataSet[ref_i].price;
  }
}

#endif //__INCLUDE_DATASET_H_

