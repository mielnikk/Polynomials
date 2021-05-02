/** @file
 *
 * @author Katarzyna Mielnik <km429567@students.mimuw.edu.pl>
 * @date 2.05.2021
 */

#include <stdlib.h>
#include "poly.h"

static poly_exp_t Max(poly_exp_t a, poly_exp_t b) {
    return a > b ? a : b;
}

/**
 * Bezpieczniejsze alokowanie pamięci.
 * @param size : rozmiar tablicy, którą trzeba zaalokować
 * @return : tablica o rozmiarze @p size
 */
static Mono *SafeMonoMalloc(size_t size){
    Mono *arr = malloc(size * sizeof(Mono));
    if (arr == NULL) exit(1);
    return arr;
}

/**
 * Sprawdza, czy wielomian jest współczynnkiem zagłębionym w struktury
 * wielomianów stopnia 0.
 * @return Czy wielomian ma postać @f$x_0^0(x_1^0(\ldots(c)))@f$?
 */
static bool PolyIsNestedCoeff(const Poly *p) {
    if (PolyIsCoeff(p))
        return true;

    if (p->size == 1 && p->arr[0].exp == 0 && PolyIsNestedCoeff(&p->arr[0].p))
        return true;
    else return false;
}

/**
 * Tworzy wielomian stały na podstawie wielomianu z zagłębionym współczynnkiem.
 * @param p : wielomian postaci @f$x_0^0(x_1^0(\ldots(c)))@f$
 * @return wielomian będący współczynnikiem @f$c@f$
 */
static Poly PolyToCoeff(const Poly *p) {
    assert(PolyIsNestedCoeff(p));
    if (p->arr == NULL) return PolyFromCoeff(p->coeff);
    else return PolyToCoeff(&p->arr[0].p);
}

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

/**
 * @brief Dodaje jednomiany z dwóch tablic.
 * Zakłada, że jednomiany są posortowane rosnąco względem wykładników oraz, że
 * w obu tablicach wykładniki jednomianów są parami różne.
 * @param p : tablica jednomianów
 * @param q : tablica jednomianów
 * @param p_size : rozmiar tablicy @p p
 * @param q_size : rozmiar tablicy @p q
 * @param array_size wskaźnik na wartość, do której zostaje wpisana liczba
 * różnych jednomianów w tablicy wynikowej
 * @return tablica jednomianów
 */
static Mono *AddMonoArrays
        (const Mono *p, const Mono *q, size_t p_size, size_t q_size, size_t *array_size) {
    assert(p_size + q_size != 0);
    Mono *array = SafeMonoMalloc(p_size + q_size);
    size_t index = 0, p_i = 0, q_i = 0;
    while (p_i != p_size || q_i != q_size) {
        /* Wpisuje kopię wielomianu o mniejszym wykładniku */
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
        /* Gdy w obu tablicach są jednomiany o tym samym wykładniku, w tablicy
         * zostaje zapisana ich suma (o ile nie jest zerem) */
        else if (p[p_i].exp == q[q_i].exp) {
            Mono new_mono = (Mono) {.exp = p[p_i].exp, .p = PolyAdd(&p[p_i].p, &q[q_i].p)};
            if (!PolyIsZero(&new_mono.p))
                array[index++] = new_mono;
            else
                MonoDestroy(&new_mono);
            p_i++;
            q_i++;
        }
    }
    (*array_size) = index;
    return array;
}

/**
 * @brief Dodaje wielomian stały do tablicy jednomianów.
 * Przekształca współczynnik @p c w jednomian oraz dodaje go do tablicy
 * jednomianów @p p.
 * @param p : tablica jednomianów
 * @param p_size : rozmiar tablicy @p p
 * @param c : wielomian stały
 * @param new_array_size : rozmiar wynikowej tablicy
 * @return tablica zawierająca elementy @p p oraz @f$c*x^0@f$
 */
static Mono *MonosAddCoeff(const Mono* p, size_t p_size, const Poly *c, size_t *new_array_size) {
    assert(PolyIsCoeff(c));
    Mono temp_arr = MonoFromPoly(c, 0);
    return AddMonoArrays(&temp_arr, p, 1, p_size, new_array_size);
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

/**
 * Tworzy kopię tablicy jednomianów bez jednomianów o współczynnikach równych zero.
 * @param count : rozmiar tablicy @p monos
 * @param monos : tablica jednomianów
 * @param copy_size : liczba jednomianów wpisanych do nowej tablicy
 * @return kopia tablicy @p monos, ale bez jednomianów o zerowych współczynnikach
 */
static Mono *CopyMonosArray(size_t count, const Mono *monos, size_t *copy_size) {
    assert(count != 0);
    Mono *array = SafeMonoMalloc(count);
    size_t last_index = 0;
    for (size_t i = 0; i < count; i++) {
        if (!PolyIsZero(&monos[i].p))
            array[last_index++] = monos[i];
        /* Z założeń, monos[i].p nie jest "zagłębionym" zerem, więc nie wywołuje PolyDestroy */
    }
    *copy_size = last_index;
    return array;
}
/**
 * Upraszcza tablicę @p monos tak, aby wykładniki jednomianów były parami różne.
 * Przejmuje zawartość @p monos na własność.
 * @param monos : tablica jednomianów
 * @param size : rozmiar tablicy @p monos
 * @param new_size : rozmiar tablicy wynikowej
 * @return tablica jednomianów posortowana rosnąco po wykładnikach
 */
static Mono *SimplifyMonos(Mono *monos, size_t size, size_t *new_size) {
    assert(size != 0);
    size_t index = 0;
    Mono *array = SafeMonoMalloc(size);
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
            /* Jeśli ostatnio dodany jednomian ma zerowy współczynnik, nadpisuje
             * go następnym jednomianem */
            if (!PolyIsZero(&array[index].p))
                index++;
            array[index] = monos[i];
        }
    }
    /* Sprawdza, czy ostatni jednomian ma zerowy współczynnik */
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

            max_deg = Max(max_deg, curr_deg);
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

            max_deg = Max(max_deg, inner_poly_deg);
        }
        return max_deg;
    }
}

Poly PolyNeg(const Poly *p) {
    if (PolyIsCoeff(p))
        return PolyFromCoeff((-1) * p->coeff);

    Mono *new_mono_array = SafeMonoMalloc(p->size);
    for (size_t i = 0; i < p->size; i++) {
        Mono new_mono = {.exp = p->arr[i].exp, .p = PolyNeg(&p->arr[i].p)};
        new_mono_array[i] = new_mono;
    }
    return (Poly) {.arr = new_mono_array, .size = p->size};
}

/**
 * @brief Mnoży wielomian przez liczbę.
 * Dodatkowo, sprawdza czy przy mnożeniu czegoś niezerowego nie doszło do
 * overflow i wielomian nie ma zerowych współczynników.
 * @param p : wielomian
 * @param c : współczynnik
 * @return @f$ p\cdot c@f$
 */
static Poly PolyMulByCoeff(const Poly *p, poly_coeff_t c) {
    if (c == 0) return PolyZero();

    if (PolyIsCoeff(p))
        return PolyFromCoeff(c * p->coeff);

    Mono *new_mono_array = malloc(p->size * sizeof(Mono));
    for (size_t i = 0; i < p->size; i++) {
        Mono new_mono = {.exp = p->arr[i].exp, .p = PolyMulByCoeff(&p->arr[i].p, c)};
        new_mono_array[i] = new_mono;
    }
    Poly new_poly = (Poly) {.arr = new_mono_array, .size = p->size};

    /* Sprawdza, czy przy mnożeniu czegoś niezerowego nie doszło do overflow
     * i wielomian nie ma zerowych współczynników. */
    if (PolyIsZero(&new_poly)) {
        PolyDestroy(&new_poly);
        return PolyZero();
    }
    else return new_poly;
}

/**
 * Mnoży dwie tablice jednomianów oraz na podstawie tablicy wynikowej tworzy
 * wielomian.
 * @param p : tablica jednomianów
 * @param q : tablica jednomianów
 * @param p_size : rozmiar tablicy @p p
 * @param q_size : rozmiar tablicy @p q
 * @return wielomian o wspólczynniku w postaci iloczynu tablic @p p i @p q
 */
static Poly PolyMulArrays(const Mono *p, const Mono *q, size_t p_size, size_t q_size) {
    assert(p != NULL && q != NULL);
    size_t index = 0;
    Poly poly_accumulator = PolyZero();
    for (size_t i = 0; i < p_size; i++) {
        Mono *new_array = SafeMonoMalloc(q_size);
        Mono curr_mono = p[i];
        /* Mnoży tablicę q przez obecny jednomian */
        for (size_t j = 0; j < q_size; j++) {
            Mono new_mono;
            new_mono.p = PolyMul(&(curr_mono.p), &(q[j].p));
            new_mono.exp = curr_mono.exp + q[j].exp;
            index++;
            new_array[j] = new_mono;
        }
        /* Dodaje wynik operacji do wielomianu gromadzącego sumy wyników */
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