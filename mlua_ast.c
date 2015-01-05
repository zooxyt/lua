/* Multiple Lua Programming Language : Abstract Syntax Tree
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

#include <stdlib.h>
#include <stdio.h>

#include "multiple_err.h"
#include "mlua_lexer.h"
#include "mlua_ast.h"


/* Parameter */

struct mlua_ast_par *mlua_ast_par_new(void)
{
    struct mlua_ast_par *new_par = NULL;

    new_par = (struct mlua_ast_par *)malloc(sizeof(struct mlua_ast_par));
    if (new_par == NULL) { goto fail; }
    new_par->name = NULL;
    new_par->prev = new_par->next = NULL;

    goto done;
fail:
    if (new_par != NULL)
    {
        mlua_ast_par_destroy(new_par);
        new_par = NULL;
    }
done:
    return new_par;
}

int mlua_ast_par_destroy(struct mlua_ast_par *par)
{
    if (par->name != NULL) token_destroy(par->name);
    free(par);

    return 0;
}

struct mlua_ast_par_list *mlua_ast_par_list_new(void)
{
    struct mlua_ast_par_list *new_par_list = NULL;

    new_par_list = (struct mlua_ast_par_list *)malloc(sizeof(struct mlua_ast_par_list));
    if (new_par_list == NULL) { goto fail; }
    new_par_list->begin = new_par_list->end = NULL;
    new_par_list->size = 0;

    goto done;
fail:
done:
    return new_par_list;
}

int mlua_ast_par_list_destroy(struct mlua_ast_par_list *par_list)
{
    struct mlua_ast_par *par_cur, *par_next;

    par_cur = par_list->begin;
    while (par_cur != NULL)
    {
        par_next = par_cur->next; 
        mlua_ast_par_destroy(par_cur);
        par_cur = par_next; 
    }
    free(par_list);

    return 0;
}

int mlua_ast_par_list_append(struct mlua_ast_par_list *par_list, \
        struct mlua_ast_par *new_par)
{
    if (par_list->begin == NULL)
    {
        par_list->begin = par_list->end = new_par;
    }
    else
    {
        new_par->prev = par_list->end;
        par_list->end->next = new_par;
        par_list->end = new_par;
    }
    par_list->size += 1;

    return 0;
}


/* Arguments */

struct mlua_ast_args *mlua_ast_args_new(enum mlua_ast_args_type type)
{
    struct mlua_ast_args *new_args = NULL;

    new_args = (struct mlua_ast_args *)malloc(sizeof(struct mlua_ast_args));
    if (new_args == NULL) { goto fail; }
    new_args->type = MLUA_AST_ARGS_TYPE_UNKNOWN;
    switch (type)
    {
        case MLUA_AST_ARGS_TYPE_STRING:
            new_args->u.str = NULL;
            break;
        case MLUA_AST_ARGS_TYPE_EXPLIST:
            new_args->u.explist = NULL;
            break;
        case MLUA_AST_ARGS_TYPE_TBLCTOR:
            new_args->u.tblctor = NULL;
            break;
        case MLUA_AST_ARGS_TYPE_UNKNOWN:
            goto fail;
    }
    new_args->type = type;

    goto done;
fail:
done:
    return new_args;
}

int mlua_ast_args_destroy(struct mlua_ast_args *args)
{
    switch (args->type)
    {
        case MLUA_AST_ARGS_TYPE_STRING:
            if (args->u.str != NULL) 
            { token_destroy(args->u.str); }
            break;
        case MLUA_AST_ARGS_TYPE_EXPLIST:
            if (args->u.explist != NULL) 
            { mlua_ast_expression_list_destroy(args->u.explist); }
            break;
        case MLUA_AST_ARGS_TYPE_TBLCTOR:
            if (args->u.tblctor != NULL) 
            { mlua_ast_expression_tblctor_destroy(args->u.tblctor); }
            break;

        case MLUA_AST_ARGS_TYPE_UNKNOWN:
            break;
    }
    free(args);

    return 0;
}


/* Expression : Table Constructor */

struct mlua_ast_field_array *mlua_ast_field_array_new(void)
{
    struct mlua_ast_field_array *new_field_array = NULL;

    new_field_array = (struct mlua_ast_field_array *)malloc(sizeof(struct mlua_ast_field_array));
    if (new_field_array == NULL) { goto fail; }
    new_field_array->index = NULL;
    new_field_array->value = NULL;

    goto done;
fail:
    if (new_field_array != NULL)
    {
        mlua_ast_field_array_destroy(new_field_array);
        new_field_array = NULL;
    }
done:
    return new_field_array;
}

int mlua_ast_field_array_destroy(struct mlua_ast_field_array *field_array)
{
    if (field_array->index != NULL) mlua_ast_expression_destroy(field_array->index);
    if (field_array->value != NULL) mlua_ast_expression_destroy(field_array->value);
    free(field_array);

    return 0;
}

struct mlua_ast_field_property *mlua_ast_field_property_new(void)
{
    struct mlua_ast_field_property *new_field_property = NULL;

    new_field_property = (struct mlua_ast_field_property *)malloc(sizeof(struct mlua_ast_field_property));
    if (new_field_property == NULL) { goto fail; }
    new_field_property->name = NULL;
    new_field_property->value = NULL;

    goto done;
fail:
    if (new_field_property != NULL)
    {
        mlua_ast_field_property_destroy(new_field_property);
        new_field_property = NULL;
    }
done:
    return new_field_property;
}

int mlua_ast_field_property_destroy(struct mlua_ast_field_property *field_property)
{
    if (field_property->name != NULL) mlua_ast_name_destroy(field_property->name);
    if (field_property->value != NULL) mlua_ast_expression_destroy(field_property->value);
    free(field_property);

    return 0;
}

struct mlua_ast_field_exp *mlua_ast_field_exp_new(void)
{
    struct mlua_ast_field_exp *new_field_exp = NULL;

    new_field_exp = (struct mlua_ast_field_exp *)malloc(sizeof(struct mlua_ast_field_exp));
    if (new_field_exp == NULL) { goto fail; }
    new_field_exp->value = NULL;

    goto done;
fail:
    if (new_field_exp != NULL)
    {
        mlua_ast_field_exp_destroy(new_field_exp);
        new_field_exp = NULL;
    }
done:
    return new_field_exp;
}

int mlua_ast_field_exp_destroy(struct mlua_ast_field_exp *field_exp)
{
    if (field_exp->value != NULL) mlua_ast_expression_destroy(field_exp->value);
    free(field_exp);

    return 0;
}

struct mlua_ast_field *mlua_ast_field_new(enum mlua_ast_field_type type)
{
    struct mlua_ast_field *new_field = NULL;

    new_field = (struct mlua_ast_field *)malloc(sizeof(struct mlua_ast_field));
    if (new_field == NULL) { goto fail; }
    new_field->next = NULL;
    new_field->prev = NULL;
    new_field->type = MLUA_AST_FIELD_TYPE_UNKNOWN;
    switch (type)
    {
        case MLUA_AST_FIELD_TYPE_ARRAY:
            new_field->u.array = NULL;
            break;
        case MLUA_AST_FIELD_TYPE_PROPERTY:
            new_field->u.property = NULL;
            break;
        case MLUA_AST_FIELD_TYPE_EXP:
            new_field->u.exp = NULL;
            break;

        case MLUA_AST_FIELD_TYPE_UNKNOWN:
            goto fail;
    }
    new_field->type = type;

    goto done;
fail:
    if (new_field != NULL)
    {
        mlua_ast_field_destroy(new_field);
        new_field = NULL;
    }
done:
    return new_field;
}

int mlua_ast_field_destroy(struct mlua_ast_field *field)
{
    switch (field->type)
    {
        case MLUA_AST_FIELD_TYPE_ARRAY:
            mlua_ast_field_array_destroy(field->u.array);
            break;
        case MLUA_AST_FIELD_TYPE_PROPERTY:
            mlua_ast_field_property_destroy(field->u.property);
            break;
        case MLUA_AST_FIELD_TYPE_EXP:
            mlua_ast_field_exp_destroy(field->u.exp);
            break;

        case MLUA_AST_FIELD_TYPE_UNKNOWN:
            break;
    }
    free(field);

    return 0;
}

struct mlua_ast_fieldlist *mlua_ast_fieldlist_new(void)
{
    struct mlua_ast_fieldlist *new_fieldlist = NULL;

    new_fieldlist = (struct mlua_ast_fieldlist *)malloc(sizeof(struct mlua_ast_fieldlist));
    if (new_fieldlist == NULL) { goto fail; }
    new_fieldlist->begin = new_fieldlist->end = NULL;
    new_fieldlist->size = 0;

    goto done;
fail:
    if (new_fieldlist != NULL)
    {
        mlua_ast_fieldlist_destroy(new_fieldlist);
        new_fieldlist = NULL;
    }
done:
    return new_fieldlist;
}

int mlua_ast_fieldlist_destroy(struct mlua_ast_fieldlist *fieldlist)
{
    struct mlua_ast_field *field_cur, *field_next;

    field_cur = fieldlist->begin;
    while (field_cur != NULL)
    {
        field_next = field_cur->next; 
        mlua_ast_field_destroy(field_cur);
        field_cur = field_next; 
    }
    free(fieldlist);

    return 0;
}

int mlua_ast_fieldlist_append(struct mlua_ast_fieldlist *fieldlist, \
        struct mlua_ast_field *new_field)
{
    if (fieldlist->begin == NULL)
    {
        fieldlist->begin = fieldlist->end = new_field;
    }
    else
    {
        new_field->prev = fieldlist->end;
        fieldlist->end->next = new_field;
        fieldlist->end = new_field;
    }
    fieldlist->size += 1;

    return 0;
}

struct mlua_ast_expression_tblctor *mlua_ast_expression_tblctor_new(void)
{
    struct mlua_ast_expression_tblctor *new_exp_tblctor = NULL;

    new_exp_tblctor = (struct mlua_ast_expression_tblctor *)malloc(sizeof(struct mlua_ast_expression_tblctor));
    if (new_exp_tblctor == NULL) { goto fail; }
    new_exp_tblctor->fieldlist = NULL;

    goto done;
fail:
    if (new_exp_tblctor != NULL)
    {
        mlua_ast_expression_tblctor_destroy(new_exp_tblctor);
        new_exp_tblctor = NULL;
    }
done:
    return new_exp_tblctor;
}

int mlua_ast_expression_tblctor_destroy(struct mlua_ast_expression_tblctor *tblctor)
{
    if (tblctor->fieldlist != NULL)
    {
        mlua_ast_fieldlist_destroy(tblctor->fieldlist);
    }
    free(tblctor);

    return 0;
}


/* Expression : Prefix */

struct mlua_ast_expression_prefix *mlua_ast_expression_prefix_new(enum mlua_ast_expression_prefix_type type)
{
    struct mlua_ast_expression_prefix *new_exp_prefix = NULL;

    new_exp_prefix = (struct mlua_ast_expression_prefix *)malloc(sizeof(struct mlua_ast_expression_prefix));
    if (new_exp_prefix == NULL) { goto fail; }
    new_exp_prefix->type = MLUA_AST_PREFIX_EXP_TYPE_UNKNOWN;

    switch (type)
    {
        case MLUA_AST_PREFIX_EXP_TYPE_VAR:
            new_exp_prefix->u.var = NULL;
            break;
        case MLUA_AST_PREFIX_EXP_TYPE_FUNCALL:
            new_exp_prefix->u.funcall = NULL;
            break;
        case MLUA_AST_PREFIX_EXP_TYPE_EXP:
            new_exp_prefix->u.exp = NULL;
            break;
        case MLUA_AST_PREFIX_EXP_TYPE_UNKNOWN:
            goto fail;
    }
    new_exp_prefix->type = type;

    goto done;
fail:
    if (new_exp_prefix != NULL)
    {
        mlua_ast_expression_prefix_destroy(new_exp_prefix);
        new_exp_prefix = NULL;
    }
done:
    return new_exp_prefix;
}

int mlua_ast_expression_prefix_destroy(struct mlua_ast_expression_prefix *exp_prefix)
{
    switch (exp_prefix->type)
    {
        case MLUA_AST_PREFIX_EXP_TYPE_VAR:
            if (exp_prefix->u.var != NULL) token_destroy(exp_prefix->u.var);
            break;
        case MLUA_AST_PREFIX_EXP_TYPE_FUNCALL:
            if (exp_prefix->u.funcall != NULL) mlua_ast_expression_funcall_destroy(exp_prefix->u.funcall);
            break;
        case MLUA_AST_PREFIX_EXP_TYPE_EXP:
            if (exp_prefix->u.exp != NULL) mlua_ast_expression_destroy(exp_prefix->u.exp);
            break;
        case MLUA_AST_PREFIX_EXP_TYPE_UNKNOWN:
            break;
    }
    free(exp_prefix);

    return 0;
}


/* Primary Expression */

struct mlua_ast_expression_primary *mlua_ast_expression_primary_new(enum mlua_ast_expression_primary_type type)
{
    struct mlua_ast_expression_primary *new_exp_primary = NULL;

    new_exp_primary = (struct mlua_ast_expression_primary *)malloc(sizeof(struct mlua_ast_expression_primary));
    if (new_exp_primary == NULL) { goto fail; }
    new_exp_primary->type = MLUA_AST_EXPRESSION_PRIMARY_TYPE_UNKNOWN;
    switch (type)
    {
        case MLUA_AST_EXPRESSION_PRIMARY_TYPE_NAME:
            new_exp_primary->u.name = NULL;
            break;
        case MLUA_AST_EXPRESSION_PRIMARY_TYPE_EXPR:
            new_exp_primary->u.exp = NULL;
            break;

        case MLUA_AST_EXPRESSION_PRIMARY_TYPE_UNKNOWN:
            goto fail;
    }
    new_exp_primary->type = type;

    goto done;
fail:
    if (new_exp_primary != NULL)
    {
        mlua_ast_expression_primary_destroy(new_exp_primary);
        new_exp_primary = NULL;
    }
done:
    return new_exp_primary;
}

int mlua_ast_expression_primary_destroy(struct mlua_ast_expression_primary *stmt_primary)
{
    switch (stmt_primary->type)
    {
        case MLUA_AST_EXPRESSION_PRIMARY_TYPE_NAME:
            if (stmt_primary->u.name != NULL) token_destroy(stmt_primary->u.name);
            break;
        case MLUA_AST_EXPRESSION_PRIMARY_TYPE_EXPR:
            if (stmt_primary->u.exp != NULL) mlua_ast_expression_destroy(stmt_primary->u.exp);
            break;

        case MLUA_AST_EXPRESSION_PRIMARY_TYPE_UNKNOWN:
            break;
    }
    free(stmt_primary);

    return 0;
}


/* Suffixed Expression */

struct mlua_ast_expression_suffixed *mlua_ast_expression_suffixed_new(enum mlua_ast_expression_suffixed_type type)
{
    struct mlua_ast_expression_suffixed *new_exp_suffixed = NULL;

    new_exp_suffixed = (struct mlua_ast_expression_suffixed *)malloc(sizeof(struct mlua_ast_expression_suffixed));
    if (new_exp_suffixed == NULL) { goto fail; }
    new_exp_suffixed->sub = NULL;
    new_exp_suffixed->type = MLUA_AST_EXPRESSION_SUFFIXED_TYPE_UNKNOWN;
    switch (type)
    {
        case MLUA_AST_EXPRESSION_SUFFIXED_TYPE_MEMBER:
            break;
        case MLUA_AST_EXPRESSION_SUFFIXED_TYPE_INDEX:
            break;

        case MLUA_AST_EXPRESSION_SUFFIXED_TYPE_UNKNOWN:
            goto fail;
    }
    new_exp_suffixed->type = type;

    goto done;
fail:
    if (new_exp_suffixed != NULL)
    {
        mlua_ast_expression_suffixed_destroy(new_exp_suffixed);
        new_exp_suffixed = NULL;
    }
done:
    return new_exp_suffixed;
}

int mlua_ast_expression_suffixed_destroy(struct mlua_ast_expression_suffixed *stmt_suffixed)
{
    switch (stmt_suffixed->type)
    {
        case MLUA_AST_EXPRESSION_SUFFIXED_TYPE_MEMBER:
            if (stmt_suffixed->u.name != NULL) token_destroy(stmt_suffixed->u.name);
            break;
        case MLUA_AST_EXPRESSION_SUFFIXED_TYPE_INDEX:
            if (stmt_suffixed->u.exp != NULL) mlua_ast_expression_destroy(stmt_suffixed->u.exp);
            break;

        case MLUA_AST_EXPRESSION_SUFFIXED_TYPE_UNKNOWN:
            break;
    }
    if (stmt_suffixed->sub != NULL) mlua_ast_expression_destroy(stmt_suffixed->sub);
    free(stmt_suffixed);

    return 0;
}


/* Expression : Function Call */

struct mlua_ast_expression_funcall *mlua_ast_expression_funcall_new(void)
{
    struct mlua_ast_expression_funcall *new_stmt_funcall = NULL;

    new_stmt_funcall = (struct mlua_ast_expression_funcall *)malloc(sizeof(struct mlua_ast_expression_funcall));
    if (new_stmt_funcall == NULL) { goto fail; }
    new_stmt_funcall->prefixexp = NULL;
    new_stmt_funcall->args = NULL;

    goto done;
fail:
    if (new_stmt_funcall != NULL)
    {
        mlua_ast_expression_funcall_destroy(new_stmt_funcall); 
        new_stmt_funcall = NULL;
    }
done:
    return new_stmt_funcall;
}

int mlua_ast_expression_funcall_destroy(struct mlua_ast_expression_funcall *stmt_funcall)
{
    if (stmt_funcall->prefixexp != NULL) { mlua_ast_expression_destroy(stmt_funcall->prefixexp); }
    if (stmt_funcall->args != NULL) { mlua_ast_args_destroy(stmt_funcall->args); }
    free(stmt_funcall);

    return 0;
}


/* Expression : Factor */

struct mlua_ast_expression_factor *mlua_ast_expression_factor_new( \
        enum mlua_ast_expression_factor_type type)
{
    struct mlua_ast_expression_factor *new_exp_factor = NULL;

    new_exp_factor = (struct mlua_ast_expression_factor *)malloc(sizeof(struct mlua_ast_expression_factor));
    if (new_exp_factor == NULL) { goto fail; }
    new_exp_factor->type = MLUA_AST_EXP_FACTOR_TYPE_UNKNOWN;
    switch (type)
    {
        case MLUA_AST_EXP_FACTOR_TYPE_NIL:
        case MLUA_AST_EXP_FACTOR_TYPE_FALSE:
        case MLUA_AST_EXP_FACTOR_TYPE_TRUE:
        case MLUA_AST_EXP_FACTOR_TYPE_INTEGER:
        case MLUA_AST_EXP_FACTOR_TYPE_FLOAT:
        case MLUA_AST_EXP_FACTOR_TYPE_STRING:
            new_exp_factor->token = NULL;
            break;
        case MLUA_AST_EXP_FACTOR_TYPE_UNKNOWN:
            goto fail;
    }
    new_exp_factor->type = type;

    goto done;
fail:
    if (new_exp_factor != NULL)
    {
        mlua_ast_expression_factor_destroy(new_exp_factor);
        new_exp_factor = NULL;
    }
done:
    return new_exp_factor;
}

int mlua_ast_expression_factor_destroy(struct mlua_ast_expression_factor *exp_factor)
{
    switch (exp_factor->type)
    {
        case MLUA_AST_EXP_FACTOR_TYPE_NIL:
        case MLUA_AST_EXP_FACTOR_TYPE_FALSE:
        case MLUA_AST_EXP_FACTOR_TYPE_TRUE:
        case MLUA_AST_EXP_FACTOR_TYPE_INTEGER:
        case MLUA_AST_EXP_FACTOR_TYPE_FLOAT:
        case MLUA_AST_EXP_FACTOR_TYPE_STRING:
            if (exp_factor->token != NULL) token_destroy(exp_factor->token);
            break;
        case MLUA_AST_EXP_FACTOR_TYPE_UNKNOWN:
            break;
    }
    free(exp_factor);

    return 0;
}


/* Expression : Unary Operation */

struct mlua_ast_expression_unop *mlua_ast_expression_unop_new(void)
{
    struct mlua_ast_expression_unop *new_exp_unop = NULL;

    new_exp_unop = (struct mlua_ast_expression_unop *)malloc(sizeof(struct mlua_ast_expression_unop));
    if (new_exp_unop == NULL) { goto fail; }
    new_exp_unop->op = NULL;
    new_exp_unop->sub = NULL;

    goto done;
fail:
    if (new_exp_unop != NULL) 
    {
        mlua_ast_expression_unop_destroy(new_exp_unop);
        new_exp_unop = NULL; 
    }
done:
    return new_exp_unop;
}

int mlua_ast_expression_unop_destroy(struct mlua_ast_expression_unop *stmt_unop)
{
    if (stmt_unop->op != NULL) token_destroy(stmt_unop->op);
    if (stmt_unop->sub != NULL) mlua_ast_expression_destroy(stmt_unop->sub);
    free(stmt_unop);

    return 0;
}


/* Expression : Binary Operation */

struct mlua_ast_expression_binop *mlua_ast_expression_binop_new(void)
{
    struct mlua_ast_expression_binop *new_exp_binop = NULL;

    new_exp_binop = (struct mlua_ast_expression_binop *)malloc(sizeof(struct mlua_ast_expression_binop));
    if (new_exp_binop == NULL) { goto fail; }
    new_exp_binop->op = NULL;
    new_exp_binop->left = NULL;
    new_exp_binop->right = NULL;

    goto done;
fail:
    if (new_exp_binop != NULL) 
    {
        mlua_ast_expression_binop_destroy(new_exp_binop);
        new_exp_binop = NULL; 
    }
done:
    return new_exp_binop;
}

int mlua_ast_expression_binop_destroy(struct mlua_ast_expression_binop *stmt_binop)
{
    if (stmt_binop->op != NULL) token_destroy(stmt_binop->op);
    if (stmt_binop->left != NULL) mlua_ast_expression_destroy(stmt_binop->left);
    if (stmt_binop->right != NULL) mlua_ast_expression_destroy(stmt_binop->right);
    free(stmt_binop);

    return 0;
}


/* Expression : Function Definition */

struct mlua_ast_expression_fundef *mlua_ast_expression_fundef_new(void)
{
    struct mlua_ast_expression_fundef *new_exp_fundef = NULL;

    new_exp_fundef = (struct mlua_ast_expression_fundef *)malloc(sizeof(struct mlua_ast_expression_fundef));
    if (new_exp_fundef == NULL) { goto fail; }
    new_exp_fundef->body = NULL;
    new_exp_fundef->pars = NULL;

    goto done;
fail:
    if (new_exp_fundef != NULL) 
    {
        mlua_ast_expression_fundef_destroy(new_exp_fundef);
        new_exp_fundef = NULL; 
    }
done:
    return new_exp_fundef;
}

int mlua_ast_expression_fundef_destroy(struct mlua_ast_expression_fundef *stmt_fundef)
{
    if (stmt_fundef->body != NULL) mlua_ast_statement_list_destroy(stmt_fundef->body);
    if (stmt_fundef->pars != NULL) mlua_ast_par_list_destroy(stmt_fundef->pars);
    free(stmt_fundef);

    return 0;
}


/* Expression */

struct mlua_ast_expression *mlua_ast_expression_new(enum mlua_ast_expression_type type)
{
    struct mlua_ast_expression *new_exp = NULL;

    new_exp = (struct mlua_ast_expression *)malloc(sizeof(struct mlua_ast_expression));
    if (new_exp == NULL) { goto fail; }
    new_exp->next = new_exp->prev = NULL;
    new_exp->type = MLUA_AST_EXPRESSION_TYPE_UNKNOWN;
    switch (type)
    {
        case MLUA_AST_EXPRESSION_TYPE_PREFIX:
            new_exp->u.prefix = NULL;
            break;

        case MLUA_AST_EXPRESSION_TYPE_FACTOR:
            new_exp->u.factor = NULL;
            break;

        case MLUA_AST_EXPRESSION_TYPE_PRIMARY:
            new_exp->u.primary = NULL;
            break;

        case MLUA_AST_EXPRESSION_TYPE_SUFFIXED:
            new_exp->u.suffixed = NULL;
            break;

        case MLUA_AST_EXPRESSION_TYPE_TBLCTOR:
            new_exp->u.tblctor = NULL;
            break;

        case MLUA_AST_EXPRESSION_TYPE_FUNCALL:
            new_exp->u.funcall = NULL;
            break;

        case MLUA_AST_EXPRESSION_TYPE_FUNDEF:
            new_exp->u.fundef = NULL;
            break;

        case MLUA_AST_EXPRESSION_TYPE_UNOP:
            new_exp->u.unop = NULL;
            break;

        case MLUA_AST_EXPRESSION_TYPE_BINOP:
            new_exp->u.binop = NULL;
            break;

        case MLUA_AST_EXPRESSION_TYPE_UNKNOWN:
            goto fail;
    }
    new_exp->type = type;

    goto done;
fail:
    mlua_ast_expression_destroy(new_exp);
done:
    return new_exp;
}

int mlua_ast_expression_destroy(struct mlua_ast_expression *exp)
{
    switch (exp->type)
    {
        case MLUA_AST_EXPRESSION_TYPE_PREFIX:
            if (exp->u.prefix != NULL) { mlua_ast_expression_prefix_destroy(exp->u.prefix); }
            break;

        case MLUA_AST_EXPRESSION_TYPE_FACTOR:
            if (exp->u.factor != NULL) { mlua_ast_expression_factor_destroy(exp->u.factor); }
            break;

        case MLUA_AST_EXPRESSION_TYPE_PRIMARY:
            if (exp->u.primary != NULL) { mlua_ast_expression_primary_destroy(exp->u.primary); }
            break;

        case MLUA_AST_EXPRESSION_TYPE_SUFFIXED:
            if (exp->u.suffixed != NULL) { mlua_ast_expression_suffixed_destroy(exp->u.suffixed); }
            break;

        case MLUA_AST_EXPRESSION_TYPE_TBLCTOR:
            if (exp->u.tblctor != NULL) { mlua_ast_expression_tblctor_destroy(exp->u.tblctor); }
            break;

        case MLUA_AST_EXPRESSION_TYPE_FUNCALL:
            if (exp->u.funcall != NULL) { mlua_ast_expression_funcall_destroy(exp->u.funcall); }
            break;

        case MLUA_AST_EXPRESSION_TYPE_FUNDEF:
            if (exp->u.fundef != NULL) { mlua_ast_expression_fundef_destroy(exp->u.fundef); }
            break;

        case MLUA_AST_EXPRESSION_TYPE_UNOP:
            if (exp->u.unop != NULL) { mlua_ast_expression_unop_destroy(exp->u.unop); }
            break;

        case MLUA_AST_EXPRESSION_TYPE_BINOP:
            if (exp->u.binop != NULL) { mlua_ast_expression_binop_destroy(exp->u.binop); }
            break;

        case MLUA_AST_EXPRESSION_TYPE_UNKNOWN:
            break;
    }
    free(exp);

    return 0;
}


/* Expression List */

struct mlua_ast_expression_list *mlua_ast_expression_list_new(void)
{
    struct mlua_ast_expression_list *new_list = NULL;

    new_list = (struct mlua_ast_expression_list *)malloc(sizeof(struct mlua_ast_expression_list));
    if (new_list == NULL) { goto fail; }
    new_list->begin = new_list->end = NULL;
    new_list->size = 0;

    goto done;
fail:
done:
    return new_list;
}

int mlua_ast_expression_list_destroy(struct mlua_ast_expression_list *list)
{
    struct mlua_ast_expression *exp_cur, *exp_next;

    exp_cur = list->begin;
    while (exp_cur != NULL)
    {
        exp_next = exp_cur->next;
        mlua_ast_expression_destroy(exp_cur);
        exp_cur = exp_next;
    }
    free(list);

    return 0;
}

int mlua_ast_expression_list_append(struct mlua_ast_expression_list *list, \
        struct mlua_ast_expression *new_exp)
{
    if (list->begin == NULL)
    {
        list->begin = list->end = new_exp;
    }
    else
    {
        new_exp->prev = list->end;
        list->end->next = new_exp;
        list->end = new_exp;
    }
    list->size += 1;

    return 0;
}


/* Statement : assignment */
/* varlist ‘=’ explist */

struct mlua_ast_statement_assignment *mlua_ast_statement_assignment_new(void)
{
    struct mlua_ast_statement_assignment *new_stmt_assign = NULL;

    new_stmt_assign = (struct mlua_ast_statement_assignment *)malloc(sizeof(struct mlua_ast_statement_assignment));
    if (new_stmt_assign == NULL) { goto fail; }
    new_stmt_assign->varlist = NULL;
    new_stmt_assign->explist = NULL;

    goto done;
fail:
    if (new_stmt_assign != NULL)
    {
        mlua_ast_statement_assignment_destroy(new_stmt_assign);
        new_stmt_assign = NULL;
    }
done:
    return new_stmt_assign;
}

int mlua_ast_statement_assignment_destroy(struct mlua_ast_statement_assignment *stmt_assign)
{
    if (stmt_assign->varlist != NULL) mlua_ast_expression_list_destroy(stmt_assign->varlist);
    if (stmt_assign->explist != NULL) mlua_ast_expression_list_destroy(stmt_assign->explist);
    free(stmt_assign);

    return 0;
}

/* Statement : expr */

struct mlua_ast_statement_expr *mlua_ast_statement_expr_new(void)
{
    struct mlua_ast_statement_expr *new_stmt_expr = NULL;

    new_stmt_expr = (struct mlua_ast_statement_expr *)malloc(sizeof(struct mlua_ast_statement_expr));
    if (new_stmt_expr == NULL) { goto fail; }
    new_stmt_expr->expr = NULL;

    goto done;
fail:
    if (new_stmt_expr != NULL)
    {
        mlua_ast_statement_expr_destroy(new_stmt_expr);
        new_stmt_expr = NULL;
    }
done:
    return new_stmt_expr;
}

int mlua_ast_statement_expr_destroy(struct mlua_ast_statement_expr *stmt_expr)
{
    if (stmt_expr->expr != NULL) mlua_ast_expression_destroy(stmt_expr->expr);
    free(stmt_expr);

    return 0;
}


/* Statement : if */

struct mlua_ast_statement_elseif *mlua_ast_statement_elseif_new(void)
{
    struct mlua_ast_statement_elseif *new_stmt_elseif = NULL;

    new_stmt_elseif = (struct mlua_ast_statement_elseif *)malloc(sizeof(struct mlua_ast_statement_elseif));
    if (new_stmt_elseif == NULL) { goto fail; }
    new_stmt_elseif->exp = NULL;
    new_stmt_elseif->block_then = NULL;
    new_stmt_elseif->elseif = NULL;

    goto done;
fail:
    if (new_stmt_elseif != NULL)
    {
        mlua_ast_statement_elseif_destroy(new_stmt_elseif);
        new_stmt_elseif = NULL;
    }
done:
    return new_stmt_elseif;
}

int mlua_ast_statement_elseif_destroy(struct mlua_ast_statement_elseif *stmt_elseif)
{
    if (stmt_elseif->exp != NULL) mlua_ast_expression_destroy(stmt_elseif->exp);
    if (stmt_elseif->block_then) mlua_ast_statement_list_destroy(stmt_elseif->block_then);
    if (stmt_elseif->elseif) mlua_ast_statement_elseif_destroy(stmt_elseif->elseif);
    free(stmt_elseif);

    return 0;
}

struct mlua_ast_statement_if *mlua_ast_statement_if_new(void)
{
    struct mlua_ast_statement_if *new_stmt_if = NULL;

    new_stmt_if = (struct mlua_ast_statement_if *)malloc(sizeof(struct mlua_ast_statement_if));
    if (new_stmt_if == NULL) { goto fail; }
    new_stmt_if->exp = NULL;
    new_stmt_if->block_then = NULL;
    new_stmt_if->elseif = NULL;
    new_stmt_if->block_else = NULL;

    goto done;
fail:
    if (new_stmt_if != NULL)
    {
        mlua_ast_statement_if_destroy(new_stmt_if);
        new_stmt_if = NULL;
    }
done:
    return new_stmt_if;
}

int mlua_ast_statement_if_destroy(struct mlua_ast_statement_if *stmt_if)
{
    if (stmt_if->exp != NULL) mlua_ast_expression_destroy(stmt_if->exp);
    if (stmt_if->block_then) mlua_ast_statement_list_destroy(stmt_if->block_then);
    if (stmt_if->elseif) mlua_ast_statement_elseif_destroy(stmt_if->elseif);
    if (stmt_if->block_else) mlua_ast_statement_list_destroy(stmt_if->block_else);
    free(stmt_if);

    return 0;
}


/* Statement : while */

struct mlua_ast_statement_while *mlua_ast_statement_while_new(void)
{
    struct mlua_ast_statement_while *new_stmt_while = NULL;

    new_stmt_while = (struct mlua_ast_statement_while *)malloc(sizeof(struct mlua_ast_statement_while));
    if (new_stmt_while == NULL) { goto fail; }
    new_stmt_while->exp = NULL;
    new_stmt_while->block = NULL;

    goto done;
fail:
    if (new_stmt_while != NULL)
    {
        mlua_ast_statement_while_destroy(new_stmt_while);
        new_stmt_while = NULL;
    }
done:
    return new_stmt_while;
}

int mlua_ast_statement_while_destroy(struct mlua_ast_statement_while *stmt_while)
{
    if (stmt_while->exp != NULL) mlua_ast_expression_destroy(stmt_while->exp);
    if (stmt_while->block) mlua_ast_statement_list_destroy(stmt_while->block);
    free(stmt_while);

    return 0;
}


/* Statement : function */

struct mlua_ast_funcname *mlua_ast_funcname_new(void)
{
    struct mlua_ast_funcname *new_funcname = NULL;

    new_funcname = (struct mlua_ast_funcname *)malloc(sizeof(struct mlua_ast_funcname));
    if (new_funcname == NULL) { goto fail; }
    new_funcname->name_list = NULL;
    new_funcname->member = NULL;

    goto done;
fail:
    if (new_funcname != NULL)
    {
        mlua_ast_funcname_destroy(new_funcname);
        new_funcname = NULL;
    }
done:
    return new_funcname;
}

int mlua_ast_funcname_destroy(struct mlua_ast_funcname *funcname)
{
    if (funcname->name_list != NULL) mlua_ast_namelist_destroy(funcname->name_list);
    if (funcname->member != NULL) mlua_ast_name_destroy(funcname->member);
    free(funcname);

    return 0;
}

struct mlua_ast_statement_fundef *mlua_ast_statement_fundef_new(void)
{
    struct mlua_ast_statement_fundef *new_stmt_fundef = NULL;

    new_stmt_fundef = (struct mlua_ast_statement_fundef *)malloc(sizeof(struct mlua_ast_statement_fundef));
    if (new_stmt_fundef == NULL) { goto fail; }
    new_stmt_fundef->funcname = NULL;
    new_stmt_fundef->parameters = NULL;
    new_stmt_fundef->body = NULL;
    new_stmt_fundef->local = 0;

    goto done;
fail:
    if (new_stmt_fundef != NULL)
    {
        mlua_ast_statement_fundef_destroy(new_stmt_fundef);
        new_stmt_fundef = NULL;
    }
done:
    return new_stmt_fundef;
}

int mlua_ast_statement_fundef_destroy(struct mlua_ast_statement_fundef *stmt_fundef)
{
    if (stmt_fundef->funcname != NULL) mlua_ast_funcname_destroy(stmt_fundef->funcname);
    if (stmt_fundef->parameters != NULL) mlua_ast_par_list_destroy(stmt_fundef->parameters);
    if (stmt_fundef->body != NULL) mlua_ast_statement_list_destroy(stmt_fundef->body);
    free(stmt_fundef);

    return 0;
}


/* Statement : repeat */

struct mlua_ast_statement_repeat *mlua_ast_statement_repeat_new(void)
{
    struct mlua_ast_statement_repeat *new_stmt_repeat = NULL;

    new_stmt_repeat = (struct mlua_ast_statement_repeat *)malloc(sizeof(struct mlua_ast_statement_repeat));
    if (new_stmt_repeat == NULL) { goto fail; }
    new_stmt_repeat->exp = NULL;
    new_stmt_repeat->block = NULL;

    goto done;
fail:
    if (new_stmt_repeat != NULL)
    {
        mlua_ast_statement_repeat_destroy(new_stmt_repeat);
        new_stmt_repeat = NULL;
    }
done:
    return new_stmt_repeat;
}

int mlua_ast_statement_repeat_destroy(struct mlua_ast_statement_repeat *stmt_repeat)
{
    if (stmt_repeat->exp != NULL) mlua_ast_expression_destroy(stmt_repeat->exp);
    if (stmt_repeat->block) mlua_ast_statement_list_destroy(stmt_repeat->block);
    free(stmt_repeat);

    return 0;
}


/* Statement : do */

struct mlua_ast_statement_do *mlua_ast_statement_do_new(void)
{
    struct mlua_ast_statement_do *new_stmt_do = NULL;

    new_stmt_do = (struct mlua_ast_statement_do *)malloc(sizeof(struct mlua_ast_statement_do));
    if (new_stmt_do == NULL) { goto fail; }
    new_stmt_do->block = NULL;

    goto done;
fail:
    if (new_stmt_do != NULL)
    {
        mlua_ast_statement_do_destroy(new_stmt_do);
        new_stmt_do = NULL;
    }
done:
    return new_stmt_do;
}

int mlua_ast_statement_do_destroy(struct mlua_ast_statement_do *stmt_do)
{
    if (stmt_do->block) mlua_ast_statement_list_destroy(stmt_do->block);
    free(stmt_do);

    return 0;
}


/* Statement : for */

struct mlua_ast_statement_for *mlua_ast_statement_for_new(void)
{
    struct mlua_ast_statement_for *new_stmt_for = NULL;

    new_stmt_for = (struct mlua_ast_statement_for *)malloc(sizeof(struct mlua_ast_statement_for));
    if (new_stmt_for == NULL) { goto fail; }
    new_stmt_for->name = NULL;
    new_stmt_for->exp1 = NULL;
    new_stmt_for->exp2 = NULL;
    new_stmt_for->exp3 = NULL;
    new_stmt_for->block = NULL;

    goto forne;
fail:
    if (new_stmt_for != NULL)
    {
        mlua_ast_statement_for_destroy(new_stmt_for);
        new_stmt_for = NULL;
    }
forne:
    return new_stmt_for;
}

int mlua_ast_statement_for_destroy(struct mlua_ast_statement_for *stmt_for)
{
    if (stmt_for->name) mlua_ast_name_destroy(stmt_for->name);
    if (stmt_for->exp1) mlua_ast_expression_destroy(stmt_for->exp1);
    if (stmt_for->exp2) mlua_ast_expression_destroy(stmt_for->exp2);
    if (stmt_for->exp3) mlua_ast_expression_destroy(stmt_for->exp3);
    if (stmt_for->block) mlua_ast_statement_list_destroy(stmt_for->block);
    free(stmt_for);

    return 0;
}


/* Statement : label */

struct mlua_ast_statement_label *mlua_ast_statement_label_new(void)
{
    struct mlua_ast_statement_label *new_stmt_label = NULL;

    new_stmt_label = (struct mlua_ast_statement_label *)malloc(sizeof(struct mlua_ast_statement_label));
    if (new_stmt_label == NULL) { goto fail; }
    new_stmt_label->name = NULL;

    goto done;
fail:
    if (new_stmt_label != NULL)
    {
        mlua_ast_statement_label_destroy(new_stmt_label);
        new_stmt_label = NULL;
    }
done:
    return new_stmt_label;
}

int mlua_ast_statement_label_destroy(struct mlua_ast_statement_label *stmt_label)
{
    if (stmt_label->name != NULL) token_destroy(stmt_label->name);
    free(stmt_label);

    return 0;
}


/* Statement : goto */

struct mlua_ast_statement_goto *mlua_ast_statement_goto_new(void)
{
    struct mlua_ast_statement_goto *new_stmt_goto = NULL;

    new_stmt_goto = (struct mlua_ast_statement_goto *)malloc(sizeof(struct mlua_ast_statement_goto));
    if (new_stmt_goto == NULL) { goto fail; }
    new_stmt_goto->name = NULL;

    goto done;
fail:
    if (new_stmt_goto != NULL)
    {
        mlua_ast_statement_goto_destroy(new_stmt_goto);
        new_stmt_goto = NULL;
    }
done:
    return new_stmt_goto;
}

int mlua_ast_statement_goto_destroy(struct mlua_ast_statement_goto *stmt_goto)
{
    if (stmt_goto->name != NULL) token_destroy(stmt_goto->name);
    free(stmt_goto);

    return 0;
}


/* Statement : local */

struct mlua_ast_name *mlua_ast_name_new(void)
{
    struct mlua_ast_name *new_ast_name = NULL;

    new_ast_name = (struct mlua_ast_name *)malloc(sizeof(struct mlua_ast_name));
    if (new_ast_name == NULL) { goto fail; }
    new_ast_name->next = NULL;
    new_ast_name->prev = NULL;
    new_ast_name->name = NULL;

    goto done;
fail:
    if (new_ast_name != NULL)
    {
        mlua_ast_name_destroy(new_ast_name);
        new_ast_name = NULL;
    }
done:
    return new_ast_name;
}

int mlua_ast_name_destroy(struct mlua_ast_name *ast_name)
{
    if (ast_name->name != NULL) token_destroy(ast_name->name);
    free(ast_name);

    return 0;
}

struct mlua_ast_namelist *mlua_ast_namelist_new(void)
{
    struct mlua_ast_namelist *new_ast_namelist = NULL;

    new_ast_namelist = (struct mlua_ast_namelist *)malloc(sizeof(struct mlua_ast_namelist));
    if (new_ast_namelist == NULL) { goto fail; }
    new_ast_namelist->begin = new_ast_namelist->end = NULL;
    new_ast_namelist->size = 0;

    goto done;
fail:
    if (new_ast_namelist != NULL)
    {
        mlua_ast_namelist_destroy(new_ast_namelist);
        new_ast_namelist = NULL;
    }
done:
    return new_ast_namelist;
}

int mlua_ast_namelist_destroy(struct mlua_ast_namelist *ast_namelist)
{
    struct mlua_ast_name *ast_name_cur, *ast_name_next;

    ast_name_cur = ast_namelist->begin;
    while (ast_name_cur != NULL)
    {
        ast_name_next = ast_name_cur->next;
        mlua_ast_name_destroy(ast_name_cur);
        ast_name_cur = ast_name_next;
    }
    free(ast_namelist);

    return 0;
}

int mlua_ast_namelist_append(struct mlua_ast_namelist *ast_namelist, \
        struct mlua_ast_name *new_ast_name)
{
    if (ast_namelist->begin == NULL)
    {
        ast_namelist->begin = ast_namelist->end = new_ast_name;
    }
    else
    {
        new_ast_name->prev = ast_namelist->end;
        ast_namelist->end->next = new_ast_name;
        ast_namelist->end = new_ast_name;
    }
    ast_namelist->size += 1;

    return 0;
}

struct mlua_ast_statement_local *mlua_ast_statement_local_new(void)
{
    struct mlua_ast_statement_local *new_stmt_local = NULL;

    new_stmt_local = (struct mlua_ast_statement_local *)malloc(sizeof(struct mlua_ast_statement_local));
    if (new_stmt_local == NULL) { goto fail; }
    new_stmt_local->namelist = NULL;
    new_stmt_local->explist = NULL;
    new_stmt_local->namelist = mlua_ast_namelist_new();
    if (new_stmt_local->namelist == NULL) { goto fail; }
    new_stmt_local->explist = mlua_ast_expression_list_new();
    if (new_stmt_local->explist == NULL) { goto fail; }

    goto done;
fail:
    if (new_stmt_local != NULL)
    {
        mlua_ast_statement_local_destroy(new_stmt_local);
        new_stmt_local = NULL;
    }
done:
    return new_stmt_local;
}

int mlua_ast_statement_local_destroy(struct mlua_ast_statement_local *stmt_local)
{
    if (stmt_local->namelist != NULL) { mlua_ast_namelist_destroy(stmt_local->namelist); }
    if (stmt_local->explist != NULL) { mlua_ast_expression_list_destroy(stmt_local->explist); }
    free(stmt_local);

    return 0;
}


/* Statement : return */

struct mlua_ast_statement_return *mlua_ast_statement_return_new(void)
{
    struct mlua_ast_statement_return *new_stmt_return = NULL;

    new_stmt_return = (struct mlua_ast_statement_return *)malloc(sizeof(struct mlua_ast_statement_return));
    if (new_stmt_return == NULL) { goto fail; }
    new_stmt_return->explist = NULL;

    goto done;
fail:
    if (new_stmt_return != NULL)
    {
        mlua_ast_statement_return_destroy(new_stmt_return);
        new_stmt_return = NULL;
    }
done:
    return new_stmt_return;
}

int mlua_ast_statement_return_destroy(struct mlua_ast_statement_return *stmt_return)
{
    if (stmt_return->explist != NULL) mlua_ast_expression_list_destroy(stmt_return->explist);
    free(stmt_return);

    return 0;
}


/* Statement */

struct mlua_ast_statement *mlua_ast_statement_new(enum mlua_ast_statement_type type)
{
    struct mlua_ast_statement *new_stmt = NULL;

    new_stmt = (struct mlua_ast_statement *)malloc(sizeof(struct mlua_ast_statement));
    if (new_stmt == NULL) { goto fail; }
    new_stmt->prev = new_stmt->next = NULL;
    new_stmt->type = MLUA_AST_STATEMENT_TYPE_UNKNOWN;
    switch (type)
    {
        case MLUA_AST_STATEMENT_TYPE_ASSIGNMENT:
            new_stmt->u.stmt_assignment = NULL;
            break;
        case MLUA_AST_STATEMENT_TYPE_EXPR:
            new_stmt->u.stmt_expr = NULL;
            break;
        case MLUA_AST_STATEMENT_TYPE_FUNCALL:
            new_stmt->u.funcall = NULL;
            break;
        case MLUA_AST_STATEMENT_TYPE_IF:
            new_stmt->u.stmt_if = NULL;
            break;
        case MLUA_AST_STATEMENT_TYPE_WHILE:
            new_stmt->u.stmt_while = NULL;
            break;
        case MLUA_AST_STATEMENT_TYPE_REPEAT:
            new_stmt->u.stmt_repeat = NULL;
            break;
        case MLUA_AST_STATEMENT_TYPE_DO:
            new_stmt->u.stmt_do = NULL;
            break;
        case MLUA_AST_STATEMENT_TYPE_FOR:
            new_stmt->u.stmt_for = NULL;
            break;
        case MLUA_AST_STATEMENT_TYPE_BREAK:
            break;
        case MLUA_AST_STATEMENT_TYPE_LABEL:
            new_stmt->u.stmt_label = NULL;
            break;
        case MLUA_AST_STATEMENT_TYPE_GOTO:
            new_stmt->u.stmt_goto = NULL;
            break;
        case MLUA_AST_STATEMENT_TYPE_LOCAL:
            new_stmt->u.stmt_local = NULL;
            break;
        case MLUA_AST_STATEMENT_TYPE_FUNDEF:
            new_stmt->u.stmt_fundef = NULL;
            break;
        case MLUA_AST_STATEMENT_TYPE_RETURN:
            new_stmt->u.stmt_return = NULL;
            break;

        case MLUA_AST_STATEMENT_TYPE_UNKNOWN:
            goto fail;
    }
    new_stmt->type = type;

    goto done;
fail:
    if (new_stmt != NULL) 
    {
        mlua_ast_statement_destroy(new_stmt);
        new_stmt = NULL;
    }
done:
    return new_stmt;
}

int mlua_ast_statement_destroy(struct mlua_ast_statement *stmt)
{
    switch (stmt->type)
    {
        case MLUA_AST_STATEMENT_TYPE_ASSIGNMENT:
            if (stmt->u.stmt_assignment != NULL)
            { mlua_ast_statement_assignment_destroy(stmt->u.stmt_assignment); }
            break;
        case MLUA_AST_STATEMENT_TYPE_EXPR:
            if (stmt->u.stmt_expr != NULL)
            { mlua_ast_statement_expr_destroy(stmt->u.stmt_expr); }
            break;
        case MLUA_AST_STATEMENT_TYPE_FUNCALL:
            if (stmt->u.funcall != NULL)
            { mlua_ast_expression_funcall_destroy(stmt->u.funcall); }
            break;
        case MLUA_AST_STATEMENT_TYPE_IF:
            if (stmt->u.stmt_if != NULL)
            { mlua_ast_statement_if_destroy(stmt->u.stmt_if); }
            break;
        case MLUA_AST_STATEMENT_TYPE_WHILE:
            if (stmt->u.stmt_while != NULL)
            { mlua_ast_statement_while_destroy(stmt->u.stmt_while); }
            break;
        case MLUA_AST_STATEMENT_TYPE_REPEAT:
            if (stmt->u.stmt_while != NULL)
            { mlua_ast_statement_repeat_destroy(stmt->u.stmt_repeat); }
            break;
        case MLUA_AST_STATEMENT_TYPE_DO:
            if (stmt->u.stmt_do != NULL)
            { mlua_ast_statement_do_destroy(stmt->u.stmt_do); }
            break;
        case MLUA_AST_STATEMENT_TYPE_FOR:
            if (stmt->u.stmt_for != NULL)
            { mlua_ast_statement_for_destroy(stmt->u.stmt_for); }
            break;
        case MLUA_AST_STATEMENT_TYPE_BREAK:
            break;
        case MLUA_AST_STATEMENT_TYPE_LABEL:
            if (stmt->u.stmt_label != NULL)
            { mlua_ast_statement_label_destroy(stmt->u.stmt_label); }
            break;
        case MLUA_AST_STATEMENT_TYPE_GOTO:
            if (stmt->u.stmt_goto != NULL)
            { mlua_ast_statement_goto_destroy(stmt->u.stmt_goto); }
            break;
        case MLUA_AST_STATEMENT_TYPE_LOCAL:
            if (stmt->u.stmt_local != NULL)
            { mlua_ast_statement_local_destroy(stmt->u.stmt_local); }
            break;
        case MLUA_AST_STATEMENT_TYPE_FUNDEF:
            if (stmt->u.stmt_fundef != NULL)
            { mlua_ast_statement_fundef_destroy(stmt->u.stmt_fundef); }
            break;
        case MLUA_AST_STATEMENT_TYPE_RETURN:
            if (stmt->u.stmt_local != NULL)
            { mlua_ast_statement_return_destroy(stmt->u.stmt_return); }
            break;

        case MLUA_AST_STATEMENT_TYPE_UNKNOWN:
            break;
    }
    free(stmt);

    return 0;
}


/* Statement List (Block) */

struct mlua_ast_statement_list *mlua_ast_statement_list_new(void)
{
    struct mlua_ast_statement_list *new_list = NULL;

    new_list = (struct mlua_ast_statement_list *)malloc(sizeof(struct mlua_ast_statement_list));
    if (new_list == NULL) { goto fail; }
    new_list->begin = new_list->end = NULL;

    goto done;
fail:
done:
    return new_list;
}

int mlua_ast_statement_list_destroy(struct mlua_ast_statement_list *list)
{
    struct mlua_ast_statement *stmt_cur, *stmt_next;

    stmt_cur = list->begin;
    while (stmt_cur != NULL) 
    {
        stmt_next = stmt_cur->next;
        mlua_ast_statement_destroy(stmt_cur);
        stmt_cur = stmt_next;
    }
    free(list);

    return 0;
}

int mlua_ast_statement_list_append(struct mlua_ast_statement_list *list, \
        struct mlua_ast_statement *new_stmt)
{
    if (list->begin == NULL)
    {
        list->begin = list->end = new_stmt;
    }
    else
    {
        new_stmt->prev = list->end;
        list->end->next = new_stmt;
        list->end = new_stmt;
    }

    return 0;
}


/* Program */

struct mlua_ast_program *mlua_ast_program_new(void)
{
    struct mlua_ast_program *new_program = NULL;

    new_program = (struct mlua_ast_program *)malloc(sizeof(struct mlua_ast_program));
    if (new_program == NULL) { goto fail; }
    new_program->stmts = NULL;

    goto done;
fail:
done:
    return new_program;
}

int mlua_ast_program_destroy(struct mlua_ast_program *program)
{
    if (program->stmts != NULL) { mlua_ast_statement_list_destroy(program->stmts); }
    free(program);

    return 0;
}

