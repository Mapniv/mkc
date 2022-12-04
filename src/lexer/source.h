#ifndef _LEXER_BUF_H_
#define _LEXER_BUF_H_

#include <stdio.h> /* for FILE */

#include "lexer.h"

struct source_info *source_create_info(void);
void source_push(struct source_info *source_info, FILE *fd);
void source_pop(struct source_info *source_info);
char source_get(struct source_info *source_info);
void source_next(struct source_info *source_info);
size_t source_line(struct source_info *source_info);
size_t source_column(struct source_info *source_info);

#endif
