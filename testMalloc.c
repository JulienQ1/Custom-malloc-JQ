#include <stdio.h>
#include <unistd.h>

#define CAPACITY 640000

typedef struct block_meta {
    size_t size;
    struct block_meta* next;
    int free;
} block_meta;

block_meta* global_base = NULL;

block_meta* find_free_block(block_meta** last, size_t size) {
    block_meta* current = global_base;
    while (current && !(current->free && current->size >= size)) {
        *last = current;
        current = current->next;
    }
    return current;
}

block_meta* request_space(block_meta* last, size_t size) {
    block_meta* block;
    block = sbrk(0);
    void* request = sbrk(size + sizeof(block_meta));
    if (request == (void*) -1) {
        return NULL; // sbrk failed.
    } else {
        if (last) { // NULL on first request.
            last->next = block;
        }
        block->size = size;
        block->next = NULL;
        block->free = 0;
        return block;
    }
}

void* heap_alloc(size_t size) {
    block_meta* block;
    if (size <= 0) {
        return NULL;
    }
    if (!global_base) { // First call.
        block = request_space(NULL, size);
        if (!block) {
            return NULL;
        }
        global_base = block;
    } else {
        block_meta* last = global_base;
        block = find_free_block(&last, size);
        if (!block) { // Failed to find free block.
            block = request_space(last, size);
            if (!block) {
                return NULL;
            }
        } else {      // Found free block
            block->free = 0;
        }
    }
    return(block+1);
}

void heap_free(void* ptr) {
    if (!ptr) {
        return;
    }
    // TODO: consider merging blocks once splitting blocks is implemented.
    block_meta* block_ptr = get_block_ptr(ptr);
    block_ptr->free = 1;
}

void heap_collect() {
    // TODO: implement this function to merge adjacent free blocks.
}