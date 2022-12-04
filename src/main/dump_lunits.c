#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* For strerror */

#include <common/exitcodes.h>
#include <common/logger.h>
#include <common/status.h>
#include <lexer/lexer.h>
#include <lexer/source.h>

#include "options.h"

#include "dump_lunits.h"

static struct options *register_options(void);

static FILE *get_output_stream(struct options *opts);

static FILE *get_input_stream(struct options *opts);

static void log_lunit(struct log *log, struct lunit *lunit);

static void log_token(struct log *log, struct lunit *lunit);

static void log_lexme(struct log *log, struct lunit *lunit);

static void log_line(struct log *log, struct lunit *lunit);

static void log_column(struct log *log, struct lunit *lunit);

static void log_length(struct log *log, struct lunit *lunit);

static char *token_to_str(enum token token);



void dump_lunits(int argc, char **argv)
{
    struct options *opts;
    FILE *out;
    FILE *in;
    struct source_info *source_info;
    struct log *log;

    opts = register_options();

    /*
      0 - program name
      1 - subcommand (dump)
      2 - mode (lunits)
      3 - first option
      Skip program name, subcommand and mode
    */
    opt_parse(opts, argc - 3, &argv[3]);

    out = get_output_stream(opts);
    in = get_input_stream(opts);

    source_info = source_create_info();
    source_push(source_info, in);

    log = logger_create();

    while (true)
    {
        struct lunit *lunit;
        bool finish;

        /* Get one lunit from lexer */
        lunit = lunit_get(source_info);

        log_lunit(log, lunit);

        /* We have to destroy lunit before quitting so save this state */
        if (lunit->token == TOK_EOF)
            finish = true;

        lunit_destroy(lunit);

        if (finish == true) break;
    }

    logger_print(log, out);
    logger_destroy(log);

    source_pop(source_info);
    free(source_info);
}

static struct options *register_options(void)
{
    struct options *opts;
    struct option_info *info;

    opts = opt_create_struct();

    info = opt_create_info(true);
    opt_add_long(info, "output");
    opt_add_long(info, "output-file");
    opt_add_short(info, 'o');
    opt_register(opts, info);

    return opts;
}


static FILE *get_output_stream(struct options *opts)
{
    char *file_name;
    struct option_info *info;
    FILE *fd;

    info = opt_find_long(opts, "output");
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

static FILE *get_input_stream(struct options *opts)
{
    char *file_name;
    FILE *fd;

    if (opts->parameter_count == 0)
    {
        fputs("No input files", stderr);
        exit(EXITCODE_INVOCATION_ERROR);
    }

    if (opts->parameter_count > 1)
    {
        fputs("Expected one input file, got more\n", stderr);
        exit(EXITCODE_INVOCATION_ERROR);
    }

    file_name = opts->parameters[0];

    fd = fopen(file_name, "r");

    if (fd == NULL)
    {
        fprintf(stderr, "Failed to open file '%s' for writing: %s\n",
            file_name, strerror(errno));
        exit(EXITCODE_INTERNAL_ERROR);
    }

    return fd;
}

static void log_lunit(struct log *log, struct lunit *lunit)
{
    log_token(log, lunit);
    log_lexme(log, lunit);
    log_line(log, lunit);
    log_column(log, lunit);
    log_length(log, lunit);
    logger_append_string(log, "\n");
}

static void log_token(struct log *log, struct lunit *lunit)
{
    char *token_str;

    token_str = token_to_str(lunit->token);
    logger_append_string(log, "Token: ");
    logger_append_string(log, token_str);
    logger_append_string(log, "\n");
}

static void log_lexme(struct log *log, struct lunit *lunit)
{
    enum token t;

    t = lunit->token;

    if (t != TOK_EOL && t != TOK_EOF && t != TOK_TAB)
    {
        logger_append_string(log, "Lexme: ");
        /* Lexme isn't null terminated!!! */
        logger_append_sequence(log, lunit->lexme, lunit->length);
        logger_append_string(log, "\n");
    }
}

static void log_line(struct log *log, struct lunit *lunit)
{
    logger_append_string(log, "Line: ");
    logger_append_size(log, lunit->line);
    logger_append_string(log, "\n");
}

static void log_column(struct log *log, struct lunit *lunit)
{
    logger_append_string(log, "Column: ");
    logger_append_size(log, lunit->column);
    logger_append_string(log, "\n");
}

static void log_length(struct log *log, struct lunit *lunit)
{
    logger_append_string(log, "Length: ");
    logger_append_size(log, lunit->length);
    logger_append_string(log, "\n");
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
