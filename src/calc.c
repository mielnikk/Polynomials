/** @file
 * Implementacja kalkulatora wielomianów.
 *
 * @author Katarzyna Mielnik <km429567@students.mimuw.edu.pl>
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include "calc_op.h"
#include "stack.h"
#include "poly.h"

#define ONE_ARG_OP_NUMBER 12

/**
 * Struktura przechowująca operację na stosie, która nie przyjmuje dodatkowych
 * argumentów, oraz odpowiadjącą jej nazwę polecenia.
 */
typedef struct {
    char *name;
    bool (*function)(Stack *);
} Operation;

/**
 * Tablica operacji na stosie wraz z nazwami ich poleceń. Znajdują się tu
 * operacje, których działanie zależy wyłącznie od stosu.
 */
const Operation operations[ONE_ARG_OP_NUMBER] = {
        {.function = Zero, .name = "ZERO"},
        {.function = IsCoeff, .name = "IS_COEFF"},
        {.function = IsZero, .name = "IS_ZERO"},
        {.function = Clone, .name = "CLONE"},
        {.function = Neg, .name = "NEG"},
        {.function = Add, .name = "ADD"},
        {.function = Sub, .name = "SUB"},
        {.function = Mul, .name = "MUL"},
        {.function = IsEq, .name = "IS_EQ"},
        {.function = Deg, .name = "DEG"},
        {.function = Print, .name = "PRINT"},
        {.function = StackPop, .name = "POP"}
};

/**
 * Zwraca wskaźnik na pierwszy znak argumentu operacji. Nadpisuje pierwsszą
 * spację, która oddziela polecenie od argumentu, znakiem zerowym.
 * @param[in,out] line : linia, czyli ciąg znaków
 * @param[in] line_size : rozmiar linii
 * @return wskaźnik na pierwszy znak argumentu operacji
 */
static char *GetArg(char *line, size_t line_size) {
    size_t i = 0;
    while (line[i] != '\n' && i != line_size) {
        if (line[i] == ' ') {
            line[i] = '\0';
            return (line + i + 1);
        }
        i++;
    }
    return NULL;
}

/**
 * Nadpisuje znak nowej linii białym znakiem.
 * @param line
 * @param line_size
 */
void RemoveNewline(char *line, size_t *line_size) {
    if(*line_size > 0 && line[*line_size - 1] == '\n'){
        line[*line_size - 1] = '\0';
        *line_size = *line_size - 1;
    }
}

void TryDegBy(Stack *s, char *arg, long line_number) {
    if (arg == NULL || !isdigit(arg[0])) {
        fprintf(stderr, "ERROR %ld DEG BY WRONG VARIABLE\n", line_number);
        return;
    }
    char *endptr;
    unsigned long long value = strtoull(arg, &endptr, 10);
    if (errno == ERANGE) {
        fprintf(stderr, "ERROR %ld DEG BY WRONG VARIABLE\n", line_number);
        errno = 0;
        return;
    }

    if (endptr[0] != '\0' && endptr[0] != '\n') {
        fprintf(stderr, "ERROR %ld DEG BY WRONG VARIABLE\n", line_number);
        return;
    }

    bool op = DegBy(s, value);
    if (!op)
        fprintf(stderr, "ERROR %ld STACK UNDERFLOW\n", line_number);
}

void ParseAt(Stack *s, char *arg, long line_number) {
    if (arg == NULL || (arg[0] != '-' && !isdigit(arg[0]))) {
        fprintf(stderr, "ERROR %ld AT WRONG VALUE\n", line_number);
        return;
    }
    char *endptr;
    long long value = strtoll(arg, &endptr, 10);

    if (errno == ERANGE) {
        fprintf(stderr, "ERROR %ld AT WRONG VALUE\n", line_number);
        errno = 0;
        return;
    }

    if (endptr[0] != '\0' && endptr[0] != '\n') {
        fprintf(stderr, "ERROR %ld AT WRONG VALUE\n", line_number);
        return;
    }

    bool op = At(s, value);
    if (!op)
        fprintf(stderr, "ERROR %ld STACK UNDERFLOW\n", line_number);
}

void ParseCommand(Stack *s, char *line, size_t line_size, long line_number) {
    char *arg = GetArg(line, line_size);
    if (strcmp("DEG_BY", line) == 0) {
        TryDegBy(s, arg, line_number);
        return;
    }
    else if (strcmp("AT", line) == 0) {
        ParseAt(s, arg, line_number);
        return;
    }
    for (int i = 0; i < ONE_ARG_OP_NUMBER; i++) {
        if (strcmp(operations[i].name, line) == 0 && strlen(operations[i].name) == line_size) {
            if (arg != NULL)
                break;
            bool op = operations[i].function(s);
            if (!op)
                fprintf(stderr, "ERROR %ld STACK UNDERFLOW\n", line_number);
            return;
        }
    }
    fprintf(stderr, "ERROR %ld WRONG COMMAND\n", line_number);
}

/**
 * Przetwarza początkowe znaki ciągu na wykładnik jednomianu.
 * Jeśli w @p s nie ma żadnych cyfr do przetworzenia, @p endptr zostaje ustawiony
 * na @p NULL.
 * @param[in] s : ciąg znaków
 * @param[in,out] endptr : wskaźnik na pierwszy znak, którego nie udało się przetworzyć
 * @param[in] s_length : długość ciągu znaków
 * @return wykładnik jednomianu
 */
static poly_exp_t StringToPolyExp(char *s, char **endptr, size_t s_length) {
    if (!isdigit(s[0])){
        *endptr = NULL;
        return 0;
    }
    *endptr = s;
    size_t index = 0;
    poly_exp_t exp = 0;
    while (index < s_length && s[index] != ')') {
        if (isdigit(s[index])) {
            exp *= 10;
            exp += s[index++] - '0';
            if (exp < 0) /* overflow */
                return -1;
            *endptr = (s + index);
        }
        else {
            return -1;
        }
    }
    return exp;
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
        index++;
    }
    return monos_counter;
}

static bool ParsePoly(Poly *p, char *line, size_t line_length, char **endptr);

static bool ParseMono(Mono *n, char *line, size_t line_length, char **endptr) {
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

    poly_exp_t exp = StringToPolyExp((poly_end + 1), endptr, line_length - (poly_end - line + 1));
    /* Brak wykładnika lub brak zakończenia jednomianu */
    if (*endptr == NULL || *endptr[0] != ')') {
        PolyDestroy(&coeff);
        return false;
    }
    else {
        (*endptr)++;
        /* Ponieważ operacje na wielomianach wykluczają istnienie 0 * x^n, gdzie n != 0 */
        if (PolyIsZero(&coeff))
            *n = MonoFromPoly(&coeff, 0);
        else
            *n = MonoFromPoly(&coeff, exp);
        return true;
    }
}

/**
 * Przetwarza początkowe znaki ciągu na współczynnik.
 * @param[in,out] p
 * @param[in] line
 * @param[in,out] endptr
 * @return
 */
bool ParseCoeff(Poly *p, char *line, char **endptr) {
    poly_coeff_t poly_coeff = strtol(line, endptr, 10);
    if (errno == ERANGE) {
        errno = 0;
        return false;
    }
    if (*endptr[0] != ',' && *endptr[0] != '\0')
        return false;
    *p = PolyFromCoeff(poly_coeff);
    return true;
}


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
    p->arr = malloc(monos_size * sizeof(Mono));
    while (index < line_length && line[index] != '\0' && line[index] != ',') {
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
        if ((*endptr)[0] == '+' || (*endptr)[0] == ',' || (*endptr)[0] == '\0') {
            continue;
        }
        else {
            PolyDestroy(p);
            return false;
        }
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
 * Wstępnie sprawdza warunki poprawności zapisu wielomianu.
 * @param[in] line : ciąg znaków
 * @return Czy zapis wielomianu jest poprawny?
 */
bool CheckInitialValidity(const char *line) {
    if(line[0] != '(' && line[0] != '-' && !isdigit(line[0]))
        return false;

    size_t index = 0;
    size_t bracket_counter = 0;
    while (line[index] != '\n' && line[index] != '\0') {
        if (line[index] == '(')
            bracket_counter++;
        else if (line[index] == ')') {
            if (line[index + 1] != '\0' && line[index + 1] != ',' && line[index + 1] != '+')
                return false;
            bracket_counter--;
        }
        else if(line[index] != '+' && line[index] != '-' && line[index] != ','
        && !isdigit(line[index]))
            return false;
        index++;
    }
    return bracket_counter == 0;
}

void ParseInputPoly(Stack *s, char *line, size_t line_length, long line_number) {
    char *endptr;
    Poly poly;
    if (!CheckInitialValidity(line)) {
        fprintf(stderr, "ERROR %ld WRONG POLY\n", line_number);
        return;
    }
    bool succesful_parsing = ParsePoly(&poly, line, line_length, &endptr);
    if (!succesful_parsing)
        fprintf(stderr, "ERROR %ld WRONG POLY\n", line_number);
    else if ((endptr[0] != '\n' && endptr[0] != '\0')) {
        PolyDestroy(&poly);
        fprintf(stderr, "ERROR %ld WRONG POLY\n", line_number);
    }
    else
        Push(s, &poly);
}

void ReadInput(Stack *s) {
    char *buff = malloc(sizeof(char));
    size_t buffsize = 1;
    errno = 0;

    long characters;
    long line_number = 0;

    while ((characters = getline(&buff, &buffsize, stdin)) != -1) {
        line_number++;
        if (characters > 0 && (buff[0] == '#' || buff[0] == '\n'))
            continue;

        size_t line_length = (size_t) characters;
        RemoveNewline(buff, &line_length);

        if(line_length == 0)
            continue;

        if (isalpha(buff[0]))
            ParseCommand(s, buff, line_length, line_number);
        else
            ParseInputPoly(s, buff, line_length, line_number);
    }

    if (errno != 0)
        exit(1);

    free(buff);
}

int main() {
    Stack stack = GetNewStack();
    ReadInput(&stack);
    Clear(&stack);
    return 0;
}