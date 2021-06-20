/** @file
 * Interfejs modułu przetwarzającego zapis polecenia kalkulatora
 *
 * @author Katarzyna Mielnik <km429567@students.mimuw.edu.pl>
 */

#ifndef POLYNOMIALS_COMMAND_PARSER_H
#define POLYNOMIALS_COMMAND_PARSER_H

#include <stddef.h>
#include "stack.h"

/**
 * Przetwarza linię z zapisanym poleceniem. Jeśli nazwa polecenia lub argument
 * są niepoprawne, wypisuje na standardowe wyjście diagnostyczne komunikat o
 * błędzie. W przypadku poprawności zapisu, wywołuje operację odpowiadającą
 * poleceniu.
 * @param[in,out] s : stos wielomianów
 * @param[in,out] line : linia, czyli ciąg znaków
 * @param[in] line_size : rozmiar linii
 * @param[in] line_number : numer linii
 */
void ParseCommand(Stack *s, char *line, size_t line_size, long line_number);

/**
 * Wypisuje na standardowe wyjście diagnostyczne komunikat o błędnym poleceniu.
 * @param[in] line_number : numer linii, w której wystąpił błąd
 */
void PrintWrongCommandError(long line_number);


#endif //POLYNOMIALS_COMMAND_PARSER_H
