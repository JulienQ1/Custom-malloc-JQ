#include <unistd.h>
#include <string.h>

typedef struct Block {
    size_t size;
    struct Block* next;
    int free;
    char data[1];
} Block;

#define BLOCK_SIZE sizeof(Block)

Block* free_list = NULL;

Block* find_free_block(Block** last, size_t size) {
    Block* block = free_list;
    while (block && !(block->free && block->size >= size)) {
        *last = block;
        block = block->next;
    }
    return block;
}

Block* extend_heap(Block* last, size_t size) {
    Block* block;
    block = sbrk(0);
    void* request = sbrk(size + BLOCK_SIZE);
    assert((void*)block == request);
    if (request == (void*) -1) {
        return NULL;
    }

    if (last) {
        last->next = block;
    }
    block->size = size;
    block->next = NULL;
    block->free = 0;
    return block;
}

void* heap_malloc(size_t size) {
    Block* block;
    if (size <= 0) {
        return NULL;
    }

    if (!free_list) {
        block = extend_heap(NULL, size);
        if (!block) {
            return NULL;
        }
        free_list = block;
    } else {
        Block* last = free_list;
        block = find_free_block(&last, size);
        if (!block) {
            block = extend_heap(last, size);
            if (!block) {
                return NULL;
            }
        } else {
            block->free = 0;
        }
    }

    return block->data;
}

void heap_free(void* ptr) {
    if (!ptr) {
        return;
    }

    Block* block = (Block*)ptr - 1;
    block->free = 1;
}

void* heap_realloc(void* ptr, size_t size) {
    if (!ptr) {
        return heap_malloc(size);
    }

    Block* block = (Block*)ptr - 1;
    if (block->size >= size) {
        return ptr;
    }

    void* new_ptr;
    new_ptr = heap_malloc(size);
    if (!new_ptr) {
        return NULL;
    }
    memcpy(new_ptr, ptr, block->size);
    heap_free(ptr);
    return new_ptr;
}