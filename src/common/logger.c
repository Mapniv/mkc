#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <convert/to_string.h>

#include "guard.h"

#include "logger.h"

struct log
{
    char *text; /* not null terminated */
    size_t length;
};

struct log *logger_create(void)
{
    struct log *log;

    log = malloc(sizeof(struct log));

    /*
      While failure to append text to a log isn't a critical failure
      but failure to allocate memory is a fatal error
    */
    GUARD(log)

    log->length = 0;
    /* We must set to NULL when we call realloc for the first time */
    log->text = NULL;

    return log;
}

void logger_destroy(struct log *log)
{
    /*
      It is unusual to create logger and then
      just free it without appending anything
      but it's better to be safe than sorry
    */
    if (log->text != NULL)
        free(log->text);
    free(log);
}

void logger_append_string(struct log *log, char *string)
{
    size_t string_length;
    size_t new_size;
    char *new_text;

    string_length = strlen(string);

    new_size = log->length + string_length;
    new_text = realloc(log->text, new_size);

    GUARD(new_text)

    log->text = new_text;

    /*
      Append string to log->text, space is allocated already
      If we use length of an array as its subscript it's going to
      point one byte after its end
      This is where we want to append this string
      & operator makes a pointer out of this position
      We use strncpy because log->text isn't null terminated
    */
    strncpy(&log->text[log->length], string, string_length);
    log->length += string_length;
}

void logger_append_sequence(struct log *log, char *sequence,
    size_t length)
{
    size_t new_size;
    char *new_text;

    new_size = log->length + length;
    new_text = realloc(log->text, new_size);

    GUARD(new_text)

    log->text = new_text;

    /*
      Append string to log->text, space is allocated already
      If we use length of an array as its subscript it's going to
      point one byte after its end
      This is where we want to append this string
      & operator makes a pointer out of this position
      We use strncpy because log->text isn't null terminated
    */
    strncpy(&log->text[log->length], sequence, length);
    log->length += length;
}

void logger_append_size(struct log *log, size_t size)
{
    char *size_as_str;

    size_as_str = size_to_string(size);

    logger_append_string(log, size_as_str);

    free(size_as_str);
}

void logger_print(struct log *log, FILE *fd)
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
    fwrite(log->text, log->length, 1, fd);
}
