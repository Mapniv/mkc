#ifndef _EXITCODES_H_
#define _EXITCODES_H_

/* User gave us wrong command line options/parameters */
#define EXITCODE_INVOCATION_ERROR 1
/* Input program was incorrect */
#define EXITCODE_INPUT_ERROR 2
/* Compiler failed */
#define EXITCODE_INTERNAL_ERROR 3
/* Child process failed */
#define EXITCODE_EXTERNAL_ERROR 4

#endif
