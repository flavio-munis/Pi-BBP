# MakeFile For Pi-BBP Project

# Project Name
PROJECT_NAME = pi-bbp

# Compilation Options
CC = gcc
CC_FLAGS =  -c \
	-lm \
	-Wall \
	-pthread \
	-pedantic \
	-o

# Folders
SRC = ./src
APP = ./app
OBJ = ./obj
INCLUDE = ./include
TEMP = ./temp

# Files
MAIN = ${APP}/main.c
MAIN_OBJ = ${OBJ}/main.o
C_HEADERS = $(wildcard ${INCLUDE}/*.h)
C_SOURCE = $(wildcard ${SRC}/*.c)
OBJ_SOURCE = $(subst .c,.o,$(subst $(SRC),$(OBJ), $(C_SOURCE)))

TEMP_SOURCES = $(wildcard ${TEMP}/*.c)
TEMP_EXEC_NAMES = $(patsubst ${TEMP}/%.c, %, ${TEMP_SOURCES})

# Builds Project
all: ${OBJ} $(PROJECT_NAME)

temp: $(TEMP_EXEC_NAMES)

$(TEMP_EXEC_NAMES): % : $(TEMP)/%.c
	@ echo 'Compiling $< as $@...'
	@ $(CC) $< -lm -Wall -mavx2 -march=native -pg -o $@
	@ echo '$@ Compiled!'

$(OBJ):
	@ mkdir obj
	@ echo "Object Folder Created"

$(PROJECT_NAME): $(OBJ_SOURCE) $(MAIN_OBJ)
	@ echo 'Compiling Executable File...'
	@ $(CC) $^ -lm -o $@ -O3
	@ echo 'Finished Building Project!'

$(OBJ)/%.o: $(SRC)/%.c $(INCLUDE)/%.h
	@ echo 'Building and Linking File: $@'
	@ $(CC) $< $(CC_FLAGS) $@ -O3
	@ echo ' '

$(OBJ)/main.o: $(APP)/main.c
	@ echo 'Building and Linking Main File: $@'
	@ $(CC) $< $(CC_FLAGS) $@
	@ echo ' '

# Clean Files
clean: clean_obj clean_core clean_auto_save

clean_obj:
	@ echo "Cleaning All Object Files..."
	@ rm -rf obj/

clean_core:
	@ echo "Cleaning All Core Files..."
	@ rm -f ./core*

clean_auto_save:
	@ echo "Cleaning All AutoSave Files.."
	@ rm -rf ./*~

clean_seq:
	@ echo "Removing seq Executable..."
	@ rm -f ./seq

# Run Project
run:
ifdef ARGS
	@ ./${PROJECT_NAME} ${ARGS}
else
	@ ./${PROJECT_NAME}
endif
