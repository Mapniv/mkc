#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/exitcodes.h>

#include "dump_lunits.h"

void dump(int argc, char **argv)
{
    char *mode;

    if (argc < 3)
    {
        puts("You must specify a mode");
        puts("Invoke mkc with 'help dump' for help");
        exit(EXITCODE_INVOCATION_ERROR);
    }

    /* 0 - program name, 1 - subcommand, 2 - mode */
    mode = argv[2];

    if (strcmp(mode, "lunits") == 0)
    {
        dump_lunits(argc, argv);
        return;
    }
    printf("Unknown mode %s\n", mode);
    puts("Invoke mkc with 'help dump' for help");
    exit(EXITCODE_INVOCATION_ERROR);

    // TODO dump ast and ir
}
