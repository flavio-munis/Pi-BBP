/*-----------------------------------------------------------------*/
/**

  @file   menu.h
  @author Flávio M.

 */
/*-----------------------------------------------------------------*/

#ifndef MENU2_HEADER_FILE
#define MENU2_HEADER_FILE

/*-----------------------------------------------------------------
                              Includes
  -----------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>


/*-----------------------------------------------------------------
                            Definitions
   -----------------------------------------------------------------*/


/*-----------------------------------------------------------------
                         Structs and Enums
  -----------------------------------------------------------------*/

typedef struct pageNode PageNode;

typedef struct menuSt {
	PageNode *table;
    uint32_t rootHash;
	size_t tableSize;
	size_t currElements;
    void* configs;
} MenuSt;

typedef enum {
	TEXT,
	INPUT,
	ACTION
}PageType;


/*-----------------------------------------------------------------
                  External Functions Declarations
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/**
   @brief  Init All Pages and Default Configs.
   @return PageList* Pointer to PagesList Struct.
*/
/*-----------------------------------------------------------------*/
MenuSt* initMenu(void*);


void freeMenu(MenuSt*);


void runMenu(MenuSt *, uint32_t);


uint32_t hashKey(const char*);

/*-----------------------------------------------------------------*/
/**
   @brief  Add a New Page To a Menu.
   @param  MenuSt*          Menu Struct.
   @param  char*            New Page Text.
   @param  bool             If Optional Arguments Will Be Passed

   // Optional
   @param  char* (*f)(char*, void*) Pointer to Text Function.
   @param  void (*f)(char*) Pointer to Action Function.
   @param  uint32_t*        Hashes to Fowards Pages (In Order of Text).
   @param  uint16_t         Number of Foward Hashes.
   @param  uint32_t         Hash of The Return Page.
   @param  PageType         Type of Action For Page.

   @return uint32_t Hash of Current Page.
*/
/*-----------------------------------------------------------------*/
uint32_t addPage(MenuSt*, char*, int, ...);


#endif
