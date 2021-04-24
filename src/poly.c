#include <stdlib.h>
#include "poly.h"

size_t max(size_t a, size_t b) {
    return a > b ? a : b;
}

Mono *AddMonoArrays(const Poly *p1, const Poly *p2, size_t *new_array_size) {
    Mono *m1 = p1->arr, *m2 = p2->arr;
    size_t size1 = p1->size, size2 = p2->size;
    Mono *new_mono_array = calloc(size1 + size2, sizeof(Mono));
    size_t last_index = size1;
    for (size_t i = 0; i < size1; i++) {
        new_mono_array[i] = m1[i];
    }
    for (size_t i = 0; i < size2; i++) {
        bool is_added = false;
        Mono curr_mono = m2[i];
        for (size_t j = 0; j < size1; j++) {
            if (curr_mono.exp == new_mono_array[j].exp) {
                new_mono_array[j].p = PolyAdd(&new_mono_array[i].p, &curr_mono.p);
                is_added = true;
                break;
            }
        }
        if (!is_added) {
            new_mono_array[last_index] = m2[i];
            last_index++;
        }
    }
    *new_array_size = last_index;
    return new_mono_array;
}

Poly PolyAddCoeff(const Poly *p, const Poly *c) {
    Mono *new_mono_array = calloc(p->size + 1, sizeof(Mono));
    size_t exp_zero_index = p->size;
    for (size_t i = 0; i < p->size; i++) {
        if ((p->arr[i]).exp == 0) exp_zero_index = i;
        new_mono_array[i] = p->arr[i];
    }
    if (exp_zero_index == p->size) {
        new_mono_array[exp_zero_index] = MonoFromPoly(c, 0);
    }
    else {
        Poly newPoly = PolyAdd(&new_mono_array[exp_zero_index].p, c);
        new_mono_array[exp_zero_index].p = newPoly;
    }
    size_t new_array_size = max(p->size, exp_zero_index + 1);
    return (Poly) {.arr = new_mono_array, .size = new_array_size};
}

Poly PolyAdd(const Poly *p, const Poly *q) {
    if (p->arr == NULL && q->arr == NULL) {
        return (Poly) {.arr = NULL, .coeff = (p->coeff + q->coeff)};
    }

    if (p->arr != NULL && q->arr != NULL) {
        size_t new_monos_size;
        Mono *new_array = AddMonoArrays(p, q, &new_monos_size);
        return (Poly) {.arr = new_array, .size = new_monos_size};
    }

    if (p->arr == NULL && q->arr != NULL) {
        return PolyAddCoeff(q, p);
    } else return PolyAddCoeff(p, q);
}
