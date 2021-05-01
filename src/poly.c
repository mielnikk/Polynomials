#include <stdlib.h>
#include "poly.h"

static poly_exp_t max(poly_exp_t a, poly_exp_t b) {
    return a > b ? a : b;
}

Poly PolyToCoeff(const Poly *p) {
    if (p->arr == NULL) return PolyFromCoeff(p->coeff);
    else return PolyToCoeff(&p->arr[0].p);
}

bool PolyExpZero(const Poly *p) {
    if (PolyIsCoeff(p))
        return true;
    if (p->size == 1 && p->arr[0].exp == 0 && PolyExpZero(&p->arr[0].p))
        return true;
    else return false;
}

/**
 * Usuwa z pamięci listę jednomianów wielomianu
 */
void PolyDestroy(Poly *p) {
    if (p->arr == NULL) return;

    for (size_t i = 0; i < p->size; i++) {
        Mono curr_mono = p->arr[i];
        if ((curr_mono.p).arr != NULL) {
            PolyDestroy(&curr_mono.p);
        }
    }
    free(p->arr);
}

Poly PolyClone(const Poly *p) {
    if (p->arr == NULL) return (Poly) {.arr = NULL, .coeff = p->coeff};
    Mono *new_mono_array = calloc(p->size, sizeof(Mono));
    for (size_t i = 0; i < p->size; i++) {
        new_mono_array[i] = MonoClone(&(p->arr)[i]);
    }
    return (Poly) {.arr = new_mono_array, .size = p->size};
}

/**
 * Komparator wykładników jednomianów.
 */
static int MonoExpCmp(const void *a, const void *b) {
    Mono mono_a = *(Mono *) a;
    Mono mono_b = *(Mono *) b;
    if (mono_a.exp < mono_b.exp)
        return -1;
    if (mono_a.exp == mono_b.exp)
        return 0;
    return 1;
}

Mono *
AddMonoArrays(const Mono *p, const Mono *q, size_t p_size, size_t q_size, size_t *new_array_size) {
    assert(p_size + q_size != 0);
    Mono *array = calloc(p_size + q_size, sizeof(Mono));
    size_t index = 0, p_i = 0, q_i = 0;
    while (p_i != p_size || q_i != q_size) {
        if (p_i == p_size || (q_i != q_size && p[p_i].exp > q[q_i].exp)) {
            if (!PolyIsZero(&q[q_i].p))
                array[index++] = MonoClone(&q[q_i]);
            q_i++;
        }
        else if (q_i == q_size || (p_i != p_size && p[p_i].exp < q[q_i].exp)) {
            if (!PolyIsZero(&p[p_i].p))
                array[index++] = MonoClone(&p[p_i]);
            p_i++;
        }
        else if (p[p_i].exp == q[q_i].exp) {
            Mono new_mono = (Mono) {.exp = p[p_i].exp, .p = PolyAdd(&p[p_i].p, &q[q_i].p)};
            if (!PolyIsZero(&new_mono.p)) {
                array[index++] = new_mono;
            }
            else {
                MonoDestroy(&new_mono);
            }
            p_i++;
            q_i++;
        }
    }
    (*new_array_size) = index;
    return array;
}

Mono *MonoAddCoeff(const Poly *p, const Poly *c, size_t *new_array_size) {
    Mono temp_arr[1];
    temp_arr[0] = MonoFromPoly(c, 0);
    return AddMonoArrays(&temp_arr[0], p->arr, 1, p->size, new_array_size);
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

Poly PolySub(const Poly *p, const Poly *q) {
    Poly q2 = PolyNeg(q);
    Poly result = PolyAdd(p, &q2);
    PolyDestroy(&q2);
    return result;
}

Mono *CopyMonosArray(size_t *count, const Mono *monos) {
    assert(*count != 0);
    Mono *array = calloc(*count, sizeof(Mono));
    size_t last_index = 0;
    for (size_t i = 0; i < *count; i++) {
        if (!PolyIsZero(&monos[i].p)) {
            array[last_index++] = monos[i];
        }
        else { // idk czy to potrzebne
            Mono m = monos[i];
            MonoDestroy(&m);
        }
    }
    *count = last_index;
    return array;
}

Mono *SimplifyMonos(Mono *monos, size_t size, size_t *new_size) {
    assert(size != 0);
    size_t index = 0;
    Mono *array = calloc(size, sizeof(Mono));
    qsort(monos, size, sizeof(Mono), MonoExpCmp);
    array[index] = monos[0];
    for (size_t i = 1; i < size; i++) {
        if (array[index].exp == monos[i].exp) {
            Poly curr_poly = array[index].p;
            array[index].p = PolyAdd(&curr_poly, &monos[i].p);
            MonoDestroy(&monos[i]);
            PolyDestroy(&curr_poly);
        }
        else {
            if (!PolyIsZero(&array[index].p))
                index++;
            array[index] = monos[i];
        }
    }
    if (PolyIsZero(&array[index].p))
        index--;
    *new_size = index + 1;
    return array;
}

Poly PolyAddMonos(size_t count, const Mono monos[]) {
    if (count == 0) return (Poly) {.arr = NULL, .coeff = 0};
    size_t new_array_size;
    Mono *poly_monos = AddMonoArrays(NULL, &monos[0], 0, count, &new_array_size);
    for (size_t i = 0; i < count; i++) {
        Poly p = monos[i].p;
        PolyDestroy(&p);
    }
    return (Poly) {.arr = poly_monos, .size = new_array_size};
}

poly_exp_t PolyDegBy(const Poly *p, size_t var_idx) {
    if (PolyIsZero(p))
        return -1;
    if (PolyIsCoeff(p))
        return 0;
    else {
        poly_exp_t max_deg = -1;
        for (size_t i = 0; i < p->size; i++) {
            Mono curr_mono = p->arr[i];
            poly_exp_t curr_deg;
            if (var_idx != 0)
                curr_deg = PolyDegBy(&(curr_mono.p), var_idx - 1);
            else
                curr_deg = curr_mono.exp;

            max_deg = max(max_deg, curr_deg);
        }
        return max_deg;
    }
}

poly_exp_t PolyDeg(const Poly *p) {
    if (PolyIsZero(p))
        return -1;
    if (PolyIsCoeff(p))
        return 0;
    else {
        poly_exp_t max_deg = -1;
        for (size_t i = 0; i < p->size; i++) {
            Mono curr_mono = p->arr[i];
            poly_exp_t inner_poly_deg = PolyDeg(&curr_mono.p);

            if (inner_poly_deg != -1)
                inner_poly_deg += curr_mono.exp;

            max_deg = max(max_deg, inner_poly_deg);
        }
        return max_deg;
    }
}

Poly PolyNeg(const Poly *p) {
    if (PolyIsCoeff(p))
        return PolyFromCoeff((-1) * p->coeff);

    Mono *new_mono_array = malloc(p->size * sizeof(Mono));
    for (size_t i = 0; i < p->size; i++) {
        Mono new_mono = {.exp = p->arr[i].exp, .p = PolyNeg(&p->arr[i].p)};
        new_mono_array[i] = new_mono;
    }
    return (Poly) {.arr = new_mono_array, .size = p->size};
}

Poly PolyMulByCoeff(const Poly *p, poly_coeff_t c) {
    if (c == 0) return PolyZero();

    if (PolyIsCoeff(p))
        return PolyFromCoeff(c * p->coeff);

    Mono *new_mono_array = malloc(p->size * sizeof(Mono));
    for (size_t i = 0; i < p->size; i++) {
        Mono new_mono = {.exp = p->arr[i].exp, .p = PolyMulByCoeff(&p->arr[i].p, c)};
        new_mono_array[i] = new_mono;
    }
    Poly new_poly = (Poly) {.arr = new_mono_array, .size = p->size};
    if (PolyIsZero(&new_poly)){
        PolyDestroy(&new_poly);
        return PolyZero();
    }
    else return new_poly;
}

static Poly PolyMulArrays(const Mono *p, const Mono *q, size_t p_size, size_t q_size) {
    assert(p != NULL && q != NULL);
    size_t index = 0;
    Poly poly_accumulator = PolyZero();
    for (size_t i = 0; i < p_size; i++) {
        Mono *new_array = malloc(q_size * sizeof(Mono));
        Mono curr_mono = p[i];
        for (size_t j = 0; j < q_size; j++) {
            Mono new_mono;
            new_mono.p = PolyMul(&(curr_mono.p), &(q[j].p));
            new_mono.exp = curr_mono.exp + q[j].exp;
            index++;
            new_array[j] = new_mono;
        }
        Poly temp_poly = (Poly) {.arr = new_array, .size = q_size};
        Poly new_poly = PolyAdd(&poly_accumulator, &temp_poly);
        PolyDestroy(&poly_accumulator);
        poly_accumulator = new_poly;
        PolyDestroy(&temp_poly);
    }
    return poly_accumulator;
}

Poly PolyMul(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p) && PolyIsCoeff(q))
        return PolyFromCoeff(p->coeff * q->coeff);

    if (!PolyIsCoeff(p) && !PolyIsCoeff(q))
        return PolyMulArrays(p->arr, q->arr, p->size, q->size);

    if (PolyIsCoeff(p) && !PolyIsCoeff(q))
        return PolyMulByCoeff(q, p->coeff);

    else return PolyMulByCoeff(p, q->coeff);
}