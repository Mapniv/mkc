#ifndef _COMMON_CHECK_IO_H_
#define _COMMON_CHECK_IO_H_

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "exitcodes.h"
#include "messages.h"

#define CHECK_IO_ERROR(condition)                                    \
    if (condition)                                                   \
    {                                                                \
        fprintf(stderr, MESSAGE_IO_ERROR ": %s\n", strerror(errno)); \
        exit(EXITCODE_INTERNAL_ERROR);                               \
    }

#endif
