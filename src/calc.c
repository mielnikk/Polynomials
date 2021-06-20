/** @file
 * Implementacja kalkulatora wielomianów.
 *
 * @author Katarzyna Mielnik <km429567@students.mimuw.edu.pl>
 * @date 20.06.2021
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE ///< Wymagane do poprawnego działania funkcji getline.
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include "command_parser.h"
#include "poly_parser.h"
#include "stack.h"
#include "poly.h"


/**
 * Nadpisuje znak nowej linii białym znakiem. Kiedy znak został zamieniony,
 * zmiejsza wartość @p line_size o 1.
 * @param[in,out] line : linia, czyli ciąg znaków
 * @param[in,out] line_size : długość linii
 */
static void RemoveNewline(char *line, size_t *line_size) {
    if (*line_size > 0 && line[*line_size - 1] == '\n') {
        line[*line_size - 1] = '\0';
        *line_size = *line_size - 1;
    }
}

/**
 * Przetwarza linię rozpoznając, czy zawiera wielomian, czy polecenie do wykonania.
 * @param[in,out] s : stos
 * @param[in,out] line : linia, czyli ciąg znaków
 * @param[in] read_length : liczba znaków zwrócona przez getline
 * @param[in] line_number : numer linii
 */
void ParseLine(Stack *s, char *line, long read_length, long line_number) {
    if (read_length > 0 && (line[0] == '#' || line[0] == '\n'))
        return;

    /* Jeśli w środku linii są zerowe znaki */
    if ((size_t) read_length != strlen(line)) {
        if (isalpha(line[0]))
            PrintWrongCommandError(line_number);
        else
            PrintWrongPolyError(line_number);
        return;
    }

    size_t line_length = (size_t) read_length;
    RemoveNewline(line, &line_length);

    if (line_length == 0)
        return;

    if (isalpha(line[0]))
        ParseCommand(s, line, line_length, line_number);
    else
        ParseInputPoly(s, line, line_length, line_number);
}

/**
 * Wczytuje dane ze standardowego wejścia oraz je przetwarza.
 * @return @p 0 w przypadku poprawnego zakończenia programu, @p 1 w przypadku
 * błędu.
 */
int main() {
    Stack stack = GetNewStack();
    char *buff = NULL;
    size_t buffsize = 1;
    errno = 0;

    long characters;
    long line_number = 0;

    while ((characters = getline(&buff, &buffsize, stdin)) != -1) {
        ParseLine(&stack, buff, characters, ++line_number);
    }

    if (errno != 0)
        exit(1);

    free(buff);
    Clear(&stack);
    return 0;
}