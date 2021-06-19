#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "command_parser.h"
#include "calc_op.h"

#define ONE_ARG_OP_NUMBER 12 ///< Liczba operacji przyjmujących jeden argument
#define BASE_10 10 ///< Wartość reprezentująca system dziesiętny

/**
 * Struktura przechowująca operację na stosie, która nie przyjmuje dodatkowych
 * argumentów, oraz odpowiadającą jej nazwę polecenia.
 */
typedef struct {
    char *name; ///< Nazwa polecenia
    bool (*function)(Stack *); ///< Operacja na stosie
} Operation;

/**
 * Tablica operacji na stosie wraz z nazwami ich poleceń. Znajdują się tu
 * operacje, które nie przyjmują innych argumentów niż stos.
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
 * Nazwa polecenia odpowiadającego operacji @ref DegBy.
 */
const char *DegByCommandName = "DEG_BY";

/**
 * Nazwa polecenia odpowiadającego operacji @ref At.
 */
const char *AtCommandName = "AT";

void PrintWrongCommandError(long line_number) {
    fprintf(stderr, "ERROR %ld WRONG COMMAND\n", line_number);
}

/**
 * Wypisuje na standardowe wyjście diagnostyczne komunikat o błędnym parametrze
 * lub jego braku przy poleceniu @ref DegBy.
 * @param[in] line_number : numer linii, w której wystąpił błąd
 */
static void PrintDegByWrongVariableError(long line_number) {
    fprintf(stderr, "ERROR %ld DEG BY WRONG VARIABLE\n", line_number);
}

/**
 * Wypisuje na standardowe wyjście diagnostyczne komunikat o błędnym parametrze
 * lub jego braku przy poleceniu @ref At.
 * @param[in] line_number : numer linii, w której wystąpił błąd
 */
static void PrintWrongAtValue(long line_number) {
    fprintf(stderr, "ERROR %ld AT WRONG VALUE\n", line_number);
}

/**
 * Wypisuje na standardowe wyjście diagnostyczne komunikat o niewystarczającej
 * liczbie wielomianów, aby wykonać operację.
 * @param[in] line_number : numer linii, w której wystąpił błąd
 */
static void PrintStackUnderflowError(long line_number) {
    fprintf(stderr, "ERROR %ld STACK UNDERFLOW\n", line_number);
}

/**
 * Zwraca wskaźnik na pierwszy znak argumentu operacji. Nadpisuje pierwsszą
 * spację, która oddziela polecenie od argumentu, znakiem zerowym.
 * @param[in,out] line : linia, czyli ciąg znaków
 * @param[in] line_size : rozmiar linii
 * @return wskaźnik na pierwszy znak argumentu operacji
 */
static char *GetArg(char *line, size_t line_size) {
    size_t i = 0;
    while (i != line_size) {
        if (line[i] == ' ') {
            line[i] = '\0';
            return (line + i + 1);
        }
        i++;
    }
    return NULL;
}

/**
 * Sprawdza poprawność argumentu polecenia @ref At oraz wykonuje operację z
 * poprawnym argumentem. W przypadku błędnego argumentu lub niewystarczającej
 * liczby argumentów na stosie, wypisuje na standardowe wyjście komunikat o
 * błędzie.
 * @param[in] s : stos
 * @param[in] arg : argument polecenia zapisany w postaci ciągu znaków
 * @param line_number : numer linii
 */
static void ParseAt(Stack *s, char *arg, long line_number) {
    if (arg == NULL || (arg[0] != '-' && !isdigit(arg[0]))) {
        PrintWrongAtValue(line_number);
        return;
    }
    char *endptr;
    long long value = strtoll(arg, &endptr, 10);
    /* Błędna wartość argumentu */
    if (errno == ERANGE) {
        PrintWrongAtValue(line_number);
        errno = 0;
        return;
    }
    /* Argument nie był liczbą */
    if (endptr[0] != '\0') {
        PrintWrongAtValue(line_number);
        return;
    }

    bool op = At(s, value);
    if (!op)
        PrintStackUnderflowError(line_number);
}

/**
 * Sprawdza poprawność argumentu polecenia @ref DegBy oraz wykonuje operację z
 * poprawnym argumentem. W przypadku błędnego argumentu lub niewystarczającej
 * liczby argumentów na stosie, wypisuje na standardowe wyjście komunikat o
 * błędzie.
 * @param[in,out] s : stos
 * @param[in] arg : argument operacji w postaci ciągu znaków
 * @param[in] line_number : numer linii
 */
static void ParseDegBy(Stack *s, char *arg, long line_number) {
    if (arg == NULL || !isdigit(arg[0])) {
        PrintDegByWrongVariableError(line_number);
        return;
    }
    char *endptr;
    unsigned long long value = strtoull(arg, &endptr, BASE_10);
    /* Niepoprawny zakres */
    if (errno == ERANGE) {
        PrintDegByWrongVariableError(line_number);
        errno = 0;
        return;
    }
    /* Argument nie był liczbą */
    if (endptr[0] != '\0') {
        PrintDegByWrongVariableError(line_number);
        return;
    }

    bool op = DegBy(s, value);
    if (!op)
        PrintStackUnderflowError(line_number);
}

void ParseCommand(Stack *s, char *line, size_t line_size, long line_number) {
    /* "Odseparowanie" nazwy i argumentu */
    char *arg = GetArg(line, line_size);

    if (strcmp(DegByCommandName, line) == 0) {
        ParseDegBy(s, arg, line_number);
        return;
    }
    else if (strcmp(AtCommandName, line) == 0) {
        ParseAt(s, arg, line_number);
        return;
    }
    for (int i = 0; i < ONE_ARG_OP_NUMBER; i++) {
        if (arg != NULL)
            break;
        size_t name_length = strlen(operations[i].name);
        if (strcmp(operations[i].name, line) == 0 && name_length == line_size) {
            bool op = operations[i].function(s);
            if (!op)
                PrintStackUnderflowError(line_number);
            return;
        }
    }
    PrintWrongCommandError(line_number);
}