/*-----------------------------------------------------------------*/
/**

  @file   menu.h
  @author Flávio M.

 */
/*-----------------------------------------------------------------*/

#ifndef MENU_HEADER_FILE
#define MENU_HEADER_FILE

/*-----------------------------------------------------------------
                              Includes
  -----------------------------------------------------------------*/
#include "bbp.h"
#include <stdint.h>


/*-----------------------------------------------------------------
                            Definitions
   -----------------------------------------------------------------*/


/*-----------------------------------------------------------------
                         Structs and Enums
  -----------------------------------------------------------------*/
typedef enum {
	PAGE_VALID_INPUT = 2,
	PAGE_NORMAL_OP = 1,
	PAGE_RETURN = 0,
	PAGE_INVALID_INPUT = -1,
	PAGE_ERROR = -2,
	PAGE_EXIT = -3
} PageCode;

typedef struct page {
	uint16_t index;
	uint16_t previousIndex;
	PageCode (*pageFunc) (struct page*, Config*, uint16_t*);
	char *pageText;
} Page;

typedef struct {
	Page *currPage;
	Page* head;
	Config* configs;
} PageList;


/*-----------------------------------------------------------------
                  External Functions Declarations
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/**
   @brief  Init All Pages and Default Configs.
   @return PageList* Pointer to PagesList Struct.
*/
/*-----------------------------------------------------------------*/
PageList* initPages();


/*-----------------------------------------------------------------*/
/**
   @brief  Handle Current Page Inputs and
   @return PageList* Pointer to PagesList Struct.
*/
/*-----------------------------------------------------------------*/
PageCode handlePage(PageList*);

#endif
