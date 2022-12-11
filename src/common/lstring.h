#ifndef _COMMON_LOGGER_H_
#define _COMMON_LOGGER_H_

#include <stdio.h>

struct lstring
{
    char *text; /* not null terminated */
    size_t length;
};

struct lstring *lstring_create(void);
void lstring_destroy(struct lstring *lstring);
void lstring_append_char(struct lstring *lstring, char c);
void lstring_append_string(struct lstring *lstring, char *str);
void lstring_append_lstring(struct lstring *dest, struct lstring *src);
void lstring_append_size(struct lstring *lstring, size_t size);
void lstring_reverse(struct lstring *lstring);
void lstring_print(struct lstring *lstring, FILE *fd);

#endif
