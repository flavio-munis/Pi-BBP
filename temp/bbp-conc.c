/*-----------------------------------------------------------------*/
/**

  @file   bbp.c
  @author Flávio M.
  @brief  Implements BBP Formulas (4-Term Original, Bellard)
          Concurrently.
 */
/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------
                              Includes
  -----------------------------------------------------------------*/
#include <omp.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include "bbp.h"
#include "timer.h"


/*-----------------------------------------------------------------
                            Definitions
-----------------------------------------------------------------*/
#define PRECISION 10     // Number of Digits after Starting Position
#define EPSILON 1e-17    // Epsilon For Floating Point Precision
//#define DEBUG            // If Code is In Debug Mode


/*-----------------------------------------------------------------
                          Global Variables
-----------------------------------------------------------------*/
// Upperbounds For Bellard Formula Terms
int64_t upperBound0, upperBound2, upperBoundNeg1, upperBoundNeg4, upperBoundNeg6;
uint64_t d;                                  // Starting Position


MyTimer* total = NULL; 

/*-----------------------------------------------------------------
                   Internal Functions Signatures
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/**
   @brief Print Result of BBP Algo (Base 16).
   @param long double Fraction Returned by bbpAlgo().
*/
/*-----------------------------------------------------------------*/
void ihex(long double);


/*-----------------------------------------------------------------*/
/**
   @brief  Left Summation For Bellard Formula (7-Terms). Calculates
           Sum from k to k + batchSize (or to upperBoundn). 
		   Uses S(m, j, l) notation.
   @param  int         m Value used in Summation.
   @param  int         j Value used in Summation.
   @param  int         l Value used in Summation.
   @param  uint64_t    Upper Bound For Current Term.
   @return long double Result of Summation.
*/
/*-----------------------------------------------------------------*/
long double lhsBell(int, int, int, int64_t);


/*-----------------------------------------------------------------*/
/**
   @brief  Right Summation For Bellard Formula (7-Terms). Calculates
           Sum from upperBoundn until value are
		   insignificant (< EPSILON).
		   Uses S(m, j, l) notation.
   @param  int         m Value used in Summation.
   @param  int         j Value used in Summation.
   @param  int         l Value used in Summation.
   @param  uint64_t    Upper Bound For Current Term.
   @return long double Result of Summation.
*/
/*-----------------------------------------------------------------*/
long double rhsBell(int, int, int, int64_t);


/*-----------------------------------------------------------------*/
/**
   @brief  Implement Barret Reduction Algorithm.
   @param  __uint128_t a*b Calculate in modMul Function.
   @param  uint64_t    Base of Current Operation.
   @param  uint64_t    Factor Used For Reduction.
   @return uint64_t    n mod base.
*/
/*-----------------------------------------------------------------*/
uint64_t barretReduction(__uint128_t, uint64_t, uint64_t);


/*-----------------------------------------------------------------*/
/**
   @brief  Implements a Modular Multiplication.
   @param  uint64_t Number to Be Multiplied (a).
   @param  uint64_t Number to Be Multiplied (b).
   @param  uint64_t    Base of Current Operation.
   @param  uint64_t    Factor Used For Reduction.
   @return uint64_t    a*b mod base.
*/
/*-----------------------------------------------------------------*/
uint64_t modMul(uint64_t, uint64_t, uint64_t, uint64_t);


/*-----------------------------------------------------------------*/
/**
   @brief  Implements Barrett Modular Exponentiation Algorithm.
   @param  uint64_t Number (n).
   @param  uint64_t Exponent (exp).
   @param  uint64_t Base of Current Operation.
   @return uint64_t n^exp mod base.
*/
/*-----------------------------------------------------------------*/
uint64_t modPowBarret(uint64_t, uint64_t, uint64_t);


/*-----------------------------------------------------------------*/
/**
   @brief Config Variables and Function for Algorithm Selected.
   @param Algoritghm Current Algorithm.
*/
/*-----------------------------------------------------------------*/
void configAlgorithm();



/*-----------------------------------------------------------------
                      Functions Implementation
  -----------------------------------------------------------------*/

uint64_t barretReduction(__uint128_t n,
                         uint64_t base,
                         uint64_t factor) {

	uint64_t q = ((__uint128_t)n * factor) >> 64;
	q = n - ((__uint128_t)q * base);
        
	while (q >= base)
		q -= base;

	return q;
}

uint64_t modMul(uint64_t a,
                 uint64_t b,
                 uint64_t mod,
                 uint64_t factor) {
	__uint128_t product = (__uint128_t)a * b;
	return barretReduction(product, mod, factor);
}

uint64_t modPowBarret(uint64_t n,
					  uint64_t exp,
					  uint64_t base) {

	uint64_t result = 1;
    uint64_t factor = UINT64_MAX / base;
	
	while (exp) {

		if (exp & 1) {
		    result = modMul(result, n, base, factor);
		}

		n = modMul(n, n, base, factor);
                
		exp >>= 1;
	}

	return result;
}



long double lhsBell(int m, int j, int l, int64_t upperBoundl) {
    const int num_threads = omp_get_max_threads();
    long double* local_sums = (long double*)aligned_alloc(64, num_threads * sizeof(long double));
    memset(local_sums, 0, num_threads * sizeof(long double));
    
    // Pre-compute constants outside the loop
    const int d_times_4 = 4 * d;
    const int l_minus_10 = l - 10;
    
    #pragma omp parallel
    {
        const int thread_id = omp_get_thread_num();
        long double thread_sum = 0.0L;
        
        #pragma omp for schedule(static) nowait
        for (uint64_t k = 0; k < upperBoundl; k++) {
            // Optimize sign calculation using bitwise operation
            long double sign = 1.0L - 2.0L * (k & 1);
            long double r = m * k + j;
            
            // Combine exponent calculation
            long double temp = modPowBarret(2, d_times_4 + l_minus_10 + (-10 * k), r);
            thread_sum += sign * (temp / r);
            
            // Apply modulo less frequently
            if ((k & 0xFF) == 0) {  // Every 256 iterations
                thread_sum = fmodl(thread_sum, 1.0L);
            }
        }
        
        local_sums[thread_id] = fmodl(thread_sum, 1.0L);
    }
    
    // Sequential reduction to combine results
    long double final_sum = 0.0L;
    for (int i = 0; i < num_threads; i++) {
        final_sum += local_sums[i];
    }
    
    free(local_sums);
    return fmodl(final_sum, 1.0L);
}

long double rhsBell(int m, int j, int l, int64_t upperBoundl) {

	long double r, sum = 0, sign = 1LL, temp, exp;

	for (uint64_t k = upperBoundl; k <= upperBoundl + 100; k++) {
		sign = (k % 2) ? -1 : 1;
		//sign *= -1LL;
		r = m * k + j;
		exp = (long double) 4*d + l - 10* k;
		temp = powl(2.0, exp);
		temp = (temp / r) * sign;

		if (fabsl(temp) < EPSILON)
			break;

		sum += temp;
	}

	sum = fmodl(sum, 1.0L);		
        
	return sum;
}


void ihex (long double x) {
	int i;
	long double y;
	char hx[] = "0123456789ABCDEF";
	
	y = x;

	for (i = 0; i < PRECISION; i++){
		y = 16. * (y - floor (y));
		printf("%c", hx[(int) y]);
	}
}


void configAlgorithm() {

	int64_t helper = 4 * d;

	upperBound0 = (int64_t) helper / 10;
	upperBound2 = (int64_t) (helper + 2) / 10;
	upperBoundNeg1 = (int64_t )(helper - 1) / 10;
	upperBoundNeg4 = (int64_t) (helper - 4) / 10;
	upperBoundNeg6 = (int64_t) (helper - 6) / 10;
}


long double bbpAlgo() {

	long double result = 0;

	// Left Side
	result -= lhsBell(4, 1, -1, upperBoundNeg1);
	result -= lhsBell(4, 3, -6, upperBoundNeg6);
	result += lhsBell(10, 1, 2, upperBound2);
	result -= lhsBell(10, 3, 0, upperBound0);
	result -= lhsBell(10, 5, -4, upperBoundNeg4);
	result -= lhsBell(10, 7, -4, upperBoundNeg4);
	result += lhsBell(10, 9, -6, upperBoundNeg6);

	fmodl(result, 1.0);

	// Right Side
	result -= rhsBell(4, 1, -1, upperBoundNeg1);
	result -= rhsBell(4, 3, -6, upperBoundNeg6);
	result += rhsBell(10, 1, 2, upperBound2);
	result -= rhsBell(10, 3, 0, upperBound0);
	result -= rhsBell(10, 5, -4, upperBoundNeg4);
	result -= rhsBell(10, 7, -4, upperBoundNeg4);
	result += rhsBell(10, 9, -6, upperBoundNeg6);

	fmodl(result, 1.0L);

	return result;	
}


int main(int argc, char* argv[]) {

	long double result;
        
	d = strtoll(argv[1], NULL, 10);
    configAlgorithm();
    
#ifdef DEBUG
    printf("\nStarting Position: %ld\n", d);
#endif

	INIT_TIMER(total);
    
	result = bbpAlgo();
	printf("\n%d digits @ %ld = ", PRECISION, d);
	ihex(result);
	puts("");

	END_TIMER(total);
	CALC_FINAL_TIME(total);

    printf("Total Exec. Time: %.5fs\n", total -> totalTime);
    free(total);

	return 0;
}
