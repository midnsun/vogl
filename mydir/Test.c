#include "SortLab.h"

int main(int args, const char* argv[]) {
    size_t size = 1000;
    size_t seqsize = 1000000;
    int seed;
    TYPE* mas;
    TYPE* ex;
    uint64_t* seq = (uint64_t*)malloc(seqsize * sizeof(uint64_t));
    mas = (TYPE*)malloc(size * sizeof(TYPE));
    ex = (TYPE*)malloc(size * sizeof(TYPE));
    seqPow2(seq, &seqsize, size);

    for (seed = 0; seed < 1000; seed++) {
        srand(seed);
        printf("seed %d\n", seed);
        fillm(mas, size, seed, -1.0, 1.0);
        _copy(mas, ex, 0, size);

        insertion(mas, size);
        if (check(mas, ex, size) == 0) printf("insertion sort - PASSED\n");
        else {
            printf("insertion sort - FAILED\n");
            scanf(" %d", &size);
            return 0;
        }
        _copy(ex, mas, 0, size);
        shake(mas, size);
        _shellsort(mas, size, seq, seqsize);
        if (check(mas, ex, size) == 0) printf("shell sort - PASSED\n");
        else {
            printf("shell sort - FAILED\n");
            scanf(" %d", &size);
            return 0;
        }
        _copy(ex, mas, 0, size);
        shake(mas, size);
        _mergesort(mas, size);
        if (check(mas, ex, size) == 0) printf("merge sort - PASSED\n");
        else {
            printf("merge sort - FAILED\n");
            scanf(" %d", &size);
            return 0;
        }
        _copy(ex, mas, 0, size);
        shake(mas, size);
        radix_sort_LSD(mas, size);
        if (check(mas, ex, size) == 0) printf("radix sort - PASSED\n");
        else {
            printf("radix sort - FAILED\n");
            scanf(" %d", &size);
            return 0;
        }

        printf("\n");
    }

    free(seq);
    free(mas);
    free(ex);

    return 0;
}
