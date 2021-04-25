#include <stdlib.h>
#include "poly.h"

size_t max(size_t a, size_t b) {
    return a > b ? a : b;
}

/**
 * Przepisuje jednomiany pierwszego wielomianu do nowej tablicy.
 * Dodaje kolejne jednomiany drugiego wielomianu do jednomianów w nowej tablicy,
 * przy odpowiednich wykładnikach potęg.
 */
Mono *AddMonoArrays(const Mono *p, const Mono *q, size_t p_size, size_t q_size, size_t *new_array_size) {
    Mono *new_mono_array = calloc(p_size + q_size, sizeof(Mono));
    size_t last_index = p_size;

    for (size_t i = 0; i < p_size; i++) {
        new_mono_array[i] = p[i];
    }
    for (size_t i = 0; i < q_size; i++) {
        bool is_added = false;
        Mono curr_mono = q[i];
        for (size_t j = 0; j < p_size; j++) {
            if (curr_mono.exp == new_mono_array[j].exp) {
                new_mono_array[j].p = PolyAdd(&new_mono_array[i].p, &curr_mono.p);
                is_added = true;
                break;
            }
        }
        if (!is_added) {
            new_mono_array[last_index] = q[i];
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
    } else {
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
        Mono *new_array = AddMonoArrays(p->arr, q->arr, p->size, q->size, &new_monos_size);
        return (Poly) {.arr = new_array, .size = new_monos_size};
    }

    if (p->arr == NULL && q->arr != NULL) {
        return PolyAddCoeff(q, p);
    } else return PolyAddCoeff(p, q);
}

/**
 * Usuwa z pamięci listę jednomianów wielomianu
 * @param[in] p : wielomian z niepustą tablicą @f$arr@f$
 */
void DestroyPolyContents(Poly *p) {
    for (size_t i = 0; i < (*p).size; i++) {
        Mono curr_mono = (*p).arr[i];
        if (curr_mono.p.arr != NULL) {
            DestroyPolyContents(&curr_mono.p);
        }
    }
    free((*p).arr);
}

/**
 * Tworzy wielomian z listy jednomianów za pomocą dodawania
 * tablicy jednomianów @f$monos@f$ do pustej tablicy.
 * Usuwa zawartość tablicy @f$monos@f$ poprzez usuwanie zawartości
 * współczynninka każdego jednomianu (jeśli współczynnik zawiera listę
 * jednomianów).
 */
Poly PolyAddMonos(size_t count, const Mono monos[]) {
    if (count == 0) return (Poly) {.arr = NULL, .coeff = 0};
    size_t new_array_size;
    Mono *poly_monos = AddMonoArrays(NULL, &monos[0], 0, count, &new_array_size);
    for (size_t i = 0; i < count; i++) {
        Poly p = monos[i].p;
        if (p.arr != NULL) {
            DestroyPolyContents(&p);
        }
    }
    return (Poly) {.arr = poly_monos, .size = new_array_size};
}