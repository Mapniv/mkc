#ifndef _LEXER_LEXER_H_
#define _LEXER_LEXER_H_

#include <inttypes.h>

#include "lunit.h"
#include "source.h"



struct lunit *lunit_get(struct sources *sources);
void lunit_destroy(struct lunit *lunit);

#endif

