#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/timer.h"
#include "../include/error-handler.h"

#define ALIGN 64
#define TOTAL_ELEMENTS 38742373
#define VLEN 4


// Integer Types
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

u64 sumArrayVec(u64* vec) {

	const u32 size = TOTAL_ELEMENTS / 4;
	const u32 remainder = TOTAL_ELEMENTS % 4;

	// Cast vec to 256-bit vector (4x64 Bit Per Element)
	__m256i* a = (__m256i*)vec;

	// Initiate sum 256-bit vector
	__m256i sum = _mm256_set1_epi64x(0);

	// For Each Element (4x64 bits)
    for (u32 i = 0; i < size; i++)
		sum = _mm256_add_epi64(a[i], sum);

    u64 *sumVec = (u64*) &sum;
	u64 result = sumVec[0] + sumVec[1] + sumVec[2] + sumVec[3];
        
	if (remainder)
		for (u32 j = size * 4; j < TOTAL_ELEMENTS; j++)
			result += vec[j];
        
	return result;
}


u64* initRandomAlignedVec() {

	u64* vec = (u64*) aligned_alloc(ALIGN, TOTAL_ELEMENTS * sizeof(u64));
	checkNullPointer((void *)vec);
        
	// Seed the random number generator
    srand(time(NULL));

    for (u32 i = 0; i < TOTAL_ELEMENTS; i++)
        vec[i] = (rand() % 100) + 1;

    return vec;
}


i32 main(i32 argc, char* argv[]) {

	MyTimer* normal = NULL, *vectorized = NULL;
	u64* vec = initRandomAlignedVec();
	u64 result;
	
        
	INIT_TIMER(normal);
	result = sumArray(vec);
	printf("Result Normal: %lu\n", result);
	END_TIMER(normal);

	INIT_TIMER(vectorized);
	result = sumArrayVec(vec);
	printf("Result Vec: %lu\n", result);
	END_TIMER(vectorized);	

	CALC_FINAL_TIME(normal);
	CALC_FINAL_TIME(vectorized);
        
	printf("Normal Sum: %.10lfs\n", normal -> totalTime);
	printf("Vectorized Sum: %.10lfs\n", vectorized -> totalTime);
        
	free(vec);
        
	return 0;
}
