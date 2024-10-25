/*-----------------------------------------------------------------*/
/**
  @file   menu.c
  @author Flávio M.
  @brief  Iterative Shell Menu for Pi-BBP Project.
 */
/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------
                              Includes
  -----------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/bbp.h"
#include "../include/menu.h"
#include "../include/error-handler.h"


#ifdef _WIN32

#include <windows.h>
#include <processthreadsapi.h>

#else

#include <unistd.h>

#endif


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
Config* initDefaultConfigs();


/*-----------------------------------------------------------------*/
/**
   @brief  Init All Pages of The App And Stores Them in a Array.
   @return Page* Array Containing All Pages.
 */
/*-----------------------------------------------------------------*/
Page* initAllPages();


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
char* format64UInteger(uint64_t);


/*-----------------------------------------------------------------*/
/**
   @brief  Convert a String to a Integer.
   @param  char*     String to be Converted.
   @param  uint32_t* Pointer in Which the Result Be Stored.
   @return PageCode  Error/Success Code in Operation.
 */
/*-----------------------------------------------------------------*/
PageCode getIntFromInput(char*, uint32_t*);


/*-----------------------------------------------------------------*/
/**
   @brief  Convert a String to a 64 Bit Unsigned Integer.
   @param  char*     String to be Converted.
   @param  uint64_t* Pointer in Which the Result Be Stored.
   @return PageCode  Error/Success Code in Operation.
 */
/*-----------------------------------------------------------------*/
PageCode getU64IntFromInput(char*, uint64_t*);


/*-----------------------------------------------------------------*/
/**
   @brief  Get User Input From Stdin.
   @return char* User Input String.
 */
/*-----------------------------------------------------------------*/
char* getUserInput();


/*-----------------------------------------------------------------*/
/**
   @brief  Get Max Threads Available in CPU.
   @return uint16_t Number of Threads.
 */
/*-----------------------------------------------------------------*/
uint16_t getMaxThreads();


/*-----------------------------------------------------------------*/
/**
   @brief  Get The Approximate Optimal Number of Threads for a Given
           Offset.
   @param  uint64_t Current Offset.
   @return char*    Suggested Number of Threads.
 */
/*-----------------------------------------------------------------*/
char* getOptimalThreads(uint64_t);


/*-----------------------------------------------------------------*/
/**
   @brief  Pages Handlers Functions.
   @param  Page*     Current Page.
   @param  Config*   Config Struct.
   @param  uint16_t* Index To Next Page.
   @return PageCode   Error/Success Code in Operation.
 */
/*-----------------------------------------------------------------*/
PageCode mainPage(Page *, Config *, uint16_t*);
PageCode algoPage(Page *, Config *, uint16_t*);
PageCode threadsPage(Page *, Config *, uint16_t*);
PageCode offsetPage(Page *, Config *, uint16_t*);


/*-----------------------------------------------------------------
                            Definitions
   -----------------------------------------------------------------*/
#define TOTAL_PAGES 4
#define BUFFER_SIZE 2048

static char *pagesText[TOTAL_PAGES] = {
    "====== Pi-BBP ======\n\n[1] Algorithm: %s\n[2] Threads: %d\n[3] Offset: "
    "%s\n[4] Exit\n\n[0] Start Computation!\n",
    "Current Algorithm: %s\n\n[1] BBP-Original (4-Term)\n[2] Bellard "
    "(7-Term)\n\n[3] Return\n",
    "Current Threads: %d\nMax Threads: %d\nOptimal for Current Offset: %s\n",
	"Current Offset: %s\n"};

static Page staticPages[TOTAL_PAGES] = {
    {.pageFunc = &mainPage, .index = 0, .previousIndex = 0, .pageText = NULL},
    {.pageFunc = &algoPage, .index = 1, .previousIndex = 0, .pageText = NULL},
    {.pageFunc = &threadsPage, .index = 2, .previousIndex = 0, .pageText = NULL},
    {.pageFunc = &offsetPage, .index = 3, .previousIndex = 0, .pageText = NULL}
};


/*-----------------------------------------------------------------
                      Functions Implementation
  -----------------------------------------------------------------*/
PageList* initPages() {

	PageList* newPages = (PageList*) malloc(sizeof(PageList));
	checkNullPointer((void*) newPages);

	newPages -> currPage = initAllPages();
	newPages -> configs = initDefaultConfigs();
	newPages -> head = newPages -> currPage;
        
	return newPages;
}

Config* initDefaultConfigs() {

	Config *defaultConfigs = (Config *)malloc(sizeof(Config));
	checkNullPointer((void*) defaultConfigs);

	defaultConfigs -> startPos = 10000;
	defaultConfigs -> nthreads = getMaxThreads();
	defaultConfigs -> algo = BELLARD;
        
	return defaultConfigs;
}

Page* initAllPages() {

	Page *pages = (Page *)malloc(sizeof(Page) * TOTAL_PAGES);
	checkNullPointer((void*) pages);

	for (int i = 0; i < TOTAL_PAGES; i++) {
		pages[i].pageFunc = staticPages[i].pageFunc;
		pages[i].pageText = pagesText[i];
		pages[i].index = staticPages[i].index;
		pages[i].previousIndex = staticPages[i].previousIndex;
	}
        
	return pages;
}

PageCode handlePage(PageList* list) {

	Page* currPage = list -> currPage;
	PageCode code;
	uint16_t nextPage;

	code = currPage -> pageFunc(currPage, list -> configs, &nextPage);
        
	switch (code) {

        case -1:
			puts("\nInvalid Input!");
			break;

        case -2:
			puts("\nError In Page!");
			break;

        case -3:
			return code;
                        
        default:
			list -> currPage = list->head + nextPage;
	}

	puts("");
        
	return code;
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

char* getUserInput() {

	char* input = (char*) malloc(sizeof(char) * BUFFER_SIZE);
    char* ret;
        
	printf("\nOption: ");
	ret = fgets(input, BUFFER_SIZE, stdin);

	if (!ret)
		return NULL;	
        
	return input;
}

PageCode getIntFromInput(char* input, uint32_t* optionPtr) {

	uint32_t option;
	char* end;
        
    option = strtol(input, &end, 10);
    
    if (*end != '\n' || end == input || option < 0)
		return PAGE_INVALID_INPUT;

    *optionPtr = option;
    
	return PAGE_VALID_INPUT;
}


PageCode getU64IntFromInput(char* input, uint64_t* optionPtr) {
	uint64_t option;
	char* end;
        
    option = strtoll(input, &end, 10);
    
    if (*end != '\n' || end == input || option < 0)
		return PAGE_INVALID_INPUT;

    *optionPtr = option;
    
	return PAGE_VALID_INPUT;
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

PageCode mainPage(Page* main, Config* configs, uint16_t* nextIndex) {
	
	char buffer[BUFFER_SIZE];
	char* algoString = getAlgoString(configs->algo);
	char* formatedOffset;
	char *input;
	uint32_t option;	
    PageCode code;

	// Check if Current Algorithm is Valid
	if (!algoString)
		return PAGE_ERROR;

	formatedOffset = format64UInteger(configs -> startPos);
        
	// Print Page Text
	snprintf(buffer,
			 BUFFER_SIZE,
			 main->pageText,
			 algoString,
			 configs->nthreads,
			 formatedOffset);

	printf("%s", buffer);

	free(formatedOffset);
        
	// Get User input and Convert to Integer
	input = getUserInput();

	if (!input)
		return PAGE_ERROR;
		
    code = getIntFromInput(input, &option);

	// Free Input Variable
	free(input);

	*nextIndex = main -> index;
        
    // Check if return code is an error
    if (code < 0)
		return code;

	// Guide The Code To The Path It Should Follow
    switch (option) {
	    case 0:
			calcBBP(configs);
			break;

	    case 1:
	    case 2:
		case 3:		
			*nextIndex = option;
			break;

	    case 4:
			return PAGE_EXIT;
		    break;

	    default:
			return PAGE_INVALID_INPUT;
	}
    
	return PAGE_NORMAL_OP;
}

PageCode algoPage(Page* curr, Config* configs, uint16_t* nextIndex) {

	char buffer[BUFFER_SIZE];
	char* algoString = getAlgoString(configs->algo);
	char *input;
	uint32_t option;	
    PageCode code;

	// Check if Current Algorithm is Valid
	if (!algoString)
		return PAGE_ERROR;

	// Print Page Text
	snprintf(buffer,
			 BUFFER_SIZE,
			 curr->pageText,
			 algoString);

	printf("%s", buffer);

	// Get User input and Convert to Integer
    input = getUserInput();

    if (!input)
		return PAGE_ERROR;
	
    code = getIntFromInput(input, &option);

    // Free Input Variable
	free(input);

	*nextIndex = curr -> previousIndex;
        
    // Check if return code is an error
    if (code < 0)
		return code;

	// Guide The Code To The Path It Should Follow
    switch (option) {
	    case 1:
			configs -> algo = BBP_ORIGINAL;
			break;

	    case 2:
			configs -> algo = BELLARD;
			break;

	    case 3:
			break;
                        
	    default:
			*nextIndex = curr -> index;
			return PAGE_INVALID_INPUT;
	}
    
	return PAGE_NORMAL_OP;
}

PageCode threadsPage(Page* curr, Config* configs, uint16_t* nextIndex) {

	char buffer[BUFFER_SIZE];
	char* algoString = getAlgoString(configs->algo);
	char *input;
	uint32_t option;
	uint16_t maxThreads;	
    PageCode code;

	// Check if Current Algorithm is Valid
	if (!algoString)
		return PAGE_ERROR;

	maxThreads = getMaxThreads();
        
	// Print Page Text
        snprintf(buffer, BUFFER_SIZE,
                 curr->pageText,
                 configs->nthreads,
                 maxThreads,
                 getOptimalThreads(configs -> startPos));

	printf("%s", buffer);

	// Get User input and Convert to Integer
	input = getUserInput();

	if (!input)
		return PAGE_ERROR;
        
    code = getIntFromInput(input, &option);

	// Free Input Variable
	free(input);

	*nextIndex = curr -> previousIndex;
        
    // Check if return code is an error
    if (code < 0)
		return code;

	// Guide The Code To The Path It Should Follow
    if (option > maxThreads)
		return PAGE_INVALID_INPUT;

    configs -> nthreads = option;
      
	return PAGE_NORMAL_OP;
}


PageCode offsetPage(Page* curr, Config* configs, uint16_t* nextIndex) {
	
	char buffer[BUFFER_SIZE];
	char* algoString = getAlgoString(configs->algo);
	char* formatedOffset;
	char *input;
	uint64_t option;	
    PageCode code;

	// Check if Current Algorithm is Valid
	if (!algoString)
		return PAGE_ERROR;

	formatedOffset = format64UInteger(configs -> startPos);
        
	// Print Page Text
	snprintf(buffer,
			 BUFFER_SIZE,
			 curr->pageText,
			 formatedOffset);

	printf("%s", buffer);

	free(formatedOffset);
        
	// Get User input and Convert to Integer
	input = getUserInput();

	if (!input)
		return PAGE_ERROR;
		
    code = getU64IntFromInput(input, &option);

	// Free Input Variable
	free(input);

    *nextIndex = curr -> previousIndex;
        
    // Check if return code is an error
    if (code < 0)
		return code;

	// Guide The Code To The Path It Should Follow
    if (option < 0)
		return PAGE_INVALID_INPUT;

    configs -> startPos = option;
    
	return PAGE_NORMAL_OP;
}
