#include "heap.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

// Helper functions
static void min_heapify_down(MinHeap* heap, size_t index);//function to maintain the heap property after extraction
static void min_heapify_up(MinHeap* heap, size_t index);//function to maintain the heap property after insertion
static void resize_heap(MinHeap* heap);//function to resize the heap's capacity
time_t get_min_time(MinHeap* heap);//function to get the time for the first node without removing it


// Function to create a min-heap
MinHeap* create_min_heap() {
    MinHeap* heap = malloc(sizeof(MinHeap));
    if (!heap) {
        fprintf(stderr, "Failed to create heap\n");
        return NULL;
    }
    // Initialize the heap
    heap->data = NULL;
    heap->size = 0;
    heap->capacity = 0;
    return heap;
}

// Function to free a min-heap
void free_min_heap(MinHeap* heap) {
    if (heap) {
        free(heap->data);
        free(heap);
    }
}

// Function to insert a file number and time into the heap
void min_heap_insert(MinHeap* heap, int file_number, time_t creation_time) {
    if (!heap) return;

    if (heap->size == heap->capacity) {//check if the heap is full
        resize_heap(heap);//resize the heap
    }
    // Insert the new node at the end of the heap
    heap->data[heap->size].file_number = file_number;
    heap->data[heap->size].creation_time = creation_time;
    heap->size++;
    min_heapify_up(heap, heap->size - 1);//maintain the heap property
}

// Function to extract the minimum time node from the heap
HeapNode min_heap_extract(MinHeap* heap) {
    if (!heap || heap->size == 0) {
        fprintf(stderr, "Heap underflow\n");
        return (HeapNode){.file_number = -1, .creation_time = 0};
    }
//extract the minimum node from the heap
    HeapNode min_node = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    min_heapify_down(heap, 0);
    return min_node;
}

// Function to print the heap contents
void print_heap(MinHeap* heap) {
    printf("Heap contents:\n");
    for (size_t i = 0; i < heap->size; i++) {
        printf("File Number: %d, Creation Time: %ld\n",
               heap->data[i].file_number, (long)heap->data[i].creation_time);
    }
    printf("\n");
    
}

// Helper function to maintain the heap property after insertion
static void min_heapify_up(MinHeap* heap, size_t index) {
    if (index == 0) return;
    // Compare the node with its parent and swap if necessary
    size_t parent = (index - 1) / 2;
    if (heap->data[index].creation_time < heap->data[parent].creation_time) {
        HeapNode temp = heap->data[index];
        heap->data[index] = heap->data[parent];
        heap->data[parent] = temp;

        min_heapify_up(heap, parent);
    }
}

// Helper function to maintain the heap property after extraction
static void min_heapify_down(MinHeap* heap, size_t index) {
    size_t smallest = index;
    size_t left = 2 * index + 1;
    size_t right = 2 * index + 2;

    if (left < heap->size && heap->data[left].creation_time < heap->data[smallest].creation_time) {//check if the left child is smaller than the parent
        smallest = left;
    }
    if (right < heap->size && heap->data[right].creation_time < heap->data[smallest].creation_time) {//check if the right child is smaller than the parent
        smallest = right;
    }

    if (smallest != index) {//swap the parent
        HeapNode temp = heap->data[index];
        heap->data[index] = heap->data[smallest];
        heap->data[smallest] = temp;

        min_heapify_down(heap, smallest);//maintain the heap property
    }
}

// Helper function to resize the heap's capacity
static void resize_heap(MinHeap* heap) {
    heap->capacity = (heap->capacity == 0) ? 2 : heap->capacity * 2;//double the capacity
    heap->data = realloc(heap->data, heap->capacity * sizeof(HeapNode));//reallocate the memory
    if (!heap->data) {
        fprintf(stderr, "Heap resizing failed\n");
        exit(EXIT_FAILURE);
    }
}

// Function to get the time for the first node without removing it
time_t get_min_time(MinHeap* heap) {
    if (!heap || heap->size == 0) {
        fprintf(stderr, "Heap is empty\n");
        return (time_t)-1;  // Return -1 to indicate an error
    }
    return heap->data[0].creation_time;//return the time of the first node
}

// remove a node from the heap by file number 
void remove_node(MinHeap* heap, int file_number) {
    if (!heap || heap->size == 0) {
        fprintf(stderr, "Heap is empty\n");
        return;
    }

    size_t i;
    for (i = 0; i < heap->size; i++) {
        if (heap->data[i].file_number == file_number) {
            break;
        }
    }

    if (i == heap->size) {
        fprintf(stderr, "File number not found in heap\n");
        return;
    }

    heap->data[i] = heap->data[heap->size - 1];
    heap->size--;
    min_heapify_down(heap, i);
}