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
#define BUFFER_SIZE 2048


/*-----------------------------------------------------------------
                         Structs and Enums
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/**
   @brief Page For Menu.
*/
/*-----------------------------------------------------------------*/
typedef struct pageNode PageNode;


/*-----------------------------------------------------------------*/
/**
   @brief Types of Pages.
*/
/*-----------------------------------------------------------------*/
typedef enum {
	TEXT,  // Has Text But Executes No Actions, Bridge Bettween Pages
	INPUT, // Has Text and Executes a Action That Need User Input
	ACTION // No Text, Executes a Action And Returns To Calle Page
} PageType;


/*-----------------------------------------------------------------*/
/**
   @brief Shell Menu Based in a Hash Table. Hashes Are Generated
   Based on Page Text, So Every Menu Page *Must* Be Unique.

   Table is of type size_t but can only hold up to 65535 (UINT16_MAX)
   Pages.
*/
/*-----------------------------------------------------------------*/
typedef struct menuSt {
	PageNode *table;     // Hash Table
    uint32_t rootHash;   // Hash Of Root Page
	size_t tableSize;    // Total Size of Table
	size_t currElements; // Total Elements in Table
    void* configs;       // Configuration Struct That The Menu Operates on
} MenuSt;


/*-----------------------------------------------------------------
                  External Functions Declarations
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/**
   @brief  Init Menu Struct and It's Members.
   @return MenuSt* Pointer to Menu Struct.
*/
/*-----------------------------------------------------------------*/
MenuSt* initMenu(void*);


/*-----------------------------------------------------------------*/
/**
   @brief Free All Memory Allocated for The Menu and It's Members.
   @param MenuSt* Pointer to Menu Struct.
*/
/*-----------------------------------------------------------------*/
void freeMenu(MenuSt*);


/*-----------------------------------------------------------------*/
/**
   @brief Loop That Executes The Menu Pages. Only Exits When Return
   Is Choosed on Root Page.
   @param MenuSt* Pointer to Menu Struct.
*/
/*-----------------------------------------------------------------*/
void runMenu(MenuSt*);


/*-----------------------------------------------------------------*/
/**
   @brief  MurmurHash2 Algorithm For Generating Hashes.
   @param  const char* Page Text.
   @return uint32_t    Hash Generated.
*/
/*-----------------------------------------------------------------*/
uint32_t hashKey(const char*);


/*-----------------------------------------------------------------*/
/**
   @brief  Add a New Page To a Menu.
   @param  MenuSt*          Menu Struct.
   @param  char*            New Page Text.
   @param  bool             If Optional Arguments Will Be Passed

   // Optional
   @param  char* (*)(char*, void*) Pointer to Text Function.
   @param  int (*)(char*, void*)   Pointer to Action Function.
   @param  uint32_t*        Hashes to Fowards Pages (In Order of Text).
   @param  uint16_t         Number of Foward Hashes.
   @param  uint32_t         Hash of The Return Page.
   @param  PageType         Type of Action For Page.

   @return uint32_t Hash of Current Page.
*/
/*-----------------------------------------------------------------*/
uint32_t addPage(MenuSt *, char *, int, ...);

#endif
