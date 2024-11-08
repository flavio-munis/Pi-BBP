/*-----------------------------------------------------------------*/
/**

  @file   bbp.h
  @author Flávio M.

 */
/*-----------------------------------------------------------------*/

#ifndef BBP_HEADER_FILE
#define BBP_HEADER_FILE

/*-----------------------------------------------------------------
                              Includes
  -----------------------------------------------------------------*/
#include <stdint.h>

/*-----------------------------------------------------------------
                          Structs and Enums
  -----------------------------------------------------------------*/
typedef enum {
	BBP_ORIGINAL,
	BELLARD  
}Algorithm;

typedef struct {
	uint64_t startPos;
	uint16_t nthreads;
	Algorithm algo;
} Config;


/*-----------------------------------------------------------------
                  External Functions Declarations
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/**
   @brief Execute BBP Formula.
   @param Config* Pointer to Configuration Struct.
*/
/*-----------------------------------------------------------------*/
void calcBBP(Config*);

#endif
