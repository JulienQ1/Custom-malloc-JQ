#include <stdlib.h>
#include <stdio.h>

//static
int my_variable = 80;

int main() {

    //dynamic 
    int *x = malloc(sizeof(int));       //give me space for one int
    int *arr = malloc(sizeof(int)*100)//give me space for 100 ints

    *x = 120;
    arr[90] = 0xFEEDBEEF
    arr[101] = 8; //OUT OF BOUNDS.This is bad.

    free(arr);//I don't want this memory anymore.
    arr= NULL;

    double * darr;
    darr = calloc(sizeof(double)100);
    //is the same as -- darr = malloc(sizeof(double)*100);

    darr = realloc(darr, sizeof(double)*500);

}