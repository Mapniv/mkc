#ifndef _LEXER_MACROS_H_
#define _LEXER_MACROS_H_


/*
  These defines prevent us from having to pass the same
  argument twice over and over again
  We can pass a to a STATE_* macros instead both a and 'a'
  To convert a to 'a' we just write _TO_CHAR_##a
  which will expand to 'a'
*/
#define _TO_CHAR_a 'a'
#define _TO_CHAR_b 'b'
#define _TO_CHAR_c 'c'
#define _TO_CHAR_d 'd'
#define _TO_CHAR_e 'e'
#define _TO_CHAR_f 'f'
#define _TO_CHAR_g 'g'
#define _TO_CHAR_h 'h'
#define _TO_CHAR_i 'i'
#define _TO_CHAR_j 'j'
#define _TO_CHAR_k 'k'
#define _TO_CHAR_l 'l'
#define _TO_CHAR_m 'm'
#define _TO_CHAR_n 'n'
#define _TO_CHAR_o 'o'
#define _TO_CHAR_p 'p'
#define _TO_CHAR_q 'q'
#define _TO_CHAR_r 'r'
#define _TO_CHAR_s 's'
#define _TO_CHAR_t 't'
#define _TO_CHAR_u 'u'
#define _TO_CHAR_w 'w'
#define _TO_CHAR_x 'x'
#define _TO_CHAR_y 'y'
#define _TO_CHAR_z 'z'
#define _TO_CHAR__ '-'

/*
  STATE_* macros implement a finite state machine
  Search the web if you don't know what this means
  Different states are represented by labels and
  transition between them is done with a goto instruction

  Number in a macro name defines how many transitions
  other than the default one state can support
  From now on this number will be called N
  There are N+1 transitions in a STATE_N macro
  because there is a default transition to ident state
  This state represents as identifier
  If macro's name ends with E it is a end macro

  In all macros first parameter is name of a state
  Following N parameters are conditions
  If input meets a condition then a transition to associated state occurs
  End macros have additional parameter: type of token to return

  Operations that STATE_* macros perform
  epilogue:
    it appends input of previous state to the lexme
  prologue:
    it moves to next character in input buffer 
    and retrives one character from it
  transitions:
    each transiton is represented by one if/goto statement
  finish:
    it it represented by return keyword, name is self-explanatory

  Note that first state isn't a macro and has to be written by hand
  It is a switch statement that transfers control to one of the macrso
*/

#define STATE_1(matched, char_a)                         \
matched_##matched:                                       \
    lexme_append(&lexme_info, c);                        \
    source_next(sources);                                \
    c = source_get(sources);                             \
    if (c == _TO_CHAR_##char_a )                         \
        goto matched_##matched##char_a;                  \
    if (test_char_ident_f(c))                            \
        goto ident;                                      \
    return lunit_create(&lexme_info, TOK_IDENTIFIER);


#define STATE_2(matched, char_a, char_b)                 \
matched_##matched:                                       \
    lexme_append(&lexme_info, c);                        \
    source_next(sources);                                \
    c = source_get(sources);                             \
    if (c == _TO_CHAR_##char_a )                         \
        goto matched_##matched##char_a;                  \
    if (c == _TO_CHAR_##char_b )                         \
        goto matched_##matched##char_b;                  \
    if (test_char_ident_f(c))                            \
        goto ident;                                      \
    return lunit_create(&lexme_info, TOK_IDENTIFIER);

#define STATE_3(matched, char_a, char_b, char_c)         \
matched_##matched:                                       \
    lexme_append(&lexme_info, c);                        \
    source_next(sources);                                \
    c = source_get(sources);                             \
    if (c == _TO_CHAR_##char_a )                         \
        goto matched_##matched##char_a;                  \
    if (c == _TO_CHAR_##char_b )                         \
        goto matched_##matched##char_b;                  \
    if (c == _TO_CHAR_##char_c )                         \
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
    if (c == _TO_CHAR_##char_a)                          \
        goto matched_##matched##char_a;                  \
    if (test_char_ident_f(c))                            \
        goto ident;                                      \
    return lunit_create(&lexme_info, token);

#endif
