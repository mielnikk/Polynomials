/** @file
 * Implementacja modułu z operacjami na wielomianach rzadkich wielu zmiennych.
 * Niezmienniki:
 * - jednomiany są posortowane rosnąco względem wykładników potęg
 * - jednomiany mają parami różne wykładniki (z wyjątkiem tablicy @p monos w
 *   funkcji @ref PolyAddMonos)
 * - współczynnik przy niezerowej potędze jest niezerowy
 * - kiedy tylko można, wielomian postaci @f$c\cdot x^0@f$ jest zamieniany na
 *   wielomian stały @f$c@f$
 * - wielomian @f$0 \cdot x^0@f$ jest rozpatrywany jako współczynnik równy 0
 *
 * @author Katarzyna Mielnik <km429567@students.mimuw.edu.pl>
 * @date 2.05.2021
 */

#include <stdlib.h>
#include "poly.h"

/**
 * Zwraca większy z dwóch wykładników.
 * @param[in] a : wykładnik
 * @param[in] b : wykładnik
 * @return max z dwóch wykładników
 */
static poly_exp_t MaxExp(poly_exp_t a, poly_exp_t b) {
    return a > b ? a : b;
}

/**
 * Bezpieczniejsze alokowanie pamięci.
 * @param[in] size : rozmiar tablicy, którą trzeba zaalokować
 * @return tablica o rozmiarze @p size
 */
static Mono *SafeMonoMalloc(size_t size) {
    Mono *arr = malloc(size * sizeof(Mono));
    if (arr == NULL) exit(1);
    return arr;
}

/**
 * Sprawdza, czy wielomian jest współczynnkiem zagłębionym w struktury
 * wielomianów stopnia 0.
 * @param[in] p : wielomian
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
 * @param[in] p : wielomian postaci @f$x_0^0(x_1^0(\ldots(c)))@f$
 * @return wielomian będący współczynnikiem @f$c@f$
 */
static Poly PolyToCoeff(const Poly *p) {
    assert(PolyIsNestedCoeff(p));
    if (p->arr == NULL)
        return PolyFromCoeff(p->coeff);
    else
        return PolyToCoeff(&p->arr[0].p);
}

void PolyDestroy(Poly *p) {
    if (!PolyIsCoeff(p)) {
        for (size_t i = 0; i < p->size; i++) {
            MonoDestroy(&p->arr[i]);
        }
        free(p->arr);
    }
}

Poly PolyClone(const Poly *p) {
    if (PolyIsCoeff(p))
        return PolyFromCoeff(p->coeff);

    Mono *new_mono_array = SafeMonoMalloc(p->size);
    for (size_t i = 0; i < p->size; i++) {
        new_mono_array[i] = MonoClone(&p->arr[i]);
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
 * Dodaje jednomiany z dwóch tablic.
 * @param[in] p : tablica jednomianów
 * @param[in] q : tablica jednomianów
 * @param[in] p_size : rozmiar tablicy @p p
 * @param[in] q_size : rozmiar tablicy @p q
 * @param[in,out] array_size : wartość, do której zostaje wpisana liczba
 * różnych jednomianów w tablicy wynikowej
 * @return tablica jednomianów
 */
static Mono *AddMonoArrays(const Mono *p, const Mono *q, size_t p_size,
                           size_t q_size, size_t *array_size) {
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
            p_i++;
            q_i++;
        }
    }
    *array_size = index;
    return array;
}

/**
 * @brief Dodaje wielomian stały do tablicy jednomianów.
 * Przekształca współczynnik @p c w jednomian oraz dodaje go do tablicy
 * jednomianów @p p.
 * @param[in] p : tablica jednomianów
 * @param[in] p_size : rozmiar tablicy @p p
 * @param[in] c : wielomian stały
 * @param[in,out] new_array_size : rozmiar wynikowej tablicy
 * @return tablica zawierająca elementy @p p oraz @f$c*x^0@f$
 */
static Mono *MonosAddCoeff(const Mono *p, size_t p_size, const Poly *c, size_t *new_array_size) {
    assert(PolyIsCoeff(c));
    assert(p != NULL);
    Mono temp = MonoFromPoly(c, 0);
    return AddMonoArrays(&temp, p, 1, p_size, new_array_size);
}


Poly PolyAdd(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p) && PolyIsCoeff(q))
        return PolyFromCoeff(p->coeff + q->coeff);

    size_t new_array_size;
    Mono *new_array;

    if (!PolyIsCoeff(p) && !PolyIsCoeff(q))
        new_array = AddMonoArrays(p->arr, q->arr, p->size, q->size, &new_array_size);
    else if (PolyIsCoeff(p) && !PolyIsCoeff(q))
        new_array = MonosAddCoeff(q->arr, q->size, p, &new_array_size);
    else
        new_array = MonosAddCoeff(p->arr, p->size, q, &new_array_size);

    /* Jeśli suma wszystkich wykładników jest zerem */
    if (new_array_size == 0) {
        free(new_array);
        return PolyZero();
    }
    /* Uproszczenie wielomianu */
    Poly new_poly = (Poly) {.arr = new_array, .size = new_array_size};
    if (PolyIsNestedCoeff(&new_poly)) {
        Poly simplified_poly = PolyToCoeff(&new_poly);
        PolyDestroy(&new_poly);
        return simplified_poly;
    }
    else return new_poly;
}

Poly PolySub(const Poly *p, const Poly *q) {
    Poly q_neg = PolyNeg(q);
    Poly result = PolyAdd(p, &q_neg);
    PolyDestroy(&q_neg);
    return result;
}

/**
 * Tworzy kopię tablicy jednomianów bez jednomianów o współczynnikach równych zero.
 * @param[in] count : rozmiar tablicy @p monos
 * @param[in] monos : tablica jednomianów
 * @param[in,out] copy_size : liczba jednomianów wpisanych do nowej tablicy
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
 * @param[in,out] monos : tablica jednomianów
 * @param[in] size : rozmiar tablicy @p monos
 * @param[in,out] new_size : rozmiar tablicy wynikowej
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
    if (count == 0)
        return PolyZero();

    size_t new_size = 0, copy_size = 0;
    Mono *new, *monos_copy = CopyMonosArray(count, monos, &copy_size);
    /* Gdy w monos[] wszystko było zerami */
    if (copy_size == 0) {
        free(monos_copy);
        return PolyZero();
    }

    new = SimplifyMonos(monos_copy, copy_size, &new_size);
    free(monos_copy);
    /* Gdy wszystko uprościło się do zera */
    if (new_size == 0) {
        free(new);
        return PolyZero();
    }

    Poly poly = (Poly) {.arr = new, .size = new_size};
    if (PolyIsNestedCoeff(&poly)) {
        Poly new_poly = PolyToCoeff(&poly);
        PolyDestroy(&poly);
        return new_poly;
    }
    return poly;
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

            max_deg = MaxExp(max_deg, curr_deg);
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
            inner_poly_deg += curr_mono.exp;

            max_deg = MaxExp(max_deg, inner_poly_deg);
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
 * Mnoży dwie tablice jednomianów oraz na podstawie tablicy wynikowej tworzy
 * wielomian.
 * @param[in] p : tablica jednomianów
 * @param[in] q : tablica jednomianów
 * @param[in] p_size : rozmiar tablicy @p p
 * @param[in] q_size : rozmiar tablicy @p q
 * @return wielomian o wspólczynniku w postaci iloczynu tablic @p p i @p q
 */
static Poly PolyMulArrays(const Mono *p, const Mono *q, size_t p_size, size_t q_size) {
    assert(p != NULL && q != NULL);
    Poly poly_accumulator = PolyZero();
    for (size_t i = 0; i < p_size; i++) {
        Mono *new_array = SafeMonoMalloc(q_size);
        Mono curr_mono = p[i];
        /* Mnoży tablicę q przez obecny jednomian */
        for (size_t j = 0; j < q_size; j++) {
            Mono new_mono;
            new_mono.p = PolyMul(&(curr_mono.p), &(q[j].p));
            new_mono.exp = curr_mono.exp + q[j].exp;
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

/**
 * Mnoży wielomian przez liczbę.
 * @param[in] p : wielomian
 * @param[in] c : współczynnik
 * @return @f$ p\cdot c@f$
 */
static Poly PolyMulByCoeff(const Poly *p, poly_coeff_t c) {
    if (c == 0) return PolyZero();

    if (PolyIsCoeff(p))
        return PolyFromCoeff(c * p->coeff);

    Mono *new_mono_array = malloc(p->size * sizeof(Mono));
    size_t index = 0;
    for (size_t i = 0; i < p->size; i++) {
        Mono new_mono = {.exp = p->arr[i].exp, .p = PolyMulByCoeff(&p->arr[i].p, c)};
        if (!PolyIsZero(&new_mono.p))
            new_mono_array[index++] = new_mono;
    }
    Poly new_poly = (Poly) {.arr = new_mono_array, .size = index};

    /* Sprawdza, czy przy mnożeniu czegoś niezerowego nie doszło do overflow
     * i wielomian nie ma zerowych współczynników. */
    if (index == 0) {
        free(new_mono_array);
        return PolyZero();
    }

    else return new_poly;
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

bool PolyIsEq(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p) != PolyIsCoeff(q))
        return false;

    if (PolyIsCoeff(p))
        return p->coeff == q->coeff;

    if (p->size != q->size)
        return false;

    for (size_t i = 0; i < p->size; i++) {
        if (p->arr[i].exp != q->arr[i].exp
            || !PolyIsEq(&p->arr[i].p, &q->arr[i].p))
            return false;
    }
    return true;
}

/**
 * Szybkie potęgowanie.
 * @param[in] base : podstawa
 * @param[in] exp : wykładnik
 * @return @f$ base^{exp}@f$
 */
static poly_coeff_t QuickPow(poly_coeff_t base, poly_coeff_t exp) {
    if (exp == 0)
        return 1;
    if (exp % 2 == 1)
        return base * QuickPow(base, exp - 1);
    poly_coeff_t result = QuickPow(base, exp / 2);
    return result * result;
}

Poly PolyAt(const Poly *p, poly_coeff_t x) {
    if (PolyIsZero(p))
        return PolyZero();
    if (PolyIsCoeff(p))
        return PolyFromCoeff(p->coeff);

    /* Wielomian, do którego są dodawane kolejne współczynniki po pomnożeniu przez x */
    Poly result = PolyZero();

    for (size_t i = 0; i < p->size; i++) {
        poly_coeff_t coeff = QuickPow(x, p->arr[i].exp);
        Poly multiplied_poly = PolyMulByCoeff(&p->arr[i].p, coeff);

        Poly prev_result = result;
        result = PolyAdd(&result, &multiplied_poly);
        PolyDestroy(&prev_result);
        PolyDestroy(&multiplied_poly);
    }
    return result;
}