/*-----------------------------------------------------------------*/
/**
  @file   menu2.c
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
    int (*actionFunc) (char*, void*);
	uint32_t* forwardHashes;
	uint16_t totalFoward;
	uint32_t backHash;
	PageType type;
} PageNode;


typedef enum {
	PAGE_VALID_INPUT,
	PAGE_NORMAL_OP,
	PAGE_RETURN,
	PAGE_INVALID_INPUT,
	PAGE_ERROR,
	PAGE_EXIT
} PageCode;


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
    
		if (menu -> table) {

			for (int i = 0; i < menu -> tableSize; i++)
				if (menu -> table + i)
					if (menu->table[i].forwardHashes)
						free(menu -> table[i].forwardHashes);
                        
			free(menu -> table);
		}
		
		if (menu -> configs)
			free(menu -> configs);
                  
		free(menu);
	}
}

PageNode* getPageIndex(MenuSt* menu, uint32_t hash) {

    uint16_t originalIndex = (hash & (uint16_t) (menu->tableSize - 1));
    PageNode *table = menu -> table;

	uint16_t indexIter = originalIndex;
                      
	while (table[indexIter].hash && table[indexIter].hash != hash) {

          
		indexIter++;

		// Makes Table Circular
		if (indexIter >= menu->tableSize)
			indexIter = 0;

		// If All Pages Where Already Checked
		if (indexIter == originalIndex) {
			fprintf(stderr, "Page Don't Exist!\n");
			return NULL;
		}
	}
	
	return table + indexIter;
}


char* getUserInput() {

	char *ret;
	char *input = (char *)malloc(sizeof(char) * BUFFER_SIZE);
	checkNullPointer((void*) input);	
        
	printf("\nOption: ");
	ret = fgets(input, BUFFER_SIZE, stdin);

	if (!ret)
		return NULL;	
        
	return input;
}


PageCode getUInt16FromInput(char* input, uint16_t* optionPtr) {
	uint32_t option;
	char* end;
        
    option = strtoll(input, &end, 10);
    
    if (*end != '\n' || end == input || option < 0)
		return PAGE_INVALID_INPUT;

    *optionPtr = (uint16_t) option;
    
	return PAGE_VALID_INPUT;
}

PageCode processOption(MenuSt* menu, PageNode** page, uint16_t opt) {

	PageNode* aux = *page;

	// Option has special value 0 (return)
	if (!opt){
		*page = getPageIndex(menu, aux -> backHash);
		return PAGE_RETURN;
	}

	// If it's not return than it must be a Foward Option
	opt -= 1;
        
	// If page has no foward hashes or opt it's not in arry
	if (!aux->forwardHashes || opt > aux->totalFoward) {
		fprintf(stderr, "\nOption is Not In Forward Hashes Array\n");  
		return PAGE_INVALID_INPUT;
	}
	
	if (!aux -> forwardHashes[opt]) {
		fprintf(stderr, "\nOption is Not In Forward Hashes Array\n");
		return PAGE_INVALID_INPUT;
	}
        
	*page = getPageIndex(menu, aux->forwardHashes[opt]);
		
	return PAGE_VALID_INPUT;	
}

PageCode runPage(MenuSt* menu, PageNode** page) {

	PageNode* aux = *page;
	char* pageText = aux -> text;
	bool hasTextFunc = false;
	char* userInput = NULL;
	uint16_t option;
	PageCode retCode = PAGE_NORMAL_OP;
        
	if (aux -> textFunc) {
		pageText = aux -> textFunc(aux -> text, menu -> configs);
		hasTextFunc = true;
	}	
                
    switch (aux -> type) {

        case TEXT:
			printf("%s", pageText);
		    userInput = getUserInput();

			if (!userInput){
			    retCode = PAGE_INVALID_INPUT;
				break;
			}

			// Stop Warnings
			option = 0;
			retCode = getUInt16FromInput(userInput, &option);

			if (retCode == PAGE_INVALID_INPUT)
			    break;

			retCode = processOption(menu, page, option);
			break;

        case INPUT:

			if (!aux->actionFunc) {
				fprintf(stderr, "\nPage Has No Action Function!\n");
				return PAGE_ERROR;
			}
                        
			printf("%s", pageText);
		    userInput = getUserInput();
                    
			if (!userInput){
			    retCode = PAGE_INVALID_INPUT;
				break;
			}
                        
			if (aux->actionFunc(userInput, menu->configs)) {
				fprintf(stderr, "\nInvalid Input!\n");
				retCode = PAGE_INVALID_INPUT;			
			}
			
			*page = getPageIndex(menu, aux -> backHash);
			retCode = PAGE_RETURN;
                        
			break;

        case ACTION:
			if (!aux->actionFunc) {
				fprintf(stderr, "\nPage Has No Action Function!");
				return PAGE_ERROR;
			}
            
		    if (aux->actionFunc(NULL, menu->configs)){
				fprintf(stderr, "\nError In Action Function!");
				retCode = PAGE_ERROR;
			}

			*page = getPageIndex(menu, aux -> backHash);
			retCode = PAGE_RETURN;
			break;
	}

	if (hasTextFunc)
		free(pageText);

	if (userInput)
		free(userInput);	

	return retCode;
}


void runMenu(MenuSt *menu, uint32_t rootHash) {

	PageCode code = PAGE_NORMAL_OP;
	PageNode* page = getPageIndex(menu, rootHash);

	if (!page)
		return;	
        
    while (page) {
		PageNode* lastPage = page;
		code = runPage(menu, &page);

		switch (code) {

		    case PAGE_ERROR:	
			   fprintf(stderr, "\nError In Page!\n");
			   return;

		   case PAGE_RETURN:
			   if (lastPage->hash == rootHash)
				   return;
			   break;

		    default:
				continue;
		}
	}
}


void setEntry(MenuSt* menu, PageNode page) {

	uint32_t hash = page.hash;
	uint16_t index = (hash & (uint16_t) (menu->tableSize - 1));
	PageNode* table = menu -> table;
        

	while (table[index].hash) {

		if (hash == table[index].hash) {
			fprintf(stderr, "\nPage Already Exists!\n");
			return;
		}

		index++;
		if (index >= menu->tableSize)
			index = 0;
	}

	memcpy(table + index, &page, sizeof(PageNode));
}


int expandTable(MenuSt* menu) {

	size_t newSize = menu->tableSize * 2;
	PageNode* newTable, *oldTable;
	
	if (newSize >= UINT16_MAX) {
		fprintf(stderr, "\nNew Table is Size is Bigger Than %u!\n", UINT16_MAX);
		return 0;
	}

    newTable = (PageNode*) calloc(sizeof(PageNode), newSize);
	checkNullPointer((void*) newTable);

	oldTable = menu->table;
	menu -> table = newTable;
	menu -> tableSize = newSize;
        
	for (uint16_t i = 0; i < newSize; i++) {
		PageNode page = oldTable[i];

		if (page.hash)
		    setEntry(menu, page);
	}

	free(oldTable);
        
	return 1;
}


uint32_t addPage(MenuSt* menu, char* text, int hasOpt, ...) {

	va_list args;
	PageNode page;
	uint32_t hash;
        
	if (!menu) {
		fprintf(stderr, "\nMenu Struct is Not Initialized!\n");
		return 0;
	}

	if (menu -> currElements >= menu->tableSize / 2)
		if (!expandTable(menu))
		    if (menu->currElements >= menu->tableSize - 1) {
				fprintf(stderr, "\nTable is Full!\n");
				return 0;
			}
        
    hash = hashKey(text);

    if (hasOpt)
		va_start(args, hasOpt);
    
	page.hash = hashKey(text);
	page.text = text;
	page.textFunc = (hasOpt) ? va_arg(args, char* (*)(char*, void*)) : NULL;
	page.actionFunc = (hasOpt) ? va_arg(args, int (*)(char*, void*)) : NULL;
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
