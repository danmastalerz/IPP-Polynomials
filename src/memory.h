/** @file
  Interfejs modułu alokującego pamięć.

  @author Daniel Mastalerz
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <sys/types.h>

/**
 * Alokuje blok pamięci o zadanej wielkości. W przypadku niepowodzenia kończy program z kodem 1.
 * @param[in] size : wielkośc bloku pamięci
 * @return wskaźnik na zaalokowany blok pamięci.
 */
void *SafeMalloc(size_t size);

/**
 * Realokuje blok pamięci o zadanej wielkości. W przypadku niepowodzenia kończy program z kodem 1.
 * @param[in] ptr : wskaźnik na blok pamięci
 * @param[in] size : nowa wielkość bloku pamięci
 * @return wskaźnik na zaalokowany blok pamięci.
 */
void *SafeRealloc(void* ptr, size_t size);

/**
 * Wczytuje wiersz ze standardowego wejścia. W przypadku niepowodzenia kończy program z kodem 1.
 * @param[in] line : wskaźnik na wskaźnik na.
 * @param[in] n : wskaźnik na liczbę określającą zaalokowaną pamięć.
 * @param[in] stream : wskaźnik na wejście.
 * @return liczba wczytanych znaków lub -1 w przypadku zakończenia wczytywania.
 */
ssize_t SafeGetLine(char** line, size_t *n, FILE* stream);

#endif //MEMORY_H