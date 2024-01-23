//Fonction free

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>

// Structure pour le suivi des blocs de mémoire allouée
typedef struct Block {
    size_t size;
    struct Block *next;
} Block;

// Pointeur vers le premier bloc dans la liste des blocs libres
static Block *free_list = NULL;

void free ( void *pointeur){

    //Vérification du pointeur 
    if (pointeur == NULL) {
    return;
    }
    // Conversion du pointeur de retour à la structure Block
    Block *block = (Block *)pointeur - 1;

    // Ajout du bloc à la liste des blocs libres
    block->next = free_list;
    free_list = block;

    // Fusionner les blocs libres adjacents
    Block *current = free_list;
    while (current != NULL && current->next != NULL) {
        if ((char *)current + sizeof(Block) + current->size == (char *)current->next) {
            current->size += sizeof(Block) + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }

}


