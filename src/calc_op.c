/** @file
 * Implementacja operacji, które umożliwia kalkulator.
 *
 * @author Katarzyna Mielnik <km429567@students.mimuw.edu.pl>
 */
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include "calc_op.h"
#include "poly.h"


bool Zero(Stack *s) {
    Poly zero = PolyZero();
    Push(s, &zero);
    return true;
}

bool IsCoeff(Stack *s) {
    if (IsEmpty(s))
        return false;
    Poly p = Top(s);
    if (PolyIsCoeff(&p))
        printf("%d\n", 1);
    else printf("%d\n", 0);
    return true;
}

bool IsZero(Stack *s) {
    if (IsEmpty(s))
        return false;
    Poly p = Top(s);
    if (PolyIsZero(&p))
        printf("%d\n", 1);
    else printf("%d\n", 0);
    return true;
}

bool Clone(Stack *s) {
    if (IsEmpty(s))
        return false;
    Poly p = Top(s);
    Poly p_clone = PolyClone(&p);
    Push(s, &p_clone);
    return true;
}

bool Neg(Stack *s) {
    if (IsEmpty(s))
        return false;
    Poly p = Pop(s);
    Poly p_neg = PolyNeg(&p);
    Push(s, &p_neg);
    PolyDestroy(&p);
    return true;
}

/**
 * Wykonuje operację @p (*op) na dwóch wielomianach z wierzchu stosu, usuwa je
 * oraz dodaje wynik na stos. Jeśli na stosie są mniej niż dwa wielomiany, nie
 * wykonuje działania i zwraca @p false. W przeciwnym przypadku, po wykonaniu
 * operacji zwraca @p true.
 * @param stack : stos
 * @param op : działanie na dwóch wielomainach
 * @return Czy operacja się powiodła?
 */
static bool BinaryOperation(Stack *stack, Poly (*op)(const Poly *, const Poly *)) {
    if (stack->size < 2)
        return false;
    Poly p = Pop(stack);
    Poly q = Pop(stack);
    Poly result = (*op)(&p, &q);
    Push(stack, &result);
    PolyDestroy(&p);
    PolyDestroy(&q);
    return true;
}

bool Add(Stack *s) {
    return BinaryOperation(s, PolyAdd);
}


bool Sub(Stack *s) {
    return BinaryOperation(s, PolySub);
}


bool Mul(Stack *s) {
    return BinaryOperation(s, PolyMul);
}

bool IsEq(Stack *s) {
    if (s->size < 2)
        return false;
    Poly fst = Top(s);
    Poly snd = SecondTop(s);
    if (PolyIsEq(&fst, &snd))
        printf("%d\n", 1);
    else
        printf("%d\n", 0);
    return true;
}

bool Deg(Stack *s) {
    if (IsEmpty(s))
        return false;
    Poly p = Top(s);
    poly_exp_t deg = PolyDeg(&p);
    printf("%d\n", deg);
    return true;
}

bool DegBy(Stack *s, size_t idx) {
    if (IsEmpty(s))
        return false;
    Poly p = Top(s);
    poly_exp_t deg = PolyDegBy(&p, idx);
    printf("%d\n", deg);
    return true;
}

bool StackPop(Stack *s) {
    if (IsEmpty(s))
        return false;
    Poly p = Pop(s);
    PolyDestroy(&p);
    return true;
}

bool At(Stack *s, poly_coeff_t x) {
    if (IsEmpty(s))
        return false;
    Poly p = Pop(s);
    Poly result = PolyAt(&p, x);
    PolyDestroy(&p);
    Push(s, &result);
    return true;
}

static void PrintPoly(Poly p);

/**
 * Wypisuje jednomian na standardowe wyjście.
 * @param m : jednomian
 */
static void PrintMono(Mono m) {
    printf("(");
    PrintPoly(m.p);
    printf(",%d)", m.exp);
}

/**
 * Wypisuje wielomian na standardowe wyjście.
 * @param[in] p : wielomian
 */
static void PrintPoly(Poly p) {
    if (PolyIsCoeff(&p))
        printf("%ld", p.coeff);
    else {
        for (size_t i = 0; i < p.size; i++) {
            PrintMono(p.arr[i]);
            if (i != p.size - 1)
                printf("+");
        }
    }
}

bool Print(Stack *s) {
    if (IsEmpty(s))
        return false;
    Poly p = Top(s);
    PrintPoly(p);
    printf("\n");
    return true;
}

bool Compose(Stack *s, size_t k) {
    if (s->size <= k)
        return false;
    Poly *tab;
    if(k == 0)
        tab = malloc(sizeof(Poly));
    else
        tab = malloc(k * sizeof(Poly));

    if (tab == NULL)
        exit(1);

    Poly p = Pop(s);
    for (size_t i = 1; i <= k; i++) {
        tab[k - i] = Pop(s);
    }

    Poly res = PolyCompose(&p, k, tab);
    Push(s, &res);
    PolyDestroy(&p);
    for (size_t i = 0; i < k; i++) {
        PolyDestroy(&tab[i]);
    }
    free(tab);
    return true;
}
