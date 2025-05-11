#include <math.h>
#include <gmp.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/timer.h"
#include "../include/error-handler.h"


// Integer Types
#define i8 int8_t
#define i32 int32_t
#define u32 uint32_t
#define u64 uint64_t
#define u128 __uint128_t

#define PRECISION 10
#define EPSILON 1e-17

typedef struct {
    u64 start;
    u64 end;
    u64 n;
    mpf_t result;
} ThreadData;


void *worker(void *);
void sumFracsFaster2_mt(u64, mpf_t*, i8);
void sumFracsFaster2(u64, mpf_t*);

void mpf_fmodl (mpf_t rop) {
	mpf_t ipart;
    mpf_init(ipart);

    mpf_trunc(ipart, rop);        // Get integer part
    mpf_sub(rop, rop, ipart);    // frac = x - trunc(x)

    mpf_clear(ipart);
}

void series (int j, uint64_t n, mpf_t* res) {
	mpf_t term, denom, basef, mult;
	mpz_t base, tmp, mod;

    mpf_inits(term, denom, basef, mult, NULL);
	mpf_set_ui(*res, 0);
    
	mpz_inits(mod, tmp, NULL);
	mpz_init_set_ui(base, 16);
	mpf_set_ui(basef, 16);
        
	if (j == 1)
		mpf_set_si(mult, 4);
	else if (j == 4)
		mpf_set_si(mult, -2);
	else
		mpf_set_si(mult, -1);
                
	for (u64 i = 0; i < n; i++) {
		mpz_set_ui(mod, 8 * i + j);
		mpz_powm_ui(tmp, base, n - i, mod);
		
		mpf_set_z(term, tmp);
		mpf_set_z(denom, mod);

		mpf_mul(term, mult, term);		
		mpf_div(term, term, denom);
		mpf_add(*res, *res, term);
		mpf_fmodl(*res);
	}

	for (u64 k = n; k <= n + 100; k++) {
		u64 r = 8 * k + j;
		mpf_pow_ui(term, basef, abs(n - k));
		mpf_ui_div(term, 1, term);
		mpf_div_ui(term, term, r);		
                
		if (!mpf_cmp_d(term, EPSILON))
			break;

		mpf_mul(term, mult, term);
		mpf_add(*res, *res, term);
		mpf_fmodl(*res);
	}
        
	mpf_fmodl(*res);
    mpf_clears(term, denom, basef, mult, NULL);
	mpz_clears(base, mod, tmp, NULL);
}

void bbp (u64 n) {

	mpf_t res, tmp;
	mpf_inits(res, tmp, NULL);

	series(4, n, &res);
	gmp_printf("j: %d -> %Ff\n", 4, res);

	series(1, n, &tmp);
	mpf_add(res, tmp, res);	
	gmp_printf("j: %d -> %Ff\n", 1, res);

	series(5, n, &tmp);
	mpf_add(res, tmp, res);
	gmp_printf("j: %d -> %Ff\n", 5, res);
        
	series(6, n, &tmp);
	mpf_add(res, tmp, res);	
	gmp_printf("j: %d -> %Ff\n\n", 6, res);

	mpf_fmodl(res);
	gmp_printf("Result: %Ff\n", res);
}

long double series2(u64 j, u64 n) {

	long double sum = 0, temp, r;
	mpz_t base, tmp, mod;

	mpz_inits(mod, tmp, NULL);
	mpz_init_set_ui(base, 16);
        
	for (uint64_t k = 0; k < n; k++) {
		r = 8.0L * k + j;
		mpz_set_ui(mod, r);
		mpz_powm_ui(tmp, base, n - k, mod);

		temp = mpz_get_ui(tmp);
		sum = sum + temp / r;
		sum = fmodl(sum, 1.0L);
	}

	mpz_clears(mod, tmp, base, NULL);
	
	for (uint64_t k = n; k <= n + 100; k++) {
		r = 8.0L*k +j;
		temp = powl(16.0L, (long double) n - k) / r;
		
		if (temp < EPSILON)
			break;
                
	    sum += temp;
		sum = fmodl(sum, 1.0L);
	}

	return sum;
}

long double bbp2(uint64_t d) {
	
	long double result;
        
    result = -2 * series2(4, d);
	printf("j: %d -> %Lf\n", 4, result);
    
    result += 4*  series2(1, d);
	printf("j: %d -> %Lf\n", 1, result);
    
    result += -1 * series2(5, d);
	printf("j: %d -> %Lf\n", 5, result);
    
    result += -1 * series2(6, d);
	printf("j: %d -> %Lf\n\n", 6, result);
    
	//result = 4.0L *s1 - 2.0L * s2 - s4 - s3;
	result = fmodl(result, 1.0L);

	return result;
}

void checkArgs(int argc, char* argv[], u64* d) {

	uint64_t digit;

	if (argc != 2) {
		invalidProgramCall(argv[0], "[inicio]");
	}

    digit = strtoll(argv[1], NULL, 10);

	if (digit < 0) {
		invalidArgumentError("Argumento Inválido!\nInicio >= 0");
	}

	*d = digit;
}

void ihex (double x) {
  int i;
  double y;
  char hx[] = "0123456789ABCDEF";

  y = x;

  for (i = 0; i < PRECISION; i++){
    y = 16. * (y - floor (y));
    printf("%c", hx[(int) y]);
  }
}


i32 main(i32 argc, char* argv[]) {

	uint64_t d;
	long double result;
	MyTimer* tbbp = NULL;

	checkArgs(argc, argv, &d);

	INIT_TIMER(tbbp);

	result = bbp2(d);

	END_TIMER(tbbp);
	CALC_FINAL_TIME(tbbp);
	printf("Total Runtime: %.5fs\n", tbbp -> totalTime);
	free(tbbp);

	printf("%d digits @ %ld = ", PRECISION, d);
	ihex(result);
	printf("\nResult = %Lf\n", result);	
	puts("");
}        
