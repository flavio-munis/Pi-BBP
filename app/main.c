/*-----------------------------------------------------------------*/
/**

  @file   main.c
  @author Flávio M.
  @brief  Main File for Pi-BBP Project
 */
/*-----------------------------------------------------------------*/

/*-----------------------------------------------------------------
                              Includes
  -----------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "error-handler.h"
#include "bbp.h"
#include "menu2.h"
#include "pages.h"


/*-----------------------------------------------------------------
                      Functions Implementation
  -----------------------------------------------------------------*/
Config* parseArguments(int argc, char* argv[]) {

	Config *configs = NULL;
	uint64_t offset;
	uint32_t threads;
	Algorithm algo;
        
	if (argc != 4) {
		invalidProgramCall(argv[0], "[algorithm] [offset] [threads]\n  [Algorithm] = bellard, original");
	}

	if (!strcmp(argv[1], "bellard"))
		algo = BELLARD;
	else if (!strcmp(argv[1], "original"))
		algo = BBP_ORIGINAL;
	else{
		invalidArgumentError("Invalid Algorithm! [bellard, orginal]");
	}	
        
    offset = strtoll(argv[2], NULL, 10);
    threads = strtoll(argv[3], NULL, 10);
    
	if (offset < 0) {
		invalidArgumentError("Invalid Offset");
	}

	if (threads < 1 || threads > 65536) {
		invalidArgumentError("Invalid Numvber of Threads!\n1 < Threads < 65536");
	}

	configs = (Config *)malloc(sizeof(Config));
	checkNullPointer((void *)configs);

	configs->algo = algo;
	configs->startPos = offset;
	configs->nthreads = threads;
        
	return configs;
}


/*-----------------------------------------------------------------
                               Main
  -----------------------------------------------------------------*/
int main(int argc, char* argv[]) {

	// Menu Mode
	if (argc == 1) {    
	    MenuSt* menu = initPages();
        
		runMenu(menu);
		freeMenu(menu);
	} else {

	    Config* configs = parseArguments(argc, argv);

		if (!configs)
			return 1;

		calcBBP(configs);
		free(configs);		
	}
	
	return 0;
}
