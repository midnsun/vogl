#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

typedef float TYPE;
typedef unsigned long long int uint64_t;
typedef unsigned long long int size_t;
typedef unsigned char uint8_t;

//utilities
void _swap(TYPE* a, TYPE* b);
void _swap_i(uint64_t* a, uint64_t* b);
void _swap_p(TYPE** a, TYPE** b);
void _swap_p_i(uint64_t** a, uint64_t** b);
int _min(int v1, int v2);
int _max(int v1, int v2);
void _copy(TYPE* src, TYPE* dst, size_t first, size_t last);
//data working
void fillm(float* m, size_t size, unsigned int seed, float left, float right);
void _reverse(uint64_t* m, size_t size);
void shake(TYPE* m, size_t size);
void mprint(TYPE* m, size_t size);
void mclear(uint64_t* m, size_t size);
int mdelete(TYPE* m, size_t* size, TYPE x);
int check(TYPE* m, TYPE* exl, size_t size);
//sorts
void foffset(uint8_t* mas, size_t size, size_t offset, uint64_t count[256]);
void radix_sort_LSD(TYPE* mas, size_t size);
void radix_sort_LSD_i(uint64_t* mas, size_t size);
void insertion(TYPE* m, size_t size);
void _shellsort(TYPE* m, size_t size, uint64_t* seq, size_t seqsize);
void seqHalfSize(uint64_t* seq, size_t* seqsize, const size_t size);
void seqPow2Pow3(uint64_t* seq, size_t* seqsize, const size_t size);
void seqPow2(uint64_t* seq, size_t* seqsize, const size_t size); //best to use
void _merge(TYPE* a, TYPE* b, TYPE* data, size_t an, size_t bn);
void _mergesort(TYPE* m, int size);