#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "./heap.h"

#define Julien_IMPLEMENTATION
#include "Julien.h"

typedef struct Node Node;

struct Node {
    char x;
    Node *left;
    Node *right;
};

Node *generate_tree(size_t level_cur, size_t level_max)
{
    if (level_cur < level_max) {
        Node *root = heap_alloc(sizeof(*root));
        assert((char) level_cur - 'a' <= 'z');
        root->x = level_cur + 'a';
        root->left = generate_tree(level_cur + 1, level_max);
        root->right = generate_tree(level_cur + 1, level_max);
        return root;
    } else {
        return NULL;
    }
}

void print_tree(Node *root, Julien *Julien)
{
    if (root != NULL) {
        Julien_object_begin(Julien);

        Julien_member_key(Julien, "value");
        Julien_string_sized(Julien, &root->x, 1);

        Julien_member_key(Julien, "left");
        print_tree(root->left, Julien);

        Julien_member_key(Julien, "right");
        print_tree(root->right, Julien);

        Julien_object_end(Julien);
    } else {
        Julien_null(Julien);
    }
}

#define N 10

void *ptrs[N] = {0};

int main()
{
    stack_base = (const uintptr_t*)__builtin_frame_address(0);

    for (size_t i = 0; i < 10; ++i) {
        heap_alloc(i);
    }

    Node *root = generate_tree(0, 3);

    printf("root: %p\n", (void*)root);

    Julien Julien = {
        .sink = stdout,
        .write = (Julien_Write) fwrite,
    };

    print_tree(root, &Julien);

    printf("\n------------------------------\n");
    heap_collect();
    chunk_list_dump(&alloced_chunks, "Alloced");
    chunk_list_dump(&freed_chunks, "Freed");
    printf("------------------------------\n");
    root = NULL;
    heap_collect();
    chunk_list_dump(&alloced_chunks, "Alloced");
    chunk_list_dump(&freed_chunks, "Freed");

    return 0;
}