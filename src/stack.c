/** @file
 * Implementacja stosu wielomianów na dynamicznie alokowanej tablicy
 *
 * @author Katarzyna Mielnik <km429567@students.mimuw.edu.pl>
 */
#include <stdlib.h>
#include "stack.h"

#define INITIAL_STACK_SIZE 16

static void IncreaseStackSize(Stack *s) {
    s->arr_size *= 2;
    Poly *temp_arr = realloc(s->arr, s->arr_size * sizeof(Poly));
    if(temp_arr == NULL)
        exit(1);
    else
        s->arr = temp_arr;
}

void Push(Stack *s, Poly *p) {
    if (s->size == s->arr_size)
        IncreaseStackSize(s);
    s->arr[s->size++] = *p;
}

Poly Pop(Stack *s) {
    if (IsEmpty(s))
        return PolyZero();
    Poly top = s->arr[s->size - 1];
    s->size--;
    return top;
}

bool IsEmpty(Stack *s) {
    return s->size == 0;
}

void Clear(Stack *s) {
    if (s == NULL)
        return;
    for (size_t i = 0; i < s->size; i++)
        PolyDestroy(&s->arr[i]);

    free(s->arr);
}

Poly Top(Stack *s) {
    if (IsEmpty(s))
        return PolyZero();
    return s->arr[s->size - 1];
}

Poly SecondTop(Stack *s) {
    if (s->size < 2)
        return PolyZero();
    return s->arr[s->size - 2];
}

Stack GetNewStack() {
    Stack s;
    Poly *arr = malloc(INITIAL_STACK_SIZE * sizeof(Poly));
    if (arr == NULL)
        exit(1);
    else
        s.arr = arr;
    s.size = 0;
    s.arr_size = INITIAL_STACK_SIZE;
    return s;
}

