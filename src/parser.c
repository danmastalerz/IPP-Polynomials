/** @file
  Implementacja parsera, którego analizuje polecenie przekazane przez użytkownika.

  @author Daniel Mastalerz
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include "memory.h"

/**
 * Sprawdza, czy wczytane polecenie ma strukturę wielomianu.
 * Konkretnie, czy nawiasy są ustawione poprawnie oraz czy linia zawiera tylko dozwolone znaki.
 * Sprawdza też, czy też, czy dwa sąsiednie znaki są poprawne.
 * @param[in] c: polecenie
 * @return wartość logiczna stwierdzająca, czy wczytanie polecenie ma strukturę wielomianu.
 */
static bool IsCorrect(char *c) {
    int open = 0, close = 0, coma = 0;
    while (*c != '\0') {
        bool okay = false;
        char *z = c + 1;
        if (*c == '\n' || isdigit(*c))
            okay = true;

        if (*c == '+' && *z == '(')
            okay = true;

        if (*c == '-' && isdigit(*z))
            okay = true;

        if (*c == '(' && (isdigit(*z) || *z == '-' || *z == '(')) {
            okay = true;
            open++;
        }

        if (*c == ')' && (*z == '+' || *z == '\n' || *z == '\0' || *z == ')' || *z == ',')) {
            okay = true;
            close++;
            if (*(c + 1) == '(') {
                return false;
            }
        }

        if (*c == ',' && (isdigit(*z) || *z == '-' || *z == '+')) {
            okay = true;
            coma++;
            if (coma > open) return false;
        }

        if (!okay || open < close)
            return false;

        c++;
    }

    return open == close;
}

/**
 * Wypisuje wielomian.
 * @param[in] p: wielomian.
 */
static void PolyPrint(const Poly *p) {
    if (PolyIsCoeff(p)) printf("%ld", p->coeff);
    else {
        for (size_t i = 0; i < p->size; i++) {
            printf("(");
            PolyPrint(&p->arr[i].p);
            printf(",%d)", p->arr[i].exp);
            if (i != p->size - 1) printf("+");
        }
    }

}

/**
 * Sprawdza, czy słowo jest poprawnym współczynnikiem ze względu na treść zadania.
 * @param[in] word: słowo
 * @return wartość logiczna stwierdzająca, czy słowo jest poprawnym współczynnikiem.
 */
static bool IsCoeff(const char *word) {
    errno = 0;
    char *endPtr = NULL;
    strtol(word, &endPtr, 10);
    bool okEndPtr = *endPtr == ',' || *endPtr == '\n' || *endPtr == '\0';
    bool okFirstCharacter = isdigit(word[0]) || word[0] == '-';
    return errno != ERANGE && okEndPtr && okFirstCharacter;
}

/**
 * Sprawdza, czy słowo jest poprawnym wykładnikiem ze względu na treść zadania.
 * @param[in] word: słowo
 * @return wartość logiczna stwierdzająca, czy słowo jest poprawnym wykładnikiem.
 */
static bool IsExp(const char *word) {
    errno = 0;
    char *endPtr = NULL;
    long x = strtol(word, &endPtr, 10);
    bool okRange = !(x > 2147483647 || x < 0);
    bool okEndPtr = (*endPtr == ')' || *endPtr == '\n' || *endPtr == '\0');
    bool okFirstCharacter = isdigit(word[0]) || word[0] == '-';
    return errno != ERANGE && okRange && okEndPtr && okFirstCharacter;
}

/**
 * Przetwarza słowo na wielomian, który jest współczynnikiem, zakładając, że słowo jest poprawnym współczynnikiem.
 * Zmienia też przekazany wskaźnik na wskaźnik wskazujący na ostatni niewczytany znak.
 * @param[in] word: słowo.
 * @param[in] endPtr: wskaźnik na wskaźnik na znak.
 * @return wielomian będący współczynnikiem przetworzonym ze słowa.
 */
static Poly WordToCoeff(const char *word, char **endPtr) {
    long x = strtol(word, endPtr, 10);
    return PolyFromCoeff(x);
}

/**
 * Funkcja parsująca słowo na wielomian. Ustawia *endPtr na ostatni niewczytany znak.
 * @param[in] word: słowo
 * @param[in] correct: wskaźnik na wartość logiczną, która mówi, czy wielomian jest poprawny.
 * @param[in] endPtr: wskaźnik na wskaźnik, który będzie ustawiony na wskaźnik na pierwszy niewczytany znak.
 * @return wielomian zbudowany z wczytanego słowa.
 */
static Poly ParsePoly(char *word, bool *correct, char **endPtr) {

    if (*correct == false)
        return PolyZero();

    if (IsCoeff(word))
        return WordToCoeff(word, endPtr);

    if (word[0] != '(' || word[1] == ',') {
        *correct = false;
        *endPtr = word;
        return PolyZero();
    }

    size_t number_of_monos = 0;
    size_t capacity = 1;
    Mono *monos = (Mono *) SafeMalloc(capacity * sizeof(Mono));

    while (true) {
        char *temp = NULL;
        char *temp2 = NULL;

        if (number_of_monos >= capacity) {
            capacity *= 2;
            monos = SafeRealloc(monos, capacity * sizeof(Mono));
        }

        monos[number_of_monos].p = ParsePoly(word + 1, correct, &temp);

        if (*correct == false) {
            free(monos);
            *endPtr = temp;
            return PolyZero();
        }

        if (*temp != ',') {
            *correct = false;
            *endPtr = temp;
            free(monos);
            return PolyZero();
        }
        temp++;

        if (!IsExp(temp)) {
            *correct = false;
            PolyDestroy(&monos[number_of_monos].p);
            free(monos);
            return PolyZero();
        }

        monos[number_of_monos].exp = strtol(temp, &temp2, 10);

        number_of_monos++;
        *endPtr = temp2;

        if (**endPtr != ')') {
            *correct = false;
            free(monos);
            return PolyZero();
        }

        *endPtr = *endPtr + 1;
        if (**endPtr == '+')
            word = *endPtr + 1;
        else if (**endPtr == '\n' || **endPtr == '\0' || **endPtr == ',')
            break;
        else {
            *correct = false;
            free(monos);
            return PolyZero();
        }

    }
    Poly p = PolyAddMonos(number_of_monos, monos);
    free(monos);
    return p;
}

/**
 * Sprawdza, czy wielomian ze szczytu stosu jest współczynnikiem.
 * W przypadku niepowodzenia wypisuje błąd na wyjście diagnostyczne.
 * Jeśli wielomian jest współczynnikiem wypisuje 1 na wyjście standardowe.
 * W przeciwnym przypadku wypisuje 0.
 * @param[in] s: stos
 * @param[in] line: numer wiersza.
 */
static void InstructionIsCoeff(Stack *s, size_t line) {
    if (s->size == 0) {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line);
        return;
    }

    Poly p = StackTop(s);
    if (PolyIsCoeff(&p)) {
        printf("1\n");
    } else printf("0\n");
}

/**
 * Sprawdza, czy wielomian ze szczytu stosu jest zerem.
 * W przypadku niepowodzenia wypisuje błąd na wyjście diagnostyczne.
 * Jeśli wielomian jest zerem wypisuje 1 na wyjście standardowe.
 * W przeciwnym przypadku wypisuje 0.
 * @param[in] s: stos
 * @param[in] line: numer wiersza.
 */
static void InstructionIsZero(Stack *s, size_t line) {
    if (s->size == 0) {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line);
        return;
    }
    Poly p = StackTop(s);
    if (PolyIsZero(&p)) printf("1\n");
    else printf("0\n");
}

/**
 * Klonuje wielomian ze szczytu stosu i wstawia go na stos.
 * W przypadku niepowodzenia wypisuje błąd na wyjście diagnostyczne.
 * @param[in] s: stos
 * @param[in] line: numer wiersza.
 */
static void InstructionClone(Stack *s, size_t line) {
    if (s->size == 0) {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line);
        return;
    }
    Poly p = StackTop(s);
    StackAdd(s, PolyClone(&p));
}

/**
 * Usuwa dwa wielomiany ze szczytu stosu, dodaje je i wstawia powstały wielomian na stos.
 * W przypadku niepowodzenia wypisuje błąd na wyjście diagnostyczne.
 * @param[in] s: stos
 * @param[in] line: numer wiersza.
 */
static void InstructionAdd(Stack *s, size_t line) {
    if (s->size < 2) {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line);
        return;
    }
    Poly p = StackTop(s);
    StackPop(s);
    Poly q = StackTop(s);
    StackPop(s);
    StackAdd(s, PolyAdd(&p, &q));
    PolyDestroy(&p);
    PolyDestroy(&q);
}

/**
 * Usuwa dwa wielomiany ze szczytu stosu, mnoży je i wstawia powstały wielomian na stos.
 * W przypadku niepowodzenia wypisuje błąd na wyjście diagnostyczne.
 * @param[in] s: stos
 * @param[in] line: numer wiersza.
 */
static void InstructionMul(Stack *s, size_t line) {
    if (s->size < 2) {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line);
        return;
    }
    Poly p = StackTop(s);
    StackPop(s);
    Poly q = StackTop(s);
    StackPop(s);
    StackAdd(s, PolyMul(&p, &q));
    PolyDestroy(&p);
    PolyDestroy(&q);
}

/**
 * Neguje wielomian ze szczytu stosu.
 * W przypadku niepowodzenia wypisuje błąd na wyjście diagnostyczne.
 * @param[in] s: stos
 * @param[in] line: numer wiersza.
 */
static void InstructionNeg(Stack *s, size_t line) {
    if (s->size == 0) {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line);
        return;
    }
    Poly p = StackTop(s);
    Poly temp = PolyFromCoeff(-1);
    StackPop(s);
    StackAdd(s, PolyMul(&p, &temp));
    PolyDestroy(&p);
}

/**
 * Usuwa dwa wielomiany ze szczytu stosu, odejmuje jeden od drugiego i wstawia powstały wielomian na stos.
 * W przypadku niepowodzenia wypisuje błąd na wyjście diagnostyczne.
 * @param[in] s: stos
 * @param[in] line: numer wiersza.
 */
static void InstructionSub(Stack *s, size_t line) {
    if (s->size < 2) {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line);
        return;
    }
    Poly p = StackTop(s);
    StackPop(s);
    Poly q = StackTop(s);
    StackPop(s);
    StackAdd(s, PolySub(&p, &q));
    PolyDestroy(&p);
    PolyDestroy(&q);
}

/**
 * Sprawdza, czy dwa wielomiany na szycie stosu są sobie równe.
 * W przypadku niepowodzenia wypisuje błąd na wyjście diagnostyczne.
 * Jeśli są równe to wypisuje 1 na wyjście standardowe.
 * W przeciwnym przypadku wypisuje 0.
 * @param[in] s: stos
 * @param[in] line: numer wiersza.
 */
static void InstructionIsEq(Stack *s, size_t line) {
    if (s->size < 2) {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line);
        return;
    }
    Poly p = s->arr[s->size - 1];
    Poly q = s->arr[s->size - 2];
    if (PolyIsEq(&p, &q)) {
        printf("1\n");
    } else printf("0\n");
}

/**
 * Wypisuje stopień wielomianu ze szczytu stosu.
 * W przypadku niepowodzenia wypisuje błąd na wyjście diagnostyczne.
 * @param[in] s: stos
 * @param[in] line: numer wiersza.
 */
static void InstructionDeg(Stack *s, size_t line) {
    if (s->size == 0) {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line);
        return;
    }
    Poly temp = StackTop(s);
    printf("%d\n", PolyDeg(&temp));
}

/**
 * Wypisuje wielomian ze szczytu stosu.
 * W przypadku niepowodzenia wypisuje błąd na wyjście diagnostyczne.
 * @param[in] s: stos
 * @param[in] line: numer wiersza.
 */
static void InstructionPrint(Stack *s, size_t line) {
    if (s->size == 0) {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line);
        return;
    }
    Poly temp = StackTop(s);
    PolyPrint(&temp);
    printf("\n");
}

/**
 * Usuwa wielomian ze szczytu stosu.
 * W przypadku niepowodzenia wypisuje błąd na wyjście diagnostyczne.
 * @param[in] s: stos
 * @param[in] line: numer wiersza.
 */
static void InstructionPop(Stack *s, size_t line) {
    if (s->size == 0) {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line);
        return;
    }
    Poly p = StackTop(s);
    PolyDestroy(&p);
    StackPop(s);
}

/**
 * Wypisuje stopień wielomianu ze szczytu stosu ze względu na podaną zmienną.
 * W przypadku niepowodzenia wypisuje błąd na wyjście diagnostyczne.
 * @param[in] s: stos
 * @param[in] line: numer wiersza.
 * @param[in] line_length: długość wiersza.
 * @param[in] curr_line: wczytany wiersz.
 */
static void InstructionDegBy(Stack *s, size_t line, size_t line_length, char *curr_line) {
    if (line_length < 8) {
        fprintf(stderr, "ERROR %zu DEG BY WRONG VARIABLE\n", line);
        return;
    }

    if (!isspace(curr_line[6])) {
        fprintf(stderr, "ERROR %zu WRONG COMMAND\n", line);
        return;
    }

    if (curr_line[6] != ' ' || !isdigit(curr_line[7])) {
        fprintf(stderr, "ERROR %zu DEG BY WRONG VARIABLE\n", line);
        return;
    }
    errno = 0;
    char *ptr = curr_line + 7;
    char *endPtr = NULL;
    unsigned long long var_idx = strtoull(ptr, &endPtr, 10);
    if (errno == ERANGE) {
        fprintf(stderr, "ERROR %zu DEG BY WRONG VARIABLE\n", line);
        return;
    }

    if (curr_line[line_length - 1] == '\n' && *endPtr != '\n') {
        fprintf(stderr, "ERROR %zu DEG BY WRONG VARIABLE\n", line);
        return;
    } else if (curr_line[line_length - 1] != '\n' && *endPtr != '\0') {
        fprintf(stderr, "ERROR %zu DEG BY WRONG VARIABLE\n", line);
        return;
    }
    if (s->size == 0) {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line);
        return;
    }
    Poly p = StackTop(s);
    printf("%d\n", PolyDegBy(&p, var_idx));
}

/**
 * Wylicza i wypisuje wartość wielomianu w zadanym punkcie.
 * W przypadku niepowodzenia wypisuje błąd na wyjście diagnostyczne.
 * @param[in] s: stos
 * @param[in] line: numer wiersza.
 * @param[in] line_length: długość wiersza.
 * @param[in] curr_line: wczytany wiersz.
 */
static void InstructionAt(Stack *s, size_t line, size_t line_length, char *curr_line) {
    if (line_length < 4) {
        fprintf(stderr, "ERROR %zu AT WRONG VALUE\n", line);
        return;
    }

    if (!isspace(curr_line[2])) {
        fprintf(stderr, "ERROR %zu WRONG COMMAND\n", line);
        return;
    }

    if (curr_line[2] != ' ' || (!isdigit(curr_line[3]) && curr_line[3] != '-')) {
        fprintf(stderr, "ERROR %zu AT WRONG VALUE\n", line);
        return;
    }
    errno = 0;
    char *ptr = curr_line + 3;
    char *endPtr = NULL;
    long long at = strtoll(ptr, &endPtr, 10);
    if (errno == ERANGE) {
        fprintf(stderr, "ERROR %zu AT WRONG VALUE\n", line);
        return;
    }

    if (curr_line[line_length - 1] == '\n' && *endPtr != '\n') {
        fprintf(stderr, "ERROR %zu AT WRONG VALUE\n", line);
        return;
    } else if (curr_line[line_length - 1] != '\n' && *endPtr != '\0') {
        fprintf(stderr, "ERROR %zu AT WRONG VALUE\n", line);
        return;
    }
    if (s->size == 0) {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line);
        return;
    }
    Poly p = StackTop(s);
    Poly q = PolyAt(&p, at);
    StackPop(s);
    PolyDestroy(&p);
    StackAdd(s, q);
}

/**
 * Polecenie to zdejmuje z wierzchołka stosu najpierw wielomian p,
 * a potem kolejno wielomiany q[k - 1], q[k - 2], …, q[0] i umieszcza na stosie wynik operacji złożenia.
 * @param[in] s: stos
 * @param[in] line: numer wiersza.
 * @param[in] line_length: długość wiersza.
 * @param[in] curr_line: wczytany wiersz.
 */
static void InstructionCompose(Stack *s, size_t line, size_t line_length, char *curr_line) {
    if (line_length < 9) {
        fprintf(stderr, "ERROR %zu COMPOSE WRONG PARAMETER\n", line);
        return;
    }

    if (!isspace(curr_line[7])) {
        fprintf(stderr, "ERROR %zu WRONG COMMAND\n", line);
        return;
    }


    if (curr_line[7] != ' ' || (!isdigit(curr_line[8]))) {
        fprintf(stderr, "ERROR %zu COMPOSE WRONG PARAMETER\n", line);
        return;
    }

    errno = 0;
    char *ptr = curr_line + 8;
    char *endPtr = NULL;
    size_t at = strtoull(ptr, &endPtr, 10);
    if (errno == ERANGE) {
        fprintf(stderr, "ERROR %zu COMPOSE WRONG PARAMETER\n", line);
        return;
    }

    if (curr_line[line_length - 1] == '\n' && *endPtr != '\n') {
        fprintf(stderr, "ERROR %zu COMPOSE WRONG PARAMETER\n", line);
        return;
    } else if (curr_line[line_length - 1] != '\n' && *endPtr != '\0') {
        fprintf(stderr, "ERROR %zu COMPOSE WRONG PARAMETER\n", line);
        return;
    }
    if (s->size - 1 < at) {
        fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line);
        return;
    }
    Poly* q = (Poly*) SafeMalloc (sizeof(Poly) * at);
    Poly p = StackTop(s);
    StackPop(s);

    for (size_t i = 0; i < at; i++) {
        q[at - 1 - i] = StackTop(s);
        StackPop(s);
    }

    Poly r = PolyCompose(&p, at, q);
    StackAdd(s, r);

    for (size_t i =0;i<at;i++) {
        PolyDestroy(&q[i]);
    }
    free(q);
    PolyDestroy(&p);
}



void LineInterpreter(char *curr_line, size_t line, size_t line_length, Stack *s) {

    if (curr_line[0] == '#' || curr_line[0] == '\n')
        return;

    //To słuzy do sprawdzania, czy w poleceniu występuje więcej niż jeden null character.
    if (strlen(curr_line) != line_length) {
        if (isalpha(curr_line[0])) {
            if (strncmp(curr_line, "AT", 2) == 0 && isspace(curr_line[2])) {
                fprintf(stderr, "ERROR %zu AT WRONG VALUE\n", line);
                return;
            }
            else if (strncmp(curr_line, "DEG_BY", 6) == 0 && isspace(curr_line[6])) {
                fprintf(stderr, "ERROR %zu DEG BY WRONG VARIABLE\n", line);
                return;
            }
            else if (strncmp(curr_line, "COMPOSE", 7) == 0 && isspace(curr_line[7])) {
                fprintf(stderr, "ERROR %zu COMPOSE WRONG PARAMETER\n", line);
                return;
            }
            else {
                fprintf(stderr, "ERROR %zu WRONG COMMAND\n", line);
                return;
            }
        } else {
            fprintf(stderr, "ERROR %zu WRONG POLY\n", line);
            return;
        }
    }

    if (strcmp(curr_line, "ZERO") == 0 || strcmp(curr_line, "ZERO\n") == 0) {
        StackAdd(s, PolyZero());
    }

    else if (strcmp(curr_line, "IS_COEFF") == 0 || strcmp(curr_line, "IS_COEFF\n") == 0)
        InstructionIsCoeff(s, line);

    else if (strcmp(curr_line, "IS_ZERO") == 0 || strcmp(curr_line, "IS_ZERO\n") == 0)
        InstructionIsZero(s, line);

    else if (strcmp(curr_line, "CLONE") == 0 || strcmp(curr_line, "CLONE\n") == 0)
        InstructionClone(s, line);

    else if (strcmp(curr_line, "ADD") == 0 || strcmp(curr_line, "ADD\n") == 0)
        InstructionAdd(s, line);

    else if (strcmp(curr_line, "MUL") == 0 || strcmp(curr_line, "MUL\n") == 0)
        InstructionMul(s, line);

    else if (strcmp(curr_line, "NEG") == 0 || strcmp(curr_line, "NEG\n") == 0)
        InstructionNeg(s, line);

    else if (strcmp(curr_line, "SUB") == 0 || strcmp(curr_line, "SUB\n") == 0)
        InstructionSub(s, line);

    else if (strcmp(curr_line, "IS_EQ") == 0 || strcmp(curr_line, "IS_EQ\n") == 0)
        InstructionIsEq(s, line);

    else if (strcmp(curr_line, "DEG") == 0 || strcmp(curr_line, "DEG\n") == 0)
        InstructionDeg(s, line);

    else if (strcmp(curr_line, "PRINT") == 0 || strcmp(curr_line, "PRINT\n") == 0)
        InstructionPrint(s, line);

    else if (strcmp(curr_line, "POP") == 0 || strcmp(curr_line, "POP\n") == 0)
        InstructionPop(s, line);

    else if (strncmp(curr_line, "DEG_BY", 6) == 0)
        InstructionDegBy(s, line, line_length, curr_line);

    else if (strncmp(curr_line, "AT", 2) == 0)
        InstructionAt(s, line, line_length, curr_line);

    else if(strncmp(curr_line, "COMPOSE", 7) == 0)
        InstructionCompose(s, line, line_length, curr_line);

    else if (isalpha(curr_line[0])) {
        fprintf(stderr, "ERROR %zu WRONG COMMAND\n", line);
        return;
    }

    else {
        if (!IsCorrect(curr_line)) {
            fprintf(stderr, "ERROR %zu WRONG POLY\n", line);
            return;
        }

        bool correct = true;
        char *endPtr = NULL;
        Poly p = ParsePoly(curr_line, &correct, &endPtr);
        if (!correct) {
            PolyDestroy(&p);
            fprintf(stderr, "ERROR %zu WRONG POLY\n", line);
            return;
        }
        StackAdd(s, p);
    }
}

