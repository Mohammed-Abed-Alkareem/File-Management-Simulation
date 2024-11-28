#ifndef HEAP_H
#define HEAP_H

#include <time.h>
#include <stddef.h>

// Structure to store file number and creation time
typedef struct {
    int file_number;
    time_t creation_time;
} HeapNode;

// Min-Heap structure
typedef struct {
    HeapNode* data;  // Dynamic array to store heap nodes
    size_t size;     // Current number of elements
    size_t capacity; // Current capacity
} MinHeap;

// Function declarations
MinHeap* create_min_heap();
void free_min_heap(MinHeap* heap);
void min_heap_insert(MinHeap* heap, int file_number, time_t creation_time);
HeapNode min_heap_extract(MinHeap* heap);
void print_heap(MinHeap* heap);

#endif // HEAP_H
