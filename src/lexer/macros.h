#ifndef _LEXER_MACROS_H_
#define _LEXER_MACROS_H_


#define _LITERAL_a 'a'
#define _LITERAL_b 'b'
#define _LITERAL_c 'c'
#define _LITERAL_d 'd'
#define _LITERAL_e 'e'
#define _LITERAL_f 'f'
#define _LITERAL_g 'g'
#define _LITERAL_h 'h'
#define _LITERAL_i 'i'
#define _LITERAL_j 'j'
#define _LITERAL_k 'k'
#define _LITERAL_l 'l'
#define _LITERAL_m 'm'
#define _LITERAL_n 'n'
#define _LITERAL_o 'o'
#define _LITERAL_p 'p'
#define _LITERAL_q 'q'
#define _LITERAL_r 'r'
#define _LITERAL_s 's'
#define _LITERAL_t 't'
#define _LITERAL_u 'u'
#define _LITERAL_w 'w'
#define _LITERAL_x 'x'
#define _LITERAL_y 'y'
#define _LITERAL_z 'z'
#define _LITERAL__ '-'

#define STATE_1(matched, char_a)                         \
matched_##matched:                                       \
	lexme_append(&lexme_info, c);                        \
	source_next(sources);                                \
	c = source_get(sources);                             \
	if (c == _LITERAL_##char_a )                         \
		goto matched_##matched##char_a;                  \
	if (test_char_ident_f(c))                            \
		goto ident;                                      \
	return lunit_create(&lexme_info, TOK_IDENTIFIER);


#define STATE_2(matched, char_a, char_b)                 \
matched_##matched:                                       \
	lexme_append(&lexme_info, c);                        \
	source_next(sources);                                \
	c = source_get(sources);                             \
	if (c == _LITERAL_##char_a )                         \
		goto matched_##matched##char_a;                  \
	if (c == _LITERAL_##char_b )                         \
		goto matched_##matched##char_b;                  \
	if (test_char_ident_f(c))                            \
		goto ident;                                      \
	return lunit_create(&lexme_info, TOK_IDENTIFIER);

#define STATE_3(matched, char_a, char_b, char_c)         \
matched_##matched:                                       \
	lexme_append(&lexme_info, c);                        \
	source_next(sources);                                \
	c = source_get(sources);                             \
	if (c == _LITERAL_##char_a )                         \
		goto matched_##matched##char_a;                  \
	if (c == _LITERAL_##char_b )                         \
		goto matched_##matched##char_b;                  \
	if (c == _LITERAL_##char_c )                         \
		goto matched_##matched##char_c;                  \
	if (test_char_ident_f(c))                            \
		goto ident;                                      \
	return lunit_create(&lexme_info, TOK_IDENTIFIER);

#define STATE_F(matched, token)                          \
matched_##matched:                                       \
	lexme_append(&lexme_info, c);                        \
	source_next(sources);                                \
	c = source_get(sources);                             \
	if (test_char_ident_f(c))                            \
		goto ident;                                      \
	return lunit_create(&lexme_info, token);

#define STATE_1F(matched, char_a, token)                 \
matched_##matched:                                       \
	lexme_append(&lexme_info, c);                        \
	source_next(sources);                                \
	c = source_get(sources);                             \
	if (c == _LITERAL_##char_a)                          \
		goto matched_##matched##char_a;                  \
	if (test_char_ident_f(c))                            \
		goto ident;                                      \
	return lunit_create(&lexme_info, token);

#endif
