#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <common/check_io.h>
#include <common/exitcodes.h>
#include <common/guard.h>
#include <common/messages.h>

#include "source.h"

/*
  If you set SOURCE_BUFFER_SIZE to a number larger than UINT16_MAX
  you will have to change type of buffer_index member of source structure
  to uint32_t or something larger
*/
#define SOURCE_BUFFER_SIZE 512

struct source_info
{
    FILE *fd;
    size_t line;
    size_t column;
    uint16_t buffer_index;
    char buffer[SOURCE_BUFFER_SIZE];
};

struct sources
{
    struct source_info **array;
    size_t count;
};


static void load(struct sources *sources);
static struct source_info *source_create_info(FILE *fd);

struct sources *source_create_struct(void)
{
    struct sources *sources;

    sources = malloc(sizeof(struct sources));

    GUARD(sources)

    sources->array = NULL;
    sources->count = 0;

    return sources;
}

void source_push(struct sources *sources, FILE *fd)
{
    size_t new_size;
    struct source_info *new_source;
    struct source_info **new_array;

    /* Allocate and initialize new_source */
    new_source = source_create_info(fd);

    /*
      We want to push new_source onto the source "stack"
      To do this we need to allocate space first
      When sources->array is NULL realloc works like malloc
    */
    new_size = (sources->count + 1) * sizeof(struct source_info *);
    new_array = realloc(sources->array, new_size);

    GUARD(new_array)

    sources->array = new_array;


    /*
      Push new_source onto the stack
      This operation can be reverted with source_pop()
      Array size used as its subscript points one element after the last one
    */
    sources->array[sources->count] = new_source;

    /* Increment number of elements on the stack */
    sources->count += 1;


    /*
      Now that sources->current points to newly created source
      we can initialize buffer and buffer_index members of current source
    */
    load(sources);
}

void source_pop(struct sources *sources)
{
    size_t new_size;
    struct source_info **new_array;


    /*
      Check whether stack is empty
      it is an error to pop element out of an empty stack
      When structure of type 'sources' is initialized
      member 'current' is initialized to NULL
      in order to prevent lexer from reading from source that doesn't exist
      See source_create_struct() in this file
    */
    if (sources->count == 0)
    {
        fprintf(stderr, "Attempted to pop source but stack is empty\n");
        exit(EXITCODE_INTERNAL_ERROR);
    }

    /*
      Array's size used as its subscript points one element past the last one
      We want to free last element hence we substract one
    */
    free(sources->array[sources->count - 1]);

    /* There is one element less on the stack */
    sources->count -= 1;

    /*
      In realloc's man page I found this:
      If size was equal to 0, either NULL or a pointer
      suitable to be passed to free is returned

      What happens when we pass this non-NULL pointer to realloc?
      I don't know and this is why I wrote this if-else statement
      I just want to be sure
      -- Mapniv
    */
    if (sources->count == 0)
    {
        free(sources->array);
        /*
          Very important, we don't want to pass invalid pointer to realloc
          We are unlikely tp push another element after popping the last one
          I want to be sure nothing goes wrong though
        */
        sources->array = NULL;
    }
    else
    {
        /*
          Resize the stack

          Is it neccessary? We are decreasing size of the stack
          We aren't requesting more memory, what can go wrong there?
          I don't know and that's why I'm checking whether reallocation failed
          -- Mapniv
        */
        new_size = sources->count * sizeof(struct source_info *);
        new_array = realloc(sources->array, new_size);

        GUARD(new_array)

        sources->array = new_array;
        /* We already decremented sources->count */
    }
}


static struct source_info *source_create_info(FILE *fd)
{
    struct source_info *new_source;

    /* Try to allocate new source */
    new_source = malloc(sizeof(struct source_info));

    GUARD(new_source)

    /*
      Initialize members of the struct
      buffer_index and buffer members are going to be
      initialized by a call to load in source_push (caller)
      previous member is going to be set by source_push (caller)
    */
    new_source->fd = fd;
    new_source->line = 1;
    new_source->column = 1;
    new_source->buffer_index = 0;

    return new_source;
}

char source_get(struct sources *sources)
{
    struct source_info *current;

    /*
      Array's size used as its subscript points one element past the last one
      and we want to access the last one, this is why we substract one
    */
    current = sources->array[sources->count - 1];

    return current->buffer[current->buffer_index];
}

void source_next(struct sources *sources)
{
    struct source_info *current;
    bool previous_char_was_newline = false;

    /* Get current (last) source
      Array's size used as its subscript points one element past the last one
      and we want to access the last one, this is why we substract one
    */
    current = sources->array[sources->count - 1];

    /*
      Check if current char is a EOL, if so set a flag
      This flag is used to decide if we want to increment
      column number or reset column number and increment line number
    */
    if (current->buffer[current->buffer_index] == '\n')
        previous_char_was_newline = true;

    /*
      Try to move to next char if possible else reload the buffer
      When we reload the buffer we set the position
      to the very beginning of it
    */
    if (current->buffer_index == SOURCE_BUFFER_SIZE - 1)
    {
        current->buffer_index = 0;
        load(sources);
    }
    else
    {
        current->buffer_index += 1;
    }

    /*
      I'm not sure whether checking if line/column number reached
      SIZE_MAX as I don't think file can have more than SIZE_MAX chars
      But it's better to be safe than sorry
      We exit because it would require extensive modification
      of the lexer, this function would no longer return
      void and that would make it way more complex
      And I'm not sure if this condition is ever met
      I wish I could just remove inner ifs #removing-code-is-good
    */
    /* #TODO Add more useful error messages maybe? */
    if (previous_char_was_newline == true)
    {
        if (current->line == SIZE_MAX)
        {
            fputs("File too big", stderr);
            exit(EXITCODE_INTERNAL_ERROR);
        }
        current->line += 1;
        current->column = 1;
    }
    else
    {
        if (current->column == SIZE_MAX)
        {
            fputs("Line too long", stderr);
            exit(EXITCODE_INTERNAL_ERROR);
        }
        current->column += 1;
    }
}

size_t source_line(struct sources *sources)
{
    struct source_info *current;
    /*
      Get current source
      Array's size used as its subscript points one element past the last one
      and we want to access the last one, this is why we substract one
    */
    current = sources->array[sources->count - 1];

    return current->line;
}

size_t source_column(struct sources *sources)
{
    struct source_info *current;
    /*
      Get current source
      Array's size used as its subscript points one element past the last one
      and we want to access the last one, this is why we substract one
    */
    current = sources->array[sources->count - 1];

    return current->column;
}

/* lead next characters to buffer */
void load(struct sources *sources)
{
    struct source_info *current;
    uint16_t chars_read;
 
    /*
      Get current source
      Array's size used as its subscript points one element past the last one
      and we want to access the last one, this is why we substract one
    */
    current = sources->array[sources->count - 1];

    chars_read = 0;

    /*
      Read BUFFER_SIZE chars (one by one, not in a 512 byte chunk) 
      Fread returns number of chunks read, and we want to know how many
      chars we just read, setting chunk size to one solves the problem
    */
    chars_read = fread(current->buffer, 1, SOURCE_BUFFER_SIZE, current->fd);

    /* We read less characters than we asked for, EOF or error */
    if (chars_read < SOURCE_BUFFER_SIZE)
    {
        /* Positive errno indicates error */
        CHECK_IO_ERROR(errno > 0)

        current->buffer[chars_read] = '\0';
    }
}
