/** @file
  Kalkulator wielomianów rzadkich wielu zmiennych.

  @author Daniel Mastalerz
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#include <stdlib.h>
#include "poly.h"
#include "stdio.h"
#include "stack.h"
#include "parser.h"
#include <ctype.h>
#include "memory.h"

/**
 * Główna cześć programu, wczytuje linie i wykonuje polecenia.
 */
int main() {
    ssize_t line_length = 0;
    size_t size = 0;
    char *curr_line = NULL;
    int line = 0;
    Stack s = NewStack();

    while ((line_length = SafeGetLine(&curr_line, &size, stdin)) != -1) {
        line++;
        LineInterpreter(curr_line, line, line_length, &s);
    }

    StackDestroy(&s);
    free(curr_line);

    return 0;

}
