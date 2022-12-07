#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
    struct source_info *previous;
    FILE *fd;
    size_t line;
    size_t column;
    uint16_t buffer_index;
    char buffer[SOURCE_BUFFER_SIZE];
};

struct sources
{
    struct source_info *current;
};


static void load(struct sources *sources);
static struct source_info *source_create_info(FILE *fd);

struct sources *source_create_struct(void)
{
    struct sources *sources;

    sources = malloc(sizeof(struct sources));

    GUARD(sources)

    sources->current = NULL;

    return sources;
}

void source_push(struct sources *sources, FILE *fd)
{
    struct source_info *new_source;

    /* Allocate and initialize new_source */
    new_source = source_create_info(fd);

    /*
      Push new_source onto the source "stack"
      From now on new_source becomes top of the stack
      new_source has a pointer to previous element
      so this operation can be reverted with a call to source_pop
    */
    new_source->previous = sources->current;
    sources->current = new_source;

    /*
      Now that source_info->current points to newly created source
      we can initialize buffer and buffer_index members of current source
    */
    load(sources);
}

void source_pop(struct sources *sources)
{
    struct source_info *tmp;


    /*
      Check whether stack is empty
      it is an error to pop element out of an empty stack
      When structure of type 'sources' is initialized
      member 'current' is initialized to NULL
      in order to prevent lexer from reading from source that doesn't exist
      See source_create_struct() in this file
    */
    if (sources->current == NULL)
    {
        fprintf(stderr, "Attempted to pop source but stack is empty\n");
        exit(EXIT_INTERNAL_ERROR);
    }

    /*
      Soon current_source is going to point to the previous element
      as we are just about to free and replace it
      We have to save it to prevent memory leaks
    */
    tmp = sources->current;


    /*
      Since our list functions like a stack we can pop a element
      This means that the top element gets removed
      and previous element becomes last (or top) element
    */
    sources->current = sources->current->previous;

    /* Duh */
    free(tmp);
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

    current = sources->current;

    return current->buffer[current->buffer_index];
}

void source_next(struct sources *sources)
{
    struct source_info *current;
    bool previous_char_was_newline = false;

    /*
      Current is easier to type than source_info->current
      We want the ech line to be no longer than 80 chars, too
    */
    current = sources->current;

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
    if (current->buffer_index == SOURCE_BUFFER_SIZE)
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
    return sources->current->line;
}

size_t source_column(struct sources *sources)
{
    return sources->current->column;
}

/* lead next characters to buffer */
void load(struct sources *sources)
{
    struct source_info *current;
    uint16_t chars_read;

    /* It's shorter this way */
    current = sources->current;
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
        if (errno > 0)
        {
            fputs("I/O error", stderr);
            printf("%d\n", errno);
            exit(EXITCODE_INTERNAL_ERROR);
        }

        current->buffer[chars_read] = '\0';
    }
}
