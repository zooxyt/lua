/* Multiple Lua Programming Language : Parser
 * Copyright(C) 2014 Cheryl Natsu

 * This file is part of multiple - Multiple Paradigm Language Interpreter

 * multiple is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * multiple is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

#include "selfcheck.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "multiple_misc.h"
#include "multiple_err.h"

#include "mlua_lexer.h"
#include "mlua_ast.h"
#include "mlua_parser.h"

#include "vm_opcode.h"


struct priority_item
{
    const int left;
    const int right;
};
static struct priority_item priority_items[] = 
{
    {6, 6},
    {7, 7},
    {10, 9},
    {5, 4},
    {3, 3},
    {2, 2},
    {1, 1},
    {-1, -1},
};
#define UNARY_PRIORITY 8

static struct priority_item *mlua_parse_expression_priority(struct token *token_cur)
{
    switch (token_cur->value)
    {
        case '+': case '-': 
            return &priority_items[0];
        case '*': case '/': case '%':
            return &priority_items[1];
        case '^': 
            return &priority_items[2];
        case TOKEN_OP_DBL_DOT: 
            return &priority_items[3];
        case TOKEN_OP_EQ: 
        case TOKEN_OP_NE: 
        case '<': 
        case '>': 
        case TOKEN_OP_LE: 
        case TOKEN_OP_GE: 
            return &priority_items[4];
        case TOKEN_KEYWORD_AND: 
            return &priority_items[5];
        case TOKEN_KEYWORD_OR: 
            return &priority_items[6];
        default:
            return NULL;
    }
}


/* Fundamental */

static int mlua_parse_expression_list(struct multiple_error *err, \
        struct mlua_ast_expression_list **exp_list_out, \
        struct token **token_cur_io);

static int mlua_parse_par_list(struct multiple_error *err, \
        struct mlua_ast_par_list **par_list_out, \
        struct token **token_cur_io);


/* Medium */

static int mlua_parse_expression_sub(struct multiple_error *err, \
        struct mlua_ast_expression **exp_out, \
        struct token **token_cur_io, \
        int level);
static int mlua_parse_statement_list(struct multiple_error *err, \
        struct mlua_ast_statement_list **stmt_list_out, \
        struct token **token_cur_io);
static int mlua_parse_expression_primary(struct multiple_error *err, \
        struct mlua_ast_expression **exp_out, \
        struct token **token_cur_io);
static int mlua_parse_expression_suffixed(struct multiple_error *err, \
        struct mlua_ast_expression **exp_out, \
        struct token **token_cur_io);
static int mlua_parse_expression_function_call(struct multiple_error *err, \
        struct mlua_ast_expression_funcall **exp_funcall_out, \
        struct mlua_ast_expression *exp_func, \
        struct token **token_cur_io);
static int mlua_parse_expression(struct multiple_error *err, \
        struct mlua_ast_expression **exp_out, \
        struct token **token_cur_io);


static int mlua_parse_par_list(struct multiple_error *err, \
        struct mlua_ast_par_list **par_list_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_par_list *new_par_list = NULL;
    struct mlua_ast_par *new_par = NULL;

    /*
     * namelist ::= Name {‘,’ Name}
     * parlist ::= namelist [‘,’ ‘...’] | ‘...’
     */

    if ((new_par_list = mlua_ast_par_list_new()) == NULL)
    { goto fail; }

    /* First name */

    /* Single '...' */
    if (token_cur->value == TOKEN_OP_TRI_DOT)
    {
        /* parlist -> ... */
        if ((new_par = mlua_ast_par_new()) == NULL)
        { goto fail; }
        new_par->name = token_clone(token_cur);
        token_cur = token_cur->next;
        mlua_ast_par_list_append(new_par_list, new_par);
        new_par = NULL;
    }
    else if (token_cur->value == ')')
    {
        /* Do nothing */
    }
    else
    {
        /* Normal name list (ends with '...' allowed) */
        for (;;)
        {
            if ((token_cur->value == TOKEN_IDENTIFIER) || \
                    (token_cur->value == TOKEN_OP_TRI_DOT))
            {
                if ((new_par = mlua_ast_par_new()) == NULL)
                { goto fail; }
                new_par->name = token_clone(token_cur);
                mlua_ast_par_list_append(new_par_list, new_par);
                new_par = NULL;
            }
            else
            {
                multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                        "%d:%d: error: expected identifier or \'...\'", \
                        token_cur->pos_ln, token_cur->pos_col);
                ret = -MULTIPLE_ERR_PARSING;
                goto fail;
            }

            if (token_cur->value == TOKEN_OP_TRI_DOT)
            {
                /* Final one */
                token_cur = token_cur->next;
                break;
            }
            token_cur = token_cur->next;

            /* Test next */
            if (token_cur->value == ',')
            {
                token_cur = token_cur->next;
            }
            else
            {
                break;
            }
        } 
    }

    *par_list_out = new_par_list;

    goto done;
fail:
    if (new_par_list != NULL)
    { mlua_ast_par_list_destroy(new_par_list); }
    if (new_par != NULL)
    { mlua_ast_par_destroy(new_par); }
done:
    *token_cur_io = token_cur;
    return ret;
}


static int mlua_parse_field(struct multiple_error *err, \
        struct mlua_ast_field **field_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_field *new_field = NULL;

    if (token_cur->value == '[')
    {
        if ((new_field = mlua_ast_field_new(MLUA_AST_FIELD_TYPE_ARRAY)) == NULL)
        { goto fail; }
        if ((new_field->u.array = mlua_ast_field_array_new()) == NULL)
        { goto fail; }
        if ((ret = mlua_parse_expression(err, &new_field->u.array->index, &token_cur)) != 0)
        { goto fail; }

        /* Skip ']' */
        if (token_cur->value != ']')
        {
            multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                    "%d:%d: error: expected \']\'", \
                    token_cur->pos_ln, token_cur->pos_col);
            ret = -MULTIPLE_ERR_PARSING;
            goto fail;
        }
        token_cur = token_cur->next;
        /* Skip '=' */
        if (token_cur->value != '=')
        {
            multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                    "%d:%d: error: expected \'=\'", \
                    token_cur->pos_ln, token_cur->pos_col);
            ret = -MULTIPLE_ERR_PARSING;
            goto fail;
        }
        token_cur = token_cur->next;
        /* value */
        if ((ret = mlua_parse_expression(err, &new_field->u.array->value, &token_cur)) != 0)
        { goto fail; }
    }
    else if (token_cur->value == TOKEN_IDENTIFIER)
    {
        if ((new_field = mlua_ast_field_new(MLUA_AST_FIELD_TYPE_PROPERTY)) == NULL)
        { goto fail; }
        if ((new_field->u.property = mlua_ast_field_property_new()) == NULL)
        { goto fail; }
        if ((new_field->u.property->name = mlua_ast_name_new()) == NULL)
        { goto fail; }

        if (token_cur->value != TOKEN_IDENTIFIER)
        {
            multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                    "%d:%d: error: expected identifier", \
                    token_cur->pos_ln, token_cur->pos_col);
            ret = -MULTIPLE_ERR_PARSING;
            goto fail;
        }
        if ((new_field->u.property->name->name = token_clone(token_cur)) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        token_cur = token_cur->next;
        /* Skip '=' */
        if (token_cur->value != '=')
        {
            multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                    "%d:%d: error: expected \'=\'", \
                    token_cur->pos_ln, token_cur->pos_col);
            ret = -MULTIPLE_ERR_PARSING;
            goto fail;
        }
        token_cur = token_cur->next;
        /* value */
        if ((ret = mlua_parse_expression(err, &new_field->u.property->value, &token_cur)) != 0)
        { goto fail; }
    }
    else
    {
        if ((new_field = mlua_ast_field_new(MLUA_AST_FIELD_TYPE_EXP)) == NULL)
        { goto fail; }
        if ((new_field->u.exp = mlua_ast_field_exp_new()) == NULL)
        { goto fail; }
        if ((ret = mlua_parse_expression(err, &new_field->u.exp->value, &token_cur)) != 0)
        { goto fail; }
    }

    *field_out = new_field;

    goto done;
fail:
    if (new_field != NULL)
    { mlua_ast_field_destroy(new_field); }
done:
    *token_cur_io = token_cur;
    return ret;
}

static int mlua_parse_fieldlist(struct multiple_error *err, \
        struct mlua_ast_fieldlist **fieldlist_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_fieldlist *new_fieldlist = NULL;
    struct mlua_ast_field *new_field = NULL;

    if ((new_fieldlist = mlua_ast_fieldlist_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    /*
     * fieldlist ::= field {fieldsep field} [fieldsep]
     * field ::= ‘[’ exp ‘]’ ‘=’ exp | Name ‘=’ exp | exp
     * fieldsep ::= ‘,’ | ‘;’
     */

    if (token_cur->value == '}')
    {
    }
    else
    {
        if ((ret = mlua_parse_field(err, &new_field, &token_cur)) != 0)
        { goto fail; }
        mlua_ast_fieldlist_append(new_fieldlist, new_field);

        while ((token_cur->value == ',') || \
                (token_cur->value == ';'))
        {
            /* Skip ',' or ';' */
            token_cur = token_cur->next;

            if (token_cur->value == '}') break;

            if ((ret = mlua_parse_field(err, &new_field, &token_cur)) != 0)
            { goto fail; }
            mlua_ast_fieldlist_append(new_fieldlist, new_field);
        }
    }

    *fieldlist_out = new_fieldlist;

    goto done;
fail:
    if (new_fieldlist != NULL)
    { mlua_ast_fieldlist_destroy(new_fieldlist); }
    if (new_field != NULL)
    { mlua_ast_field_destroy(new_field); }
done:
    *token_cur_io = token_cur;
    return ret;
}


static int mlua_parse_arguments(struct multiple_error *err, \
        struct mlua_ast_args **args_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_args *new_args = NULL;

    if (token_cur->value == TOKEN_CONSTANT_STRING)
    {
        new_args = mlua_ast_args_new(MLUA_AST_ARGS_TYPE_STRING);
        if (new_args == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

        new_args->u.str = token_clone(token_cur);
        token_cur = token_cur->next;
    }
    else if (token_cur->value == '(')
    {
        new_args = mlua_ast_args_new(MLUA_AST_ARGS_TYPE_EXPLIST);
        if (new_args == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

        /* Skip '(' */
        token_cur = token_cur->next;

        /* explist */
        if ((ret = mlua_parse_expression_list(err, &new_args->u.explist, &token_cur)) != 0)
        { goto fail; }

        /* Skip ')' */
        token_cur = token_cur->next;
    }
    else if (token_cur->value == '{')
    {
        MULTIPLE_ERROR_NOT_IMPLEMENTED(); ret = -MULTIPLE_ERR_NOT_IMPLEMENTED; goto fail;
    }
    else
    {
        MULTIPLE_ERROR_INTERNAL(); ret = -MULTIPLE_ERR_INTERNAL; goto fail;
    }

    *args_out = new_args;

    goto done;
fail:
    if (new_args != NULL) { mlua_ast_args_destroy(new_args); }
done:
    *token_cur_io = token_cur;
    return ret;
}


static int mlua_parse_expression_primary(struct multiple_error *err, \
        struct mlua_ast_expression **exp_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_expression *new_exp = NULL;

    switch (token_cur->value)
    {
        case '(':
            if ((new_exp = mlua_ast_expression_new(MLUA_AST_EXPRESSION_TYPE_PRIMARY)) == NULL)
            { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
            if ((new_exp->u.primary = mlua_ast_expression_primary_new( \
                            MLUA_AST_EXPRESSION_PRIMARY_TYPE_EXPR)) == NULL)
            { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

            /* Skip '(' */
            token_cur = token_cur->next;

            if ((ret = mlua_parse_expression(err, &new_exp->u.primary->u.exp, &token_cur)) != 0)
            { goto fail; }

            /* Skip ')' */
            if (token_cur->value != ')')
            {
                multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                        "%d:%d: error: expected \')\'", \
                        token_cur->pos_ln, token_cur->pos_col);
                ret = -MULTIPLE_ERR_PARSING;
                goto fail;
            }
            token_cur = token_cur->next;

            break;

        case TOKEN_IDENTIFIER:
            if ((new_exp = mlua_ast_expression_new(MLUA_AST_EXPRESSION_TYPE_PRIMARY)) == NULL)
            { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
            if ((new_exp->u.primary = mlua_ast_expression_primary_new( \
                            MLUA_AST_EXPRESSION_PRIMARY_TYPE_NAME)) == NULL)
            { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

            if ((new_exp->u.primary->u.name = token_clone(token_cur)) == NULL)
            { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
            token_cur = token_cur->next;

            break;

        default:
            multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                    "%d:%d: error: expected identifier or \'(\'", \
                    token_cur->pos_ln, token_cur->pos_col);
            ret = -MULTIPLE_ERR_PARSING;
            goto fail;
    }

    *exp_out = new_exp;

    goto done;
fail:
    if (new_exp != NULL)
    { mlua_ast_expression_destroy(new_exp); }
done:
    *token_cur_io = token_cur;
    return ret;
}

static int mlua_parse_expression_suffixed(struct multiple_error *err, \
        struct mlua_ast_expression **exp_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_expression *new_exp = NULL, *new_exp2 = NULL;
    struct mlua_ast_expression_suffixed *new_exp_suffixed = NULL;

    if ((ret = mlua_parse_expression_primary(err, \
                    &new_exp, \
                    &token_cur)) != 0)
    { goto fail; }

    for (;;)
    {
        if (token_cur->value == '.')
        {
            /* Skip '.' */
            token_cur = token_cur->next;

            if ((new_exp2 = mlua_ast_expression_new(MLUA_AST_EXPRESSION_TYPE_SUFFIXED)) == NULL)
            { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
            if ((new_exp2->u.suffixed = mlua_ast_expression_suffixed_new( \
                            MLUA_AST_EXPRESSION_SUFFIXED_TYPE_MEMBER)) == NULL)
            { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

            if (token_cur->value != TOKEN_IDENTIFIER)
            {
                multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                        "%d:%d: error: expected identifier", \
                        token_cur->pos_ln, token_cur->pos_col);
                ret = -MULTIPLE_ERR_PARSING;
                goto fail;
            }
            if ((new_exp2->u.suffixed->u.name = token_clone(token_cur)) == NULL)
            { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

            /* Skip member name */
            token_cur = token_cur->next;

            new_exp2->u.suffixed->sub = new_exp;
            new_exp = new_exp2; new_exp2 = NULL;
        }
        else if (token_cur->value == '[')
        {
            /* Skip '[' */
            token_cur = token_cur->next;

            if ((new_exp2 = mlua_ast_expression_new(MLUA_AST_EXPRESSION_TYPE_SUFFIXED)) == NULL)
            { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
            if ((new_exp2->u.suffixed = mlua_ast_expression_suffixed_new( \
                            MLUA_AST_EXPRESSION_SUFFIXED_TYPE_INDEX)) == NULL)
            { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

            /* Parse the index */
            if ((ret = mlua_parse_expression(err, \
                            &new_exp2->u.suffixed->u.exp, \
                            &token_cur)) != 0)
            { goto fail; }

            /* Skip ']' */
            token_cur = token_cur->next;

            new_exp2->u.suffixed->sub = new_exp;
            new_exp = new_exp2; new_exp2 = NULL;
        }
        else if (token_cur->value == '(')
        {

            if ((new_exp2 = mlua_ast_expression_new(MLUA_AST_EXPRESSION_TYPE_FUNCALL)) == NULL)
            { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
            if ((ret = mlua_parse_expression_function_call(err, \
                            &new_exp2->u.funcall, \
                            new_exp, \
                            &token_cur)) != 0)
            { goto fail; }

            new_exp = new_exp2; new_exp2 = NULL;
        }
        else
        {
            break;
        }
    }

    *exp_out = new_exp;

    goto done;
fail:
    if (new_exp_suffixed != NULL)
    { mlua_ast_expression_suffixed_destroy(new_exp_suffixed); }
    if (new_exp != NULL)
    { mlua_ast_expression_destroy(new_exp); }
    if (new_exp2 != NULL)
    { mlua_ast_expression_destroy(new_exp2); }
done:
    *token_cur_io = token_cur;
    return ret;
}

static int mlua_parse_expression_simple(struct multiple_error *err, \
        struct mlua_ast_expression **exp_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_expression *new_exp = NULL;
    enum mlua_ast_expression_factor_type factor_type = MLUA_AST_EXP_FACTOR_TYPE_UNKNOWN;
    int suffixed_branch = 0;
    int factor_function = 0;
    int tblctor = 0;

    switch (token_cur->value)
    {
        case TOKEN_KEYWORD_NIL:
            factor_type = MLUA_AST_EXP_FACTOR_TYPE_NIL;
            break;
        case TOKEN_KEYWORD_TRUE:
            factor_type = MLUA_AST_EXP_FACTOR_TYPE_TRUE;
            break;
        case TOKEN_KEYWORD_FALSE:
            factor_type = MLUA_AST_EXP_FACTOR_TYPE_FALSE;
            break;

        case TOKEN_CONSTANT_INTEGER_BINARY:
        case TOKEN_CONSTANT_INTEGER_DECIMAL:
        case TOKEN_CONSTANT_INTEGER_OCTAL:
        case TOKEN_CONSTANT_INTEGER_HEXADECIMAL:
            factor_type = MLUA_AST_EXP_FACTOR_TYPE_INTEGER;
            break;

        case TOKEN_CONSTANT_FLOAT_BINARY:
        case TOKEN_CONSTANT_FLOAT_DECIMAL:
        case TOKEN_CONSTANT_FLOAT_OCTAL:
        case TOKEN_CONSTANT_FLOAT_HEXADECIMAL:
            factor_type = MLUA_AST_EXP_FACTOR_TYPE_FLOAT;
            break;

        case TOKEN_CONSTANT_STRING:
            factor_type = MLUA_AST_EXP_FACTOR_TYPE_STRING;
            break;

        case '{':
            tblctor = 1;
            break;

        case TOKEN_KEYWORD_FUNCTION:
            factor_function = 1;
            break;

        default:
            suffixed_branch = 1;
            break;
    }
    if (tblctor != 0)
    {
        /* Skip '{' */
        token_cur = token_cur->next;

        if ((new_exp = mlua_ast_expression_new(MLUA_AST_EXPRESSION_TYPE_TBLCTOR)) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        if ((new_exp->u.tblctor = mlua_ast_expression_tblctor_new()) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        
        if ((ret = mlua_parse_fieldlist(err, \
                        &new_exp->u.tblctor->fieldlist, \
                        &token_cur)) != 0)
        { goto fail; }

        /* Skip '}' */
        if (token_cur->value != '}')
        {
            multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                    "%d:%d: error: expected \'}\'", \
                    token_cur->pos_ln, token_cur->pos_col);
            ret = -MULTIPLE_ERR_PARSING;
            goto fail;
        }
        token_cur = token_cur->next;
    }
    else if (factor_function != 0)
    {
        /* Skip 'function' */
        token_cur = token_cur->next;

        if ((new_exp = mlua_ast_expression_new(MLUA_AST_EXPRESSION_TYPE_FUNDEF)) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        if ((new_exp->u.fundef = mlua_ast_expression_fundef_new()) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

        /* '(' parlist ')' */
        if (token_cur->value != '(')
        {
            multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                    "%d:%d: error: expected \'(\'", \
                    token_cur->pos_ln, token_cur->pos_col);
            ret = -MULTIPLE_ERR_PARSING;
            goto fail;
        }
        token_cur = token_cur->next;
        if ((ret = mlua_parse_par_list(err, \
                        &new_exp->u.fundef->pars, \
                        &token_cur)) != 0)
        { goto fail; }
        if (token_cur->value != ')')
        {
            multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                    "%d:%d: error: expected \')\'", \
                    token_cur->pos_ln, token_cur->pos_col);
            ret = -MULTIPLE_ERR_PARSING;
            goto fail;
        }
        token_cur = token_cur->next;

        /* body */
        if ((ret = mlua_parse_statement_list(err, \
                        &new_exp->u.fundef->body, \
                        &token_cur)) != 0)
        { goto fail; }

        /* Skip 'end' in statements */
        if (token_cur->value != TOKEN_KEYWORD_END)
        {
            multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                    "%d:%d: error: expected \'end\'", \
                    token_cur->pos_ln, token_cur->pos_col);
            ret = -MULTIPLE_ERR_PARSING;
            goto fail;
        }
        token_cur = token_cur->next;
    }
    else if (suffixed_branch != 0)
    {
        if ((ret = mlua_parse_expression_suffixed(err, \
                        &new_exp, \
                        &token_cur)) != 0)
        { goto fail; }
    }
    else
    {
        if ((new_exp = mlua_ast_expression_new(MLUA_AST_EXPRESSION_TYPE_FACTOR)) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        if ((new_exp->u.factor = mlua_ast_expression_factor_new(factor_type)) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        new_exp->u.factor->token = token_clone(token_cur);
        token_cur = token_cur->next;
    }

    *exp_out = new_exp;

    goto done;
fail:
    if (new_exp != NULL) { mlua_ast_expression_destroy(new_exp); }
done:
    *token_cur_io = token_cur;
    return ret;
}


static int mlua_parse_expression_unop(struct multiple_error *err, \
        struct mlua_ast_expression_unop **exp_unop_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_expression_unop *new_exp_unop = NULL;

    if ((new_exp_unop = mlua_ast_expression_unop_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    new_exp_unop->op = token_clone(token_cur);
    token_cur = token_cur->next;

    if ((ret = mlua_parse_expression_sub(err, &new_exp_unop->sub, &token_cur, UNARY_PRIORITY)) != 0)
    { goto fail; }

    *exp_unop_out = new_exp_unop;

    goto done;
fail:
    if (new_exp_unop != NULL) { mlua_ast_expression_unop_destroy(new_exp_unop); }
done:
    *token_cur_io = token_cur;
    return ret;
}

static int mlua_parse_expression_sub(struct multiple_error *err, \
        struct mlua_ast_expression **exp_out, \
        struct token **token_cur_io, \
        int limit)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_expression *new_exp = NULL;
    struct mlua_ast_expression *new_exp_bin = NULL;
    struct priority_item *pi;

    switch (token_cur->value)
    {
        case TOKEN_KEYWORD_NOT:
        case '-':
        case '#':
            if ((new_exp = mlua_ast_expression_new(MLUA_AST_EXPRESSION_TYPE_UNOP)) == NULL)
            { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
            if ((ret = mlua_parse_expression_unop(err, &new_exp->u.unop, &token_cur)) != 0)
            { goto fail; }
            break;

        default:
            if ((ret = mlua_parse_expression_simple(err, &new_exp, &token_cur)) != 0)
            { goto fail; }
            break;
    }

    pi = mlua_parse_expression_priority(token_cur);
    while ((pi != NULL) && (pi->left > limit))
    {
        if ((new_exp_bin = mlua_ast_expression_new(MLUA_AST_EXPRESSION_TYPE_BINOP)) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        if ((new_exp_bin->u.binop = mlua_ast_expression_binop_new()) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        new_exp_bin->u.binop->op = token_clone(token_cur);
        new_exp_bin->u.binop->left = new_exp; new_exp = NULL;

        /* Skip infix operator */
        token_cur = token_cur->next;

        if ((ret = mlua_parse_expression_sub(err, \
                        &new_exp_bin->u.binop->right, \
                        &token_cur, \
                        pi->right)) != 0)
        { goto fail; }

        pi = mlua_parse_expression_priority(token_cur);
        new_exp = new_exp_bin; new_exp_bin = NULL;
    }

    switch (token_cur->value)
    {
        case '+': case '-': case '*': case '/':
        case '%': case '^':
        case TOKEN_OP_DBL_DOT: /* .. */
        case TOKEN_OP_NE: /* =~ */
        case TOKEN_OP_EQ: /* == */
        case '<': /* < */
        case TOKEN_OP_LE: /* <= */
        case '>': /* < */
        case TOKEN_OP_GE: /* >= */
        case TOKEN_KEYWORD_AND: /* and */
        case TOKEN_KEYWORD_OR: /* or */

            break;
    }

    *exp_out = new_exp;

    goto done;
fail:
    if (new_exp != NULL) { mlua_ast_expression_destroy(new_exp); }
    if (new_exp_bin != NULL) { mlua_ast_expression_destroy(new_exp_bin); }
done:
    *token_cur_io = token_cur;
    return ret;
}

static int mlua_parse_expression(struct multiple_error *err, \
        struct mlua_ast_expression **exp_out, \
        struct token **token_cur_io)
{
    return mlua_parse_expression_sub(err, exp_out, token_cur_io, 0);
}

static int mlua_parse_expression_list(struct multiple_error *err, \
        struct mlua_ast_expression_list **exp_list_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_expression_list *new_exp_list = NULL;
    struct mlua_ast_expression *new_exp = NULL;

    new_exp_list = mlua_ast_expression_list_new();
    if (new_exp_list == NULL) { goto fail; }

    if (token_cur->value != ')')
    {
        /* At lease 1 argument */
        if ((ret = mlua_parse_expression(err, &new_exp, &token_cur)) != 0)
        { goto fail; }
        mlua_ast_expression_list_append(new_exp_list, new_exp);
        new_exp = NULL;

        while ((token_cur != NULL) && (token_cur->value == ','))
        {
            /* Skip the ',' */
            token_cur = token_cur->next;

            if ((ret = mlua_parse_expression(err, &new_exp, &token_cur)) != 0)
            { goto fail; }
            mlua_ast_expression_list_append(new_exp_list, new_exp);
            new_exp = NULL;
        }
    }

    *exp_list_out = new_exp_list;
    goto done;
fail:
    if (new_exp != NULL) { mlua_ast_expression_destroy(new_exp); }
    if (new_exp_list != NULL) { mlua_ast_expression_list_destroy(new_exp_list); }
done:
    *token_cur_io = token_cur;
    return ret;
}

/*static int mlua_parse_expression_prefix(struct multiple_error *err, \*/
/*struct mlua_ast_expression **exp_out, \*/
/*struct token **token_cur_io)*/
/*{*/
/*int ret = 0;*/
/*struct token *token_cur = *token_cur_io;*/
/*struct mlua_ast_expression *new_exp = NULL;*/

/*if (token_cur->value == TOKEN_IDENTIFIER)*/
/*{*/
/**//* prefixexp -> var */

/*if ((new_exp = mlua_ast_expression_new(MLUA_AST_EXPRESSION_TYPE_PREFIX)) == NULL)*/
/*{ MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }*/
/*if ((new_exp->u.prefix = mlua_ast_expression_prefix_new(MLUA_AST_PREFIX_EXP_TYPE_VAR)) == NULL)*/
/*{ MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }*/
/*if ((new_exp->u.prefix->u.var = token_clone(token_cur)) == NULL)*/
/*{ MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }*/
/*token_cur = token_cur->next;*/
/*}*/
/*else if (token_cur->value == '(')*/
/*{*/
/**//* prefixexp -> '(' exp ')' */

/**//* Skip '(' */
/*token_cur = token_cur->next;*/

/*if ((new_exp = mlua_ast_expression_new(MLUA_AST_EXPRESSION_TYPE_PREFIX)) == NULL)*/
/*{ MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }*/
/*if ((new_exp->u.prefix = mlua_ast_expression_prefix_new(MLUA_AST_PREFIX_EXP_TYPE_EXP)) == NULL)*/
/*{ MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }*/

/**//* Parse exp */
/*if ((ret = mlua_parse_expression(err, &new_exp->u.prefix->u.exp, &token_cur)) != 0)*/
/*{ goto fail; }*/

/**//* Skip ')' */
/*token_cur = token_cur->next;*/

/*MULTIPLE_ERROR_INTERNAL(); ret = -MULTIPLE_ERR_MALLOC; goto fail;*/
/*}*/
/*else*/
/*{*/
/**//* prefixexp -> functioncall */

/**//*if ((new_exp = mlua_ast_expression_new(MLUA_AST_EXPRESSION_TYPE_PREFIX)) == NULL)*/
/**//*{ MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }*/
/**//*if ((new_exp->u.prefix = mlua_ast_expression_prefix_new(MLUA_AST_PREFIX_EXP_TYPE_FUNCALL)) == NULL)*/
/**//*{ MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }*/

/**//*if ((ret = mlua_parse_expression_function_call(err, \*/
/**//*new_exp->u.prefix->u.funcall, \*/
/**//*)*/

/*MULTIPLE_ERROR_INTERNAL(); ret = -MULTIPLE_ERR_MALLOC; goto fail;*/
/*}*/

/**exp_out = new_exp;*/

/*goto done;*/
/*fail:*/
/*if (new_exp != NULL) { mlua_ast_expression_destroy(new_exp); }*/
/*done:*/
/**token_cur_io = token_cur;*/
/*return ret;*/
/*}*/

static int mlua_parse_expression_function_call(struct multiple_error *err, \
        struct mlua_ast_expression_funcall **exp_funcall_out, \
        struct mlua_ast_expression *exp_func, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_expression_funcall *new_exp_funcall = NULL;

    if ((new_exp_funcall = mlua_ast_expression_funcall_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    /*if ((ret = mlua_parse_expression_prefix(err, \*/
    /*&new_exp_funcall->prefixexp,*/
    /*&token_cur)) != 0)*/
    /*{ goto fail; }*/

    /* Arguments */
    if ((ret = mlua_parse_arguments(err, \
                    &new_exp_funcall->args,
                    &token_cur)) != 0)
    { goto fail; }

    /* Prefix exp */
    new_exp_funcall->prefixexp = exp_func;
    exp_func = NULL;

    *exp_funcall_out = new_exp_funcall;

    goto done;
fail:
    if (new_exp_funcall != NULL) { mlua_ast_expression_funcall_destroy(new_exp_funcall); }
done:
    *token_cur_io = token_cur;
    return ret;
}

static int mlua_parse_statement_elseif(struct multiple_error *err, \
        struct mlua_ast_statement_elseif **stmt_elseif_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_statement_elseif *new_stmt_elseif = NULL;
    struct mlua_ast_statement_elseif *stmt_elseif_root = NULL;
    struct mlua_ast_statement_elseif *stmt_elseif_prev = NULL;

    stmt_elseif_root = NULL;

    while (token_cur->value == TOKEN_KEYWORD_ELSEIF)
    {
        /* Skip 'elseif' */
        token_cur = token_cur->next;

        if ((new_stmt_elseif = mlua_ast_statement_elseif_new()) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

        /* exp */
        if ((ret = mlua_parse_expression(err, \
                        &new_stmt_elseif->exp, \
                        &token_cur)) != 0)
        { goto fail; }

        if (token_cur->value != TOKEN_KEYWORD_THEN) 
        {
            multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                    "%d:%d: error: expected \'then\'", \
                    token_cur->pos_ln, token_cur->pos_col);
            ret = -MULTIPLE_ERR_PARSING;
            goto fail;
        }

        /* Skip 'then' */
        token_cur = token_cur->next;

        /* then block */
        if ((ret = mlua_parse_statement_list(err, \
                        &new_stmt_elseif->block_then, \
                        &token_cur)) != 0)
        { goto fail; }

        if (stmt_elseif_prev != NULL)
        {
            stmt_elseif_prev->elseif = new_stmt_elseif;
        }
        if (stmt_elseif_root == NULL)
        {
            stmt_elseif_root = new_stmt_elseif;
        }
        stmt_elseif_prev = new_stmt_elseif; new_stmt_elseif = NULL;
    }

    *stmt_elseif_out = stmt_elseif_root;

    goto done;
fail:
    if (new_stmt_elseif != NULL) { mlua_ast_statement_elseif_destroy(new_stmt_elseif); }
done:
    *token_cur_io = token_cur;
    return ret;
}

static int mlua_parse_statement_if(struct multiple_error *err, \
        struct mlua_ast_statement **stmt_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_statement *new_stmt = NULL;

    /* Skip 'if' */
    token_cur = token_cur->next;

    if ((new_stmt = mlua_ast_statement_new(MLUA_AST_STATEMENT_TYPE_IF)) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    if ((new_stmt->u.stmt_if = mlua_ast_statement_if_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    /* exp */
    if ((ret = mlua_parse_expression(err, \
                    &new_stmt->u.stmt_if->exp, \
                    &token_cur)) != 0)
    { goto fail; }

    if (token_cur->value != TOKEN_KEYWORD_THEN) 
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected \'then\'", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }

    /* Skip 'then' */
    token_cur = token_cur->next;

    /* then block */
    if ((ret = mlua_parse_statement_list(err, \
                    &new_stmt->u.stmt_if->block_then, \
                    &token_cur)) != 0)
    { goto fail; }

    if (token_cur->value == TOKEN_KEYWORD_ELSE)
    {
        /* Skip 'else' */
        token_cur = token_cur->next;

        /* else block */
        if ((ret = mlua_parse_statement_list(err, \
                        &new_stmt->u.stmt_if->block_else, \
                        &token_cur)) != 0)
        { goto fail; }
    }
    else if (token_cur->value == TOKEN_KEYWORD_ELSEIF)
    {
        if ((ret = mlua_parse_statement_elseif(err, \
                        &new_stmt->u.stmt_if->elseif, \
                        &token_cur)) != 0)
        { goto fail; }
    }

    if (token_cur->value != TOKEN_KEYWORD_END) 
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected \'end\'", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }
    token_cur = token_cur->next;

    *stmt_out = new_stmt;

    goto done;
fail:
    if (new_stmt != NULL) { mlua_ast_statement_destroy(new_stmt); }
done:
    *token_cur_io = token_cur;
    return ret;
}


static int mlua_parse_statement_while(struct multiple_error *err, \
        struct mlua_ast_statement **stmt_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_statement *new_stmt = NULL;

    /* Skip 'while' */
    token_cur = token_cur->next;

    if ((new_stmt = mlua_ast_statement_new(MLUA_AST_STATEMENT_TYPE_WHILE)) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    if ((new_stmt->u.stmt_while = mlua_ast_statement_while_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    /* exp */
    if ((ret = mlua_parse_expression(err, \
                    &new_stmt->u.stmt_while->exp, \
                    &token_cur)) != 0)
    { goto fail; }

    if (token_cur->value != TOKEN_KEYWORD_DO) 
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected \'do\'", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }

    /* Skip 'do' */
    token_cur = token_cur->next;

    /* block */
    if ((ret = mlua_parse_statement_list(err, \
                    &new_stmt->u.stmt_while->block, \
                    &token_cur)) != 0)
    { goto fail; }

    if (token_cur->value != TOKEN_KEYWORD_END) 
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected \'end\'", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }

    /* Skip 'end' */
    if (token_cur->value != TOKEN_KEYWORD_END) 
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected \'end\'", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }
    token_cur = token_cur->next;

    *stmt_out = new_stmt;

    goto done;
fail:
    if (new_stmt != NULL) { mlua_ast_statement_destroy(new_stmt); }
done:
    *token_cur_io = token_cur;
    return ret;
}


static int mlua_parse_statement_repeat(struct multiple_error *err, \
        struct mlua_ast_statement **stmt_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_statement *new_stmt = NULL;

    /* Skip 'repeat' */
    token_cur = token_cur->next;

    if ((new_stmt = mlua_ast_statement_new(MLUA_AST_STATEMENT_TYPE_REPEAT)) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    if ((new_stmt->u.stmt_repeat = mlua_ast_statement_repeat_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    /* block */
    if ((ret = mlua_parse_statement_list(err, \
                    &new_stmt->u.stmt_repeat->block, \
                    &token_cur)) != 0)
    { goto fail; }

    if (token_cur->value != TOKEN_KEYWORD_UNTIL) 
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected \'until\'", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }

    /* Skip 'until' */
    token_cur = token_cur->next;

    /* exp */
    if ((ret = mlua_parse_expression(err, \
                    &new_stmt->u.stmt_repeat->exp, \
                    &token_cur)) != 0)
    { goto fail; }

    *stmt_out = new_stmt;

    goto done;
fail:
    if (new_stmt != NULL) { mlua_ast_statement_destroy(new_stmt); }
done:
    *token_cur_io = token_cur;
    return ret;
}


static int mlua_parse_statement_do(struct multiple_error *err, \
        struct mlua_ast_statement **stmt_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_statement *new_stmt = NULL;

    /* Skip 'do' */
    token_cur = token_cur->next;

    if ((new_stmt = mlua_ast_statement_new(MLUA_AST_STATEMENT_TYPE_DO)) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    if ((new_stmt->u.stmt_do = mlua_ast_statement_do_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    /* block */
    if ((ret = mlua_parse_statement_list(err, \
                    &new_stmt->u.stmt_do->block, \
                    &token_cur)) != 0)
    { goto fail; }

    if (token_cur->value != TOKEN_KEYWORD_END) 
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected \'end\'", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }

    /* Skip 'end' */
    if (token_cur->value != TOKEN_KEYWORD_END) 
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected \'end\'", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }
    token_cur = token_cur->next;

    *stmt_out = new_stmt;

    goto done;
fail:
    if (new_stmt != NULL) { mlua_ast_statement_destroy(new_stmt); }
done:
    *token_cur_io = token_cur;
    return ret;
}


static int mlua_parse_statement_for(struct multiple_error *err, \
        struct mlua_ast_statement **stmt_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_statement *new_stmt = NULL;

    /* Skip 'for' */
    token_cur = token_cur->next;

    /* stat -> 'for' name '=' exp ',' exp [',' exp] 'do' block 'end' */

    if ((new_stmt = mlua_ast_statement_new(MLUA_AST_STATEMENT_TYPE_FOR)) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    if ((new_stmt->u.stmt_for = mlua_ast_statement_for_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    /* name */
    if (token_cur->value != TOKEN_IDENTIFIER) 
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected identifier", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }
    if ((new_stmt->u.stmt_for->name = mlua_ast_name_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    if ((new_stmt->u.stmt_for->name->name = token_clone(token_cur)) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    token_cur = token_cur->next;

    /* = */
    if (token_cur->value != '=') 
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected \'=\'", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }
    token_cur = token_cur->next;

    /* exp1 */
    if ((ret = mlua_parse_expression(err, \
                    &new_stmt->u.stmt_for->exp1, \
                    &token_cur)) != 0)
    { goto fail; }

    /* , */
    if (token_cur->value != ',') 
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected \',\'", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }
    token_cur = token_cur->next;

    /* exp2 */
    if ((ret = mlua_parse_expression(err, \
                    &new_stmt->u.stmt_for->exp2, \
                    &token_cur)) != 0)
    { goto fail; }

    /* [, exp3] */
    if (token_cur->value == ',') 
    {
        token_cur = token_cur->next;
        if ((ret = mlua_parse_expression(err, \
                        &new_stmt->u.stmt_for->exp3, \
                        &token_cur)) != 0)
        { goto fail; }
    }

    /* block */
    if ((ret = mlua_parse_statement_list(err, \
                    &new_stmt->u.stmt_do->block, \
                    &token_cur)) != 0)
    { goto fail; }

    /* 'end' */
    if (token_cur->value != TOKEN_KEYWORD_END) 
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected \'end\'", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }
    token_cur = token_cur->next;

    *stmt_out = new_stmt;

    goto done;
fail:
    if (new_stmt != NULL) { mlua_ast_statement_destroy(new_stmt); }
done:
    *token_cur_io = token_cur;
    return ret;
}


static int mlua_parse_statement_break(struct multiple_error *err, \
        struct mlua_ast_statement **stmt_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_statement *new_stmt = NULL;

    /* Skip 'break' */
    token_cur = token_cur->next;

    if ((new_stmt = mlua_ast_statement_new(MLUA_AST_STATEMENT_TYPE_BREAK)) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    *stmt_out = new_stmt;

    goto done;
fail:
    if (new_stmt != NULL) { mlua_ast_statement_destroy(new_stmt); }
done:
    *token_cur_io = token_cur;
    return ret;
}


static int mlua_parse_statement_label(struct multiple_error *err, \
        struct mlua_ast_statement **stmt_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_statement *new_stmt = NULL;

    /* Skip first '::' */
    token_cur = token_cur->next;

    if ((new_stmt = mlua_ast_statement_new(MLUA_AST_STATEMENT_TYPE_LABEL)) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    if ((new_stmt->u.stmt_label = mlua_ast_statement_label_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    if (token_cur->value != TOKEN_IDENTIFIER)
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected identifier", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }

    /* Label name */
    if ((new_stmt->u.stmt_label->name = token_clone(token_cur)) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    token_cur = token_cur->next;

    /* Skip second '::' */
    token_cur = token_cur->next;

    *stmt_out = new_stmt;

    goto done;
fail:
    if (new_stmt != NULL) { mlua_ast_statement_destroy(new_stmt); }
done:
    *token_cur_io = token_cur;
    return ret;
}


static int mlua_parse_statement_goto(struct multiple_error *err, \
        struct mlua_ast_statement **stmt_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_statement *new_stmt = NULL;

    /* Skip 'goto' */
    token_cur = token_cur->next;

    if ((new_stmt = mlua_ast_statement_new(MLUA_AST_STATEMENT_TYPE_GOTO)) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    if ((new_stmt->u.stmt_goto = mlua_ast_statement_goto_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    if (token_cur->value != TOKEN_IDENTIFIER)
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected identifier", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }

    /* Label name */
    if ((new_stmt->u.stmt_goto->name = token_clone(token_cur)) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    token_cur = token_cur->next;

    *stmt_out = new_stmt;

    goto done;
fail:
    if (new_stmt != NULL) { mlua_ast_statement_destroy(new_stmt); }
done:
    *token_cur_io = token_cur;
    return ret;
}

static int mlua_parse_statement_local(struct multiple_error *err, \
        struct mlua_ast_statement **stmt_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_statement *new_stmt = NULL;
    struct mlua_ast_name *new_ast_name = NULL;
    struct mlua_ast_expression *new_ast_exp = NULL;

    /* Skip 'local' */
    token_cur = token_cur->next;

    if ((new_stmt = mlua_ast_statement_new(MLUA_AST_STATEMENT_TYPE_LOCAL)) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    if ((new_stmt->u.stmt_local = mlua_ast_statement_local_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    /* namelist */
    /* At least one name */
    if ((token_cur != NULL) && (token_cur->value != TOKEN_IDENTIFIER))
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected identifier", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }

    while ((token_cur != NULL) && (token_cur->value == TOKEN_IDENTIFIER))
    {
        new_ast_name = mlua_ast_name_new();
        if (new_ast_name == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        new_ast_name->name = token_clone(token_cur);
        if (new_ast_name->name == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        mlua_ast_namelist_append(new_stmt->u.stmt_local->namelist, \
                new_ast_name);
        new_ast_name = NULL;

        /* Skip the scanned name */
        token_cur = token_cur->next;

        if (token_cur->value == ',')
        {
            token_cur = token_cur->next;
        }
        else if (token_cur->value == '=')
        {
            break;
        }
        else
        {
            multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                    "%d:%d: error: expected ',' or '='", \
                    token_cur->pos_ln, token_cur->pos_col);
            ret = -MULTIPLE_ERR_PARSING;
            goto fail;
        }
    }


    /* '=' explist */
    if ((token_cur != NULL) && (token_cur->value == '='))
    {
        /* Skip '=' */ 
        token_cur = token_cur->next;

        /* At least one exp */
        if ((ret = mlua_parse_expression(err, \
                        &new_ast_exp, \
                        &token_cur)) != 0)
        { goto fail; }
        mlua_ast_expression_list_append(new_stmt->u.stmt_local->explist, \
                new_ast_exp);
        new_ast_exp = NULL;
        while ((token_cur != NULL) && (token_cur->value == ','))
        {
            /* Skip ',' */
            token_cur = token_cur->next; 
            /* Parse the next expression */
            if ((ret = mlua_parse_expression(err, \
                            &new_ast_exp, \
                            &token_cur)) != 0)
            { goto fail; }
            mlua_ast_expression_list_append(new_stmt->u.stmt_local->explist, \
                    new_ast_exp);
            new_ast_exp = NULL;
        }
    }

    *stmt_out = new_stmt;

    goto done;
fail:
    if (new_stmt != NULL) { mlua_ast_statement_destroy(new_stmt); }
    if (new_ast_name != NULL) { mlua_ast_name_destroy(new_ast_name); }
    if (new_ast_exp != NULL) { mlua_ast_expression_destroy(new_ast_exp); }
done:
    *token_cur_io = token_cur;
    return ret;
}


static int mlua_parse_funcname(struct multiple_error *err, \
        struct mlua_ast_funcname **funcname_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_funcname *new_funcname = NULL;
    struct mlua_ast_name *new_name = NULL;

    /* funcname ::= Name {‘.’ Name} [‘:’ Name] */

    if ((new_funcname = mlua_ast_funcname_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    if ((new_funcname->name_list = mlua_ast_namelist_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    /* First Name */
    if (token_cur->value != TOKEN_IDENTIFIER)
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected identifier", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }
    if ((new_name = mlua_ast_name_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    if ((new_name->name = token_clone(token_cur)) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    token_cur = token_cur->next;
    mlua_ast_namelist_append(new_funcname->name_list, new_name);
    new_name = NULL;

    while (token_cur->value == '.')
    {
        /* Skip '.' */
        token_cur = token_cur->next;

        if (token_cur->value != TOKEN_IDENTIFIER)
        {
            multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                    "%d:%d: error: expected identifier", \
                    token_cur->pos_ln, token_cur->pos_col);
            ret = -MULTIPLE_ERR_PARSING;
            goto fail;
        }

        if ((new_name = mlua_ast_name_new()) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        if ((new_name->name = token_clone(token_cur)) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        token_cur = token_cur->next;
        mlua_ast_namelist_append(new_funcname->name_list, new_name);
        new_name = NULL;
    }

    if (token_cur->value == ':')
    {
        /* Skip ':' */
        token_cur = token_cur->next;

        if (token_cur->value != TOKEN_IDENTIFIER)
        {
            multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                    "%d:%d: error: expected identifier", \
                    token_cur->pos_ln, token_cur->pos_col);
            ret = -MULTIPLE_ERR_PARSING;
            goto fail;
        }

        if ((new_name = mlua_ast_name_new()) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        if ((new_name->name = token_clone(token_cur)) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        token_cur = token_cur->next;
        new_funcname->member = new_name; new_name = NULL;
    }

    *funcname_out = new_funcname;

    goto done;
fail:
    if (new_name != NULL) { mlua_ast_name_destroy(new_name); }
    if (new_funcname != NULL) { mlua_ast_funcname_destroy(new_funcname); }
done:
    *token_cur_io = token_cur;
    return ret;
}


enum mlua_parse_statement_fundef_scope 
{
    MLUA_PARSE_STATEMENT_FUNDEF_GLOBAL = 0,
    MLUA_PARSE_STATEMENT_FUNDEF_LOCAL = 1,
};
static int mlua_parse_statement_fundef(struct multiple_error *err, \
        struct mlua_ast_statement **stmt_out, \
        struct token **token_cur_io, \
        enum mlua_parse_statement_fundef_scope scope)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_statement *new_stmt = NULL;

    if ((new_stmt = mlua_ast_statement_new(MLUA_AST_STATEMENT_TYPE_FUNDEF)) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    if ((new_stmt->u.stmt_fundef = mlua_ast_statement_fundef_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    switch (scope)
    {
        case MLUA_PARSE_STATEMENT_FUNDEF_GLOBAL:
            /* Skip 'function' */
            token_cur = token_cur->next; 
            new_stmt->u.stmt_fundef->local = 0;
            break;
        case MLUA_PARSE_STATEMENT_FUNDEF_LOCAL:
            /* Skip 'local function' */
            token_cur = token_cur->next->next; 
            new_stmt->u.stmt_fundef->local = 1;
            break;
    }

    /* funcname */
    /* stat ::= 'function' funcname funcbody */
    if ((ret = mlua_parse_funcname(err, &new_stmt->u.stmt_fundef->funcname, &token_cur)) != 0)
    { goto fail; }

    /* '(' parlist ')' */
    if (token_cur->value != '(')
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected \'(\'", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }
    token_cur = token_cur->next;
    if ((ret = mlua_parse_par_list(err, \
                    &new_stmt->u.stmt_fundef->parameters, \
                    &token_cur)) != 0)
    { goto fail; }
    if (token_cur->value != ')')
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected \')\'", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }
    token_cur = token_cur->next;

    /* body */
    if ((ret = mlua_parse_statement_list(err, \
                    &new_stmt->u.stmt_fundef->body, \
                    &token_cur)) != 0)
    { goto fail; }

    /* Skip 'end' in statements */
    if (token_cur->value != TOKEN_KEYWORD_END)
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected \'end\'", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }
    token_cur = token_cur->next;

    *stmt_out = new_stmt;

    goto done;
fail:
    if (new_stmt != NULL) { mlua_ast_statement_destroy(new_stmt); }
done:
    *token_cur_io = token_cur;
    return ret;
}


static int mlua_parse_statement_return(struct multiple_error *err, \
        struct mlua_ast_statement **stmt_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_statement *new_stmt = NULL;
    struct mlua_ast_name *new_ast_name = NULL;
    struct mlua_ast_expression *new_ast_exp = NULL;

    if ((new_stmt = mlua_ast_statement_new(MLUA_AST_STATEMENT_TYPE_RETURN)) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    if ((new_stmt->u.stmt_return = mlua_ast_statement_return_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    if ((new_stmt->u.stmt_return->explist = mlua_ast_expression_list_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    /* Skip 'local' */
    token_cur = token_cur->next;

    /* explist */

    while (token_cur->value != ';')
    {
        if ((ret = mlua_parse_expression(err, \
                        &new_ast_exp, \
                        &token_cur)) != 0)
        { goto fail; }
        mlua_ast_expression_list_append(new_stmt->u.stmt_return->explist, new_ast_exp);
        new_ast_exp = NULL;

        if (token_cur->value == ',')
        {
            token_cur = token_cur->next;
        }
        else
        {
            break;
        }
    }

    *stmt_out = new_stmt;

    goto done;
fail:
    if (new_stmt != NULL) { mlua_ast_statement_destroy(new_stmt); }
    if (new_ast_name != NULL) { mlua_ast_name_destroy(new_ast_name); }
    if (new_ast_exp != NULL) { mlua_ast_expression_destroy(new_ast_exp); }
done:
    *token_cur_io = token_cur;
    return ret;
}


static int mlua_parse_statement_assignment(struct multiple_error *err, \
        struct mlua_ast_statement *stmt_assign, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_expression *new_exp = NULL;

    if (token_cur->value == ',')
    {
        /* assignment -> ',' suffixedexpr assignment */

        /* Skip ',' */
        token_cur = token_cur->next;

        if ((ret = mlua_parse_expression_suffixed(err, \
                        &new_exp, \
                        &token_cur)) != 0)
        { goto fail; }

        /* Append the previous var */
        mlua_ast_expression_list_append(stmt_assign->u.stmt_assignment->varlist, new_exp);

        /* Deep into the next var */
        if ((ret = mlua_parse_statement_assignment(err, \
                        stmt_assign, \
                        &token_cur)) != 0)
        { goto fail; }
    }
    else if (token_cur->value == '=')
    {
        /* assignment -> '=' explist */

        /* Skip '=' */
        token_cur = token_cur->next;

        if ((ret = mlua_parse_expression_list(err,\
                        &stmt_assign->u.stmt_assignment->explist, \
                        &token_cur)) != 0)
        { goto fail; }
    }
    else
    {
        multiple_error_update(err, -MULTIPLE_ERR_PARSING, \
                "%d:%d: error: expected \',\' or \'=\'", \
                token_cur->pos_ln, token_cur->pos_col);
        ret = -MULTIPLE_ERR_PARSING;
        goto fail;
    }

    goto done;
fail:
    if (new_exp != NULL) { mlua_ast_expression_destroy(new_exp); }
done:
    *token_cur_io = token_cur;
    return ret;
}


/*static int mlua_parse_statement_function_call(struct multiple_error *err, \*/
/*struct mlua_ast_statement **stmt_out, \*/
/*struct mlua_ast_expression *exp_func, \*/
/*struct token **token_cur_io)*/
/*{*/
/*int ret = 0;*/
/*struct token *token_cur = *token_cur_io;*/
/*struct mlua_ast_statement *new_stmt = NULL;*/

/*if ((new_stmt = mlua_ast_statement_new(MLUA_AST_STATEMENT_TYPE_FUNCALL)) == NULL)*/
/*{ MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }*/

/*if ((ret = mlua_parse_expression_function_call(err, \*/
/*&new_stmt->u.funcall, \*/
/*exp_func, \*/
/*&token_cur)) != 0)*/
/*{ goto fail; }*/

/**stmt_out = new_stmt;*/

/*goto done;*/
/*fail:*/
/*if (new_stmt != NULL) { mlua_ast_statement_destroy(new_stmt); }*/
/*done:*/
/**token_cur_io = token_cur;*/
/*return ret;*/
/*}*/

static int mlua_parse_statement_expr(struct multiple_error *err, \
        struct mlua_ast_statement **stmt_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_statement *new_stmt = NULL;
    struct mlua_ast_expression *new_exp = NULL;

    /* Whatever the type is assignment or function call, 
     * it starts with an expression ??
     * --------------------------
     * stat -> varlist ‘=’ explist 
     * stat -> functioncall
     * --------------------------
     * prefixexp ::= var | functioncall | ‘(’ exp ‘)’
     * functioncall ::=  prefixexp args | prefixexp ‘:’ Name args 
	 * args ::=  ‘(’ [explist] ‘)’ | tableconstructor | String 
     * */
    if ((ret = mlua_parse_expression_suffixed(err, \
                    &new_exp, \
                    &token_cur)) != 0)
    { goto fail; }

    if ((token_cur->value == '=') || (token_cur->value == ','))
    {
        /* stat -> assignment */

        if ((new_stmt = mlua_ast_statement_new(MLUA_AST_STATEMENT_TYPE_ASSIGNMENT)) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        if ((new_stmt->u.stmt_assignment = mlua_ast_statement_assignment_new()) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        if ((new_stmt->u.stmt_assignment->varlist = mlua_ast_expression_list_new()) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

        /* Append the previous var */
        mlua_ast_expression_list_append(new_stmt->u.stmt_assignment->varlist, new_exp);
        new_exp = NULL;

        if ((ret = mlua_parse_statement_assignment(err, \
                        new_stmt, \
                        &token_cur)) != 0)
        { goto fail; }
    }
    else
    {
        if ((new_stmt = mlua_ast_statement_new(MLUA_AST_STATEMENT_TYPE_EXPR)) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        if ((new_stmt->u.stmt_expr = mlua_ast_statement_expr_new()) == NULL)
        { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
        new_stmt->u.stmt_expr->expr = new_exp; new_exp = NULL;
    }

    *stmt_out = new_stmt;

    goto done;
fail:
    if (new_stmt != NULL) { mlua_ast_statement_destroy(new_stmt); }
    if (new_exp != NULL) { mlua_ast_expression_destroy(new_exp); }
done:
    *token_cur_io = token_cur;
    return ret;
}

static int mlua_parse_statement(struct multiple_error *err, \
        struct mlua_ast_statement **stmt_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_statement *new_stmt = NULL;

    if (token_cur->value == TOKEN_KEYWORD_IF)
    {
        if ((ret = mlua_parse_statement_if(err, \
                        &new_stmt, \
                        &token_cur)) != 0)
        { goto fail; }
    }
    else if (token_cur->value == TOKEN_KEYWORD_WHILE)
    {
        if ((ret = mlua_parse_statement_while(err, \
                        &new_stmt, \
                        &token_cur)) != 0)
        { goto fail; }
    }
    else if (token_cur->value == TOKEN_KEYWORD_REPEAT)
    {
        if ((ret = mlua_parse_statement_repeat(err, \
                        &new_stmt, \
                        &token_cur)) != 0)
        { goto fail; }
    }
    else if (token_cur->value == TOKEN_KEYWORD_DO)
    {
        if ((ret = mlua_parse_statement_do(err, \
                        &new_stmt, \
                        &token_cur)) != 0)
        { goto fail; }
    }
    else if (token_cur->value == TOKEN_KEYWORD_FOR)
    {
        if ((ret = mlua_parse_statement_for(err, \
                        &new_stmt, \
                        &token_cur)) != 0)
        { goto fail; }
    }
    else if (token_cur->value == TOKEN_KEYWORD_BREAK)
    {
        if ((ret = mlua_parse_statement_break(err, \
                        &new_stmt, \
                        &token_cur)) != 0)
        { goto fail; }
    }
    else if (token_cur->value == TOKEN_OP_DBL_COLON)
    {
        if ((ret = mlua_parse_statement_label(err, \
                        &new_stmt, \
                        &token_cur)) != 0)
        { goto fail; }
    }
    else if (token_cur->value == TOKEN_KEYWORD_GOTO)
    {
        if ((ret = mlua_parse_statement_goto(err, \
                        &new_stmt, \
                        &token_cur)) != 0)
        { goto fail; }
    }
    else if (token_cur->value == TOKEN_KEYWORD_LOCAL)
    {
        if ((token_cur->next != NULL) && \
                (token_cur->next->value == TOKEN_KEYWORD_FUNCTION))
        {
            /* Local Function */
            if ((ret = mlua_parse_statement_fundef(err, \
                            &new_stmt, \
                            &token_cur, \
                            MLUA_PARSE_STATEMENT_FUNDEF_LOCAL)) != 0)
            { goto fail; }
        }
        else
        {
            if ((ret = mlua_parse_statement_local(err, \
                            &new_stmt, \
                            &token_cur)) != 0)
            { goto fail; }
        }
    }
    else if (token_cur->value == TOKEN_KEYWORD_FUNCTION)
    {
        /* Global Function */
        if ((ret = mlua_parse_statement_fundef(err, \
                        &new_stmt, \
                        &token_cur, \
                        MLUA_PARSE_STATEMENT_FUNDEF_GLOBAL)) != 0)
        { goto fail; }
    }
    else if (token_cur->value == TOKEN_KEYWORD_RETURN)
    {
        if ((ret = mlua_parse_statement_return(err, \
                        &new_stmt, \
                        &token_cur)) != 0)
        { goto fail; }
    }
    else 
    {
        if ((ret = mlua_parse_statement_expr(err, \
                        &new_stmt, \
                        &token_cur)) != 0)
        { goto fail; }
    }

    *stmt_out = new_stmt;

    goto done;
fail:
    if (new_stmt != NULL) { mlua_ast_statement_destroy(new_stmt); }
done:
    *token_cur_io = token_cur;
    return ret;
}

static int mlua_parse_statement_list(struct multiple_error *err, \
        struct mlua_ast_statement_list **stmt_list_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_statement_list *new_stmt_list = NULL;
    struct mlua_ast_statement *new_stmt = NULL;

    if ((new_stmt_list = mlua_ast_statement_list_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    while ((token_cur != NULL) && (token_cur->value != TOKEN_FINISH))
    {
        if ((token_cur->value == TOKEN_KEYWORD_END) || \
                (token_cur->value == TOKEN_KEYWORD_ELSEIF) || \
                (token_cur->value == TOKEN_KEYWORD_ELSE) || \
                (token_cur->value == TOKEN_KEYWORD_UNTIL) || \
                (token_cur->value == TOKEN_FINISH))
        {
            break;
        }

        if ((ret = mlua_parse_statement(err, &new_stmt, &token_cur)) != 0)
        { goto fail; }
        mlua_ast_statement_list_append(new_stmt_list, new_stmt);
        new_stmt = NULL;
    }

    *stmt_list_out = new_stmt_list;

    goto done;
fail:
    if (new_stmt_list != NULL) { mlua_ast_statement_list_destroy(new_stmt_list); }
    if (new_stmt != NULL) { mlua_ast_statement_destroy(new_stmt); }
done:
    *token_cur_io = token_cur;
    return ret;
}

static int mlua_parse_program(struct multiple_error *err, \
        struct mlua_ast_program **program_out, \
        struct token **token_cur_io)
{
    int ret = 0;
    struct token *token_cur = *token_cur_io;
    struct mlua_ast_program *new_program = NULL;

    if ((new_program = mlua_ast_program_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    if ((ret = mlua_parse_statement_list(err, \
                    &new_program->stmts, \
                    &token_cur)) != 0)
    { goto fail; }

    *program_out = new_program;

    goto done;
fail:
    if (new_program != NULL) { mlua_ast_program_destroy(new_program); }
done:
    *token_cur_io = token_cur;
    return ret;
}

int mlua_parse(struct multiple_error *err, \
        struct mlua_ast_program **program_out, \
        struct token_list *list)
{
    int ret = 0;
    struct mlua_ast_program *new_program = NULL;
    struct token *token_cur = list->begin;

    *program_out = NULL;

    if ((ret = mlua_parse_program(err, \
                    &new_program, \
                    &token_cur)) != 0)
    { goto fail; }

    *program_out = new_program;

    goto done;
fail:
    if (new_program != NULL) { mlua_ast_program_destroy(new_program); }
done:
    return ret;
}

