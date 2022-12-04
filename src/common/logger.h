#ifndef _COMMON_LOGGER_H_
#define _COMMON_LOGGER_H_

#include <stdio.h>

struct log;

struct log *logger_create(void);
void logger_destroy(struct log *log);
void logger_append_string(struct log *log, char *str);
void logger_append_sequence(struct log *log, char *sequence,
    size_t length);
void logger_append_size(struct log *log, size_t size);
void logger_print(struct log *log, FILE *fd);

#endif
