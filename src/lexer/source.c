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

#include "lexer.h"

#include "source.h"

static void load(struct source_info *source_info);
static struct source *source_new(FILE *fd);

struct source_info *source_create_info(void)
{
    struct source_info *source_info;

    source_info = malloc(sizeof(struct source_info));

    GUARD(source_info)

    source_info->current = NULL;

    return source_info;
}

void source_push(struct source_info *source_info, FILE *fd)
{
	struct source *new_source;

	/* Allocate and initialize new_source */
	new_source = source_new(fd);

	/*
	  Push new_source onto the source "stack"
	  From now on new_source becomes top of the stack
	  new_source has a pointer to previous element
	  so this operation can be reverted with a call to source_pop
	*/
    new_source->previous = source_info->current;
	source_info->current = new_source;

	/*
	  Now that source_info->current points to newly created source
	  we can initialize buffer and buffer_index members of current source
	*/
	load(source_info);
}

void source_pop(struct source_info *source_info)
{
	struct source *tmp;

	/*
	  Soon current_source is going to point to the previous element
	  as we are just about to free and replace it
	  We have to save it to prevent memory leaks
	*/
	tmp = source_info->current;

	/*
	  Since our list functions like a stack we can pop a element
	  This means that the top element gets removed
	  and previous element becomes last (or top) element
	*/
	source_info->current = source_info->current->previous;

	/* Duh */
	free(tmp);
}


static struct source *source_new(FILE *fd)
{
	struct source *new_source;

	/* Try to allocate new source */
	new_source = malloc(sizeof(struct source));

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

char source_get(struct source_info *source_info)
{
    struct source *current;

    current = source_info->current;

	return current->buffer[current->buffer_index];
}

void source_next(struct source_info *source_info)
{
    struct source *source;
	bool previous_char_was_newline = false;

    /*
      Current is easier to type than source_info->current
      We want the ech line to be no longer than 80 chars, too
    */
    source = source_info->current;

	/*
	  Check if current char is a EOL, if so set a flag
	  This flag is used to decide if we want to increment
	  column number or reset column number and increment line number
	*/
	if (source->buffer[source->buffer_index] == '\n')
		previous_char_was_newline = true;

	/*
	  Try to move to next char if possible else reload the buffer
	  When we reload the buffer we set the position
	  to the very beginning of it
	*/
	if (source->buffer_index == SOURCE_BUFFER_SIZE)
	{
		source->buffer_index = 0;
		load(source_info);
	}
	else
	{
		source->buffer_index += 1;
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
		if (source->line == SIZE_MAX)
		{
			fputs("File too big", stderr);
			exit(EXITCODE_INTERNAL_ERROR);
		}
		source->line += 1;
		source->column = 1;
	}
	else
	{
		if (source->column == SIZE_MAX)
		{
			fputs("Line too long", stderr);
			exit(EXITCODE_INTERNAL_ERROR);
		}
		source->column += 1;
	}
}

size_t source_line(struct source_info *source_info)
{
	return source_info->current->line;
}

size_t source_column(struct source_info *source_info)
{
	return source_info->current->column;
}

/* lead next characters to buffer */
void load(struct source_info *source_info)
{
    struct source *source;
	uint16_t chars_read;

    /* It's shorter this way */
    source = source_info->current;
	chars_read = 0;

	/*
	  Read BUFFER_SIZE chars (one by one, not in a 512 byte chunk) 
	  Fread returns number of chunks read, and we want to know how many
	  chars we just read, setting chunk size to one solves the problem
	*/
	chars_read = fread(source->buffer, 1, SOURCE_BUFFER_SIZE, source->fd);

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

		source->buffer[chars_read] = '\0';
	}
}
