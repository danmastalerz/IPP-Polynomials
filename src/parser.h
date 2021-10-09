/** @file
  Interfejs parsera, którego analizuje polecenie przekazane przez użytkownika.

  @author Daniel Mastalerz
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#ifndef PARSER_H
#define PARSER_H

#include "poly.h"
#include "stack.h"

/**
 * Analizuję i przetwarza linię wpisaną przez użytkownika
 * @param[in] curr_line: polecenie
 * @param[in] line: numer wiersza
 * @param[in] line_length: długość polecenia
 * @param[in] s: stos
 */
void LineInterpreter(char* curr_line, size_t line, size_t line_length, Stack* s);

#endif //PARSER_H