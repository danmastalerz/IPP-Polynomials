/** @file
  Implementacja klasy wielomianów rzadkich wielu zmiennych.

  @authors Daniel Mastalerz
  @copyright Uniwersytet Warszawski
  @date 2021
*/

#include <stdlib.h>
#include "poly.h"
#include "memory.h"

/**
 * Podnosi wielomian do zadanej potęgi i go zwraca.
 * @param[in] q : wielomian
 * @param[in] pow : potęga, do jakiej podnoszony będzie wielomian
 * @return wielomian podniesiony do zadanej potęgi
 */
static Poly PolyPower(const Poly* q, size_t pow) {

    if (pow == 1)
        return PolyClone(q);
    if (pow == 0)
        return PolyFromCoeff(1);
    if (pow % 2 == 0) {
        Poly temp = PolyPower(q, pow/2);
        Poly temp2 = PolyMul(&temp, &temp);
        PolyDestroy(&temp);
        return temp2;
    }
    else {
        Poly temp = PolyPower(q, (pow-1)/2);
        Poly temp2 = PolyMul(&temp, &temp);
        Poly temp3 = PolyMul(q, &temp2);
        PolyDestroy(&temp);
        PolyDestroy(&temp2);
        return temp3;
    }
}



/**
 * Składa wielomian dany z wielomianami danymi w tablicy i zwraca wynik operacji złożenia.
 * @param[in] p : wielomian
 * @param[in] k : liczba wielomianów w tablicy
 * @param[in] q : tablica wielomianów
 * @return wynik operacji złożenia
 */
Poly PolyCompose(const Poly *p, size_t k, const Poly q[]) {

    if (PolyIsCoeff(p)) {
        return PolyClone(p);
    }
    Poly _q;
    size_t _k;
    if (k == 0) {
        _q = PolyZero();
        _k = 0;
    }
    else {
        _q = *q;
        _k = k - 1;
    }


    Poly res = PolyZero();
    for (size_t i = 0; i < p->size; i++) {
        Poly temp = PolyPower(&_q, p->arr[i].exp);
        Poly temp2 = PolyCompose(&p->arr[i].p, _k, q+1);
        Poly temp3 = PolyMul(&temp, &temp2);
        Poly temp4 = res;
        res = PolyAdd(&temp3,&temp4);
		PolyDestroy(&temp2);
        PolyDestroy(&temp3);
        PolyDestroy(&temp);
        PolyDestroy(&temp4);
    }

    return res;

}

void PolyDestroy(Poly *p) {
    assert(p != NULL);
    if (p->arr == NULL) return;
    for (size_t i = 0; i < p->size; i++) {
        PolyDestroy(&p->arr[i].p);
    }
    free(p->arr);
}

Poly PolyClone(const Poly *p) {
    assert(p != NULL);
    if (PolyIsCoeff(p)) {
        return (Poly) {.coeff = p->coeff, .arr = NULL};
    }
    Mono *arr = (Mono *) SafeMalloc((p->size) * sizeof(Mono));
    for (size_t i = 0; i < p->size; i++) {
        arr[i] = MonoClone(&p->arr[i]);
    }
    return (Poly) {.size = p->size, .arr = arr};
}

/**
 * Dodaje skalar do wielomianu.
 * @param[in] q : wielomian @f$q@f$
 * @param[in] scalar : skalar @f$scalar@f$
 * @return @f$p + scalar@f$
 */
static Poly PolyAddCoeff(const Poly *q, poly_coeff_t scalar) {
    assert(q != NULL);
    if (PolyIsCoeff(q)) return PolyFromCoeff(scalar + q->coeff);
    //Tutaj dwa przypadki w zależności od tego, czy w wielomianie jest już jakiś niezerowy skalar.
    //Korzystamy z tego, że stworzone wielomiany mają posortowane jednomiany względem wykładnika.
    if (q->arr[0].exp == 0) {
        Poly coeff = PolyAddCoeff(&q->arr[0].p, scalar);
        if (PolyIsZero(&coeff)) {
            Mono *_arr = (Mono *) SafeMalloc((q->size - 1) * sizeof(Mono));
            for (size_t i = 0; i < q->size - 1; i++) {
                _arr[i] = MonoClone(&q->arr[i + 1]);
            }
            return (Poly) {.size = q->size - 1, .arr = _arr};
        }

        Mono *_arr = (Mono *) SafeMalloc((q->size) * sizeof(Mono));
        for (size_t i = 1; i < q->size; i++) {
            _arr[i] = MonoClone(&q->arr[i]);
        }
        _arr[0].exp = 0;
        _arr[0].p = coeff;

        return (Poly) {.size = q->size, .arr = _arr};
    }

    Mono *_arr = (Mono *) SafeMalloc((q->size + 1) * sizeof(Mono));
    _arr[0].p = PolyFromCoeff(scalar);
    _arr[0].exp = 0;

    for (size_t i = 0; i < q->size; i++) {
        _arr[i + 1] = MonoClone(&q->arr[i]);
    }

    return (Poly) {.size = q->size + 1, .arr = _arr};
}

Poly PolyAdd(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);

    if (PolyIsZero(p))
        return PolyClone(q);

    if (PolyIsZero(q))
        return PolyClone(p);

    if (PolyIsCoeff(p) && PolyIsCoeff(q))
        return PolyFromCoeff(p->coeff + q->coeff);

    if (PolyIsCoeff(p))
        return PolyAddCoeff(q, p->coeff);

    if (PolyIsCoeff(q))
        return PolyAddCoeff(p, q->coeff);

    size_t size = 0;

    Mono *arr = (Mono *) SafeMalloc((p->size + q->size) * sizeof(Mono));
    size_t i = 0, j = 0;
    Poly temp;
    //Korzystamy z tego, że jednomiany są posortowane względem wykładnika i przesuwamy odpowiednie indeksy.
    while (i < p->size && j < q->size) {
        if (p->arr[i].exp < q->arr[j].exp) {
            arr[size] = MonoClone(&p->arr[i]);
            size++;
            i++;
        } else if (p->arr[i].exp > q->arr[j].exp) {
            arr[size] = MonoClone(&q->arr[j]);
            size++;
            j++;
        } else {
            temp = PolyAdd(&p->arr[i].p, &q->arr[j].p);
            if (PolyIsZero(&temp)) {
                i++;
                j++;
            } else {
                bool added = false;
                if (temp.arr != NULL) {
                    if (temp.size == 1 && PolyIsZero(&temp.arr[0].p)) {
                        PolyDestroy(&temp);
                        added = true;
                    }
                }

                if (!added) {
                    arr[size].exp = p->arr[i].exp;
                    arr[size].p = temp;
                    size++;
                }

                i++;
                j++;
            }
        }
    }
    size_t ii = i;
    size_t jj = j;

    if (ii == p->size) {
        while (j < q->size) {
            arr[size] = MonoClone(&q->arr[j]);
            size++;
            j++;
        }
    } else if (jj == q->size) {
        while (i < p->size) {
            arr[size] = MonoClone(&p->arr[i]);
            size++;
            i++;
        }
    }

    if (size == 0) {
        free(arr);
        return PolyZero();
    }
    //Jeśli powstały wielomian jest współczynnikiem, ale wygenerowało się tak, że ma jeden jednomian
    //który jest wspóczynnikiem, to zamieniamy to na wielomian o pustej tablicy i danym wspóczynniku.
    if (size == 1 && PolyIsCoeff(&arr[0].p) && arr[0].exp == 0) {
        poly_coeff_t c = arr[0].p.coeff;
        free(arr);
        return PolyFromCoeff(c);
    } else return (Poly) {.size = size, .arr = arr};
}

bool PolyIsEq(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);

    if (p->arr == NULL && q->arr == NULL)
        return p->coeff == q->coeff;


    if (p->arr == NULL || q->arr == NULL)
        return false;

    if (p->size != q->size)
        return false;

    for (size_t i = 0; i < p->size; i++) {
        if (p->arr[i].exp != q->arr[i].exp) return false;
        if (!PolyIsEq(&p->arr[i].p, &q->arr[i].p)) return false;
    }

    return true;

}

/**
 * Komparator dla jednomianów ze względu na wykładnik.
 * @param[in] a : jednomian @f$p@f$
 * @param[in] b : jednomian @f$q@f$
 * @return liczba dodatnia jeśli wykładnik w pierwszym jednomianie większy, zero jeśli równe, liczba ujemna w przeciwnym przypadku
 */
static int MonoCmp(const void *a, const void *b) {
    return ((Mono *) a)->exp - ((Mono *) b)->exp;
}

Poly PolyAddMonos(size_t count, const Mono monos[]) {
    if (count == 0)
        return PolyZero();

    Mono *arr = (Mono *) SafeMalloc(count * sizeof(Mono));
    for (size_t i = 0; i < count; i++) {
        arr[i] = monos[i];
    }

    //Zwracany wielomian będzie posortowany względem wykładników jednomianów.
    qsort(arr, count, sizeof(Mono), MonoCmp);

    for (size_t i = 0; i < count - 1; i++) {
        if (arr[i].exp == arr[i + 1].exp) {
            Poly temp = arr[i + 1].p;
            arr[i + 1].p = PolyAdd(&arr[i + 1].p, &arr[i].p);
            PolyDestroy(&temp);
            PolyDestroy(&arr[i].p);
            arr[i].p = PolyZero();
        }
    }
    size_t counter = 0;

    for (size_t i = 0; i < count; i++) {
        if (!PolyIsZero(&arr[i].p))
            counter++;
    }

    if (counter == 0) {
        free(arr);
        return PolyZero();
    }

    size_t j = 0;
    for (size_t i = 0; i < count; i++) {
        if (!PolyIsZero(&arr[i].p)) {
            arr[j] = arr[i];
            j++;
        }
    }

    if (counter == 1 && arr[0].exp == 0 && PolyIsCoeff(&arr[0].p)) {
        poly_coeff_t temp = arr[0].p.coeff;
        free(arr);
        return PolyFromCoeff(temp);

    }

    return (Poly) {.size = counter, .arr = arr};

}

Poly PolyOwnMonos(size_t count, Mono *monos) {
    Poly p = PolyAddMonos(count, monos);
    free(monos);
    return p;
}

Poly PolyCloneMonos(size_t count, const Mono monos[]) {
    Mono* _monos = (Mono*) SafeMalloc (sizeof(Mono) * count);
    for (size_t i = 0; i < count; i++) {
        _monos[i] = MonoClone(monos+i);
    }
    Poly p = PolyAddMonos(count, _monos);
    free(_monos);
    return p;
}

/**
 * Mnoży wielomian przez skalar.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] scalar : skalar @f$scalar@f$
 * @return @f$p * scalar@f$
 */
static Poly PolyMulScalar(const Poly *p, long scalar) {
    assert(p != NULL);

    if (scalar == 0) {
        return PolyZero();
    }

    if (PolyIsZero(p))
        return PolyZero();

    if (PolyIsCoeff(p)) {
        return PolyFromCoeff(scalar * p->coeff);
    }

    Mono *arr = (Mono *) SafeMalloc((p->size) * sizeof(Mono));
    for (size_t i = 0; i < p->size; i++) {
        arr[i].p = PolyMulScalar(&p->arr[i].p, scalar);
        arr[i].exp = p->arr[i].exp;
    }
    Poly result = PolyAddMonos(p->size, arr);
    free(arr);
    return result;

}

Poly PolyMul(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);

    if (PolyIsZero(p) || PolyIsZero(q))
        return PolyZero();

    if (PolyIsCoeff(p) && PolyIsCoeff(q))
        return PolyFromCoeff(p->coeff * q->coeff);

    if (PolyIsCoeff(p))
        return PolyMulScalar(q, p->coeff);

    if (PolyIsCoeff(q))
        return PolyMulScalar(p, q->coeff);


    int size = 0;
    Mono *arr = (Mono *) SafeMalloc((p->size * q->size) * sizeof(Mono));
    for (size_t i = 0; i < p->size; i++) {
        for (size_t j = 0; j < q->size; j++) {
            arr[size].exp = p->arr[i].exp + q->arr[j].exp;
            arr[size].p = PolyMul(&p->arr[i].p, &q->arr[j].p);
            size++;
        }
    }
    Poly temp = PolyAddMonos(size, arr);
    free(arr);
    return temp;

}

Poly PolyNeg(const Poly *p) {
    assert(p != NULL);

    if (PolyIsCoeff(p))
        return (Poly) {.coeff = -p->coeff, .arr = NULL};

    Mono *arr = (Mono *) SafeMalloc((p->size) * sizeof(Mono));
    for (size_t i = 0; i < p->size; i++) {
        arr[i].p = PolyNeg(&p->arr[i].p);
        arr[i].exp = p->arr[i].exp;
    }
    return (Poly) {.size = p->size, .arr = arr};
}

Poly PolySub(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);
    Poly temp = PolyNeg(q);
    Poly to_return = PolyAdd(p, &temp);
    PolyDestroy(&temp);
    return to_return;
}

/**
 * Zwraca większą z dwóch liczb
 * @param[in] a : liczba @f$a@f$
 * @param[in] b : liczba @f$b@f$
 * @return większa z @f$a, b@f$
 */
static int max(int a, int b) {
    if (a > b)
        return a;
    return b;
}

poly_exp_t PolyDegBy(const Poly *p, size_t var_idx) {
    assert(p != NULL);
    if (var_idx == 0) {
        if (PolyIsZero(p))
            return -1;
        if (PolyIsCoeff(p))
            return 0;
        return p->arr[p->size - 1].exp;
    }
    poly_exp_t deg = -2;

    if (PolyIsZero(p))
        return -1;

    if (PolyIsCoeff(p))
        return 0;

    for (size_t i = 0; i < p->size; i++) {
        deg = max(deg, PolyDegBy(&p->arr[i].p, var_idx - 1));
    }
    return deg;
}

poly_exp_t PolyDeg(const Poly *p) {
    assert(p != NULL);
    if (PolyIsZero(p)) return -1;
    if (PolyIsCoeff(p)) return 0;
    poly_exp_t deg = -2;
    for (size_t i = 0; i < p->size; i++) {
        deg = max(deg, p->arr[i].exp + PolyDeg(&p->arr[i].p));
    }
    return deg;

}

/**
 * Podnosi liczbę całkowitą do potęgi.
 * @param[in] a : liczba @f$p@f$
 * @param[in] n : liczba @f$q@f$
 * @return pierwsza z liczb do potęgi drugiej z liczb
 */
static long power(long a, long n) {
    long acc = 1;
    while (n > 0) {
        if (n % 2 == 1) {
            acc *= a;
        }
        a *= a;
        n /= 2;
    }
    return acc;
}

Poly PolyAt(const Poly *p, poly_coeff_t x) {
    if (PolyIsCoeff(p)) return PolyFromCoeff(p->coeff);
    Poly temp = PolyZero();
    for (size_t i = 0; i < p->size; i++) {
        Poly temp2 = temp;
        Poly temp3 = PolyMulScalar(&p->arr[i].p, power(x, p->arr[i].exp));
        temp = PolyAdd(&temp2, &temp3);
        PolyDestroy(&temp2);
        PolyDestroy(&temp3);
    }
    return temp;
}

