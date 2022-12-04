#ifndef _LEXER_LEXER_H_
#define _LEXER_LEXER_H_

#include <inttypes.h>

#include "lunit.h"

/*
  If you set SOURCE_BUFFER_SIZE to a number larger than UINT16_MAX
  you will have to change type of buffer_index member of source structure
  to uint32_t or something larger
*/
#define SOURCE_BUFFER_SIZE 512

struct source
{
	struct source *previous;
	FILE *fd;
	size_t line;
	size_t column;
	uint16_t buffer_index;
	char buffer[SOURCE_BUFFER_SIZE];
};

struct source_info
{
    struct source *current;
};


struct lunit *lunit_get(struct source_info *source_info);
void lunit_destroy(struct lunit *lunit);

#endif

