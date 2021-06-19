/** @file
 * Implementacja kalkulatora wielomianów.
 *
 * @author Katarzyna Mielnik <km429567@students.mimuw.edu.pl>
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE ///< Wymagane do poprawnego działania funkcji getline.
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include "calc_op.h"
#include "stack.h"
#include "poly.h"


#define MAX_EXP 2147483647 ///< Maksymalna wartość wykładnika jednomianu
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
 * Znaki, które mogą wystąpić w zapisie wielomianu.
 */
const char *ValidPolyCharacters = "0123456789()+-,";

/**
 * Nazwa polecenia odpowiadającego operacji @ref DegBy.
 */
const char *DegByCommandName = "DEG_BY";

/**
 * Nazwa polecenia odpowiadającego operacji @ref At.
 */
const char *AtCommandName = "AT";

/**
 * Wypisuje komunikat o błędzie podczas parsowania wielomianu na standardowe
 * wyjście diagnostyczne.
 * @param[in] line_number : numer linii, w której wystąpił błąd
 */
static void PrintWrongPolyError(long line_number) {
    fprintf(stderr, "ERROR %ld WRONG POLY\n", line_number);
}

/**
 * Wypisuje komunikat o błędnym poleceniu na standardowe wyjście diagnostyczne.
 * @param[in] line_number : numer linii, w której wystąpił błąd
 */
static void PrintWrongCommandError(long line_number) {
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
 * Sprawdza poprawność argumentu polecenia @ref DegBy oraz wykonuje operację z
 * poprawnym argumentem. W przypadku błędnego argumentu lub niewystarczającej
 * liczby argumentów na stosie, wypisuje na standardowe wyjście komunikat o
 * błędzie.
 * @param[in,out] s : stos
 * @param[in] arg : argument operacji w postaci ciągu znaków
 * @param[in] line_number : numer linii
 */
void ParseDegBy(Stack *s, char *arg, long line_number) {
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

/**
 * Sprawdza poprawność argumentu polecenia @ref At oraz wykonuje operację z
 * poprawnym argumentem. W przypadku błędnego argumentu lub niewystarczającej
 * liczby argumentów na stosie, wypisuje na standardowe wyjście komunikat o
 * błędzie.
 * @param[in] s : stos
 * @param[in] arg : argument polecenia zapisany w postaci ciągu znaków
 * @param line_number : numer linii
 */
void ParseAt(Stack *s, char *arg, long line_number) {
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
 * Przetwarza linię z zapisanym poleceniem. Jeśli nazwa polecenia lub argument
 * są niepoprawne, wypisuje na standardowe wyjście diagnostyczne komunikat o
 * błędzie. W przypadku poprawności zapisu, wywołuje operację odpowiadającą
 * poleceniu.
 * @param[in,out] s : stos wielomianów
 * @param[in,out] line : linia, czyli ciąg znaków
 * @param[in] line_size : rozmiar linii
 * @param[in] line_number : numer linii
 */
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
    if (monos_size ==  0)
        return false;
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
bool CheckCharacters(const char *line, size_t line_length) {
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

/**
 * Przetwarza linię zawierającą zapis wielomianu. W przypadku błędu w zapisie,
 * na standardowe wyjście diagnostyczne zostaje wypisany komunikat o błędzie.
 * @param[in,out] s : stos
 * @param[in] line : linia, czyli ciąg znaków
 * @param[in] line_length : liczba niezerowych znaków w linii
 * @param[in] line_number : numer linii
 */
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
 * błędu
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