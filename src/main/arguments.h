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
  Fields short_switches, long_switches, short_count and long_count
  are set in a preparation to parsing
  Fields parameters and parameter_count are set during parsing process
*/
struct switch_info
{
    char **parameters; /* array of strings */
    char *short_switches; /* array of chars */
    char **long_switches; /* array of strings */
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
struct arguments
{
    struct switch_info **switches; /* array of pointers */
    char **parameters;
    size_t switch_count;
    int parameter_count;
};


struct arguments *arg_create_struct(void);

void arg_destroy_struct(struct arguments *args);

void arg_register(struct arguments *args, struct switch_info *info);

void arg_parse(struct arguments *args, int argc, char **argv);

struct switch_info *arg_create_switch_info(bool takes_parameters);

void arg_add_short(struct switch_info *info, char c);

void arg_add_long(struct switch_info *info, char *str);

struct switch_info *arg_find_short(struct arguments *args, char c);

struct switch_info *arg_find_long(struct arguments *args, char *str);

#endif
