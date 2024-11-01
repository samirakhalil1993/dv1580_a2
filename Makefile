# Compiler and Linking Variables
CC = gcc
CFLAGS = -Wall -fPIC -pthread -g
LIB_NAME = libmemory_manager.so
LDFLAGS = -L. -lmemory_manager -lm

# Source and Object Files
SRC = memory_manager.c
OBJ = $(SRC:.c=.o)

# Default target
all: mmanager list test_mmanager test_list

# Rule to create the dynamic library
$(LIB_NAME): $(OBJ)
	$(CC) -shared -o $@ $(OBJ)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build the memory manager library
mmanager: $(LIB_NAME)

# Build the linked list object file (dependency for test_list)
linked_list.o: linked_list.c
	$(CC) $(CFLAGS) -c linked_list.c -o linked_list.o

# Build the test for the memory manager
test_mmanager: $(LIB_NAME)
	$(CC) $(CFLAGS) -o test_memory_manager test_memory_manager.c $(LDFLAGS)

# Build the test for the linked list
test_list: $(LIB_NAME) linked_list.o
	$(CC) $(CFLAGS) -o test_linked_list linked_list.o test_linked_list.c $(LDFLAGS)

# Run both test programs
run_tests: run_test_mmanager run_test_list

# Run the memory manager test cases with a default argument
run_test_mmanager: test_mmanager
	LD_LIBRARY_PATH=. ./test_memory_manager 0

# Run the linked list test cases
run_test_list: test_list
	LD_LIBRARY_PATH=. ./test_linked_list 0

# Clean up build files
clean:
	rm -f $(OBJ) $(LIB_NAME) test_memory_manager test_linked_list linked_list.o
