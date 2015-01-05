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

#include "selfcheck.h"
#include <stdlib.h>
#include <string.h>

#include "multiple_err.h"

#include "multiply_lexer.h"

#include "mlua_lexer.h"

/* Status definitions for lexical analysis */
enum {
    LEX_STATUS_INIT = 0,

    LEX_STATUS_COMMENT, /* --.*?<EOL> */
    LEX_STATUS_COMMENT_ML, /* --[[.*?]] */
    LEX_STATUS_EOL, /* EOL of Windows? Mac? */
    LEX_STATUS_IDENTIFIER_P_1, /* Identifier ? */

    LEX_STATUS_INTEGER_BOH, /* 0[b|[0-9]|h] */
    LEX_STATUS_INTEGER_BOH_B, /* 0b */
    LEX_STATUS_INTEGER_BOH_B1, /* 0b[01] */
    LEX_STATUS_INTEGER_BOH_O, /* 0[0-9] */
    LEX_STATUS_INTEGER_BOH_H, /* 0x */
    LEX_STATUS_INTEGER_BOH_H1, /* 0x[01] */
    LEX_STATUS_INTEGER_D, /* 0x */

    LEX_STATUS_FLOAT_DOT_B, /* 0b???. */
    LEX_STATUS_FLOAT_DOT_O, /* 0???. */
    LEX_STATUS_FLOAT_DOT_D, /* ???. */
    LEX_STATUS_FLOAT_DOT_H, /* 0x???. */

    LEX_STATUS_STRING, /* " */
    LEX_STATUS_STRING_ESCAPE, /* \ */

    LEX_STATUS_BACK_FINISH, /* Finished, and break */
    LEX_STATUS_FINISH, /* Finished */
    LEX_STATUS_ERROR, /* Error */
};

#define JMP(status,dst) do{(status)=(dst);}while(0);
#define FIN(x) do{(x)=LEX_STATUS_FINISH;}while(0);
#define BFIN(x) do{(x)=LEX_STATUS_BACK_FINISH;}while(0);
#define UND(x) do{(x)=LEX_STATUS_ERROR;}while(0);
#define KEEP() do{}while(0);

#define APPLY_KEYWORD_BEGIN()\
    do{
#define APPLY_KEYWORD(token,keyword,keyword_value) \
    if ((token->len==strlen(keyword))&&(strncmp(token->str,keyword,token->len)==0)){\
        token->value=keyword_value;break;}
#define APPLY_KEYWORD_END()\
    }while(0)

#define ENOUGH_SPACE(p, endp, delta) (((endp)-(p)>=(delta))?(1):(0))

/* Get one token from the char stream */
static int eat_token(struct multiple_error *err, struct token *new_token, const char *p, const char *endp, uint32_t *pos_col, uint32_t *pos_ln, const int eol_type, size_t *move_on)
{
    const char *p_init = p;
    int status = LEX_STATUS_INIT;
    /*int remain_len = endp - p;*/
    int ch = 0, ch2 = 0, ch3 = 0;
    size_t bytes_number;
    size_t prefix_strip = 0, postfix_strip = 0;

    int is_eol = 0; /* For updating EOL and Ln */

    /* Clean template */
    new_token->value = TOKEN_UNDEFINED;
    new_token->str = (char *)p_init;
    new_token->len = 0;
    new_token->pos_col = *pos_col;
    new_token->pos_ln = *pos_ln;

    while (p != endp)
    {
        ch = *p;
        if (endp - p >= 2)
        {
            /* 2-bytes operator */
            ch2 = *(p + 1);
            if (endp - p >= 3)
            {
                /* 3-bytes operator */
                ch3 = *(p + 2);
            }
        }
        switch (status)
        {
            case LEX_STATUS_EOL:
                if (ch == CHAR_LF) { FIN(status); } else { BFIN(status); }
                break;
            case LEX_STATUS_COMMENT:
                if (IS_EOL(ch)) 
                {
                    /* Reset location */
                    *pos_col = 1;
                    *pos_ln += 1;
                    is_eol = 1;
                    /* "" (Null String) */
                    new_token->value = TOKEN_WHITESPACE;
                    FIN(status);
                }
                else
                {
                    KEEP();
                }
                break;
            case LEX_STATUS_COMMENT_ML:
                if ((ENOUGH_SPACE(p,endp,2))&&(ch == ']')&&(ch2==']'))
                {
                    p += 1;
                    JMP(status, LEX_STATUS_INIT);
                }
                break;
            case LEX_STATUS_INIT:
                if (IS_EOL(ch)) 
                {
                    /* Reset location */
                    *pos_col = 1;
                    *pos_ln += 1;
                    is_eol = 1;

                    new_token->value = TOKEN_WHITESPACE; 
                    switch (eol_type)
                    {
                        case EOL_UNIX:
                        case EOL_MAC:
                            FIN(status);
                            break;
                        case EOL_DOS:
                            JMP(status, LEX_STATUS_EOL);
                            break;
                    }
                }

                /* Comments */
                if ((ch == '#') && (*pos_col == 1) && (*pos_ln == 1))
                {
                    p += 1;
                    JMP(status, LEX_STATUS_COMMENT);
                }
                else if ((ENOUGH_SPACE(p,endp,2))&&(ch == '-')&&(ch2=='-'))
                {
                    if ((ENOUGH_SPACE(p,endp,4)) && \
                            ((ch3 == '[')&&(*(p + 3) == '[')))
                    {
                        p += 3;
                        JMP(status, LEX_STATUS_COMMENT_ML);
                    }
                    else
                    {
                        p += 1;
                        JMP(status, LEX_STATUS_COMMENT);
                    }
                }
                else if (IS_WHITESPACE(ch)) 
                {
                    new_token->value = TOKEN_WHITESPACE; FIN(status);
                }
                else if ((ENOUGH_SPACE(p,endp,3)) && (ch == '.') && (ch2 == '.') && (ch3 == '.'))
                {new_token->value = TOKEN_OP_TRI_DOT; FIN(status);p+=2;} /* ... */
                else if ((ENOUGH_SPACE(p,endp,2)) && (ch == '.') && (ch2 == '.')) 
                {new_token->value = TOKEN_OP_DBL_DOT; FIN(status);p++;} /* .. */
                else if ((ENOUGH_SPACE(p,endp,2)) && (ch == ':') && (ch2 == ':'))
                {new_token->value = TOKEN_OP_DBL_COLON; FIN(status);p++;} /* :: */
                else if ((ENOUGH_SPACE(p,endp,2)) && (ch == '<') && (ch2 == '='))
                {new_token->value = TOKEN_OP_LE; FIN(status);p++;} /* <= */
                else if ((ENOUGH_SPACE(p,endp,2)) && (ch == '>') && (ch2 == '='))
                {new_token->value = TOKEN_OP_GE; FIN(status);p++;} /* >= */
                else if ((ENOUGH_SPACE(p,endp,2)) && (ch == '=') && (ch2 == '='))
                {new_token->value = TOKEN_OP_EQ; FIN(status);p++;} /* == */
                else if ((ENOUGH_SPACE(p,endp,2)) && (ch == '~') && (ch2 == '='))
                {new_token->value = TOKEN_OP_NE; FIN(status);p++;} /* =~ */
                else if ((ENOUGH_SPACE(p,endp,2)) && (ch == ':') && (ch2 == ':'))
                {new_token->value = TOKEN_OP_DBL_COLON; FIN(status);p++;} /* :: */
                else if (ch == '=') {new_token->value = '='; FIN(status);}
                else if (ch == '+') {new_token->value = '+'; FIN(status);}
                else if (ch == '-') {new_token->value = '-'; FIN(status);}
                else if (ch == '*') {new_token->value = '*'; FIN(status);}
                else if (ch == '/') {new_token->value = '/'; FIN(status);}
                else if (ch == '%') {new_token->value = '%'; FIN(status);}
                else if (ch == '(') {new_token->value = '('; FIN(status);}
                else if (ch == ')') {new_token->value = ')'; FIN(status);}
                else if (ch == ',') {new_token->value = ','; FIN(status);}
                else if (ch == '&') {new_token->value = '&'; FIN(status);}
                else if (ch == '|') {new_token->value = '|'; FIN(status);}
                else if (ch == '^') {new_token->value = '^'; FIN(status);}
                else if (ch == '~') {new_token->value = '~'; FIN(status);}
                else if (ch == '<') {new_token->value = '<'; FIN(status);}
                else if (ch == '>') {new_token->value = '>'; FIN(status);}
                else if (ch == '[') {new_token->value = '['; FIN(status);}
                else if (ch == ']') {new_token->value = ']'; FIN(status);}
                else if (ch == '{') {new_token->value = '{'; FIN(status);}
                else if (ch == '}') {new_token->value = '}'; FIN(status);}
                else if (ch == ':') {new_token->value = ':'; FIN(status);}
                else if (ch == ';') {new_token->value = ';'; FIN(status);}
                else if (ch == '.') {new_token->value = '.'; FIN(status);}
                else if (ch == ',') {new_token->value = ','; FIN(status);}
                else if (ch == '#') {new_token->value = '#'; FIN(status);}
                else if (IS_ID(ch)) 
                {
                    /* Identifier ? */
                    new_token->value = TOKEN_IDENTIFIER;
                    JMP(status, LEX_STATUS_IDENTIFIER_P_1);
                }
                else if (IS_ID_HYPER(ch)) 
                {
                    bytes_number = id_hyper_length((char)ch);
                    if ((bytes_number == 0) || ((size_t)(endp - p) < bytes_number))
                    {
                        MULTIPLE_ERROR_INTERNAL();
                        return -MULTIPLE_ERR_LEXICAL;
                    }
                    bytes_number--;
                    while (bytes_number-- != 0) { p += 1; }
                    /* Identifier ? */
                    new_token->value = TOKEN_IDENTIFIER;
                    JMP(status, LEX_STATUS_IDENTIFIER_P_1);
                }
                else if (ch == '0')
                {
                    /* 0x???? -> Hex */
                    /* 0b???? -> Bin */
                    /* 0???? -> Oct */
                    new_token->value = TOKEN_CONSTANT_INTEGER_DECIMAL;
                    JMP(status, LEX_STATUS_INTEGER_BOH);
                }
                else if (('1' <= ch) && (ch <= '9'))
                {
                    new_token->value = TOKEN_CONSTANT_INTEGER_DECIMAL;
                    JMP(status, LEX_STATUS_INTEGER_D);
                }
                else if (ch == '\"') {JMP(status, LEX_STATUS_STRING);}
                else {new_token->value = TOKEN_UNDEFINED; UND(status);} /* Undefined! */
                break;
            case LEX_STATUS_IDENTIFIER_P_1:
                if (IS_ID(ch)||IS_INTEGER_DECIMAL(ch)) {KEEP();}
                else if (IS_ID_HYPER(ch)) 
                {
                    bytes_number = id_hyper_length((char)ch);
                    if ((bytes_number == 0) || ((size_t)(endp - p) < bytes_number))
                    {
                        MULTIPLE_ERROR_INTERNAL();
                        return -MULTIPLE_ERR_LEXICAL;
                    }
                    bytes_number--;
                    while (bytes_number-- != 0) { p += 1; }
                }
                else {new_token->value = TOKEN_IDENTIFIER; BFIN(status);} /* Identifier! */
                break;
            case LEX_STATUS_INTEGER_BOH: 
                /* 0<- */
                if ((ch == 'b')||(ch == 'B')) {JMP(status, LEX_STATUS_INTEGER_BOH_B);}
                else if (IS_INTEGER_DECIMAL(ch)) 
                {
                    new_token->value = TOKEN_CONSTANT_INTEGER_OCTAL;
                    JMP(status, LEX_STATUS_INTEGER_BOH_O);
                }
                else if ((ch == 'x')||(ch == 'X')) {JMP(status, LEX_STATUS_INTEGER_BOH_H);}
                else if (ch == '.') 
                {
                    new_token->value = TOKEN_CONSTANT_FLOAT_DECIMAL;
                    JMP(status, LEX_STATUS_FLOAT_DOT_D);
                }
                else {BFIN(status);} /* Decimal 0 */
                break;
            case LEX_STATUS_INTEGER_BOH_B:
                if (IS_INTEGER_BINARY(ch)) 
                {
                    new_token->value = TOKEN_CONSTANT_INTEGER_BINARY;
                    JMP(status, LEX_STATUS_INTEGER_BOH_B1);
                }
                else 
                {
                    /* 0b2 */
                    p -= 1;
                    BFIN(status);
                }
                break;
            case LEX_STATUS_INTEGER_BOH_B1:
                if (IS_INTEGER_BINARY(ch)) {KEEP();}
                else if (ch == '.')
                {
                    new_token->value = TOKEN_CONSTANT_FLOAT_BINARY;
                    JMP(status, LEX_STATUS_FLOAT_DOT_B);
                }
                else {BFIN(status);} /* Binary Integer! */
                break;
            case LEX_STATUS_INTEGER_BOH_O:
                if (IS_INTEGER_OCTAL(ch)) {KEEP();}
                else if (ch == '.')
                {
                    new_token->value = TOKEN_CONSTANT_FLOAT_OCTAL;
                    JMP(status, LEX_STATUS_FLOAT_DOT_O);
                }
                else {BFIN(status);} /* Octal Integer! */
                break;
            case LEX_STATUS_INTEGER_BOH_H:
                if (IS_INTEGER_HEXADECIMAL(ch)) 
                {
                    new_token->value = TOKEN_CONSTANT_INTEGER_HEXADECIMAL;
                    JMP(status, LEX_STATUS_INTEGER_BOH_H1);
                }
                else 
                {
                    /* 0xq */
                    p -= 1;
                    BFIN(status);
                }
                break;
            case LEX_STATUS_INTEGER_BOH_H1:
                if (IS_INTEGER_HEXADECIMAL(ch)){KEEP();}
                else if (ch == '.')
                {
                    new_token->value = TOKEN_CONSTANT_FLOAT_HEXADECIMAL;
                    JMP(status, LEX_STATUS_FLOAT_DOT_H);
                }
                else {BFIN(status);} /* Hexadecimal Integer! */
                break;
            case LEX_STATUS_INTEGER_D:
                if (IS_INTEGER_DECIMAL(ch)){KEEP();}
                else if (ch == '.')
                {
                    new_token->value = TOKEN_CONSTANT_FLOAT_DECIMAL;
                    JMP(status, LEX_STATUS_FLOAT_DOT_D);
                }
                else {BFIN(status);} /* Decimal Integer! */
                break;
            case LEX_STATUS_FLOAT_DOT_B:
                if (IS_INTEGER_BINARY(ch)){KEEP();}
                else {BFIN(status);} /* Binary Float! */
                break;
            case LEX_STATUS_FLOAT_DOT_O:
                if (IS_INTEGER_OCTAL(ch)){KEEP();}
                else {BFIN(status);} /* Octal Float! */
                break;
            case LEX_STATUS_FLOAT_DOT_D:
                if (IS_INTEGER_DECIMAL(ch)){KEEP();}
                else {BFIN(status);} /* Decimal Float! */
                break;
            case LEX_STATUS_FLOAT_DOT_H:
                if (IS_INTEGER_HEXADECIMAL(ch)){KEEP();}
                else {BFIN(status);} /* Hexadecimal Float! */
                break;
            case LEX_STATUS_STRING:
                /* "<- */
                if (ch == '\"')
                {
                    /* "" (Null String) */
                    prefix_strip = 1;
                    postfix_strip = 1;
                    new_token->value = TOKEN_CONSTANT_STRING;
                    FIN(status);
                }
                else if (ch == '\\')
                {
                    /* "" (Null String) */
                    prefix_strip = 1;
                    postfix_strip = 1;
                    new_token->value = TOKEN_CONSTANT_STRING;
                    JMP(status, LEX_STATUS_STRING_ESCAPE);
                }
                else
                {
                    KEEP();
                }
                break;
            case LEX_STATUS_STRING_ESCAPE:
                KEEP();
                JMP(status, LEX_STATUS_STRING);
                break;
            case LEX_STATUS_ERROR:
                new_token->str = NULL;
                new_token->len = 0;
                multiple_error_update(err, -MULTIPLE_ERR_LEXICAL, "%d:%d: undefined token", *pos_ln, *pos_col);
                return -MULTIPLE_ERR_LEXICAL;
                break;
            case LEX_STATUS_BACK_FINISH:
                p--;
            case LEX_STATUS_FINISH:
                goto done;
                break;
            default:
                new_token->str = NULL;
                new_token->len = 0;
                multiple_error_update(err, -MULTIPLE_ERR_LEXICAL, "%d:%d: undefined lexical analysis state, something impossible happened", *pos_ln, *pos_col);
                return -MULTIPLE_ERR_LEXICAL;
                break;
        }
        if (status == LEX_STATUS_BACK_FINISH) break;
        p += 1;
    }
    if (status == LEX_STATUS_INTEGER_BOH_B || status == LEX_STATUS_INTEGER_BOH_H)
    {
        /* 0b$ and 0x$ */
        p -= 1;
    }
done:
    if (!is_eol)
    {
        *pos_col += (uint32_t)(p - p_init);
    }
    if (new_token->value == TOKEN_UNDEFINED)
    {
        new_token->len = 0;
        *move_on = new_token->len;
    }
    else
    {
        new_token->len = (size_t)(p - p_init);
        *move_on = new_token->len;
        new_token->str += prefix_strip;
        new_token->len -= (size_t)(prefix_strip + postfix_strip);
    }
    return 0;
}

static int token_patch(struct multiple_error *err, struct token_list *list)
{
    struct token *token_cur;

	if (list == NULL) 
    {
        MULTIPLE_ERROR_NULL_PTR();
        return -MULTIPLE_ERR_NULL_PTR;
    }

	token_cur = list->begin;
    while (token_cur != NULL)
    {
        if (token_cur->value == TOKEN_IDENTIFIER)
        {
            APPLY_KEYWORD_BEGIN();
            /* Keywords */
            APPLY_KEYWORD(token_cur, "and", TOKEN_KEYWORD_AND);
            APPLY_KEYWORD(token_cur, "break", TOKEN_KEYWORD_BREAK);
            APPLY_KEYWORD(token_cur, "do", TOKEN_KEYWORD_DO);
            APPLY_KEYWORD(token_cur, "else", TOKEN_KEYWORD_ELSE);
            APPLY_KEYWORD(token_cur, "elseif", TOKEN_KEYWORD_ELSEIF);
            APPLY_KEYWORD(token_cur, "end", TOKEN_KEYWORD_END);
            APPLY_KEYWORD(token_cur, "false", TOKEN_KEYWORD_FALSE);
            APPLY_KEYWORD(token_cur, "for", TOKEN_KEYWORD_FOR);
            APPLY_KEYWORD(token_cur, "function", TOKEN_KEYWORD_FUNCTION);
            APPLY_KEYWORD(token_cur, "goto", TOKEN_KEYWORD_GOTO);
            APPLY_KEYWORD(token_cur, "if", TOKEN_KEYWORD_IF);
            APPLY_KEYWORD(token_cur, "in", TOKEN_KEYWORD_IN);
            APPLY_KEYWORD(token_cur, "local", TOKEN_KEYWORD_LOCAL);
            APPLY_KEYWORD(token_cur, "nil", TOKEN_KEYWORD_NIL);
            APPLY_KEYWORD(token_cur, "not", TOKEN_KEYWORD_NOT);
            APPLY_KEYWORD(token_cur, "or", TOKEN_KEYWORD_OR);
            APPLY_KEYWORD(token_cur, "repeat", TOKEN_KEYWORD_REPEAT);
            APPLY_KEYWORD(token_cur, "return", TOKEN_KEYWORD_RETURN);
            APPLY_KEYWORD(token_cur, "then", TOKEN_KEYWORD_THEN);
            APPLY_KEYWORD(token_cur, "true", TOKEN_KEYWORD_TRUE);
            APPLY_KEYWORD(token_cur, "until", TOKEN_KEYWORD_UNTIL);
            APPLY_KEYWORD(token_cur, "while", TOKEN_KEYWORD_WHILE);

            APPLY_KEYWORD_END();
        }
        token_cur = token_cur->next;
    }
    return 0;
}

int mlua_tokenize(struct multiple_error *err, struct token_list **list_out, const char *data, const size_t data_len)
{
    int ret = 0;
    uint32_t pos_col = 1, pos_ln = 1;
    struct token_list *new_list = NULL;
    struct token *token_template = NULL;
    const char *data_p = data, *data_endp = data_p + data_len;

    int eol_type = eol_detect(err, data, data_len);
    size_t move_on;

    if (eol_type < 0)
    {
        goto fail;
    }

    *list_out = NULL;

    if ((new_list = token_list_new()) == NULL)
    {
        MULTIPLE_ERROR_MALLOC();
        ret = -MULTIPLE_ERR_MALLOC;
        goto fail;
    }

    if ((token_template = token_new()) == NULL)
    {
        MULTIPLE_ERROR_MALLOC();
        ret = -MULTIPLE_ERR_MALLOC;
        goto fail;
    }

    while (data_p != data_endp)
    {
        if ((ret = eat_token(err, token_template, data_p, data_endp, &pos_col, &pos_ln, eol_type, &move_on)) != 0)
        {
            goto fail;
        }
        if (token_template->value != TOKEN_WHITESPACE)
        {
            if ((ret = token_list_append_token_with_template(new_list, token_template)) != 0)
            {
                goto fail;
            }
        }
        /* Move on */
        data_p += move_on;
    }
    ret = token_list_append_token_with_configure(new_list, TOKEN_FINISH, NULL, 0, pos_col, pos_ln);
    if (ret != 0) goto fail;

    if ((ret = token_patch(err, new_list)) != 0)
    {
        goto fail;
    }

    *list_out = new_list;
    ret = 0;
fail:
    if (token_template != NULL)
    {
        token_template->str = NULL;
        free(token_template);
    }
    if (ret != 0)
    {
        if (new_list != NULL) token_list_destroy(new_list);
    }
    return ret;
}

struct token_value_name_tbl_item
{
    const int value;
    const char *name;
};

static struct token_value_name_tbl_item token_value_name_tbl_items[] = 
{
};
#define TOKEN_VALUE_NAME_TBL_ITEMS_COUNT (sizeof(token_value_name_tbl_items)/sizeof(struct token_value_name_tbl_item))

/* Get token name */
int mlua_token_name(char **token_name, size_t *token_name_len, const int value)
{
    size_t i;

    if (generic_token_name(token_name, token_name_len, value) == 0)
    { return 0; }

    for (i = 0; i != TOKEN_VALUE_NAME_TBL_ITEMS_COUNT; i++)
    {
        if (value == token_value_name_tbl_items[i].value)
        {
            *token_name = (char *)token_value_name_tbl_items[i].name;
            *token_name_len = strlen(token_value_name_tbl_items[i].name);
            return 0;
        }
    }
    return -1;
}

