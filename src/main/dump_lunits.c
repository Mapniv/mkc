#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* For strerror */

#include <common/exitcodes.h>
#include <common/lstring.h>
#include <common/status.h>
#include <lexer/lexer.h>
#include <lexer/source.h>

#include "arguments.h"

#include "dump_lunits.h"

static struct arguments *register_options(void);

static FILE *get_output_stream(struct arguments *args);

static FILE *get_input_stream(struct arguments *args);

static void log_lunit(FILE *fd, struct lunit *lunit);

static void log_token(struct lstring *log, struct lunit *lunit);

static void log_lexme(struct lstring *log, struct lunit *lunit);

static void log_line(struct lstring *log, struct lunit *lunit);

static void log_column(struct lstring *log, struct lunit *lunit);

static void log_length(struct lstring *log, struct lunit *lunit);

static char *token_to_str(enum token token);



void dump_lunits(int argc, char **argv)
{
    struct arguments *args;
    FILE *out;
    FILE *in;
    struct sources *sources;

    args = register_options();

    /*
      0 - program name
      1 - subcommand (dump)
      2 - mode (lunits)
      3 - first option
      Skip program name, subcommand and mode
    */
    arg_parse(args, argc - 3, &argv[3]);

    out = get_output_stream(args);
    in = get_input_stream(args);

    arg_destroy_struct(args);

    sources = source_create_struct();
    source_push(sources, in);

    while (true)
    {
        struct lunit *lunit;
        bool finish;

        finish = false;

        /* Get one lunit from lexer */
        lunit = lunit_get(sources);

        log_lunit(out, lunit);

        /* We have to destroy lunit before quitting so save this state */
        if (lunit->token == TOK_EOF)
            finish = true;

        lunit_destroy(lunit);

        if (finish == true) break;
    }

    source_pop(sources);
    free(sources);

    fclose(in);
    fclose(out);
}

static struct arguments *register_options(void)
{
    struct arguments *args;
    struct switch_info *info;

    args = arg_create_struct();

    info = arg_create_switch_info(true);
    arg_add_long(info, "output");
    arg_add_long(info, "output-file");
    arg_add_short(info, 'o');
    arg_register(args, info);

    return args;
}


static FILE *get_output_stream(struct arguments *args)
{
    char *file_name;
    struct switch_info *info;
    FILE *fd;

    info = arg_find_long(args, "output");
    assert(info != NULL);

    if (info->occurrences > 1)
    {
        fprintf(stderr, "Expected up to one occurrence of option output, "
            "got more\n");
        printf("%d\n", info->occurrences);
        exit(EXITCODE_INVOCATION_ERROR);
    }

    /* Output stream defaults to stdout */
    if (info->occurrences == 0)
        return stdout;

    file_name = info->parameters[0];

    fd = fopen(file_name, "w");

    if (fd == NULL)
    {
        fprintf(stderr, "Failed to open file '%s' for reading: %s\n",
            file_name, strerror(errno));
        exit(EXITCODE_INTERNAL_ERROR);
    }

    return fd;
}

static FILE *get_input_stream(struct arguments *args)
{
    char *file_name;
    FILE *fd;

    if (args->parameter_count == 0)
    {
        fputs("No input files", stderr);
        exit(EXITCODE_INVOCATION_ERROR);
    }

    if (args->parameter_count > 1)
    {
        fputs("Expected one input file, got more\n", stderr);
        exit(EXITCODE_INVOCATION_ERROR);
    }

    file_name = args->parameters[0];

    fd = fopen(file_name, "r");

    if (fd == NULL)
    {
        fprintf(stderr, "Failed to open file '%s' for writing: %s\n",
            file_name, strerror(errno));
        exit(EXITCODE_INTERNAL_ERROR);
    }

    return fd;
}

static void log_lunit(FILE *fd, struct lunit *lunit)
{
    struct lstring *log;

    log = lstring_create();

    log_token(log, lunit);
    log_lexme(log, lunit);
    log_line(log, lunit);
    log_column(log, lunit);
    log_length(log, lunit);
    lstring_append_string(log, "\n");

    lstring_print(log, fd);
    lstring_destroy(log);
}

static void log_token(struct lstring *log, struct lunit *lunit)
{
    char *token_str;

    token_str = token_to_str(lunit->token);
    lstring_append_string(log, "Token: ");
    lstring_append_string(log, token_str);
    lstring_append_string(log, "\n");
}

static void log_lexme(struct lstring *log, struct lunit *lunit)
{
    enum token t;

    t = lunit->token;

    if (t != TOK_EOL && t != TOK_EOF && t != TOK_TAB)
    {
        lstring_append_string(log, "Lexme: ");
        lstring_append_lstring(log, lunit->lexme);
        lstring_append_string(log, "\n");
    }
}

static void log_line(struct lstring *log, struct lunit *lunit)
{
    lstring_append_string(log, "Line: ");
    lstring_append_size(log, lunit->line);
    lstring_append_string(log, "\n");
}

static void log_column(struct lstring *log, struct lunit *lunit)
{
    lstring_append_string(log, "Column: ");
    lstring_append_size(log, lunit->column);
    lstring_append_string(log, "\n");
}

static void log_length(struct lstring *log, struct lunit *lunit)
{
    lstring_append_string(log, "Length: ");
    lstring_append_size(log, lunit->lexme->length);
    lstring_append_string(log, "\n");
}


#define CASE(tok) case tok: return #tok;

static char *token_to_str(enum token token)
{
    switch (token)
    {
        CASE(TOK_PROCEDURE)
        CASE(TOK_RETURN)
        CASE(TOK_IDENTIFIER)
        CASE(TOK_INTEGER)
        CASE(TOK_TAB)
        CASE(TOK_EOL)
        CASE(TOK_EOF)
        CASE(TOK_UNKNOWN)
    }
}
