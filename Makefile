# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -fPIC -pthread
LDFLAGS = -L. -lm -lpthread

# Targets and file names
MMANAGER_LIB = libmemory_manager.so
MMANAGER_SRC = memory_manager.c
MMANAGER_OBJ = memory_manager.o

LIST_APP = list_app
LIST_SRC = linked_list.c test_linked_list.c
LIST_OBJ = linked_list.o test_linked_list.o

# Default target to build both the library and the list application
all: mmanager list

# Build the memory manager as a shared library
mmanager: $(MMANAGER_SRC)
	$(CC) $(CFLAGS) -shared -o $(MMANAGER_LIB) $(MMANAGER_SRC)

# Build the linked list application and link with the memory manager library
list: $(LIST_SRC) $(MMANAGER_LIB)
	$(CC) $(CFLAGS) -o $(LIST_APP) $(LIST_SRC) -lmemory_manager $(LDFLAGS)

# Clean up object files, the dynamic library, and the list application
clean:
	rm -f $(MMANAGER_OBJ) $(LIST_OBJ) $(MMANAGER_LIB) $(LIST_APP)

# Phony targets to avoid conflicts with files named "all", "clean", etc.
.PHONY: all clean mmanager list
