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
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../include/bbp.h"
#include "../include/menu2.h"
#include "../include/error-handler.h"

/*-----------------------------------------------------------------
                            Definitions
   -----------------------------------------------------------------*/
#define INITIAL_SIZE 16
#define OPT_ARGS 6


/*-----------------------------------------------------------------
                         Structs and Enums
  -----------------------------------------------------------------*/
typedef struct pageNode {
	uint32_t hash;
	char* text;
    char* (*textFunc) (char*, void*);
	void (*actionFunc) (char*);
	uint32_t* forwardHashes;
	uint16_t totalFoward;
	uint32_t backHash;
	PageType type;
} PageNode;


/*-----------------------------------------------------------------
                   Internal Functions Signatures
  -----------------------------------------------------------------*/

/*-----------------------------------------------------------------*/
/**
   @brief  Free Menu.
   @return Config* Pointer to Configuration Struct.
 */
/*-----------------------------------------------------------------*/
void freeMenu(MenuSt*);


/*-----------------------------------------------------------------
                      Functions Implementation
  -----------------------------------------------------------------*/
MenuSt* initMenu(void* configs) {
	MenuSt *newMenuSt = (MenuSt*) malloc(sizeof(MenuSt));
	checkNullPointer((void*) newMenuSt);

	newMenuSt -> table = (PageNode*) calloc(INITIAL_SIZE, sizeof(PageNode));
	checkNullPointer((void *) newMenuSt->table);

	newMenuSt -> tableSize = INITIAL_SIZE;
	newMenuSt -> currElements = 0;
	newMenuSt -> configs = configs;
        
	return newMenuSt;
}

void freeMenu(MenuSt* menu) {

	if (menu) {
		if (menu -> table)
			free(menu->table);

		free(menu);
	}
}


void runMenu(MenuSt *menu, uint32_t rootHash) {

	for (int i = 0; i < menu -> tableSize; i++) {
          if (menu->table[i].hash) {
			  printf("BackHash: %u\n", menu->table[i].backHash);
			  printf("Total Forward: %u\n", menu -> table[i].totalFoward);
                          
			  for (int j = 0; j < menu -> table[i].totalFoward; j++)
				  printf("Forward[%d]: %u\n", j, menu -> table[i].forwardHashes[j]);
                          
			  printf("%s\n", menu->table[i].text);
		  }
	}
}


void setEntry(MenuSt* menu, PageNode page) {

	uint32_t hash = page.hash;
	uint16_t index = (hash & (uint16_t) (menu->tableSize - 1));
	PageNode* table = menu -> table;
        

	while (table[index].hash) {

		if (hash == table[index].hash) {
			fprintf(stderr, "Page Already Exists!");
			return;
		}

		index++;
		if (index >= menu->tableSize)
			index = 0;
	}

    table[index].hash = page.hash;
    table[index].text = page.text;
	table[index].textFunc = page.textFunc;
	table[index].actionFunc = page.actionFunc;
	table[index].forwardHashes = page.forwardHashes;
	table[index].totalFoward = page.totalFoward;
	table[index].backHash = page.backHash;
	table[index].type = page.type;
}


int expandTable(MenuSt* menu) {

	size_t newSize = menu->tableSize * 2;
	PageNode* newTable, *oldTable;
	
	if (newSize >= UINT16_MAX) {
		fprintf(stderr, "New Table is Size is Bigger Than %u!", UINT16_MAX);
		return 0;
	}

    newTable = (PageNode*) calloc(sizeof(PageNode), newSize);
	checkNullPointer((void*) newTable);

	oldTable = menu->table;
	menu -> table = newTable;
	menu -> tableSize = newSize;
        
	for (uint16_t i = 0; i < newSize; i++) {
		PageNode page = oldTable[i];

		if (page.hash) {
		    setEntry(menu, page);
		}
	}

	free(oldTable);
        
	return 1;
}


uint32_t addPage(MenuSt* menu, char* text, int hasOpt, ...) {

	va_list args;
	PageNode page;
	uint32_t hash;
	uint16_t index;
        
	if (!menu) {
		fprintf(stderr, "Menu Struct is Not Initialized!\n");
		return 0;
	}

	if (menu -> currElements >= menu->tableSize / 2)
		if (!expandTable(menu))
		    if (menu->currElements >= menu->tableSize - 1) {
				fprintf(stderr, "Table is Full!");
				return 0;
			}
        
    hash = hashKey(text);

    if (hasOpt)
		va_start(args, hasOpt);
    
	page.hash = hashKey(text);
	page.text = text;
	page.textFunc = (hasOpt) ? va_arg(args, char* (*)(char*, void*)) : NULL;
	page.actionFunc = (hasOpt) ? va_arg(args, void (*)(char*)) : NULL;
	page.forwardHashes = (hasOpt) ? va_arg(args, uint32_t*) : NULL;
	page.totalFoward = (hasOpt) ? va_arg(args, int) : 0;
	page.backHash = (hasOpt) ? va_arg(args, uint32_t) : hash;
	page.type = (hasOpt) ? va_arg(args, PageType) : TEXT;

    setEntry(menu, page);
        
	return hash;	
}


// MurmurHash2 - https://en.wikipedia.org/wiki/MurmurHash
uint32_t hashKey(const char* key) {
    size_t len = strlen(key);
    const uint32_t m = 0x5bd1e995;
    const int r = 24;
    uint32_t h = 0x811c9dc5 ^ len;

    const unsigned char* data = (const unsigned char*)key;
    while (len >= 4) {
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    switch (len) {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0];
                h *= m;
    };

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}
