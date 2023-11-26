#include "SortLab.h"

////////////////////////////////////////////////////UTILITIES////////////////////////////////////////////////////

void _swap(TYPE* a, TYPE* b) {
    TYPE tmp = *a;
    *a = *b;
    *b = tmp;
}

void _swap_i(uint64_t* a, uint64_t* b) {
    uint64_t tmp = *a;
    *a = *b;
    *b = tmp;
}

void _swap_p(TYPE** a, TYPE** b) {
    TYPE* tmp = *a;
    *a = *b;
    *b = tmp;
}

void _swap_p_i(uint64_t** a, uint64_t** b) {
    uint64_t* tmp = *a;
    *a = *b;
    *b = tmp;
}

int _min(int v1, int v2) { return  v1 < v2 ? v1 : v2; }
int _max(int v1, int v2) { return  v1 > v2 ? v1 : v2; }

void _copy(TYPE* src, TYPE* dst, size_t first, size_t last) {
    size_t i;
    for (i = first; i < last; i++) dst[i] = src[i];
}

////////////////////////////////////////////////////DATA WORKING////////////////////////////////////////////////////

void fillm(float* m, size_t size, unsigned int seed, float left, float right) {
    size_t i;
    srand(seed);
    for (i = 0; i < size; i++) m[i] = left + ((double)rand() / RAND_MAX)*(right - left);
}

void _reverse(uint64_t* m, size_t size) {
    size_t i;
    for (i = 0; i < size / 2; i++) {
        _swap_i(&m[i], &m[size - i - 1]);
    }
}

void shake(TYPE* m, size_t size) {
    size_t i, ind = 0;
    for (i = 0; i < size; i++) {
        ind = i + rand() % (size - i);
        _swap(&m[i], &m[ind]);
    }
}

void mprint(TYPE* m, size_t size) {
    size_t i;
    for (i = 0; i < size; i++) printf("%f ", m[i]);
    printf("\n");
}

void mclear(uint64_t* m, size_t size) {
    size_t i;
    for (i = 0; i < size; i++) m[i] = 0;
}

int mdelete(TYPE* m, size_t* size, TYPE x) {
    size_t i;
    TYPE temp = m[*size - 1];
    for (i = *size - 1; i >= 0; i--) {
        if (temp == x) {
            *size = *size - 1;
            return 0;
        }
        _swap(&temp, &m[i - 1]);
    }
    return -1;
}

int check(TYPE* m, TYPE* exl, size_t size) {
    size_t tsize = size;
    size_t i;
    for (i = 1; i < size; i++) if (m[i] < m[i - 1]) return -1;

    for (i = 0; i < size; i++) {
        if (mdelete(m, &tsize, exl[i]) == -1) return -1;
    }
    if (tsize != 0) return -1;
    return 0;
}

////////////////////////////////////////////////////SORTS////////////////////////////////////////////////////

void foffset(uint8_t* mas, size_t size, size_t offset, uint64_t count[255 + 1]) {
    size_t i;
    uint64_t tmp;
    memset(count, 0, (255 + 1) * 8);
    for (i = 0; i < size * sizeof(TYPE); i += sizeof(TYPE))
        count[mas[i + offset]]++;
    tmp = count[0];
    count[0] = 0;
    for (i = 0; i < 255; i++) {
        _swap_i(&tmp, &count[i + 1]);
        count[i + 1] += count[i];
    }
}

void radix_sort_LSD(TYPE* mas, size_t size) {
    size_t i, j, k;
    uint8_t* pm;
    uint64_t count[255 + 1];
    TYPE* mas2 = (TYPE*)malloc(size * sizeof(TYPE));
    pm = (uint8_t*)mas;

    for (i = 0; i < sizeof(TYPE); i++) {
        foffset(pm, size, i, count);
        for (j = 0; j < size; j++)
            mas2[count[pm[j * sizeof(TYPE) + i]]++] = mas[j];
        _swap_p(&mas, &mas2);
        pm = (uint8_t*)mas;
    }

    k = 0;
    if (mas[size - 1] < 0) {
        for (i = size - 1; i >= 0; i--) {
            if (mas[i] > 0) break;
            mas2[k++] = mas[i];
        }

        for (i = 0; i < size; i++) {
            if (mas[i] < 0) break;
            mas2[k++] = mas[i];
        }

        for (i = 0; i < size; i++) mas[i] = mas2[i];
    }

    free(mas2);
}

void radix_sort_LSD_i(uint64_t* mas, size_t size) {
    size_t i, j;
    uint8_t* pm;
    uint64_t count[255 + 1];
    uint64_t* mas2 = (uint64_t*)malloc(size * sizeof(uint64_t));
    pm = (uint8_t*)mas;

    for (i = 0; i < sizeof(TYPE); i++) {
        foffset(pm, size, i, count);
        for (j = 0; j < size; j++)
            mas2[count[pm[j * sizeof(TYPE) + i]]++] = mas[j];
        _swap_p_i(&mas, &mas2);
        pm = (uint8_t*)mas;
    }

    free(mas2);
}

void insertion(TYPE* m, size_t size) {
    size_t i, j;
    for (i = 0; i < size - 1; i++) {
        j = i + 1;
        while (j > 0 && m[j - 1] > m[j]) {
            _swap(&m[j - 1], &m[j]);
            j--;
        }
    }
}

void _shellsort(TYPE* m, size_t size, uint64_t* seq, size_t seqsize) {
    int s, i, j;
    for (s = 0; s < seqsize && seq[s] > 0 && size / seq[s] > 0; s++) {
        for (i = seq[s]; i < size; i++) {
            for (j = i - seq[s]; j >= 0 && m[j + seq[s]] < m[j]; j -= seq[s]) {
                _swap(&m[j], &m[j + seq[s]]);
            }
        }
    }
}

void seqHalfSize(uint64_t* seq, size_t* seqsize, const size_t size) {
    size_t i;
    mclear(seq, *seqsize);
    seq[0] = size / 2;
    for (i = 1; i < *seqsize; i++) {
        seq[i] = seq[i - 1] / 2;
        if (seq[i] == 0) break;
    }
    *seqsize = i;
}

void seqPow2Pow3(uint64_t* seq, size_t* seqsize, const size_t size) {
    size_t i, a, b;
    mclear(seq, *seqsize);
    i = 0;
    for (a = 0; pow(2, a) <= size / 2; a++) {
        for (b = 0; pow(2, a) * pow(3, b) <= size / 2; b++) {
            seq[i++] = (uint64_t)(pow(2, a) * pow(3, b));
        }
    }
    *seqsize = i;
    radix_sort_LSD_i(seq, *seqsize);
    _reverse(seq, *seqsize);
}

void seqPow2(uint64_t* seq, size_t* seqsize, const size_t size) { //best to use
    size_t i;
    mclear(seq, *seqsize);
    *seqsize = (size_t)(log2(size)) + 2;
    for (i = (size_t)(log2(size)) + 1; i > 0; i--) seq[(size_t)(log2(size)) + 1 - i] = (size_t)(pow(2, i - 1)) - 1;
}

void _merge(TYPE* a, TYPE* b, TYPE* data, size_t an, size_t bn) {
    size_t ai = 0;
    size_t bi = 0;
    size_t di = 0;

    while (ai < an && bi < bn) {
        if (a[ai] < b[bi]) data[di++] = a[ai++];
        else data[di++] = b[bi++];
    }
    for (; ai < an; ai++) data[di++] = a[ai];
    for (; bi < bn; bi++) data[di++] = b[bi];
}

void _mergesort(TYPE* m, int size) {
    size_t counter = 0;
    int step, i;
    TYPE* tmpdata = (TYPE*)malloc(size * sizeof(TYPE));
    for (step = 1; step < size; step *= 2) {
        for (i = 0; i < size; i += 2 * step) {
            _merge(m + i, m + i + step, tmpdata + i, _max(0, _min(step, size - i)), _max(0, _min(step, size - step - i)));
        }
        _swap_p(&m, &tmpdata);
        counter++;
    }
    if ((counter % 2) != 0) {
        _swap_p(&m, &tmpdata);
        _copy(tmpdata, m, 0, size);
    }
    free(tmpdata);
}