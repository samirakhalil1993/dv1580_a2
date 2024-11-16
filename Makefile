# Compiler and Linking Variables
CC = gcc
CFLAGS = -Wall -fPIC
LIB_NAME = libmemory_manager.so

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

# Build the memory manager
mmanager: $(LIB_NAME)

# Build the linked list
list: linked_list.o

# Test target to run the memory manager test program
test_mmanager: $(LIB_NAME)
	$(CC) -o test_memory_manager test_memory_manager.c -L. -lmemory_manager -lm



# Test target to run the linked list test program
test_list: $(LIB_NAME) linked_list.o
	$(CC) -o test_linked_list linked_list.c test_linked_list.c -L. -lmemory_manager -lpthread -lm
	
#run tests
run_tests: run_test_mmanager run_test_list
	
# run test cases for the memory manager
run_test_mmanager: test_mmanager
	LD_LIBRARY_PATH=. ./test_memory_manager 0

# run test cases for the linked list
run_test_list: test_list
	LD_LIBRARY_PATH=. ./test_linked_list 0


# Clean target to clean up build files
clean:
	rm -f $(OBJ) $(LIB_NAME) test_memory_manager test_linked_list linked_list.o