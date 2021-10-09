/** @file
  Implementacja stosu, którego elementami są wielomiany.

  @author Daniel Mastalerz
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#include <stdlib.h>
#include "poly.h"
#include "stack.h"
#include "memory.h"

Stack NewStack() {
    Poly *arr = (Poly *) SafeMalloc(sizeof(Poly));
    Stack s = (Stack) {.size = 0, .capacity = 1, .arr = arr};
    return s;
}

void StackAdd(Stack *stack, Poly p) {
    if (stack->size >= stack->capacity) {
        stack->capacity *= 2;
        stack->arr = SafeRealloc(stack->arr, stack->capacity * sizeof(Poly));
    }
    stack->arr[stack->size] = p;
    stack->size++;
}

Poly StackTop(Stack *stack) {
    return stack->arr[stack->size - 1];
}

void StackPop(Stack *stack) {
    if (stack->size == 0) return;
    stack->size--;
}

void StackDestroy(Stack *stack) {
    for (size_t i = 0; i < stack->size; i++) {
        PolyDestroy(&stack->arr[i]);
    }
    free(stack->arr);
}


