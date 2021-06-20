/** @file
 * Implementacja modułu przetwarzającego zapis wielomianu
 *
 * @author Katarzyna Mielnik <km429567@students.mimuw.edu.pl>
 */
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "poly_parser.h"

#define MAX_EXP 2147483647 ///< Maksymalna wartość wykładnika jednomianu
#define BASE_10 10 ///< Wartość reprezentująca system dziesiętny


/** Znaki, które mogą wystąpić w zapisie wielomianu. */
const char *ValidPolyCharacters = "0123456789()+-,";

void PrintWrongPolyError(long line_number) {
    fprintf(stderr, "ERROR %ld WRONG POLY\n", line_number);
}

/**
 * Przetwarza początkowe znaki ciągu na wykładnik jednomianu.
 * Jeśli w @p s nie ma żadnych cyfr do przetworzenia lub wartość wykładnika
 * przekracza dopuszczalny zakres, @p endptr zostaje ustawiony na @p NULL.
 * @param[in] s : ciąg znaków
 * @param[in,out] endptr : wskaźnik na pierwszy znak, którego nie udało się przetworzyć
 * @return wykładnik jednomianu
 */
static poly_exp_t StringToPolyExp(char *s, char **endptr) {
    if (!isdigit(s[0])) {
        *endptr = NULL;
        return 0;
    }
    unsigned long exp = strtoul(s, endptr, BASE_10);
    if (exp > MAX_EXP) {
        *endptr = NULL;
        return -1;
    }

    return (poly_exp_t) exp;
}

/**
 * Szacuje liczbę jednomianów wielomianu na podstawie liczby zamkniętych nawiasów.
 * @param[in] line : wiersz zawierający zapis wielomianu
 * @return oszacowana liczba jednomianów wielomianu
 */
size_t CountMonos(const char *line) {
    size_t monos_counter = 1;
    long unclosed_brackets = 0;
    size_t index = 0;
    while (line[index] != '\n' && line[index] != '\0') {
        if (line[index] == '(')
            unclosed_brackets++;
        else if (line[index] == ')')
            unclosed_brackets--;

        if (unclosed_brackets == 0)
            monos_counter++;
        else if (unclosed_brackets < 0)
            break;

        index++;
    }
    return monos_counter;
}

static bool ParsePoly(Poly *p, char *line, size_t line_length, char **endptr);

/**
 * Przetwarza początkowe znaki ciągu na jednomian. Zwraca @p false, jeśli
 * w zapisie jednomianu występuje błąd, w przeczwnym przypadku zwraca @p true.
 * Jednomian zostaje zapisany do zmiennej @p m.
 * @param[in,out] m
 * @param[in] line
 * @param[in] line_length
 * @param[in,out] endptr : wskaźnik na pierwszy element, który nie został
 * przetworzony do jednomianu
 * @return Czy parsowanie się powiodło?
 */
static bool ParseMono(Mono *m, char *line, size_t line_length, char **endptr) {
    if (line[0] != '(')
        return false;

    char *poly_end;
    Poly coeff;
    bool op = ParsePoly(&coeff, (line + 1), line_length - 1, &poly_end);
    /* Błąd w wielomianie lub brak wielomianu */
    if (!op || line[1] == poly_end[0])
        return false;
    /* Błędny znak na końcu wielomianu */
    if (poly_end[0] != ',') {
        PolyDestroy(&coeff);
        return false;
    }

    poly_exp_t exp = StringToPolyExp((poly_end + 1), endptr);
    /* Brak wykładnika lub brak zakończenia jednomianu */
    if (*endptr == NULL || *endptr[0] != ')') {
        PolyDestroy(&coeff);
        return false;
    }
    else {
        (*endptr)++;
        /* Ponieważ operacje na wielomianach wykluczają istnienie 0 * x^m, gdzie m != 0 */
        if (PolyIsZero(&coeff))
            *m = MonoFromPoly(&coeff, 0);
        else
            *m = MonoFromPoly(&coeff, exp);
        return true;
    }
}

/**
 * Przetwarza początkowe znaki ciągu na wielomian stały. Jeśli w zapisie
 * wielomianu występuje błąd, zwraca @p false.
 * @param[in,out] p : wielomian, w którym zostanie zapisana wartość
 * @param[in] line : ciąg znaków
 * @param[in,out] endptr : wskaźnik na pierwszy element, który nie został
 * przetworzony do wielomianu
 * @return Czy wielomian został prawidłowo przetworzony?
 */
bool ParseCoeff(Poly *p, char *line, char **endptr) {
    poly_coeff_t poly_coeff = strtol(line, endptr, BASE_10);
    if (errno == ERANGE) {
        errno = 0;
        return false;
    }
    if (*endptr[0] != ',' && *endptr[0] != '\0')
        return false;
    *p = PolyFromCoeff(poly_coeff);
    return true;
}

/**
 * Na podstawie napisu @p line tworzy wielomian, który zapisuje w zmiennej @p p.
 * Zwraca @p false, jeśli w zapisie wielomianu wystąpił błąd, w przeciwnym
 * przypadku zwraca @p true. Pierwszy znak, który wystąpi po wielomianie, zapisuje
 * w @p endptr.
 * @param[in,out] p : wielomian
 * @param[in] line : linia, czyli ciąg znaków
 * @param[in] line_length : długość linii
 * @param[in,out] endptr : wskaźnik na znak, na którym zostanie zakończone
 * parsowanie wielomianu
 * @return Czy operacja się powiodła?
 */
static bool ParsePoly(Poly *p, char *line, size_t line_length, char **endptr) {
    p->arr = NULL;
    p->size = 0;
    p->coeff = 0;
    /* Jeśli zaczyna się od liczby, musi być współczynnikiem. */
    if (line_length > 0 && (isdigit(line[0]) || line[0] == '-'))
        return ParseCoeff(p, line, endptr);

    bool plus = false;
    size_t index = 0;
    size_t monos_size = CountMonos(line);
    p->arr = SafeMonoMalloc(monos_size);
    while (index < line_length && line[index] != '\0' && line[index] != ',') {
        /* Jeśli obecnym znakiem powinien być + */
        if (plus) {
            *endptr = (line + index);
            if (line[index] != '+' || (line[index] == '+' && line[index + 1] != '(')) {
                PolyDestroy(p);
                return false;
            }
            else
                index++;
        }
        Mono m;
        bool op = ParseMono(&m, (line + index), line_length - index, endptr);
        if (!op) {
            if (p->arr != NULL)
                PolyDestroy(p);
            return false;
        }
        p->arr[p->size++] = m;
        index = *endptr - line;
        plus = true;
    }
    *endptr = (line + index);
    if (line[index] != ',' && line[index] != '\0') {
        PolyDestroy(p);
        return false;
    }
    else {
        Poly new_p = PolyAddMonos(p->size, p->arr);
        free(p->arr);
        *p = new_p;
        return true;
    }
}

/**
 * Przeprowadza wstępne sprawdzenie poprawności wielomianu.
 * Sprawdzane warunki:
 * - wielomian zawiera tylko dozwolone znaki
 * - rozpoczyna się prawidłowym znakiem
 * - wszystkie nawiasy są "domknięte"
 * - po zamykającym nawiasie występuje tylko koniec linii, przecinek lub plus
 * @param[in] line : ciąg znaków
 * @param[in] line_length : długość ciągu znaków
 * @return @p false, jeśli zapis wielomianu jest niepoprawny
 */
static bool CheckCharacters(const char *line, size_t line_length) {
    if (line[0] != '(' && line[0] != '-' && !isdigit(line[0]))
        return false;

    /* Nie może wystąpić sam minus */
    if (line[0] == '-' && !isdigit(line[1]))
        return false;

    /* Wielomian stały powinien zawierać tylko i wyłącznie liczby*/
    if (isdigit(line[0]) || line[0] == '-') {
        for (size_t i = 1; i < line_length; i++) {
            if (!isdigit(line[i]))
                return false;
        }
        return true;
    }

    size_t brackets_counter = 0;
    size_t index = 0;
    while (index < line_length) {
        if (line[index] == '(') {
            brackets_counter++;
        }
        else if (line[index] == ')') {
            if (line[index + 1] != '+' && line[index + 1] != '\0' && line[index + 1] != ',')
                return false;
            brackets_counter--;
        }
        else if (!strchr(ValidPolyCharacters, line[index]))
            return false;
        index++;
    }
    return true && brackets_counter == 0;
}

void ParseInputPoly(Stack *s, char *line, size_t line_length, long line_number) {
    char *endptr;
    Poly poly;
    /* Wystąpiły niedozwolone znaki lub zły początek wielomianu */
    if (!CheckCharacters(line, line_length)) {
        PrintWrongPolyError(line_number);
        return;
    }

    bool succesful_parsing = ParsePoly(&poly, line, line_length, &endptr);
    if (!succesful_parsing)
        PrintWrongPolyError(line_number);
    else if (endptr[0] != '\0') {
        PolyDestroy(&poly);
        PrintWrongPolyError(line_number);
    }
    else
        Push(s, &poly);
}