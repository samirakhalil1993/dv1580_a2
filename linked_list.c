#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "memory_manager.h"

// Definition of a singly linked list node.
typedef struct Node {
    uint16_t data;        // The data stored in the node.
    struct Node* next;    // Pointer to the next node in the list.
} Node;

// Synchronization primitives for thread safety.
pthread_rwlock_t list_rwlock = PTHREAD_RWLOCK_INITIALIZER; // Read-Write lock for read-heavy functions.
pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;    // Mutex lock for write-heavy functions.

// Initializes a linked list and the custom memory manager.
// Parameters:
// - head: Pointer to the head pointer of the linked list.
// - size: Size of the memory pool to be initialized.
void list_init(Node** head, size_t size) {
    *head = NULL;
    mem_init(size);
}

// Inserts a new node at the end of the list.
void list_insert(Node** head, uint16_t data) {
    pthread_mutex_lock(&list_mutex); // Exclusive lock for write operation

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        printf("Memory allocation failed\n");
        pthread_mutex_unlock(&list_mutex); // Release lock on failure
        return;
    }

    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        Node* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }

    pthread_mutex_unlock(&list_mutex); // Release lock after insertion
}

// Inserts a new node immediately after a given node.
void list_insert_after(Node* prev_node, uint16_t data) {
    if (prev_node == NULL) {
        printf("Previous node cannot be NULL\n");
        return;
    }

    pthread_mutex_lock(&list_mutex); // Exclusive lock for write operation

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        printf("Memory allocation failed\n");
        pthread_mutex_unlock(&list_mutex); // Release lock on failure
        return;
    }

    new_node->data = data;
    new_node->next = prev_node->next;
    prev_node->next = new_node;

    pthread_mutex_unlock(&list_mutex); // Release lock after insertion
}

// Inserts a new node before a given node.
void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (next_node == NULL) {
        printf("Next node cannot be NULL\n");
        return;
    }

    pthread_mutex_lock(&list_mutex); // Exclusive lock for write operation

    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (!new_node) {
        printf("Memory allocation failed\n");
        pthread_mutex_unlock(&list_mutex); // Release lock on failure
        return;
    }
    new_node->data = data;

    if (*head == next_node) {
        new_node->next = *head;
        *head = new_node;
    } else {
        Node* current = *head;
        while (current != NULL && current->next != next_node) {
            current = current->next;
        }

        if (current == NULL) {
            printf("The specified next node is not in the list\n");
            mem_free(new_node);
            pthread_mutex_unlock(&list_mutex); // Release lock on failure
            return;
        }

        new_node->next = next_node;
        current->next = new_node;
    }

    pthread_mutex_unlock(&list_mutex); // Release lock after insertion
}

// Deletes the first node with the specified data.
void list_delete(Node** head, uint16_t data) {
    pthread_mutex_lock(&list_mutex); // Exclusive lock for write operation

    if (*head == NULL) {
        printf("List is empty\n");
        pthread_mutex_unlock(&list_mutex); // Release lock if list is empty
        return;
    }

    Node* current = *head;
    Node* previous = NULL;

    while (current != NULL && current->data != data) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("Data not found in the list\n");
        pthread_mutex_unlock(&list_mutex); // Release lock if data not found
        return;
    }

    if (previous == NULL) {
        *head = current->next;
    } else {
        previous->next = current->next;
    }

    mem_free(current);

    pthread_mutex_unlock(&list_mutex); // Release lock after deletion
}

// Searches for a node with the specified data.
Node* list_search(Node** head, uint16_t data) {
    pthread_rwlock_rdlock(&list_rwlock); // Shared lock for read-only operation

    Node* current = *head;
    while (current != NULL) {
        if (current->data == data) {
            pthread_rwlock_unlock(&list_rwlock); // Release lock on find
            return current;
        }
        current = current->next;
    }

    pthread_rwlock_unlock(&list_rwlock); // Release lock if not found
    return NULL;
}

// Displays all elements in the list.
void list_display(Node** head) {
    pthread_rwlock_rdlock(&list_rwlock); // Shared lock for read-only operation

    Node* current = *head;
    printf("[");
    while (current != NULL) {
        printf("%u", current->data);
        if (current->next != NULL) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]");

    pthread_rwlock_unlock(&list_rwlock); // Release lock after display
}

void list_display_range(Node** head, Node* start_node, Node* end_node) {
    pthread_rwlock_rdlock(&list_rwlock); // Acquire read lock

    Node* current = start_node ? start_node : *head;
    printf("[");
    while (current != NULL && (end_node == NULL || current != end_node->next)) {
        printf("%u", current->data);
        if (current->next != NULL && current != end_node) {
            printf(", ");
        }
        current = current->next;
    }
    printf("]");

    pthread_rwlock_unlock(&list_rwlock); // Release read lock
}

// Counts the number of nodes in the list.
int list_count_nodes(Node** head) {
    pthread_rwlock_rdlock(&list_rwlock); // Shared lock for read-only operation

    int count = 0;
    Node* current = *head;
    while (current != NULL) {
        count++;
        current = current->next;
    }

    pthread_rwlock_unlock(&list_rwlock); // Release lock after counting
    return count;
}

// Frees all nodes in the list and deinitializes the memory manager.
void list_cleanup(Node** head) {
    pthread_mutex_lock(&list_mutex); // Exclusive lock for cleanup

    Node* current = *head;
    while (current != NULL) {
        Node* next_node = current->next;
        mem_free(current);
        current = next_node;
    }
    *head = NULL;
    mem_deinit();

    pthread_mutex_unlock(&list_mutex); // Release lock after cleanup
}
