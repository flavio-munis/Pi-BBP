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
#include "../include/error-handler.h"
#include "../include/bbp.h"
#include "../include/menu2.h"
#include "../include/pages.h"


/*-----------------------------------------------------------------
                               Main
  -----------------------------------------------------------------*/
int main(int argc, char* argv[]) {

	MenuSt* menu = initPages();

	runMenu(menu);
	freeMenu(menu);	
        
	return 0;
}
