#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/guard.h>
#include <common/status.h>

#include "arguments.h"


struct arguments *arg_create_struct(void);

void arg_destroy_struct(struct arguments *args);

void arg_register(struct arguments *args, struct switch_info *info);

void arg_parse(struct arguments *args, int argc, char **argv);

struct switch_info *arg_create_switch_info(bool takes_parameters);

static void arg_destroy_switch_info(struct switch_info *info);

void arg_add_short(struct switch_info *info, char c);

void arg_add_long(struct switch_info *info, char *str);

struct switch_info *arg_find_short(struct arguments *args, char c);

struct switch_info *arg_find_long(struct arguments *args, char *str);

static void handle_parameter(struct arguments *args,
    char **argv, int *iter_ptr);

static void handle_short_switch(struct arguments *args,
    int argc, char **argv, int *iter_ptr);

static void handle_long_switch(struct arguments *args,
    int argc, char **argv, int *iter_ptr);

static void handle_final(struct arguments *args,
    int argc, char **argv, int *iter_ptr);

static struct switch_info *find_short_switch(struct arguments *args, char c);

static struct switch_info *find_long_switch(struct arguments *args, char *str);

static void append(struct switch_info *info, char *value);

static char *copy_switch_name(char *arg);



struct arguments *arg_create_struct(void)
{
    struct arguments *args;

    args = malloc(sizeof(struct arguments));

    GUARD(args)

    /*
      These NULL values are going to be used
      in arg_register() and arg_parse()
    */
    args->switches = NULL;
    args->parameters = NULL;
    args->switch_count = 0;
    args->parameter_count = 0;

    return args;
}

void arg_destroy_struct(struct arguments *args)
{
    /* Free struct arguments recursively */

    /*
      Destroy all switch_info structures
      args->switch_count is a size_t
    */
    for (size_t iter = 0; iter != args->switch_count; ++iter)
        arg_destroy_switch_info(args->switches[iter]);

    /* Free array that used to hold pointers to arguments structs */
    free(args->switches);

    /*
      Free array that used to hold pointers to parameters 
      Parameters are string literals and shouldn't be freed
    */
    free(args->parameters);

    free(args);
}

void arg_register(struct arguments *args, struct switch_info *info)
{
    size_t new_size;
    struct switch_info **new_switches;

    /*
      Right now we have space allocated for args->switch_count pointers
      We need one more
      Also if args->switches is NULL realloc works like malloc
    */
    new_size = (args->switch_count + 1) * sizeof(struct switch_info *);
    new_switches = realloc(args->switches, new_size);

    /*
      If we can't parse all the arguments
      what is the point of running the program?
    */
    GUARD(new_switches)

    args->switches = new_switches;

    /*
      In reality we have args->switch_count + 1 infos
      but we use it as an array subscript
      Think about it, count = 0, count + 1 (1) infos
      Where do we put that info? At array subscript 0 (count)!
      Think again, count = 1, count + 1 (2) infos,
      Where do we put that second info? At array subscript 1!
      Afterwards we update count of course
    */
    args->switches[args->switch_count] = info;
    args->switch_count += 1;
}

void arg_parse(struct arguments *args, int argc, char **argv)
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
            handle_parameter(args, argv, &iter);
            continue;
        }

        /* Check if we are dealing with lone hyphen */
        if (strcmp(argv[iter], "-") == 0)
        {
            fputs("Got lone hyphen as an argument", stderr);
            exit(EXITCODE_INVOCATION_ERROR);
        }       

        /* Check whether it is a short switch */
        if (strncmp(argv[iter], "--", 2) != 0)
        {
            handle_short_switch(args, argc, argv, &iter);
            continue;
        }

        /* Check whether we are dealing with lone two dashes */
        if (strcmp(argv[iter], "--") == 0)
        {
            handle_final(args, argc, argv, &iter);
            return; /* Yes, we return */
        }

        /* Option with no name? This is obviously invalid */
        if (strncmp(argv[iter], "--=", 3) == 0)
        {
            fputs("No switch can start with '--='", stderr);
            exit(EXITCODE_INVOCATION_ERROR);
        }

        /* We are dealing with long switch */
        handle_long_switch(args, argc, argv, &iter);
    }
}

struct switch_info *arg_create_switch_info(bool takes_parameter)
{
    struct switch_info *info;

    info = malloc(sizeof(struct switch_info));

    /* This is a fatal error if we can't parse program's arguments */
    GUARD(info)

    /*
      We are going to realloc these
      realloc(NULL, size) works like malloc(size)
    */
    info->parameters = NULL;
    info->short_switches = NULL;
    info->long_switches = NULL;

    info->takes_parameter = takes_parameter;

    info->short_count = 0;
    info->long_count = 0;
    info->occurrences = 0;

    return info;
}

/* Free struct switch_info */
static void arg_destroy_switch_info(struct switch_info *info)
{
    /*
      Array of strings, free the array but not the strings
      Strings are string literals, they weren't created using malloc
    */
    free(info->long_switches);

    /* Array of chars, free it */
    free(info->short_switches);

    /*
      We do not free info->parameters[x] because these are string literals
      The array info->parameters must be freed though
    */
    free(info->parameters);

    free(info);
}

void arg_add_short(struct switch_info *info, char c)
{
    size_t new_size;
    char *new_short_switches;

    /*
      Right now we have space allocated for short_count characters
      We need one more
      Also if short_switches is NULL realloc works like malloc
    */
    new_size = info->short_count + 1;
    new_short_switches = realloc(info->short_switches, new_size);

    /*
      If we can't parse all the switches then
      what is the point of running the program?
    */
    GUARD(new_short_switches)

    info->short_switches = new_short_switches;

    /*
      short_count is no longer valid but it can be used to
      insert new character, that is it can be used as array subscript
      Afterwards update short_count
    */
    info->short_switches[info->short_count] = c;
    info->short_count += 1;
}

void arg_add_long(struct switch_info *info, char *str)
{
    size_t new_size;
    char **new_long_switches;

    /*
      Right now we have space allocated for long_count strings
      We need one more
      Also if long_switches is NULL realloc works like malloc
    */
    new_size = (info->long_count + 1) * sizeof(char *);
    new_long_switches = realloc(info->long_switches, new_size);

    /*
      If we can't parse all the switches then
      what is the point of running the program?
    */
    GUARD(new_long_switches)

    info->long_switches = new_long_switches;

    /*
      long_count is no longer valid but it can be used to
      insert new string, that is it can be used as array subscript
      Afterwards update long_count
      Note we aren't copying str, just copying its pointer
      This is because str is a constant
      TODO add const modifier to str maybe?
    */
    info->long_switches[info->long_count] = str;
    info->long_count += 1;
}

struct switch_info *arg_find_short(struct arguments *args, char c)
{
    /*
      We use size_t as iterators because we don't know how many
      switches user will register, he might even register more
      than value of INT_MAX (maximal number of arguments to main)
    */
    for (size_t i = 0; i != args->switch_count; ++i)
    {
        struct switch_info *info;

        info = args->switches[i];

        for (size_t j = 0; j != info->short_count; ++j)
            if (info->short_switches[j] == c)
                return info;
    }
    return NULL;
}

struct switch_info *arg_find_long(struct arguments *args, char *str)
{
    /*
      We use size_t as iterators because we don't know how many
      switches user will register, he might even register more
      than value of INT_MAX (maximal number of arguments to main)
    */
    for (size_t i = 0; i != args->switch_count; ++i)
    {
        struct switch_info *info;

        info = args->switches[i];

        for (size_t j = 0; j != info->long_count; ++j)
            if (strcmp(info->long_switches[j], str) == 0)
                return info;
    }

    return NULL;
}

static void handle_parameter(struct arguments *args,
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
      There is space allocated for args->parameter_count parameters
      but we want to allocate apace for one more
      Calling realloc(NULL, size) is equivalent to malloc(size)
    */
    new_size = (args->parameter_count + 1) * sizeof(char *);
    new_parameters = realloc(args->parameters, new_size);

    GUARD(new_parameters)

    args->parameters = new_parameters;

    /* parameter_count is invalid but can be used as a subscript */
    args->parameters[args->parameter_count] = argv[iter];
    /* It is valid now */
    args->parameter_count += 1;

    /* Update iterator */
    *iter_ptr += 1;
}

static void handle_short_switch(struct arguments *args,
    int argc, char **argv, int *iter_ptr)
{
    struct switch_info *info;
    int iter;
    char switch_name;

    /*
      It's easier to understand this way
      Of course this means that we must update iter_ptr later on
    */
    iter = *iter_ptr;

    /* First character after hyphen is our switch name */
    switch_name = argv[iter][1];

    /*
      Retrieve information about switch we are dealing with
      We are going to store information about it there too
      This function won't return if it fails
      No error checking needed
    */
    info = find_short_switch(args, switch_name);

    /* We just encountered another switch of this type, nothing spacial */
    info->occurrences += 1;

    /* Nothing to do if there are no parameters */
    if (info->takes_parameter == false)
    {
        ++iter;
        *iter_ptr = iter;

        return;
    }

    /*
      If this is true we are dealing with a switch that is separated
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
            fprintf(stderr, "Switch %c (the last one) requires a parameter",
                switch_name);
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
          if you don't know what the '&' part does TODO
          We append switch's parameter to info->parameters
        */
        append(info, &argv[iter][2]);
    }

    /* Prepare to parse next switch/parameter */
    iter += 1;
    /* Update caller's copy of iter */
    *iter_ptr = iter;
}

static void handle_long_switch(struct arguments *args,
    int argc, char **argv, int *iter_ptr)
{
    struct switch_info *info;
    int iter;
    char *switch_name;
    char *tail;

    /* This is so that we don't have to type asterisk all the time */
    iter = *iter_ptr;

    /* 
      Option is everything between two leading hyphens and '='/null byte
      Copy it to this variable (and don't forget to free it)
    */
    switch_name = copy_switch_name(argv[iter]);

    /*
      Retrieve information about switch we are dealing with
      We are going to store information about it there too
      This function won't return if it fails
      No error checking needed
    */
    info = find_long_switch(args, switch_name);

    /* We just encountered one more switch of this type */
    info->occurrences += 1;

    /* There are no parameters to parse, nothing to do */
    if (info->takes_parameter == false)
    {
        iter += 1;
        *iter_ptr = iter;

        return;
    }

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
      If this is true we are dealing with a switch that
      is separated from its parameter, else they are joined
    */
    if (tail == NULL)
    {
        if (iter == argc - 1)
        {
            fprintf(stderr, "Option %s (the last one) requires a parameter",
                switch_name);
            exit(EXITCODE_INVOCATION_ERROR);
        }

        /* Move to next argument */
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

    /*
      It was used to find appropriate struct switch_info
      and to print an error message, it is no longer needed, free it
    */
    free(switch_name);

    /* Make iter point to next switch/parameter to parse */
    iter += 1;

    /* Update caller's copy of iter */
    *iter_ptr = iter;
}

static void handle_final(struct arguments *args,
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
        handle_parameter(args, argv, &iter);
    }
}

static struct switch_info *find_short_switch(struct arguments *args, char c)
{
    struct switch_info *info;

    info = arg_find_short(args, c);

    if (info == NULL)
    {
        fprintf(stderr, "Unknown short switch %c\n", c);
        exit(EXITCODE_INVOCATION_ERROR);
    }

    return info;
}

static struct switch_info *find_long_switch(struct arguments *args, char *str)
{
    struct switch_info *info;

    info = arg_find_long(args, str);

    if (info == NULL)
    {
        fprintf(stderr, "Unknown long switch %s\n", str);
        exit(EXITCODE_INVOCATION_ERROR);
    }

    return info;
}

static void append(struct switch_info *info, char *value)
{
    size_t new_size;
    char **new_parameters;

    /*
      We have space for info->occurrences strings
      and need space for one more
      However info->occurrences has ALREADY been incremented
      That is we have occurrences - 1 elements in the array
      If info->parameters is NULL realloc works like malloc
    */
    new_size = info->occurrences * sizeof(char *);
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
      Insert our new value into the list
      Remember info->occurrences has already been incremented
      We copy pointer and not the string, value is a string literal
    */
    info->parameters[info->occurrences - 1] = value;
}

static char *copy_switch_name(char *arg)
{
    char *switch_name;
    size_t start, stop, length;

    /* 0 and 1 - leanding hyphens */
    start = 2;

    /*
      We are sure that there is at least one character
      between '--' and '=' or '\0'
      No need to check arg[2], it isn't '=' nor '\0'
      Cases "--" and "--=" are handled in arg_parse()
      Start at index 3
    */
    stop = 3;

    while (arg[stop] != '=' && arg[stop] != '\0') ++stop;

    /* Just like Python ranges arg[stop] is not included */
    length = stop - start;

    /* +1 because terminating null byte */
    switch_name = malloc(length + 1);

    GUARD(switch_name)

    /* We skip two leading hyphens */
    strncpy(switch_name, &arg[2], length);

    /* Terminate string with null byte */
    switch_name[length] = '\0';

    return switch_name;
}
