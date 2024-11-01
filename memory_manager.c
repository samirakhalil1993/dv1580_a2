#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

pthread_mutex_t memory_mutex;

// Structure to represent a memory block in the pool
typedef struct Block {
    size_t size;           // Size of the block
    int is_free;           // 1 if the block is free, 0 if it is allocated
    struct Block* next;    // Pointer to the next block
    void* ptr;             // Pointer to the memory within the pool
} Block;

void* memory_pool = NULL;  // Pointer to the start of the memory pool
Block* head_block = NULL;  // Head of the linked list of memory blocks
size_t memory_pool_size = 0;

// Initializes the memory pool with the specified size
void mem_init(size_t size) {
    pthread_mutex_init(&memory_mutex, NULL);
    memory_pool = malloc(size);
    if (!memory_pool) {
        perror("Memory pool allocation failed");
        exit(EXIT_FAILURE);
    }

    memory_pool_size = size;

    head_block = (Block*)malloc(sizeof(Block));
    if (!head_block) {
        perror("Block metadata allocation failed");
        free(memory_pool);
        exit(EXIT_FAILURE);
    }

    head_block->size = size;
    head_block->is_free = 1;
    head_block->ptr = memory_pool;
    head_block->next = NULL;

    #ifdef DEBUG
    printf("Initialized memory pool of size %zu at %p\n", size, memory_pool);
    #endif
}

void* mem_alloc(size_t size) {
    
    pthread_mutex_lock(&memory_mutex);

    Block* current = head_block;
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            if (current->size > size) {
                Block* new_block = (Block*)malloc(sizeof(Block));
                if (!new_block) {
                    perror("New block metadata allocation failed");
                    pthread_mutex_unlock(&memory_mutex);
                    return NULL;
                }

                new_block->size = current->size - size;
                new_block->is_free = 1;
                new_block->ptr = (char*)current->ptr + size;
                new_block->next = current->next;

                current->size = size;
                current->is_free = 0;
                current->next = new_block;
            } else {
                current->is_free = 0;
            }

            #ifdef DEBUG
            printf("Allocated %zu bytes at %p\n", size, current->ptr);
            #endif

            pthread_mutex_unlock(&memory_mutex);
            return current->ptr;
        }
        current = current->next;
    }

    pthread_mutex_unlock(&memory_mutex);
    return NULL;  // Allocation failed
}


// Frees a previously allocated block of memory
void mem_free(void* ptr) {
    if (!ptr) {
        fprintf(stderr, "Warning: Attempted to free a NULL pointer.\n");
        return;
    }

    pthread_mutex_lock(&memory_mutex);

    Block* current = head_block;
    while (current != NULL) {
        if (current->ptr == ptr) {
            if (current->is_free) {
                fprintf(stderr, "Warning: Attempted to free an already freed block at %p.\n", ptr);
                pthread_mutex_unlock(&memory_mutex);
                return;
            }

            current->is_free = 1;

            // Attempt to coalesce adjacent free blocks
            Block* next_block = current->next;
            while (next_block != NULL && next_block->is_free) {
                current->size += next_block->size;
                current->next = next_block->next;
                free(next_block);
                next_block = current->next;
            }

            #ifdef DEBUG
            printf("Freed block at %p\n", ptr);
            #endif

            pthread_mutex_unlock(&memory_mutex);
            return;
        }
        current = current->next;
    }

    fprintf(stderr, "Warning: Pointer %p not found in the memory pool.\n", ptr);
    pthread_mutex_unlock(&memory_mutex);
}

// Resizes a previously allocated block of memory
void* mem_resize(void* ptr, size_t size) {
    pthread_mutex_lock(&memory_mutex);

    if (!ptr) {
        void* new_ptr = mem_alloc(size);
        pthread_mutex_unlock(&memory_mutex);
        return new_ptr;
    }

    Block* block = head_block;
    while (block != NULL) {
        if (block->ptr == ptr) {
            if (block->size >= size) {
                pthread_mutex_unlock(&memory_mutex);
                return ptr;
            } else {
                void* new_ptr = mem_alloc(size);
                if (new_ptr) {
                    memcpy(new_ptr, ptr, block->size);
                    mem_free(ptr);
                }
                pthread_mutex_unlock(&memory_mutex);
                return new_ptr;
            }
        }
        block = block->next;
    }

    fprintf(stderr, "Warning: Pointer %p not found for resizing.\n", ptr);
    pthread_mutex_unlock(&memory_mutex);
    return NULL;
}

// Deinitializes the memory pool and frees all associated resources
void mem_deinit() {
    pthread_mutex_lock(&memory_mutex);

    free(memory_pool);
    memory_pool = NULL;

    Block* current = head_block;
    while (current != NULL) {
        Block* next = current->next;
        free(current);
        current = next;
    }

    head_block = NULL;
    memory_pool_size = 0;

    pthread_mutex_unlock(&memory_mutex);
    pthread_mutex_destroy(&memory_mutex);

    #ifdef DEBUG
    printf("Deinitialized memory pool\n");
    #endif
}
