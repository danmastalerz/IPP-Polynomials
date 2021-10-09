/** @file
  Implementacja modułu alokującego pamięć.

  @author Daniel Mastalerz
  @copyright Uniwersytet Warszawski
  @date 2021
*/

//To jest makro potrzebne do działania funkcji getline.
#define _GNU_SOURCE

#include <stdlib.h>
#include <errno.h>
#include "memory.h"

void *SafeMalloc(size_t size) {
    void *allocated = malloc(size);
    if (allocated != NULL) return allocated;
    exit(1);
}

void *SafeRealloc(void* ptr, size_t size) {
    void* allocated = realloc(ptr, size);
    if (allocated != NULL) {
        return allocated;
    }
    exit(1);
}

ssize_t SafeGetLine(char** line, size_t *n, FILE* stream) {
    errno = 0;
    ssize_t x = getline(line, n, stream);
    if (errno == ENOMEM) exit(1);
    return x;
}