#ifndef _MAIN_OPTIONS_H_
#define _MAIN_OPTIONS_H_

/*
  This structure records information about one kind of option
  There may be multiple strings or chars that match as one kind of option
  Example:
    long_options = ["output", "output-file"]
    short_options = ['o', 'f']
  This causes --output --output-file -o -f to be treated as one kind of option
  That is parameters of these options will be kept in field parameters
  and parameter_count will be sum of number of uses of these options
  Field takes_parameter is set when creating option_info
  Fields short_options, long_options, short_count and long_count
  are set in a preparation to parsing
  Fields parameters and parameter_count are set during parsing process
*/
struct option_info
{
    char **parameters; /* array of strings */
    char *short_options; /* array of chars */
    char **long_options; /* array of strings */
    size_t short_count;
    size_t long_count;
    int occurrences;
    bool takes_parameter;
};

/* 
  This structure records information about all the options available
  It also records all the "standalone" parameters
  (not to be confused with option parameters)
  Field option_count is set before parsing takes place
  Field options is set before parsing but its members are modified during it
  Fields parameters and parameter_count are set during parsing process
*/
struct options
{
    struct option_info **options; /* array of pointers */
    char **parameters;
    size_t option_count;
    int parameter_count;
};


struct options *opt_create_struct(void);

void opt_register(struct options *opts, struct option_info *info);

void opt_parse(struct options *opts, int argc, char **argv);

struct option_info *opt_create_info(bool takes_parameters);

void opt_add_short(struct option_info *info, char c);

void opt_add_long(struct option_info *info, char *str);

struct option_info *opt_find_short(struct options *opts, char c);

struct option_info *opt_find_long(struct options *opts, char *str);

#endif
