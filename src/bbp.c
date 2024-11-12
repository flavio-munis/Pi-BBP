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
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include "../include/bbp.h"
#include "../include/timer.h"


/*-----------------------------------------------------------------
                            Definitions
-----------------------------------------------------------------*/
#define PRECISION 10     // Number of Digits after Starting Position
#define EPSILON 1e-17    // Epsilon For Floating Point Precision
#define TOTAL_ACC 15     // Total Accumulators
//#define DEBUG            // If Code is In Debug Mode


/*-----------------------------------------------------------------
                          Global Variables
-----------------------------------------------------------------*/
Algorithm algoInUse;
uint64_t upperBound;
long double (*leftSum) (uint64_t);     // Wrapper For Left Summation Function
long double (*rightSum)();             // Wrapper For Right Summation Function

// Upperbounds For Bellard Formula Terms
int64_t upperBoundNeg1, upperBoundNeg6, upperBoundNeg4, upperBound0, upperBound2;


uint16_t activeThreads;                     // Threads Used
uint64_t d;                                  // Starting Position

// Number of Elements Each Thread Will Work Per Interation
uint64_t batchSize = 100;


pthread_mutex_t counterMutex, accIndexMutex;
uint64_t count = 0;
long double acc[TOTAL_ACC] = {0};
pthread_mutex_t accMutex[TOTAL_ACC];
int accIndex = 0;

MyTimer* total = NULL; 

/*-----------------------------------------------------------------
                   Internal Functions Signatures
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/**
   @brief  Execute BBP Algo Starting at d up to n Digits.
   @param  uint64_t   Starting Digit.
   @return long double Fractional Part Containing The Result.
*/
/*-----------------------------------------------------------------*/
long double bbpAlgo();


/*-----------------------------------------------------------------*/
/**
   @brief  Modular Exponentiation Algorithm (b^x mod n).
   @param  uint64_t Exponentiation base (b).
   @param  uint64_t Exponent (x).
   @param  uint64_t Modular Base (n).
   @return uint64_t Result of modular exp.
*/
/*-----------------------------------------------------------------*/
uint64_t modPow(uint64_t, uint64_t, uint64_t);


/*-----------------------------------------------------------------*/
/**
   @brief Print Result of BBP Algo (Base 16).
   @param long double Fraction Returned by bbpAlgo().
*/
/*-----------------------------------------------------------------*/
void ihex(long double);


/*-----------------------------------------------------------------*/
/**
   @brief Init/Destroy All Threads and Mutexes.
*/
/*-----------------------------------------------------------------*/
void initThreads();


/*-----------------------------------------------------------------*/
/**
   @brief  Thread Function That Calculate BBP Left Summation at
           BatchSize Elements Per Iteration.
   @param  void* Null Pointer.
   @return void* Null Pointer.
*/
/*-----------------------------------------------------------------*/
void* thPool(void*);


/*-----------------------------------------------------------------*/
/**
   @brief  Left Summation For Original Formula (4-Terms). Calculates
           Sum from k to k + batchSize (or to d).
   @param  int         j Value used in Summation, Different For
                       Each Term.
   @param  uint64_t    Current Starting Position (k).
   @return long double Result of Summation.
*/
/*-----------------------------------------------------------------*/
long double lhs(int, uint64_t);


/*-----------------------------------------------------------------*/
/**
   @brief  Right Summation For Original Formula (4-Terms). Calculates
           Sum from d until values are insignificant (< EPSILON).
   @param  int         j Value used in Summation, Different For
                       Each Term.
   @return long double Result of Summation.
*/
/*-----------------------------------------------------------------*/
long double rhs(int);


/*-----------------------------------------------------------------*/
/**
   @brief  Calculate Left Summation from k to k + batchSize for Every
           Term in the Original Formula (4-Term).
   @param  uint64_t    Current Starting Position (k).
   @return long double Result of Summation.
*/
/*-----------------------------------------------------------------*/
long double bbpAlgoOriginalLfS(uint64_t);


/*-----------------------------------------------------------------*/
/**
   @brief  Calculate Right Summation, for every Term in The Original
           Formula (4-Terms) from d until value are
		   insignificant (< EPSILON).
   @return long double Result of Summation.
*/
/*-----------------------------------------------------------------*/
long double bbpAlgoOriginalRfS();


/*-----------------------------------------------------------------*/
/**
   @brief  Left Summation For Bellard Formula (7-Terms). Calculates
           Sum from k to k + batchSize (or to upperBoundn). 
		   Uses S(m, j, l) notation.
   @param  int         m Value used in Summation.
   @param  int         j Value used in Summation.
   @param  int         l Value used in Summation.
   @param  uint64_t    Current Starting Position (k).
   @param  uint64_t    Upper Bound For Current Term.
   @return long double Result of Summation.
*/
/*-----------------------------------------------------------------*/
long double lhsBell(int, int, int, uint64_t, int64_t);


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
   @brief  Left Summation For Bellard Formula (7-Terms). Calculates
           Sum from k to k + batchSize (or to upperBoundn) For Every 
		   Term.
   @param  uint64_t    Current Starting Position (k).
   @return long double Result of Summation.
*/
/*-----------------------------------------------------------------*/
long double bellardLfS(uint64_t);


/*-----------------------------------------------------------------*/
/**
   @brief  Right Summation For Bellard Formula (7-Terms). Calculates
           Sum, for every term, from upperBoundn until value are
		   insignificant (< EPSILON).
   @return long double Result of Summation.
*/
/*-----------------------------------------------------------------*/
long double bellardRfs();


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


/*-----------------------------------------------------------------*/
/**
   @brief Reset Global Variables.
*/
/*-----------------------------------------------------------------*/
void resetVariables();

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


long double lhs(int j, uint64_t s) {

	long double r, sum = 0.0L, mult = -1, temp;
	uint64_t loopLimit = s + batchSize;

	if (j == 1)
		mult = 4;
	else if (j == 4)
		mult = -2;

	if (loopLimit > upperBound)
		loopLimit = upperBound;

	for (uint64_t k = s; k < loopLimit; k++) {
		r = 8.0L * k + j;
		temp = modPowBarret(16, upperBound - k, r);
		sum += (mult * temp) / r;
	    sum = fmodl(sum, 1.0L);
	}

	return sum;
}

long double rhs(int j) {
	
	long double sum = 0.0L, temp, r;
    long double mult = -1.0;

	if (j == 1)
		mult = 4.0L;
	else if (j == 4)
		mult = -2.0L;

	for (uint64_t k = upperBound; k <= upperBound + 100; k++) {
		r = 8.0L*k + j;
		temp = powl(16.0L, (long double) upperBound - k) / r;
		
		if (temp < EPSILON)
			break;

		sum += mult * temp;
	    sum = fmodl(sum, 1.0L);
	}
	
	return sum;
}

void* thPool(void* arg) {
  
	while (true) {
		uint64_t localCount;
		int localIndex;
      
		pthread_mutex_lock(&counterMutex);
		if (count >= upperBound) {
			pthread_mutex_unlock(&counterMutex);
			break;
		}

		localCount = count;
		count += batchSize;
                
		pthread_mutex_unlock(&counterMutex);

		pthread_mutex_lock(&accIndexMutex);
		localIndex = accIndex;
		accIndex = (accIndex + 1) % TOTAL_ACC;
		pthread_mutex_unlock(&accIndexMutex);
                
		pthread_mutex_lock(accMutex + localIndex);
		acc[localIndex] += leftSum(localCount);	
		pthread_mutex_unlock(accMutex + localIndex);
	}
	
	return NULL;
}

void initThreads() {

	pthread_t producers[activeThreads];
  
	pthread_mutex_init(&counterMutex, NULL);
	pthread_mutex_init(&accIndexMutex, NULL);
        
	for (int i = 0; i < TOTAL_ACC; i++)
		pthread_mutex_init(accMutex + i, NULL);	
			    
	// Produce Threads
    for (int i = 0; i < activeThreads; i++) {
		if (pthread_create(producers + i, NULL, &thPool, NULL) != 0) {
			unexpectedError("Error Creating Threads!");
		}
	}

	// Join Threads
	for (int i = 0; i < activeThreads; i++) {
		if (pthread_join(producers[i], NULL) != 0) {
			unexpectedError("Error Joining Threads!");
		}
	}

	pthread_mutex_destroy(&counterMutex);
	pthread_mutex_destroy(&accIndexMutex);
        
	for (int i = 0; i < TOTAL_ACC; i++)
		pthread_mutex_destroy(accMutex + i);
}


long double bbpAlgoOriginalLfS(uint64_t s) {

	long double result;

	result = lhs(1, s);
	result += lhs(4, s);
	result += lhs(5, s);
	result += lhs(6, s);

	return result;
}

long double bbpAlgoOriginalRfS() {

    long double result;

    result = rhs(1);
	result = fmodl(result, 1.0L);
	result += rhs(4);
    result = fmodl(result, 1.0L);
	result += rhs(5);
	result = fmodl(result, 1.0L);
    result += rhs(6);
	result = fmodl(result, 1.0L);
        
	return result;
}


long double lhsBell(int m, int j, int l, uint64_t s, int64_t upperBoundl) {

	long double r, sum = 0, sign, temp;
	int64_t loopLimit = s + batchSize;

	if (s >= upperBoundl)
		return 0;
        
	if (loopLimit > upperBoundl)
		loopLimit = upperBoundl;

	for (uint64_t k = s; k < loopLimit; k++) {
		sign = (k % 2) ? -1 : 1;
		r = m * k + j;
		temp = modPowBarret(2, 4*d + l - 10*k, r);
		sum += sign * (temp / r);
	    sum = fmodl(sum, 1.0L);
	}

	return sum;
}

long double bellardLfS(uint64_t s) {

	long double result = 0;

    result -= lhsBell(4, 1, -1, s, upperBoundNeg1);
	result -= lhsBell(4, 3, -6, s, upperBoundNeg6);
	result += lhsBell(10, 1, 2, s, upperBound2);
	result -= lhsBell(10, 3, 0, s, upperBound0);
	result -= lhsBell(10, 5, -4, s, upperBoundNeg4);
	result -= lhsBell(10, 7, -4, s, upperBoundNeg4);
	result += lhsBell(10, 9, -6, s, upperBoundNeg6);

	fmodl(result, 1.0);
        
	return result;
}

long double rhsBell(int m, int j, int l, int64_t upperBoundl) {

	long double r, sum = 0, sign, temp, exp;

	for (uint64_t k = upperBoundl; k <= upperBoundl + 100; k++) {
		sign = (k % 2) ? -1 : 1;
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

long double bellardRfs() {

	long double result = 0;
	
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


long double bbpAlgo() { 

	long double result = 0;

	initThreads();
        
	for (int i = 0; i < TOTAL_ACC; i++)
		result += acc[i];

	result += rightSum();
	fmodl(result, 1.0L);	
        
	return result;
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
  
	switch (algoInUse) {

        case BBP_ORIGINAL:
			leftSum = bbpAlgoOriginalLfS;
			rightSum = bbpAlgoOriginalRfS;
			upperBound = d;
			break;

        case BELLARD:
			leftSum = bellardLfS;
			rightSum = bellardRfs;

			helper = 4 * d;

			upperBound0 = (int64_t) helper / 10;
			upperBound2 = (int64_t) (helper + 2) / 10;
			upperBoundNeg1 = (int64_t )(helper - 1) / 10;
            upperBoundNeg4 = (int64_t) (helper - 4) / 10;
            upperBoundNeg6 = (int64_t) (helper - 6) / 10;
			upperBound = upperBound2;
            break;
	}
  
	if (upperBound < batchSize)
		batchSize = upperBound;
}


void resetVariables() {

	for (int i = 0; i < TOTAL_ACC; i++) {
        acc[i] = 0;
    }

	total = NULL;
    
    accIndex = 0;
    count = 0;
}


void calcBBP(Config* currConfigs) {

	long double result;

	resetVariables();
        
	d = currConfigs -> startPos;
    activeThreads = currConfigs -> nthreads;
    algoInUse = currConfigs->algo;
    configAlgorithm();
    
#ifdef DEBUG
    printf("\nStarting Position: %ld\n", d);
	printf("Threads: %d\n", activeThreads);
	printf("User Algo: %s\n\n", (algoInUse == BBP_ORIGINAL) ? "BBP-Original" : "Bellard");
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
}
