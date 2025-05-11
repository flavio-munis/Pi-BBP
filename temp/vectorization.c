#include <immintrin.h>
#include <gmp.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/timer.h"
#include "../include/error-handler.h"

#define ALIGN 64
#define TOTAL_ELEMENTS 54365436
#define VLEN 4


// Integer Types
#define i8 int8_t
#define i32 int32_t
#define u32 uint32_t
#define u64 uint64_t
#define u128 __uint128_t

u64 sumArray(u64* vec) {

	u64 sum = 0;

	for (i32 i = 0; i < TOTAL_ELEMENTS; i++)
		sum += vec[i];
          
	return sum;
}

// Expects a padded vector to avoid vectors where size mod 4 != 0
u64 sumArrayVec(u64* vec) {

	const u32 rem = TOTAL_ELEMENTS % 4;
	const u64 pad = (!rem) ? 0 : 4 - rem; 	
	const u32 size = (TOTAL_ELEMENTS + pad) >> 2;
        
	// Cast vec to 256-bit vector (4x64 Bit Per Element)
	__m256i* a = (__m256i*)vec;

	// Initiate sum 256-bit vector
	__m256i sum = _mm256_setzero_si256();

	// For Each Element (4x64 bits)
    for (u32 i = 0; i < size; i++)
		sum = _mm256_add_epi64(a[i], sum);
    
    
    u64 *sumVec = (u64*) &sum;
	u64 result = sumVec[0] + sumVec[1] + sumVec[2] + sumVec[3];
        
	return result;
}

u64* initRandomAlignedPaddedVec(u64 size) {

	u64 rem = size % 4;
	u64 pad = (!rem) ? 0 : 4 - rem; 	
	u64* vec = (u64*) aligned_alloc(ALIGN, (size + pad) * sizeof(u64));
	checkNullPointer((void *) vec);
        
	// Seed the random number generator
    srand(time(NULL));

    for (u32 i = 0; i < size; i++)
        vec[i] = (rand() % 100) + 1;
    
    for (u32 i = size; i < size + pad; i++)
		vec[i] = 0;
    
    return vec;
}

long double sumOfFracsVec (u64 n) {

	long double res = 0;
    
	return res;
}

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

void sumFracsRational (u64 n, mpf_t* res) {
	mpq_t frac, mid;
	mpq_inits(frac, mid, NULL);	
	mpq_set_ui(frac, 1, 1);
        
	for (u64 i = 0; i < n; i++) {
		u64 r = 8 * i + 1;
		u64 temp = modPowBarret(16, n - i, r);
		mpq_set_ui(mid, temp, r);
		mpq_add(frac, frac, mid);
		//mpq_canonicalize(frac);	
	}

	mpf_set_q (*res, frac);
	mpq_clears(frac, mid, NULL);
}

void sumFracsRational2 (u64 n, mpf_t* res) {
	mpq_t frac, mid;
	mpz_t num, den, base;
	
	mpz_inits(num, den, base,NULL);
	mpz_set_ui(base, 16);
        
	mpq_inits(frac, mid, NULL);	
	mpq_set_ui(frac, 1, 1);
        
	for (u64 i = 0; i < n; i++) {
		mpz_set_ui(den, 8 * i + 1);
		mpz_powm_ui(num, base, n - i, den);
		mpq_set_num(mid, num);		
		mpq_set_den(mid, den);
		mpq_add(frac, frac, mid);
		//mpq_canonicalize(frac);	
	}

	mpf_set_q (*res, frac);
	mpq_clears(frac, mid, NULL);
}

typedef struct {
    u64 start;
    u64 end;
    u64 n;
    mpf_t result;
} ThreadData;

void* worker(void* arg) {
    ThreadData* data = (ThreadData*) arg;

    mpz_t base, mod, tmp;
    mpf_t term, denom;
    mpz_inits(base, mod, tmp, NULL);
    mpf_inits(term, denom, NULL);

    mpz_set_ui(base, 16);
    mpf_set_ui(data->result, 0);

    for (u64 i = data->start; i < data->end; i++) {
        u64 r = 8 * i + 1;
        mpz_set_ui(mod, r);
        mpz_powm_ui(tmp, base, data->n - i, mod);  // tmp = 16^(n - i) mod r

        mpf_set_z(term, tmp);
        mpf_set_ui(denom, r);
        mpf_div(term, term, denom);
        mpf_add(data->result, data->result, term);
    }

    mpz_clears(base, mod, tmp, NULL);
    mpf_clears(term, denom, NULL);
    return NULL;
}

void sumFracsFaster2_mt(u64 n, mpf_t* res, i8 n_threads) {
    pthread_t threads[n_threads];
    ThreadData thread_data[n_threads];

    u64 chunk = n / n_threads;

    // Initialize thread data and create threads
    for (int i = 0; i < n_threads; i++) {
        thread_data[i].start = i * chunk;
        thread_data[i].end = (i == n_threads - 1) ? n : (i + 1) * chunk;
        thread_data[i].n = n;
        mpf_init2(thread_data[i].result, mpf_get_prec(*res));
        pthread_create(&threads[i], NULL, worker, &thread_data[i]);
    }

    // Join threads and accumulate results
    mpf_set_ui(*res, 0);
    for (int i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
        mpf_add(*res, *res, thread_data[i].result);
        mpf_clear(thread_data[i].result);
    }
}


void sumFracsFaster(u64 n, mpf_t* res) {
    mpf_t term, denom;
    mpf_inits(term, denom, NULL);
    mpf_set_ui(*res, 0);

    for (u64 i = 0; i < n; i++) {
        u64 r = 8 * i + 1;
        u64 temp = modPowBarret(16, n - i, r);

        mpf_set_ui(term, temp);
        mpf_set_ui(denom, r);
        mpf_div(term, term, denom);
        mpf_add(*res, *res, term);
    }

    mpf_clears(term, denom, NULL);
}

void sumFracsFaster2(u64 n, mpf_t* res) {
	mpf_t term, denom;
	mpz_t base, tmp, mod;
			
    mpf_inits(term, denom, NULL);
	mpf_set_ui(*res, 0);
    
	mpz_inits(mod, tmp, NULL);
	mpz_init_set_ui(base, 16);

	for (u64 i = 0; i < n; i++) {
		mpz_set_ui(mod, 8 * i + 1);
		mpz_powm_ui(tmp, base, n - i, mod);

		mpf_set_z(term, tmp);
		mpf_set_z(denom, mod);		

		mpf_div(term, term, denom);		
		mpf_add(*res, *res, term);
    }

    mpf_clears(term, denom, NULL);
	mpz_clears(base, mod, tmp, NULL);
}

long double sumFracsNormal (u64 n) {
	long double res = 0;

	for (u64 i = 0; i < n; i++) {
		long double r = 8*i+1;
		long double temp = modPowBarret(16, n - i, r);

		res += temp/r;
	}

	return res;
}

void sumArrayTest() {
	MyTimer* normal = NULL, *vectorized = NULL;
	u64* vec = initRandomAlignedPaddedVec(TOTAL_ELEMENTS);
	u64 result;
        
	INIT_TIMER(normal);
	result = sumArray(vec);
	END_TIMER(normal);
	printf("Result Normal: %lu\n", result);

	INIT_TIMER(vectorized);
	result = sumArrayVec(vec);
	END_TIMER(vectorized);
	printf("Result Vec: %lu\n", result);
        
	CALC_FINAL_TIME(normal);
	CALC_FINAL_TIME(vectorized);
        
	printf("Normal Sum: %.10lfs\n", normal -> totalTime);
	printf("Vectorized Sum: %.10lfs\n", vectorized -> totalTime);
        
	free(vec);
}

void sumFracsTest() {
	MyTimer *normal = NULL , *rational1 = NULL, *rational2 = NULL, 
		 *faster = NULL , *faster2 = NULL, *faster_mt = NULL;
	u64 n = 100000000;
	mpf_t res, res2;
	mpf_inits (res, res2, NULL);
        
	/* INIT_TIMER(normal); */
	/* long double res_n = sumFracsNormal(n); */
	/* END_TIMER(normal);	 */
	/* gmp_printf("Result Normal: %Lf\n", res_n); */
        
	/* INIT_TIMER(rational1); */
	/* sumFracsRational(n, &res); */
	/* END_TIMER(rational1); */
	/* gmp_printf("Result Rational1: %Ff\n", res); */

	/* INIT_TIMER(rational2); */
	/* sumFracsRational(n, &res); */
	/* END_TIMER(rational2);	 */
	/* gmp_printf("Result Rational2: %Ff\n", res); */

	/* INIT_TIMER(faster); */
	/* sumFracsFaster(n, &res); */
	/* END_TIMER(faster);	 */
	/* gmp_printf("Result Faster: %Ff\n", res); */

	INIT_TIMER(faster2);
	sumFracsFaster2(n, &res);
	END_TIMER(faster2);	
	gmp_printf("Result Faster2: %Ff\n", res);

	i8 th = 12;
        
	INIT_TIMER(faster_mt);
	sumFracsFaster2_mt(n, &res2, th);
	END_TIMER(faster_mt);
	gmp_printf("Result Faster2 (%d threads): %Ff\n", th, res2);
        
	mpf_clears(res, res2, NULL);

	puts("");
        
	/* CALC_FINAL_TIME(normal); */
	/* CALC_FINAL_TIME(rational1); */
	/* CALC_FINAL_TIME(rational2); */
	/* CALC_FINAL_TIME(faster); */
	CALC_FINAL_TIME(faster2);
	CALC_FINAL_TIME(faster_mt);
        
	/* printf("Normal Sum: %.10lfs\n",normal -> totalTime); */
	/* printf("Rational1 Sum: %.10lfs\n",rational1 -> totalTime); */
	/* printf("Rational2 Sum: %.10lfs\n",rational2 -> totalTime); */
	/* printf("Faster Sum: %.10lfs\n",faster -> totalTime); */
	printf("Faster2 Sum: %.10lfs\n",faster2 -> totalTime);
	printf("Faster2 (%d threads) Sum: %.10lfs\n", th,faster_mt -> totalTime);
}

i32 main(i32 argc, char* argv[]) {

	//sumArrayTest();
	//puts("");	
	sumFracsTest();
	
	return 0;
}
