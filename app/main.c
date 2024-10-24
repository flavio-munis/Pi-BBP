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
#include "../include/bbp.h"
#include "../include/menu.h"

/*-----------------------------------------------------------------
                               Main
  -----------------------------------------------------------------*/
int main(int argc, char* argv[]) {

    PageList* list = initPages();
	PageCode result;

	do {
		result = handlePage(list);
	} while (result != PAGE_EXIT);
    
	return 0;
}
