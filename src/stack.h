/** @file
  Interfejs stosu, którego elementami są wielomiany.

  @author Daniel Mastalerz
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#ifndef STACK_H
#define STACK_H

#include "poly.h"

/**
 * To jest struktura przechowująca stos.
 * Stos ma obecny rozmiar, pojemność oraz tablicę wielomianów.
 * Element na szczycie stostu to element tablicy o indeksie size - 1.
 */
typedef struct Stack {
    size_t size; ///< rozmiar stosu
    size_t capacity; ///< pojemność stosu
    Poly* arr; ///< tablica, w której przechowywane są elementy stosu
} Stack;

/**
 * Tworzy nowy stos o pojemności równej jeden.
 * @return nowy stos o pojemności równej jeden.
 */
Stack NewStack();

/**
 * Wstawia element na szczyt stosu.
 * @param[in] stack : stos
 * @param[in] p : wielomian
 */
void StackAdd(Stack* stack, Poly p);

/**
 * Zwraca element ze szczytu stosu.
 * @param[in] stack: stos
 * @return wielomian ze szczytu stosu.
 */
Poly StackTop(Stack* stack);

/**
 * Usuwa element ze szczytu stosu.
 * @param[in] stack: stos
 */
void StackPop(Stack* stack);

/**
 * Zwalnia pamięć przeznaczoną na stos, wraz z wielomianami znajdującymi się na stosie.
 * @param[in] stack: stos
 */
void StackDestroy(Stack* stack);

#endif //STACK_H