/** @file
 * Interfejs stosu wielomianów
 *
 * @author Katarzyna Mielnik <km429567@students.mimuw.edu.pl>
 * @date 24.05.2021
 */
#ifndef POLYNOMIALS_STACK_H
#define POLYNOMIALS_STACK_H

#include "poly.h"


/**
 * Stos zaimplementowany na tablicy;
 */
typedef struct {
    /** Tablica, w której trzymane są wielomiany */
    Poly *arr;
    /** Liczba elementów na stosie. */
    size_t size;
    /** Rozmiar tablicy @p arr */
    size_t arr_size;
} Stack;

/**
 * Dodaje wielomian na górę stosu.
 * @param s : stos
 * @param p : wielomian
 */
void Push(Stack *s, Poly *p);

/**
 * Zdejmuje element z góry stosu.
 * @param s : stos
 * @return element, który znajdował się na górze stosu
 */
Poly Pop(Stack *s);

/**
 * Usuwa stos i jego zawartość z pamięci.
 * @param s : stos
 */
void Clear(Stack *s);

/**
 * Sprawdza, czy na stosie nie ma żadnych elementów.
 * @param s : stos
 * @return Czy stos jest pusty?
 */
bool IsEmpty(Stack *s);

/**
 * Zwraca element z góry stosu bez zdejmowania go ze stosu.
 * @param s : stos
 * @return element z góry stosu
 */
Poly Top(Stack *s);

/**
 * Zwraca drugi od góry element stosu.
 * @param s : stos
 * @return drugi od góry element stosu
 */
Poly SecondTop(Stack *s);

/**
 * Tworzy nowy stos.
 * @return pusty stos
 */
Stack GetNewStack();

#endif //POLYNOMIALS_STACK_H
