#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <common/guard.h>

#include "to_lstring.h"


struct lstring *size_to_lstring(size_t size)
{
    struct lstring *lstr;

    lstr = lstring_create();

    /*
      This loop converts size_t to string except this string is reversed
      It is equivalent of do while loop
    */
    while (true)
    {
        char digit;
        char character;

        /*
          This retrieves last digit of a number and
          shifts that number right by 1 IN DECIMAL SYSTEM
          Example: 1519
            1519 % 10 = 9 (last digit)
            1519 / 10 = 151 (result of 1591 >> 1 in decimal system)
          Next iteration:
            151 % 10 = 1
            151 / 10 = 15
          Next iteration:
            15 % 10 = 5
            15 / 10 = 1
          Next iteration:
            1 % 10 = 1
            1 / 10 = 0
          Stop, size is 0
          As you can see we retrieved digits of size in reverse order
          That is 9, 1, 5, 1 instead of 1, 5, 1, 9 (1519)
          String will be reversed later on
        */
        digit = size % 10;
        size = size / 10;

        /*
          This converts number into ascii character
          It works as follows: "0" is 0x30, "1" is 0x31 and so on
          If we add 0x30 to base digit we get it's char representation
        */
        character = digit + 0x30;

        /*
          This appends character to the lstring, due to order in which
          characters were decoded string will be reversed
        */
        lstring_append_char(lstr, character);

        /*
          If it were while condition then size=0
          would result in emply string
        */
        if (size == 0) break;
    }

    lstring_reverse(lstr);

    return lstr;
}
