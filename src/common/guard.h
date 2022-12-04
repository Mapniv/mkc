#ifndef _COMMON_GUARD_H_
#define _COMMON_GUARD_H_

#include <stddef.h>

#include "exitcodes.h"
#include "messages.h"

#define GUARD(ptr)                             \
    if (ptr == NULL)                           \
    {                                          \
        fputs(MESSAGE_MEMORY_ERROR, stderr);   \
        exit(EXITCODE_INTERNAL_ERROR);         \
    }

#endif
