#ifndef ASSERTIONS_H_
#define ASSERTIONS_H_

#ifdef _MSC_VER

#include <stdio.h>

#define _Assert_(cOND, aCTION, fORMAT, ...)                                         \
do {                                                                                \
    if (!(cOND)) {                                                                  \
        printf("Assertion ("#cOND") failed! "fORMAT"\n", __VA_ARGS__);              \
        aCTION;                                                                     \
    }                                                                               \
} while(0)

#define _AssertNoPrint_(cOND, aCTION)                                               \
do {                                                                                \
    if (!(cOND)) {                                                                  \
        aCTION;                                                                     \
    }                                                                               \
} while(0)

#define AssertError(cOND, aCTION, fORMAT, ...)  _Assert_(cOND, aCTION, fORMAT, __VA_ARGS__)
#define AssertErrorNoPrint(cOND, aCTION)        _AssertNoPrint_(cOND, aCTION)

#define AssertBreak(cOND, aCTION, fORMAT, ...)  {AssertError(cOND, aCTION, fORMAT, __VA_ARGS__);if (!(cOND)) break;}
#define AssertBreakNoPrint(cOND, aCTION)        {AssertErrorNoPrint(cOND, aCTION);if (!(cOND)) break;}

#define AssertContinue(cOND, aCTION, fORMAT, ...)  {AssertError(cOND, aCTION, fORMAT, __VA_ARGS__);if (!(cOND)) continue;}
#define AssertContinueNoPrint(cOND, aCTION)        {AssertErrorNoPrint(cOND, aCTION);if (!(cOND)) continue;}

#else
#include "rtdbg.h"

#define _Assert_(cOND, aCTION, fORMAT, aRGS...)                                     \
do {                                                                                \
    if (!(cOND)) {                                                                  \
        LOG_E("Assertion ("#cOND") failed! "fORMAT"\n", ##aRGS);\
        aCTION;                                                                     \
    }                                                                               \
} while(0)

#define _AssertNoPrint_(cOND, aCTION)                                               \
do {                                                                                \
    if (!(cOND)) {                                                                  \
        aCTION;                                                                     \
    }                                                                               \
} while(0)

#define AssertError(cOND, aCTION, fORMAT, aRGS...)  _Assert_(cOND, aCTION, fORMAT, ##aRGS)
#define AssertErrorNoPrint(cOND, aCTION)            _AssertNoPrint_(cOND, aCTION)

#define AssertBreak(cOND, aCTION, fORMAT, aRGS...)  {AssertError(cOND, aCTION, fORMAT, ##aRGS);if (!(cOND)) break;}
#define AssertBreakNoPrint(cOND, aCTION)            {AssertErrorNoPrint(cOND, aCTION);if (!(cOND)) break;}

#define AssertContinue(cOND, aCTION, fORMAT, aRGS...)  {AssertError(cOND, aCTION, fORMAT, ##aRGS);if (!(cOND)) continue;}
#define AssertContinueNoPrint(cOND, aCTION)            {AssertErrorNoPrint(cOND, aCTION);if (!(cOND)) continue;}

#define AssertReturn(cOND, aCTION, fORMAT, aRGS...)  {AssertError(cOND, aCTION, fORMAT, ##aRGS);if (!(cOND)) aCTION;}
#define AssertReturnNoPrint(cOND, aCTION)            {AssertErrorNoPrint(cOND, aCTION);if (!(cOND)) aCTION;}

#endif
#endif /* ASSERTIONS_H_ */

