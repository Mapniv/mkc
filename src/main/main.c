#include <stdio.h>
#include <string.h>

#include <common/exitcodes.h>

#include "dump.h"

// dump 
//   lunits
//   ast
//   ir
// compile
//   assembly (asm)
//   object (obj)
//   executable (exe)
// assemble
// link
// help

#define NOT_IMPLEMENTED                 \
    puts("Subcommand not implemented"); \
    return EXITCODE_INVOCATION_ERROR;


int main(int argc, char **argv)
{
    char *subcommand;

    if (argc < 2)
    {
        puts("No subcommand specified");
        return EXITCODE_INVOCATION_ERROR;
    }

    /* 0 - program name, 1 - subcommand */
    subcommand = argv[1];

    /*
      If a subcommand encounters errors it calls exit()
      That's why all ifs return 0
    */

    if (strcmp(subcommand, "assemble") == 0)
    {
        NOT_IMPLEMENTED
    }

    if (strcmp(subcommand, "compile") == 0)
    {
        NOT_IMPLEMENTED
    }

    if (strcmp(subcommand, "dump") == 0)
    {
        dump(argc, argv);
        return 0;
    }

    if (strcmp(subcommand, "help") == 0)
    {
        NOT_IMPLEMENTED
    }

    if (strcmp(subcommand, "link") == 0)
    {
        NOT_IMPLEMENTED
    }

    printf("No such subcommand: %s\n"
        "Pass help as first argument for help\n", subcommand);
}
