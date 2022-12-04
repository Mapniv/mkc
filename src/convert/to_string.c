#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <common/guard.h>

#include "to_string.h"

#define STRING_ALLOCATION_STEP 64


struct string
{
    char *text;
    size_t length;
    size_t allocated_space;
};


static struct string *string_create(void);

static void string_destroy(struct string *string);

static void string_append(struct string *string, char c);

static char *string_unwrap(struct string *string);



char *size_to_string(size_t size)
{
    struct string *str;
    char *result_string;

    str = string_create();

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
          This appends string to the struct, due to order in which
          characters were decoded string will be reversed
        */
        string_append(str, character);

        /*
          If it were while condition then size=0
          would result in emply string
        */
        if (size == 0) break;
    }

    /* This reverses str and makes it standard null terminated C string */
    result_string = string_unwrap(str);
    string_destroy(str);

    return result_string;
}

static struct string *string_create(void)
{
    struct string *string;

    string = malloc(sizeof(struct string));

    GUARD(string)

    string->text = NULL;
    string->length = 0;
    string->allocated_space = 0;

    return string;
}

static void string_destroy(struct string *string)
{
    /*
      It is very unusual to create string struct and then just free it
      As always it is better to be safe than sorry
    */
    if (string->text != NULL)
        free(string->text);
    free(string);
}

static void string_append(struct string *string, char c)
{
    if (string->length == string->allocated_space)
    {
        size_t new_size;
        char *new_text;

        new_size = string->allocated_space + STRING_ALLOCATION_STEP;
        new_text = realloc(string->text, new_size);

        GUARD(new_text)

        string->text = new_text;
        string->allocated_space = new_size;
    }

    string->text[string->length] = c;
    string->length += 1;
}

static char *string_unwrap(struct string *string)
{
    char *result_string;
    size_t from, to;

    /* +1 because terminating null byte */
    result_string = malloc(string->length + 1);

    /*
      Length of a given array used as its subscript points
      one element after the last element of an array. Bad
      We want to start at the last element
    */
    from = string->length - 1;

    /* We want to start inserting chars at the very beginning */
    to = 0;


    while (true)
    {
        result_string[to] = string->text[from];

        /*
          This is the end condition, we just moved first char
          Next one would be -1, there is no such thing
        */
        if (from == 0)
            break;

        /*
          Move to next char in text->string
          and to its destination in result_string
        */
        from -= 1;
        to += 1;
    }

    result_string[string->length] = '\0';

    return result_string;
}
