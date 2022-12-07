#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <convert/to_string.h>

#include "guard.h"

#include "lstring.h"


struct lstring *lstring_create(void)
{
    struct lstring *lstring;

    lstring = malloc(sizeof(struct lstring));

    /*
      While failure to append text to a lstring isn't a critical failure
      but failure to allocate memory is a fatal error
    */
    GUARD(lstring)

    lstring->length = 0;
    /* We must set it to NULL because we pass it to realloc */
    lstring->text = NULL;

    return lstring;
}

void lstring_destroy(struct lstring *lstring)
{
    /*
      It is unusual to create lstring and then
      just free it without appending anything
      but it's better to be safe than sorry
    */
    if (lstring->text != NULL)
        free(lstring->text);
    free(lstring);
}

void lstring_append_char(struct lstring *lstring, char c)
{
    size_t new_size;
    char *new_text;

    /*
      We need to extend lstring->text by one byte
      realloc when supplied with NULL as first
      argument works just like malloc
    */
    new_size = lstring->length + 1;
    new_text = realloc(lstring->text, new_size);

    GUARD(new_text)

    lstring->text = new_text;
    /*
      Append string to lstring->text, space is allocated already
      If we use length of an array as its subscript it's going to
      point one byte after its end
      This is where we want to insert this char
    */
    lstring->text[lstring->length] = c;
    lstring->length += 1;
}

void lstring_append_string(struct lstring *lstring, char *string)
{
    size_t string_length;
    size_t new_size;
    char *new_text;

    string_length = strlen(string);

    /*
      Reallocate buffer to accomodate new string
      When you invoke realloc with NULL as first
      argument it works like malloc
    */
    new_size = lstring->length + string_length;
    new_text = realloc(lstring->text, new_size);

    GUARD(new_text)

    lstring->text = new_text;

    /*
      Append string to lstring->text, space is allocated already
      If we use length of an array as its subscript it's going to
      point one byte after its end
      This is where we want to append this string
      & operator makes a pointer out of this position
      We use strncpy because log->text isn't null terminated
    */
    strncpy(&lstring->text[lstring->length], string, string_length);
    lstring->length += string_length;
}

void lstring_append_lstring(struct lstring *dest, struct lstring *src)
{
    size_t new_size;
    char *new_text;

    /*
      Reallocate buffer to accomodate new string
      When you invoke realloc with NULL as first
      argument it works like malloc
    */
    new_size = dest->length + src->length;
    new_text = realloc(dest->text, new_size);

    GUARD(new_text)

    dest->text = new_text;

    /*
      Append string to dest->text, space is allocated already
      If we use length of an array as its subscript it's going to
      point one byte after its end
      This is where we want to append this string
      & operator makes a pointer out of this position
      We use strncpy because log->text isn't null terminated
    */
    strncpy(&dest->text[dest->length], src->text, src->length);
    dest->length += src->length;
}

void lstring_append_size(struct lstring *lstring, size_t size)
{
    char *size_as_str;

    size_as_str = size_to_string(size);

    lstring_append_string(lstring, size_as_str);

    free(size_as_str);
}

void lstring_print(struct lstring *lstring, FILE *fd)
{
    /*
      Write contents of the log all at once, not byte by byte
      It prevents following situation:
        we write a byte
        someone else writes a byte
        we write a byte
        and so on
      It is common to run compiler four times at once (eg. make -j4)
      Again, we do not want to end up with garbage on the screen
    */
    fwrite(lstring->text, lstring->length, 1, fd);
}