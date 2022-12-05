#ifndef _LEXER_BUF_H_
#define _LEXER_BUF_H_

#include <stdio.h> /* for FILE */

struct sources;

struct sources *source_create_struct(void);
void source_push(struct sources *sources, FILE *fd);
void source_pop(struct sources *sources);
char source_get(struct sources *sources);
void source_next(struct sources *sources);
size_t source_line(struct sources *sources);
size_t source_column(struct sources *sources);

#endif
