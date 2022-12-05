#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include <common/status.h>
#include <common/exitcodes.h>
#include <common/guard.h>
#include <common/messages.h>

#include "macros.h"
#include "source.h"

#include "lexer.h"

#define LEXME_ALLOCATION_STEP 64

/* We add one because we want to make space for terminating null byte */
struct lexme_info
{
    size_t line;
    size_t column;
    size_t index;
    size_t allocated_space;
    char *text;
};

struct lunit *lunit_get(struct sources *sources);
void lunit_destroy(struct lunit *lunit);
static struct lunit *lunit_create(struct lexme_info *lexme_info,
    enum token token);
static void lexme_append(struct lexme_info *lexme_info, char c);
static void skip_whitespace_and_comments(struct sources *sources);
static inline bool test_char_ident_i(char c);
static inline bool test_char_ident_f(char c);
static inline bool test_char_whitespace(char c);


struct lunit *lunit_get(struct sources *sources)
{
    struct lexme_info lexme_info;
	char c;
	
    lexme_info.index = 0;
    lexme_info.allocated_space = 0;
    /* We are going to call realloc not malloc, see lexme_append */ 
    lexme_info.text = NULL;

    skip_whitespace_and_comments(sources);

	/* Save location of the lexme */
	lexme_info.line = source_line(sources);
	lexme_info.column = source_column(sources);

	c = source_get(sources);

	if (test_char_ident_i(c))
	{
		switch (c)
		{
			case 'p': goto matched_p;
			case 'r': goto matched_r;
		}

		goto ident;
	}
	
	if (c == '\t')
	{
        lexme_append(&lexme_info, c);
		source_next(sources);
		return lunit_create(&lexme_info, TOK_TAB);
	}

	if (c == '\n') 
	{
        lexme_append(&lexme_info, c);
		source_next(sources);
		return lunit_create(&lexme_info, TOK_EOL);
	}

	if (c == '\0')
	{
		/*
		  we shouldn't call lexme_append() nor source_next() on EOF
		  First of all EOF isn't a character, so we don't call lexme_append()
		  When it comes to source_next()... there is no such thing
		  as next character, we are dealing with EOF after all
		*/
		return lunit_create(&lexme_info, TOK_EOF);
	}

	/* Fallback */
	lexme_append(&lexme_info, c);
	source_next(sources);
	return lunit_create(&lexme_info, TOK_UNKNOWN);

    /* Finite state machine begins here */

	STATE_1(p, r)
	STATE_1(pr, o)
	STATE_1(pro, c)
	STATE_1(proc, e)
	STATE_1(proce, d)
	STATE_1(proced, u)
	STATE_1(procedu, r)
	STATE_1(procedur, e)
	STATE_F(procedure, TOK_PROCEDURE)

	STATE_1(r, e)
	STATE_1(re, t)
	STATE_1(ret, u)
	STATE_1(retu, r)
	STATE_1(retur, n)
	STATE_F(return, TOK_RETURN)

ident:
	/* Epilogue; finishes last state */
	lexme_append(&lexme_info, c);
	source_next(sources);

	/* Prologue; new state begins */
	c = source_get(sources);

	if (test_char_ident_f(c))
		goto ident;
	else
		return lunit_create(&lexme_info, TOK_IDENTIFIER);
}

void lexme_append(struct lexme_info *lexme_info, char c)
{
    char *new_text;
	/*
	  Check if there is space to accomodate new character
      We reallocate space every LEXME_ALLOCATION_STEP chars
	*/
	if (lexme_info->index == lexme_info->allocated_space)
    {
        size_t new_size;

        new_size = lexme_info->allocated_space + LEXME_ALLOCATION_STEP;
        new_text = realloc(lexme_info->text, new_size);

        GUARD(new_text)

        lexme_info->allocated_space = new_size;
        lexme_info->text = new_text;
    }

	lexme_info->text[lexme_info->index] = c;
	lexme_info->index += 1;
}

static struct lunit *lunit_create(struct lexme_info *lexme_info,
    enum token token)
{
	struct lunit *lunit;

	lunit = malloc(sizeof(struct lunit));

	GUARD(lunit)

	lunit->next = NULL;
	lunit->token = token;
	lunit->line = lexme_info->line;
	lunit->column = lexme_info->column;
	lunit->length = lexme_info->index;
    /*
      Yup, we don't copy it with strncpy
      This way is way simpler, we don't have to free lexme_info->text
      In lunit_get we can just return lunit_create(...)
      Otherwise we would have to assign return value to some variable
      then free lexme_info->text and then return that variable
      We reallocate memory because we don't want to waste space
      Size of lexme_info->text is multiple of LEXME_ALLOCATION_STEP
      Most likely there are unused bytes there... realloc it
    */
    lunit->lexme = realloc(lexme_info->text, lexme_info->index);

	return lunit;
}

void lunit_destroy(struct lunit *lunit)
{
    /* Some tokens (like newline) leave lexme field empty */
    /* TODO Perhaps we don't have to free it but just pass the pointer? */
	if (lunit->lexme != NULL)
        free(lunit->lexme);
	free(lunit);
}

/* TODO skip comments */
static void skip_whitespace_and_comments(struct sources *sources)
{
	char c;

	while (true)
	{
		c = source_get(sources);

		/* if c is whitespace skip it */
		if (test_char_whitespace(c))
		{
			source_next(sources);
			continue;
		}

		/*
		  if it is not do not move cursor,
		  next call to source_get will return this character
		*/
		return;
	}
}

/* Check if char can begin a identifier i - initial */
static inline bool test_char_ident_i(char c)
{
	if (c >= 'a' && c <= 'z') return true;
	if (c >= 'A' && c <= 'Z') return true;
	if (c == '-') return true;

	return false;
}

/* Check if char can be part of identifier f - following */
static inline bool test_char_ident_f(char c)
{
	if (test_char_ident_i(c)) return true;
	if (c >= '0' && c <= '9') return true;

	return false;
}

/* Name is self-explanatory */
static inline bool test_char_whitespace(char c)
{
	return c == ' ';
}
