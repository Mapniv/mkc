#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/guard.h>
#include <common/status.h>
#include "options.h"


/* TODO descriebe these functions */
struct options *opt_create_struct(void);

void opt_register(struct options *opts, struct option_info *info);

void opt_parse(struct options *opts, int argc, char **argv);

struct option_info *opt_create_info(bool takes_parameters);

void opt_add_short(struct option_info *info, char c);

void opt_add_long(struct option_info *info, char *str);

struct option_info *opt_find_short(struct options *opts, char c);

struct option_info *opt_find_long(struct options *opts, char *str);

static void handle_parameter(struct options *opts,
    char **argv, int *iter_ptr);

static void handle_short_option(struct options *opts,
    int argc, char **argv, int *iter_ptr);

static void handle_long_option(struct options *opts,
    int argc, char **argv, int *iter_ptr);

static void handle_final(struct options *opts,
    int argc, char **argv, int *iter_ptr);

static struct option_info *find_short_option(struct options *opts, char c);

static struct option_info *find_long_option(struct options *opts, char *str);

static void append(struct option_info *info, char *value);

static char *copy_option_name(char *arg);



struct options *opt_create_struct(void)
{
    struct options *opts;

    opts = malloc(sizeof(struct options));

    GUARD(opts)

    /*
      These NULL values are going to be used
      in opt_register() and opt_parse()
    */
    opts->options = NULL;
    opts->parameters = NULL;
    opts->option_count = 0;
    opts->parameter_count = 0;

    return opts;
}

void opt_register(struct options *opts, struct option_info *info)
{
    size_t new_size;
    struct option_info **new_options;

    /*
      Right now we have space allocated for opts->count pointers
      We need one more
      Also if opts->options is NULL realloc works like malloc
    */
    new_size = (opts->option_count + 1) * sizeof(struct option_info *);
    new_options = realloc(opts->options, new_size);

    /*
      If we can't parse all the arguments
      what is the point of running the program?
    */
    GUARD(new_options)

    opts->options = new_options;

    /*
      In reality we have opts->option_count + 1 infos
      but we use it as an array subscript
      Think about it, count = 0, count + 1 (1) infos
      Where do we put that info? At array subscript 0 (count)!
      Think again, count = 1, count + 1 (2) infos,
      Where do we put that second info? At array subscript 1!
      Afterwards we update count of course
    */
    opts->options[opts->option_count] = info;
    opts->option_count += 1;
}

void opt_parse(struct options *opts, int argc, char **argv)
{
    int iter;

    iter = 0;

    while (iter != argc)
    {
        /* Check if argument is empty */
        if (strcmp(argv[iter], "") == 0)
        {
            fputs("Got empty argument", stderr);
            exit(EXITCODE_INVOCATION_ERROR);
        }

        /* Check if it is a parameter */
        if (strncmp(argv[iter], "-", 1) != 0)
        {
            handle_parameter(opts, argv, &iter);
            continue;
        }

        /* Check if we are dealing with lone hyphen */
        if (strcmp(argv[iter], "-") == 0)
        {
            fputs("Got lone hyphen as an argument", stderr);
            exit(EXITCODE_INVOCATION_ERROR);
        }       

        /* Check whether it is a short option */
        if (strncmp(argv[iter], "--", 2) != 0)
        {
            handle_short_option(opts, argc, argv, &iter);
            continue;
        }

        /* Check whether we are dealing with lone two dashes */
        if (strcmp(argv[iter], "--") == 0)
        {
            handle_final(opts, argc, argv, &iter);
            return; /* Yes, we return */
        }

        /* Option with no name? This is obviously invalid */
        if (strncmp(argv[iter], "--=", 3) == 0)
        {
            fputs("No option can start with '--='", stderr);
            exit(EXITCODE_INVOCATION_ERROR);
        }

        /* We are dealing with long option */
        handle_long_option(opts, argc, argv, &iter);
    }
}

struct option_info *opt_create_info(bool takes_parameter)
{
    struct option_info *info;

    info = malloc(sizeof(struct option_info));

    /* This is a fatal error if we can't parse program's arguments */
    GUARD(info)

    /*
      We are going to realloc these
      realloc(NULL, size) works like malloc(size)
    */
    info->parameters = NULL;
    info->short_options = NULL;
    info->long_options = NULL;

    info->takes_parameter = takes_parameter;

    info->short_count = 0;
    info->long_count = 0;
    info->occurrences = 0;

    return info;
}

void opt_add_short(struct option_info *info, char c)
{
    size_t new_size;
    char *new_short_options;

    /*
      Right now we have space allocated for short_count characters
      We need one more
      Also if short_options is NULL realloc works like malloc
    */
    new_size = (info->short_count + 1) * sizeof(char);
    new_short_options = realloc(info->short_options, new_size);

    /*
      If we can't parse all the options then
      what is the point of running the program?
    */
    GUARD(new_short_options)

    info->short_options = new_short_options;

    /*
      short_count is no longer valid but it can be used to
      insert new character, that is it can be used as array subscript
      Afterwards update short_count
    */
    info->short_options[info->short_count] = c;
    info->short_count += 1;
}

void opt_add_long(struct option_info *info, char *str)
{
    size_t new_size;
    char **new_long_options;

    /*
      Right now we have space allocated for long_count strings
      We need one more
      Also if long_options is NULL realloc works like malloc
    */
    new_size = (info->long_count + 1) * sizeof(char *);
    new_long_options = realloc(info->long_options, new_size);

    /*
      If we can't parse all the options then
      what is the point of running the program?
    */
    GUARD(new_long_options)

    info->long_options = new_long_options;

    /*
      long_count is no longer valid but it can be used to
      insert new string, that is it can be used as array subscript
      Afterwards update long_count
      Note we aren't copying str, just copying its pointer
      This is because str is a constant
      TODO add const modifier to str maybe?
    */
    info->long_options[info->long_count] = str;
    info->long_count += 1;
}

struct option_info *opt_find_short(struct options *opts, char c)
{
    /*
      We use size_t as iterators because we don't know how many
      options user will register, he might even register more
      than value of INT_MAX (maximal number of arguments to main)
    */
    for (size_t i = 0; i != opts->option_count; ++i)
    {
        struct option_info *info;

        info = opts->options[i];

        for (size_t j = 0; j != info->short_count; ++j)
            if (info->short_options[j] == c)
                return info;
    }
    return NULL;
}

struct option_info *opt_find_long(struct options *opts, char *str)
{
    /*
      We use size_t as iterators because we don't know how many
      options user will register, he might even register more
      than value of INT_MAX (maximal number of arguments to main)
    */
    for (size_t i = 0; i != opts->option_count; ++i)
    {
        struct option_info *info;

        info = opts->options[i];

        for (size_t j = 0; j != info->long_count; ++j)
            if (strcmp(info->long_options[j], str) == 0)
                return info;
    }

    return NULL;
}

static void handle_parameter(struct options *opts,
    char **argv, int *iter_ptr)
{
    /*
      int is enough, main takes up to INT_MAX arguments
      Not all of the arguments are parameters
    */
    int iter;
    int new_size;
    char **new_parameters;

    /* It is more readable this way */
    iter = *iter_ptr;

    /*
      There is space allocated for opts->parameter_count parameters
      but we want to allocate apace for one more
      Calling realloc(NULL, size) is equivalent to malloc(size)
    */
    new_size = (opts->parameter_count + 1) * sizeof(char *);
    new_parameters = realloc(opts->parameters, new_size);

    GUARD(new_parameters)

    opts->parameters = new_parameters;

    /* parameter_count is invalid but can be used as a subscript */
    opts->parameters[opts->parameter_count] = argv[iter];
    /* It is valid now */
    opts->parameter_count += 1;

    /* Update iterator */
    *iter_ptr += 1;
}

static void handle_short_option(struct options *opts,
    int argc, char **argv, int *iter_ptr)
{
    struct option_info *info;
    int iter;
    char option;

    /*
      It's easier to understand this way
      Of course this means that we must update iter_ptr later on
    */
    iter = *iter_ptr;

    /* First character after hyphen is our option name */
    option = argv[iter][1];

    /*
      Retrieve information about option we are dealing with
      We are going to store information about it there too
      This function won't return if it fails
      No error checking needed
    */
    info = find_short_option(opts, option);



    /* Nothing to do if there are no parameters */
    if (info->takes_parameter == false)
    {
        /* We encountered one more option of this type, nothing special */
        info->occurrences += 1;

        ++iter;
        *iter_ptr += 1;

        return;
    }

    /*
      If this is true we are dealing with option that is separated
      from its parameter else they are joined
      Separated example: -o output-file.o
      Joined example: -ooutput-file.o
    */
    if (argv[iter][2] == '\0')
    {
        /*
          Here we check whether next argument exists
          We substract 1 from argc because array subscripts
          start at 0 but argument counting starts at 1
        */
        if (iter == argc - 1)
        {
            fprintf(stderr, "Option %c (the last one) requires a parameter",
                option);
            exit(EXITCODE_INVOCATION_ERROR);
        }
        /* Next argument exists, mark it as current one */
        iter += 1;
        /* Append encountered parameter to info->parameters */
        append(info, argv[iter]);
    }
    else
    {
        /*
          Read first comment in this file
          if you don't know what the '&' part does
          We append option's parameter to info->parameters
        */
        append(info, &argv[iter][2]);
    }

    /* Prepare to parse next option */
    iter += 1;
    /* Update caller's copy of iter */
    *iter_ptr = iter;
}

static void handle_long_option(struct options *opts,
    int argc, char **argv, int *iter_ptr)
{
    struct option_info *info;
    int iter;
    char *option;
    char *tail;

    /* This is so that we don't have to type asterisk all the time */
    iter = *iter_ptr;

    /* 
      Option is everything between two leading hyphens and '='/null byte
      Copy it to this variable (and don't forget to free it)
    */
    option = copy_option_name(argv[iter]);

    /*
      Retrieve information about option we are dealing with
      We are going to store information about it there too
      This function won't return if it fails
      No error checking needed
    */
    info = find_long_option(opts, option);

    /* We just encountered one more option of this type */
    info->occurrences += 1;

    /* There are no parameters to parse, nothing to do */
    if (info->takes_parameter == false)
        return;

    /*
      We use this to check if we are dealing with joined argument or not
      Examples:
        separate --output file.o
        joined --output=file.o
      If we find '=' in current argument tail won't be NULL
      We use this variable if argument is joined later on
    */
    tail = strchr(argv[iter], '=');

    /*
      If this is true we are dealing with a option that
      is separated from its parameter, else they are joined
    */
    if (tail == NULL)
    {
        if (iter == argc - 1)
        {
            fprintf(stderr, "Option %s (the last one) requires a parameter",
                option);
            exit(EXITCODE_INVOCATION_ERROR);
        }

        iter += 1;
        /* Append parameter (argv[iter]) to info->parameters */
        append(info, argv[iter]);
    }
    else
    {
        /*
          tail points at '=' character, make it point at next one 
          Yes, this is pointer arithmetic
        */
        tail += 1;
        /* Append parameter (tail) to info->parameters */
        append(info, tail);
    }

    free(option);

    /* Make iter point to next option/parameter to parse */
    iter += 1;

    /* Update caller's copy of iter */
    *iter_ptr = iter;
}

static void handle_final(struct options *opts,
    int argc, char **argv, int *iter_ptr)
{
    int iter;

    iter = *iter_ptr;

    /*
      Argument counting starts at 1 but array subscripts start at 0
      It is possible that '--' is followed by nothing, handle this case
    */
    if (iter == argc - 1)
        return;

    /* Skip '--' */
    iter += 1;

    while (iter != argc)
    {
        /* Why repeat ourselves? */
        handle_parameter(opts, argv, &iter);
    }
}

static struct option_info *find_short_option(struct options *opts, char c)
{
    struct option_info *info;

    info = opt_find_short(opts, c);

    if (info == NULL)
    {
        fprintf(stderr, "Unknown short option %c\n", c);
        exit(EXITCODE_INVOCATION_ERROR);
    }

    return info;
}

static struct option_info *find_long_option(struct options *opts, char *str)
{
    struct option_info *info;

    info = opt_find_long(opts, str);

    if (info == NULL)
    {
        fprintf(stderr, "Unknown long option %s\n", str);
        exit(EXITCODE_INVOCATION_ERROR);
    }

    return info;
}

static void append(struct option_info *info, char *value)
{
    size_t new_size;
    char **new_parameters;

    /*
      We have space for info->occurrences strings
      and need space for one more
      If info->values is null realloc works like malloc
    */
    new_size = (info->occurrences + 1) * sizeof(char *);
    new_parameters = realloc(info->parameters, new_size);

    /*
      We can't parse all the parameters, what now? Nothing.
      If we don't full understand what our user intended to do then
      what should we do? Nothing. Compilers don't guess what has to be done
    */
    GUARD(new_parameters)

    info->parameters = new_parameters;

    /*
      Just like in all similar functions above
      Insert our new value into the list and update occurrence count
    */
    info->parameters[info->occurrences] = value;
    info->occurrences += 1;
}

static char *copy_option_name(char *arg)
{
    char *option_name;
    size_t start, stop, length;

    /* 0 and 1 - leanding hyphens */
    start = 2;

    /*
      We are sure that there is at least one character
      between '--' and '=' or '\0'
      No need to check arg[2], it isn't '=' nor '\0'
      start at index 3
    */
    stop = 3;

    while (arg[stop] != '=' || arg[stop] != '\0') ++stop;

    /* Just like Python ranges arg[stop] is not included */
    length = stop - start;

    /* +1 because terminating null byte */
    option_name = malloc(length + 1);

    GUARD(option_name)

    /* We skip two leading hyphens */
    strncpy(option_name, &arg[2], length);

    /* Terminate string with null byte */
    option_name[length] = '\0';

    return option_name;
}
