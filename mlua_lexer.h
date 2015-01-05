/* Multiple Lua Programming Language : Lexical Scanner
   Copyright(C) 2014 Cheryl Natsu

   This file is part of multiple - Multiple Paradigm Language Interpreter

   multiple is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   multiple is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. 
   */

/* An experimental implementation of the Lua Programming Language.
 * The project will try to compatible with the original implementation
 * as more as possible. */

#ifndef _MLUA_LEXER_H_
#define _MLUA_LEXER_H_

#include <stdio.h>

#include "multiply_lexer.h"

/* Token Types */
enum
{
    /* Keywords */
    TOKEN_KEYWORD_AND = CUSTOM_TOKEN_STARTPOINT,
    TOKEN_KEYWORD_BREAK,
    TOKEN_KEYWORD_DO,
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_ELSEIF,
    TOKEN_KEYWORD_END,
    TOKEN_KEYWORD_FALSE,
    TOKEN_KEYWORD_FOR,
    TOKEN_KEYWORD_FUNCTION,
    TOKEN_KEYWORD_GOTO,
    TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_IN,
    TOKEN_KEYWORD_LOCAL,
    TOKEN_KEYWORD_NIL,
    TOKEN_KEYWORD_NOT,
    TOKEN_KEYWORD_OR,
    TOKEN_KEYWORD_REPEAT,
    TOKEN_KEYWORD_RETURN,
    TOKEN_KEYWORD_THEN,
    TOKEN_KEYWORD_TRUE,
    TOKEN_KEYWORD_UNTIL,
    TOKEN_KEYWORD_WHILE,

    TOKEN_OP_EQ, /* == */
    TOKEN_OP_NE, /* ~= */
    TOKEN_OP_LE, /* <= */
    TOKEN_OP_GE, /* >= */

    TOKEN_OP_DBL_COLON, /* :: */
    TOKEN_OP_SEMI_COLON, /* ; */
    TOKEN_OP_DBL_DOT, /* .. */
    TOKEN_OP_TRI_DOT, /* ... */
};

/* Get token name */
int mlua_token_name(char **token_name, size_t *token_name_len, const int value);

/* Lexical scan source code */
int mlua_tokenize(struct multiple_error *err, struct token_list **list_out, const char *data, const size_t data_len);

#endif

