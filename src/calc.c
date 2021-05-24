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


typedef struct {
    char *name;

    bool (*function)(Stack *);
} Operation;


Operation operations[12] = {
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

char *GetArg(char **line, size_t line_size) {
    size_t i = 0;
    while ((*line)[i] != '\n' && i != line_size) {
        if ((*line)[i] == ' ') {
            (*line)[i] = '\0';
            return (*line + i + 1);
        }
        i++;
    }
    return NULL;
}

void RemoveNewline(char *line, size_t line_size) {
    size_t i = 0;
    while (line[i] != '\0' && i != line_size) {
        if (line[i] == '\n') {
            line[i] = '\0';
            return;
        }
        i++;
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

void ParseCommand(Stack *s, char *line, long line_size, long line_number) {
    char *arg = GetArg(&line, line_size);
    RemoveNewline(line, line_size);
    if (strcmp("DEG_BY", line) == 0) {
        TryDegBy(s, arg, line_number);
        return;
    }
    else if (strcmp("AT", line) == 0) {
        ParseAt(s, arg, line_number);
        return;
    }
    for (int i = 0; i < 12; i++) {
        if (strcmp(operations[i].name, line) == 0) {
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
 * @param[in] s : ciąg znaków
 * @param[in,out] endptr : wskaźnik na pierwszy znak, którego nie udało się przetworzyć
 * @param[in] s_length : długość ciągu znaków
 * @return wykładnik jednomianu
 */
static poly_exp_t StringToPolyExp(char *s, char **endptr, long s_length) {
    *endptr = s;
    long index = 0;
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

size_t CountMonos(const char *line) {
    size_t monos_counter = 1;
    long unclosed_brackets = 0;
    size_t index = 0;
    while (line[index] != '\n' && line[index] != '\0') {
        if (line[index] == '(')
            unclosed_brackets++;
        else if (line[index] == ')')
            unclosed_brackets--;
//        else if (line[index] == '+') {
        if (unclosed_brackets == 0)
            monos_counter++;
//        }
        index++;
    }
    return monos_counter;
}

static bool ParsePoly(Poly *p, char *line, long line_length, char **endptr);

static bool ParseMono(Mono *n, char *line, long line_length, char **endptr) {
    if (line[0] != '(')
        return false;

    char *comma;
    Poly coeff;
    bool op = ParsePoly(&coeff, (line + 1), line_length - 1, &comma);
    if (!op)
        return false;
    if (comma[0] != ',') {
        PolyDestroy(&coeff);
        return false;
    }

    poly_exp_t exp = StringToPolyExp((comma + 1), endptr, line_length - (comma - line + 1));
    if (*endptr[0] != ')') {
        PolyDestroy(&coeff);
        return false;
    }
    else {
        (*endptr)++;
        if (PolyIsZero(&coeff))
            *n = MonoFromPoly(&coeff, 0);
        else
            *n = MonoFromPoly(&coeff, exp);
        return true;
    }
}

bool ParseCoeff(Poly *p, char *line, char **endptr) {
    poly_coeff_t poly_coeff = strtol(line, endptr, 0);
    if (errno == ERANGE) {
        errno = 0;
        return false;
    }
    if (*endptr[0] != ',' && *endptr[0] != '\n' && *endptr[0] != '\0')
        return false;
    *p = PolyFromCoeff(poly_coeff);
    return true;
}


static bool ParsePoly(Poly *p, char *line, long line_length, char **endptr) {
    p->arr = NULL;
    p->size = 0;
    p->coeff = 0;
    if (line_length > 0 && (isdigit(line[0]) || line[0] == '-'))
        return ParseCoeff(p, line, endptr);

    bool plus = false;
    long index = 0;
    size_t monos_size = CountMonos(line);
    p->arr = malloc(monos_size * sizeof(Mono));
    while (index < line_length && line[index] != '\n' && line[index] != ',') {
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
        if ((*endptr)[0] == '\n' || (*endptr)[0] == '+' || (*endptr)[0] == ',' ||
            (*endptr)[0] == '\0') {
            continue;
        }
        else {
            PolyDestroy(p);
            return false;
        }
    }
    *endptr = (line + index);
    if (line[index] != '\n' && line[index] != ',' && line[index] != '\0') {
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

bool CheckCharacters(const char *line) {
    size_t index = 0;
    size_t bracket_counter = 0;
    while (line[index] != '\n' && line[index] != '\0') {
        if (line[index] == '(')
            bracket_counter++;
        else if (line[index] == ')') {
            if (line[index + 1] != '\n' && line[index + 1] != '\0' && line[index + 1] != ',' &&
                line[index + 1] != '+')
                return false;
            bracket_counter--;
        }
        index++;
    }
    return bracket_counter == 0;
}

void ParseInputPoly(Stack *s, char *line, long line_length, long line_number) {
    char *endptr;
    Poly poly;
    if (!CheckCharacters(line)) {
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

        if (isalpha(buff[0]))
            ParseCommand(s, buff, characters, line_number);
        else
            ParseInputPoly(s, buff, characters, line_number);
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