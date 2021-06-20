/** @file
 * Interfejs modułu z operacjami, których używa kalkulator.
 *
 * @author Katarzyna Mielnik <km429567@students.mimuw.edu.pl>
 * @date 20.06.2021
 */
#ifndef POLYNOMIALS_CALC_OP_H
#define POLYNOMIALS_CALC_OP_H

#include "stack.h"

/**
 * Wstawia wielomian zerowy na górę stosu.
 * @param[in,out] s : stos
 * @return @p true -
 */
bool Zero(Stack *s);

/**
 * Wypisuje na statndardowe wyjście @p 1, gdy wielomian na wierzchołku stosu
 * jest współczynnikiem, w przeciwnym przypadku wypisuje @p 0. Jeśli na stosie
 * nie ma żadnego wielomianu, nic nie wypisuje i zwraca @p false.
 * @param[in,out] s : stos
 * @return Czy operacja się powiodła?
 */
bool IsCoeff(Stack *s);

/**
 * Wypisuje na statndardowe wyjście @p 1, gdy wielomian na wierzchołku stosu
 * jest tożsamościowo równy zeru, w przeciwnym przypadku wypisuje @p 0. Jeśli
 * na stosie nie ma żadnego wielomianu, nic nie wypisuje i zwraca @p false.
 * @param[in,out] s : stos
 * @return Czy operacja się powiodła?
 */
bool IsZero(Stack *s);

/**
 * Wstawia na stos kopię wielomianu z wierzchołka. Zwraca @p false, gdy na
 * stosie nie ma żadnych wielomianów.
 * @param[in,out] s : stos
 * @return Czy operacja się powiodła?
 */
bool Clone(Stack *s);

/**
 * Neguje wielomian na górze stosu. Zwraca @p false, gdy na stosie nie ma
 * żadnych wielomianów.
 * @param[in,out] s : stos
 * @return Czy operacja się powiodła?
 */
bool Neg(Stack *s);

/**
 * Dodaje dwa wielomiany z wierzchu stosu, usuwa je i wstawia na wierzchołek
 * stosu ich iloczyn. Jeśli na stosie są mniej niż dwa wielomiany, nie wykonuje
 * działania i zwraca @p false. W przeciwnym przypadku, po wykonaniu operacji
 * zwraca @p true.
 * @param[in,out] s : stos
 * @return Czy operacja się powiodła?
 */
bool Add(Stack *s);

/**
 * Odejmuje dwa wielomiany z wierzchu stosu, usuwa je i wstawia na wierzchołek
 * stosu ich iloczyn. Jeśli na stosie są mniej niż dwa wielomiany, nie wykonuje
 * działania i zwraca @p false. W przeciwnym przypadku, po wykonaniu operacji
 * zwraca @p true.
 * @param[in,out] s : stos
 * @return Czy operacja się powiodła?
 */
bool Sub(Stack *s);

/**
 * Mnoży dwa wielomiany z wierzchu stosu, usuwa je i wstawia na wierzchołek
 * stosu ich iloczyn. Jeśli na stosie są mniej niż dwa wielomiany, nie wykonuje
 * działania i zwraca @p false. W przeciwnym przypadku, po wykonaniu operacji
 * zwraca @p true.
 * @param[in,out] s : stos
 * @return Czy operacja się powiodła?
 */
bool Mul(Stack *s);

/**
 * Wypisuje na standardowe wyjście @p 1, jeśli dwa wielomiany na wierzchu stosu
 * są równe, w przeciwnym przypadku wypisuje @p 0. Jeśli na stosie są mniej niż
 * dwa wielomiany, nie wykonuje porównania i zwraca @p false.
 * @param[in,out] s : stos
 * @return Czy operacja się powiodła?
 */
bool IsEq(Stack *s);

/**
 * Wypisuje na standardowe wyjście stopień wielomianu z góry stosu.
 * Gdy na stosie nie ma żadnych wielomianów, nie wypisuje nic oraz zwraca @p false.
 * @param[in,out] s : stos
 * @return Czy operacja się powiodła?
 */
bool Deg(Stack *s);

/**
 * Wypisuje na standardowe wyjście stopień wielomianu ze względu na zmienną @p idx.
 * Zwraca @p false, gdy na stosie nie ma żadnych wielomianów.
 * @param[in,out] s : stos
 * @param[in] idx : zmienna, względem której oblicza stopień wielomianu
 * @return Czy operacja się powiodła?
 */
bool DegBy(Stack *s, size_t idx);

/**
 * Wylicza wartość wielomianu, który znajduje się na górze stosu, w punkcie @p x.
 * Usuwa oryginalny wielomian z wierzchołka stosu. Wstawia na stos wynik operacji.
 * Jeśli na stosie nie ma żadnych wielomianów, zwraca @p false.
 * @param[in,out] s : stos
 * @param[in] x : punkt, w którym zostaje obliczona wartość wielomianu
 * @return Czy operacja się powiodła?
 */
bool At(Stack *s, poly_coeff_t x);

/**
 * Zdejmuje wielomian z góry stosu i usuwa go. Jeśli na stosie nie było żadnych
 * elementów zwraca @p false. W przeciwnym wypadku, po wykonaniu operacji,
 * zwraca @p true.
 * @param[in,out] s : stos
 * @return Czy operacja się powiodła?
 */
bool StackPop(Stack *s);

/**
 * Wypisuje na standardowe wyjście wielomian z wierzchołka stosu.
 * Zwraca @p false, jeśli stos jest pusty.
 * @param[in,out] s : stos wielomianów
 * @return Czy operacja się powiodła?
 */
bool Print(Stack *s);

/**
 * Zdejmuje z wierzchołka stosu pierwszy wielomian, a następnie kolejno @p k
 * wielomianów, które podstawia na miejsce kolejnych zmiennych w pierwszym wielomianie.
 * @param s : stos
 * @param k : liczba wielomianów
 * @return : złożenie wielomianu z wierzhołka oraz @p k następnych wielomianów
 */
bool Compose(Stack *s, size_t k);

#endif //POLYNOMIALS_CALC_OP_H
