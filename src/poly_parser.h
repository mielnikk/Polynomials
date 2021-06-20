/** @file
 * Interfejs modułu przetwarzającego zapis wielomianu
 *
 * @author Katarzyna Mielnik <km429567@students.mimuw.edu.pl>
 */

#ifndef POLYNOMIALS_POLY_PARSER_H
#define POLYNOMIALS_POLY_PARSER_H

#include <stddef.h>
#include "stack.h"

/**
 * Przetwarza linię zawierającą zapis wielomianu. W przypadku błędu w zapisie,
 * na standardowe wyjście diagnostyczne zostaje wypisany komunikat o błędzie.
 * @param[in,out] s : stos
 * @param[in] line : linia, czyli ciąg znaków
 * @param[in] line_length : liczba niezerowych znaków w linii
 * @param[in] line_number : numer linii
 */
void ParseInputPoly(Stack *s, char *line, size_t line_length, long line_number);

/**
 * Wypisuje komunikat o błędzie podczas parsowania wielomianu na standardowe
 * wyjście diagnostyczne.
 * @param[in] line_number : numer linii, w której wystąpił błąd
 */
void PrintWrongPolyError(long line_number);

#endif //POLYNOMIALS_POLY_PARSER_H
