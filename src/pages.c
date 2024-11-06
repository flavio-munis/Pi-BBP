/*-----------------------------------------------------------------*/
/**
  @file   pages.c
  @author Flávio M.
  @brief  Define Pages Functions.
 */
/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------
                              Includes
  -----------------------------------------------------------------*/
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../include/error-handler.h"
#include "../include/bbp.h"
#include "../include/menu2.h"
#include "../include/pages.h"

#ifdef _WIN32

#include <windows.h>
#include <processthreadsapi.h>

#else

#include <unistd.h>

#endif


#define TOTAL_PAGES 7

static char *pagesText[TOTAL_PAGES] = {
    "\n====== Pi-BBP ======\n\n[1] Algorithm: %s\n[2] Threads: %d\n[3] Offset: %s\n\n[4] Start Computation!\n\n[0] Exit\n",
    "\nCurrent Algorithm: %s\n\n[1] BBP-Original (4-Term)\n[2] Bellard (7-Term)\n\n[0] Return\n",
    "\nCurrent Threads: %d\nMax Threads: %d\nOptimal for Current Offset: %s\n",
    "\nCurrent Offset: %s\n", "algo_action1", "algo_action2", "compute_bbp_action1"};


/*-----------------------------------------------------------------
                   Internal Functions Signatures
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/**
   @brief  Init a New Configuration Struct With Default Values.
           startPos -> 10.000
		   nthreads -> SYSTEM_MAX
		   algo     -> BELLARD
   @return Config* Pointer to Configuration Struct.
 */
/*-----------------------------------------------------------------*/
Config *initDefaultConfigs();


/*-----------------------------------------------------------------*/
/**
   @brief  Convert a Algorithm Type to It's String Equivalent.
   @param  Algorithm Current Algorithm.
   @return char* String Equivalent of Current Algorithm.
 */
/*-----------------------------------------------------------------*/
char* getAlgoString(Algorithm);


/*-----------------------------------------------------------------*/
/**
   @brief  Receives a Number and Format it to have dot separators.
           10      -> 10
		   1000    -> 1.000
		   1000000 -> 1.000.000
   @param  uint64_t Number To Be Formated.
   @return char*    Formated String.
 */
/*-----------------------------------------------------------------*/
char *format64UInteger(uint64_t);

/*-----------------------------------------------------------------*/
/**
   @brief  Get Max Threads Available in CPU.
   @return uint16_t Number of Threads.
 */
/*-----------------------------------------------------------------*/
uint16_t getMaxThreads();


char* mainPage(char*, void*);

char* algoPageText(char*, void*);
int algoPageAction1(char*, void*);
int algoPageAction2(char*, void*);

char* threadsPageText(char*, void*);
int threadsPageAction1(char*, void*);


char* offsetPageText(char*, void*);
int offsetPageAction1(char*, void*);

int computeBBP(char *, void *);

Config *initDefaultConfigs();

/*-----------------------------------------------------------------
                      Functions Implementation
  -----------------------------------------------------------------*/
MenuSt* initPages(uint32_t* rootHash) {

	Config* configs = initDefaultConfigs();
	MenuSt* menu = initMenu((void*) configs);
	uint32_t hash[TOTAL_PAGES];
        
	for (int i = 0; i < TOTAL_PAGES; i++)
		hash[i] = hashKey(pagesText[i]);

	// Root Page
	uint32_t *forwardHashes1 =(uint32_t*) malloc(sizeof(uint32_t) * 4);
	checkNullPointer((void *) forwardHashes1);
	forwardHashes1[0] = hash[1];
	forwardHashes1[1] = hash[2];
	forwardHashes1[2] = hash[3];
	forwardHashes1[3] = hash[6];	
	addPage(menu, pagesText[0], 1, &mainPage, NULL, forwardHashes1, 4, hash[0], TEXT);

	// Algorithm Selection Page
	uint32_t *forwardHashes2 = (uint32_t*) malloc(sizeof(uint32_t) * 2);
	checkNullPointer((void *) forwardHashes1);
	forwardHashes2[0] = hash[4];
    forwardHashes2[1] = hash[5];
	addPage(menu, pagesText[1], 1, &algoPageText, NULL, forwardHashes2, 2, hash[0], TEXT);
	addPage(menu, pagesText[4], 1, NULL, &algoPageAction1, NULL, 0, hash[0], ACTION);
	addPage(menu, pagesText[5], 1, NULL, &algoPageAction2, NULL, 0, hash[0], ACTION);

	// Number of Threads Selection
	addPage(menu, pagesText[2], 1, &threadsPageText, &threadsPageAction1, NULL, 0, hash[0], INPUT);

	// Offset Selection
	addPage(menu, pagesText[3], 1, &offsetPageText, &offsetPageAction1, NULL, 0, hash[0], INPUT);

	// Compute BBP
	addPage(menu, pagesText[6], 1, NULL, &computeBBP, NULL, 0, hash[0],
                ACTION);

	*rootHash = hash[0];
            
	return menu;
}

Config* initDefaultConfigs() {

	Config *defaultConfigs = (Config *)malloc(sizeof(Config));
	checkNullPointer((void*) defaultConfigs);

	defaultConfigs -> startPos = 10000;
	defaultConfigs -> nthreads = 12;
	defaultConfigs -> algo = BELLARD;
        
	return defaultConfigs;
}

char* getAlgoString(Algorithm algo) {

	switch (algo) {

        case BBP_ORIGINAL:
			return "Original (4-Terms)";

	    case BELLARD:
			return "Bellard (7-Term)";
	}

    return NULL;
}

char* format64UInteger(uint64_t num) {
    
    uint64_t temp = num;
    uint16_t digitCount = 0;
	uint16_t dots, bufferSize, pos, count;
	char* buffer;
    
    // Calculate number of digits
    do {
        digitCount++;
        temp /= 10;
    } while (temp > 0);

    // Calculate buffer size needed (digits + dots + null terminator)
    dots = (digitCount - 1) / 3;
    bufferSize = digitCount + dots + 1;
    
	// Max size for 64-bit int with dots (20 digits + 6 dots + null)
    buffer = (char*) malloc(sizeof(char) * 32);
	checkNullPointer((void*) buffer);
    
    // Fill buffer from right to left
    pos = bufferSize - 1;
    buffer[pos--] = '\0';
    
    temp = num;
    count = 0;
    
    do {
        if (count > 0 && count % 3 == 0) {
            buffer[pos--] = '.';
        }
        buffer[pos--] = '0' + (temp % 10);
        temp /= 10;
        count++;
    } while (temp > 0);

    return buffer;
}

uint16_t getMaxThreads() {

#ifdef _WIN32
	uint16_t maxThreads;
	SYSTEM_INFO sysInfo;
	
	GetSystemInfo(&sysInfo);
	maxThreads = sysInfo.dwNumberOfProcessors;

	return maxThreads;	
#else
	return sysconf(_SC_NPROCESSORS_CONF);
#endif  
}

int getU64IntFromInput(char* input, uint64_t* optionPtr) {
	uint64_t option;
	char* end;
        
    option = strtoll(input, &end, 10);
    
    if (*end != '\n' || end == input || option < 0)
		return 1;

    *optionPtr = option;
    
	return 0;
}

int getU16IntFromInput(char* input, uint16_t* optionPtr) {
	uint32_t option;
	char* end;
        
    option = strtoll(input, &end, 10);
    
    if (*end != '\n' || end == input || option < 0)
		return 1;

	if (option >= UINT16_MAX)
		return 1;
          
    *optionPtr = (uint16_t) option;
    
	return 0;
}

char* getOptimalThreads(uint64_t offset) {

	uint16_t maxThreads = getMaxThreads();
  
	if (offset < 1000 || maxThreads <= 2)
		return "1-2 Threads";

	if (offset < 10000 && maxThreads >= 4)
		return "3-4 Threads";

	if (offset < 1000000 && maxThreads >= 8)
		return "6-8 Threads";

	if (offset > 1000000 && maxThreads >= 12)
          return "12+ Threads";

	return "1-2 Threads";
}

char* mainPage(char* text, void* configSt) {
	
	Config* configs = (Config*) configSt;
	char* buffer;
	char* algoString = getAlgoString(configs ->algo);
	char* formatedOffset;

	// Check if Current Algorithm is Valid
	if (!algoString)
		return NULL;

	formatedOffset = format64UInteger(configs -> startPos);

	buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
	checkNullPointer((void*) buffer);	
        
	// Print Page Text
	snprintf(buffer,
			 BUFFER_SIZE,
			 text,
			 algoString,
			 configs->nthreads,
			 formatedOffset);

	free(formatedOffset);
        
    return buffer;
}

char* algoPageText(char* text, void* configSt) {

	char* buffer;
	Config* configs = (Config*) configSt;
	char* algoString = getAlgoString(configs->algo);

	buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
	checkNullPointer((void*) buffer);
        
	// Check if Current Algorithm is Valid
	if (!algoString)
		return NULL;

	// Print Page Text
	snprintf(buffer,
			 BUFFER_SIZE,
			 text,
			 algoString);

	return buffer;
}


int algoPageAction1(char* input, void* configSt) {

	Config *configs = (Config *)configSt;
	configs->algo = BBP_ORIGINAL;
		
	return 0;	
}

int algoPageAction2(char* input, void* configSt) {

	Config *configs = (Config *)configSt;
	configs->algo = BELLARD;
		
	return 0;	
}

char* threadsPageText(char* text, void* configSt) {

	char* buffer;
	Config* configs = (Config*) configSt;
	char* algoString = getAlgoString(configs->algo);
	uint16_t maxThreads;	
        
	buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
	checkNullPointer((void*) buffer);

	// Check if Current Algorithm is Valid
	if (!algoString)
		return NULL;

	maxThreads = getMaxThreads();
        
	// Print Page Text
	snprintf(buffer, BUFFER_SIZE,
			 text,
			 configs->nthreads,
			 maxThreads,
			 getOptimalThreads(configs -> startPos));

	return buffer;
}

int threadsPageAction1(char* input, void* configSt) {

	Config *configs = (Config *)configSt;
	uint16_t threads;

	if (getU16IntFromInput(input, &threads))
		return 1;
        
	configs -> nthreads = threads;
        
	return 0;
}



char* offsetPageText(char* text, void* configSt) {

	Config *configs = (Config *)configSt;
	char *buffer;
	char* formatedOffset;

	buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
	checkNullPointer((void*) buffer);

	formatedOffset = format64UInteger(configs -> startPos);
        
	// Print Page Text
	snprintf(buffer, BUFFER_SIZE, text, formatedOffset);

	free(formatedOffset);

	return buffer;
}

int offsetPageAction1(char* input, void* configSt) {

	Config *configs = (Config *)configSt;
	uint64_t offset;

	if (getU64IntFromInput(input, &offset))
		return 1;

	configs->startPos = offset;
		
	return 0;	
}

int computeBBP(char* input, void* configSt) {

	calcBBP((Config*) configSt);
	return 0;
}
