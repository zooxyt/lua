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

/* 
 * Reference:
 * http://www.lua.org/manual/5.2/manual.html
 */

#ifndef _MLUA_AST_H_
#define _MLUA_AST_H_

#include <stdio.h>
#include <stdint.h>
#include "mlua_lexer.h"


struct mlua_ast_expression;
struct mlua_ast_name;

/* Parameter */

struct mlua_ast_par
{
    struct token *name;

    struct mlua_ast_par *next;
    struct mlua_ast_par *prev;
};
struct mlua_ast_par *mlua_ast_par_new(void);
int mlua_ast_par_destroy(struct mlua_ast_par *par);

struct mlua_ast_par_list
{
    struct mlua_ast_par *begin;
    struct mlua_ast_par *end;
    size_t size;
};
struct mlua_ast_par_list *mlua_ast_par_list_new(void);
int mlua_ast_par_list_destroy(struct mlua_ast_par_list *par_list);
int mlua_ast_par_list_append(struct mlua_ast_par_list *par_list, \
        struct mlua_ast_par *new_par);


/* Expression : Table Constructor */
/* tableconstructor ::= ‘{’ [fieldlist] ‘}’
   fieldlist ::= field {fieldsep field} [fieldsep]
   field ::= ‘[’ exp ‘]’ ‘=’ exp | Name ‘=’ exp | exp
   fieldsep ::= ‘,’ | ‘;’
   */

enum mlua_ast_field_type
{
    MLUA_AST_FIELD_TYPE_UNKNOWN = 0,
    MLUA_AST_FIELD_TYPE_ARRAY,
    MLUA_AST_FIELD_TYPE_PROPERTY,
    MLUA_AST_FIELD_TYPE_EXP,
};

struct mlua_ast_field_array
{
    struct mlua_ast_expression *index;
    struct mlua_ast_expression *value;
};
struct mlua_ast_field_array *mlua_ast_field_array_new(void);
int mlua_ast_field_array_destroy(struct mlua_ast_field_array *field_array);

struct mlua_ast_field_property
{
    struct mlua_ast_name *name;
    struct mlua_ast_expression *value;
};
struct mlua_ast_field_property *mlua_ast_field_property_new(void);
int mlua_ast_field_property_destroy(struct mlua_ast_field_property *field_property);

struct mlua_ast_field_exp
{
    struct mlua_ast_expression *value;
};
struct mlua_ast_field_exp *mlua_ast_field_exp_new(void);
int mlua_ast_field_exp_destroy(struct mlua_ast_field_exp *field_exp);

struct mlua_ast_field
{
    enum mlua_ast_field_type type;
    union 
    {
        struct mlua_ast_field_array *array;
        struct mlua_ast_field_property *property;
        struct mlua_ast_field_exp *exp;
    } u;
    struct mlua_ast_field *prev;
    struct mlua_ast_field *next;
};
struct mlua_ast_field *mlua_ast_field_new(enum mlua_ast_field_type type);
int mlua_ast_field_destroy(struct mlua_ast_field *field);

struct mlua_ast_fieldlist
{
    struct mlua_ast_field *begin;
    struct mlua_ast_field *end;
    size_t size;
};
struct mlua_ast_fieldlist *mlua_ast_fieldlist_new(void);
int mlua_ast_fieldlist_destroy(struct mlua_ast_fieldlist *fieldlist);
int mlua_ast_fieldlist_append(struct mlua_ast_fieldlist *fieldlist, \
        struct mlua_ast_field *new_field);

struct mlua_ast_expression_tblctor
{
    struct mlua_ast_fieldlist *fieldlist;
};
struct mlua_ast_expression_tblctor *mlua_ast_expression_tblctor_new(void);
int mlua_ast_expression_tblctor_destroy(struct mlua_ast_expression_tblctor *tblctor);


/* Arguments */
/*
 * args ::= ‘(’ [explist] ‘)’
 * args ::= tableconstructor
 * args ::= String
 */

enum mlua_ast_args_type
{
    MLUA_AST_ARGS_TYPE_UNKNOWN = 0,
    MLUA_AST_ARGS_TYPE_EXPLIST,
    MLUA_AST_ARGS_TYPE_TBLCTOR,
    MLUA_AST_ARGS_TYPE_STRING,
};

struct mlua_ast_args
{
    enum mlua_ast_args_type type;
    union
    {
        struct token *str;
        struct mlua_ast_expression_list *explist;
        struct mlua_ast_expression_tblctor *tblctor;
    } u;
};
struct mlua_ast_args *mlua_ast_args_new(enum mlua_ast_args_type type);
int mlua_ast_args_destroy(struct mlua_ast_args *args);


/* Primary Expression */
/* primaryexp -> NAME | '(' expr ')' */

enum mlua_ast_expression_primary_type
{
    MLUA_AST_EXPRESSION_PRIMARY_TYPE_UNKNOWN = 0,
    MLUA_AST_EXPRESSION_PRIMARY_TYPE_NAME, 
    MLUA_AST_EXPRESSION_PRIMARY_TYPE_EXPR, 
};

struct mlua_ast_expression_primary
{
    enum mlua_ast_expression_primary_type type;
    union 
    {
        struct token *name;
        struct mlua_ast_expression *exp;
    } u;
};
struct mlua_ast_expression_primary *mlua_ast_expression_primary_new(enum mlua_ast_expression_primary_type type);
int mlua_ast_expression_primary_destroy(struct mlua_ast_expression_primary *stmt_primary);


/* Suffixed Expression */
/* suffixedexp ->
   primaryexp { '.' NAME | '[' exp ']' | ':' NAME funcargs | funcargs } */

enum mlua_ast_expression_suffixed_type
{
    MLUA_AST_EXPRESSION_SUFFIXED_TYPE_UNKNOWN = 0,
    MLUA_AST_EXPRESSION_SUFFIXED_TYPE_MEMBER,
    MLUA_AST_EXPRESSION_SUFFIXED_TYPE_INDEX,
};

struct mlua_ast_expression_suffixed
{
    enum mlua_ast_expression_suffixed_type type;

    struct mlua_ast_expression *sub;

    union 
    {
        struct token *name;
        struct mlua_ast_expression *exp;
    } u;
};
struct mlua_ast_expression_suffixed *mlua_ast_expression_suffixed_new(enum mlua_ast_expression_suffixed_type type);
int mlua_ast_expression_suffixed_destroy(struct mlua_ast_expression_suffixed *stmt_suffixed);


/* Expression : Prefix */
/* prefixexp ::= var | functioncall | '(' exp ')' */

enum mlua_ast_expression_prefix_type
{
    MLUA_AST_PREFIX_EXP_TYPE_UNKNOWN = 0,
    MLUA_AST_PREFIX_EXP_TYPE_VAR,
    MLUA_AST_PREFIX_EXP_TYPE_FUNCALL,
    MLUA_AST_PREFIX_EXP_TYPE_EXP,
};

struct mlua_ast_expression_prefix
{
    enum mlua_ast_expression_prefix_type type;
    union 
    {
        struct token *var;
        struct mlua_ast_expression_funcall *funcall;
        struct mlua_ast_expression *exp;
    } u;
};
struct mlua_ast_expression_prefix *mlua_ast_expression_prefix_new(enum mlua_ast_expression_prefix_type);
int mlua_ast_expression_prefix_destroy(struct mlua_ast_expression_prefix *stmt_prefix);


/* Expression : Function Call 
 * --------------
 * functioncall ::= prefixexp args
 * functioncall ::= prefixexp ‘:’ Name args 
 * ----------------
 */

struct mlua_ast_expression_funcall
{
    struct mlua_ast_expression *prefixexp;
    /*struct mlua_ast_expression *member_name;*/
    struct mlua_ast_args *args;
};
struct mlua_ast_expression_funcall *mlua_ast_expression_funcall_new(void);
int mlua_ast_expression_funcall_destroy(struct mlua_ast_expression_funcall *stmt_funcall);


/* Expression : Factor */

enum mlua_ast_expression_factor_type
{
    MLUA_AST_EXP_FACTOR_TYPE_UNKNOWN = 0,
    MLUA_AST_EXP_FACTOR_TYPE_NIL,
    MLUA_AST_EXP_FACTOR_TYPE_FALSE,
    MLUA_AST_EXP_FACTOR_TYPE_TRUE,
    MLUA_AST_EXP_FACTOR_TYPE_INTEGER,
    MLUA_AST_EXP_FACTOR_TYPE_FLOAT,
    MLUA_AST_EXP_FACTOR_TYPE_STRING,
};
struct mlua_ast_expression_factor
{
    enum mlua_ast_expression_factor_type type;
    struct token *token;
};
struct mlua_ast_expression_factor *mlua_ast_expression_factor_new( \
        enum mlua_ast_expression_factor_type type);
int mlua_ast_expression_factor_destroy(struct mlua_ast_expression_factor *stmt_factor);


/* Expression : Unary Operation */

struct mlua_ast_expression_unop
{
    struct token *op;
    struct mlua_ast_expression *sub;
};
struct mlua_ast_expression_unop *mlua_ast_expression_unop_new(void);
int mlua_ast_expression_unop_destroy(struct mlua_ast_expression_unop *stmt_unop);


/* Expression : Binary Operation */

struct mlua_ast_expression_binop
{
    struct token *op;
    struct mlua_ast_expression *left;
    struct mlua_ast_expression *right;
};
struct mlua_ast_expression_binop *mlua_ast_expression_binop_new(void);
int mlua_ast_expression_binop_destroy(struct mlua_ast_expression_binop *stmt_binop);


/* Expression : Function Definition */

struct mlua_ast_expression_fundef
{
    struct mlua_ast_par_list *pars;
    struct mlua_ast_statement_list *body;
};
struct mlua_ast_expression_fundef *mlua_ast_expression_fundef_new(void);
int mlua_ast_expression_fundef_destroy(struct mlua_ast_expression_fundef *stmt_fundef);


/* Expression */

enum mlua_ast_expression_type
{
    MLUA_AST_EXPRESSION_TYPE_UNKNOWN = 0,
    MLUA_AST_EXPRESSION_TYPE_FACTOR,
    MLUA_AST_EXPRESSION_TYPE_PREFIX,
    MLUA_AST_EXPRESSION_TYPE_PRIMARY,
    MLUA_AST_EXPRESSION_TYPE_SUFFIXED,
    MLUA_AST_EXPRESSION_TYPE_TBLCTOR,
    /*MLUA_AST_EXPRESSION_TYPE_FUNDEF,*/
    /*MLUA_AST_EXPRESSION_TYPE_TRIDOT,*/
    MLUA_AST_EXPRESSION_TYPE_BINOP,
    MLUA_AST_EXPRESSION_TYPE_UNOP,
    MLUA_AST_EXPRESSION_TYPE_FUNCALL,
    MLUA_AST_EXPRESSION_TYPE_FUNDEF,
};

struct mlua_ast_expression
{
    enum mlua_ast_expression_type type;
    union 
    {
        struct mlua_ast_expression_factor *factor;
        struct mlua_ast_expression_prefix *prefix;
        struct mlua_ast_expression_primary *primary;
        struct mlua_ast_expression_suffixed *suffixed;
        struct mlua_ast_expression_tblctor *tblctor; 
        struct mlua_ast_expression_unop *unop;
        struct mlua_ast_expression_binop *binop;
        struct mlua_ast_expression_funcall *funcall;
        struct mlua_ast_expression_fundef *fundef;
    } u;

    struct mlua_ast_expression *next;
    struct mlua_ast_expression *prev;
};
struct mlua_ast_expression *mlua_ast_expression_new(enum mlua_ast_expression_type type);
int mlua_ast_expression_destroy(struct mlua_ast_expression *exp);


/* Expression List */

struct mlua_ast_expression_list
{
    struct mlua_ast_expression *begin;
    struct mlua_ast_expression *end;
    size_t size;
};
struct mlua_ast_expression_list *mlua_ast_expression_list_new(void);
int mlua_ast_expression_list_destroy(struct mlua_ast_expression_list *list);
int mlua_ast_expression_list_append(struct mlua_ast_expression_list *list, \
        struct mlua_ast_expression *new_exp);


/* Statement : assignment */
/* varlist ‘=’ explist */
struct mlua_ast_statement_assignment
{
    struct mlua_ast_expression_list *varlist;
    struct mlua_ast_expression_list *explist;
};
struct mlua_ast_statement_assignment *mlua_ast_statement_assignment_new(void);
int mlua_ast_statement_assignment_destroy(struct mlua_ast_statement_assignment *stmt_assignment);


/* Statement : expr */
/* stat -> expr */
struct mlua_ast_statement_expr
{
    struct mlua_ast_expression *expr;
};
struct mlua_ast_statement_expr *mlua_ast_statement_expr_new(void);
int mlua_ast_statement_expr_destroy(struct mlua_ast_statement_expr *stmt_expr);


/* Statement : if */
/* stat -> "if" exp "then" block {"elseif" exp "then" block} ["else" block] "end" | */
struct mlua_ast_statement_elseif
{
    struct mlua_ast_expression *exp;
    struct mlua_ast_statement_list *block_then;

    struct mlua_ast_statement_elseif *elseif;
};
struct mlua_ast_statement_elseif *mlua_ast_statement_elseif_new(void);
int mlua_ast_statement_elseif_destroy(struct mlua_ast_statement_elseif *stmt_elseif);

struct mlua_ast_statement_if
{
    struct mlua_ast_expression *exp;
    struct mlua_ast_statement_list *block_then;

    struct mlua_ast_statement_elseif *elseif;

    struct mlua_ast_statement_list *block_else;
};
struct mlua_ast_statement_if *mlua_ast_statement_if_new(void);
int mlua_ast_statement_if_destroy(struct mlua_ast_statement_if *stmt_if);


/* Statement : while */
/* stat -> 'while' exp 'do' block 'end' */
struct mlua_ast_statement_while
{
    struct mlua_ast_expression *exp;
    struct mlua_ast_statement_list *block;
};
struct mlua_ast_statement_while *mlua_ast_statement_while_new(void);
int mlua_ast_statement_while_destroy(struct mlua_ast_statement_while *stmt_while);


/* Statement : repeat */
/* stat -> 'repeat' block 'until' exp */
struct mlua_ast_statement_repeat
{
    struct mlua_ast_statement_list *block;
    struct mlua_ast_expression *exp;
};
struct mlua_ast_statement_repeat *mlua_ast_statement_repeat_new(void);
int mlua_ast_statement_repeat_destroy(struct mlua_ast_statement_repeat *stmt_repeat);


/* Statement : do */
/* stat -> 'do' block 'end' */
struct mlua_ast_statement_do
{
    struct mlua_ast_statement_list *block;
};
struct mlua_ast_statement_do *mlua_ast_statement_do_new(void);
int mlua_ast_statement_do_destroy(struct mlua_ast_statement_do *stmt_do);


/* Statement : for */
/* stat -> 'for' name '=' exp ',' exp [',' exp] 'do' block 'end' */
struct mlua_ast_statement_for
{
    struct mlua_ast_name *name;
    struct mlua_ast_expression *exp1;
    struct mlua_ast_expression *exp2;
    struct mlua_ast_expression *exp3;
    struct mlua_ast_statement_list *block;
};
struct mlua_ast_statement_for *mlua_ast_statement_for_new(void);
int mlua_ast_statement_for_destroy(struct mlua_ast_statement_for *stmt_for);


/* Statement : label */
/* stat -> '::' label '::' */
struct mlua_ast_statement_label
{
    struct token *name;
};
struct mlua_ast_statement_label *mlua_ast_statement_label_new(void);
int mlua_ast_statement_label_destroy(struct mlua_ast_statement_label *stmt_label);


/* Statement : goto */
/* stat -> goto label */
struct mlua_ast_statement_goto
{
    struct token *name;
};
struct mlua_ast_statement_goto *mlua_ast_statement_goto_new(void);
int mlua_ast_statement_goto_destroy(struct mlua_ast_statement_goto *stmt_goto);


/* Statement : local */
/* stat -> 'local' namelist ['=' explist] */

struct mlua_ast_name
{
    struct token *name;

    struct mlua_ast_name *next;
    struct mlua_ast_name *prev;
};
struct mlua_ast_name *mlua_ast_name_new(void);
int mlua_ast_name_destroy(struct mlua_ast_name *ast_name);

struct mlua_ast_namelist
{
    struct mlua_ast_name *begin;
    struct mlua_ast_name *end;
    size_t size;
};
struct mlua_ast_namelist *mlua_ast_namelist_new(void);
int mlua_ast_namelist_destroy(struct mlua_ast_namelist *ast_namelist);
int mlua_ast_namelist_append(struct mlua_ast_namelist *ast_namelist, \
        struct mlua_ast_name *new_ast_name);

struct mlua_ast_statement_local
{
    struct mlua_ast_namelist *namelist;
    struct mlua_ast_expression_list *explist;
};
struct mlua_ast_statement_local *mlua_ast_statement_local_new(void);
int mlua_ast_statement_local_destroy(struct mlua_ast_statement_local *stmt_local);


/* Statement : function */

/* stat ::= 'function' funcname funcbody 
 * stat ::= 'local' 'function' Name funcbody | 
 *
 * funcname ::= Name {‘.’ Name} [‘:’ Name]
 * funcbody ::= ‘(’ [parlist] ‘)’ block 'end'
 */
struct mlua_ast_funcname
{
    struct mlua_ast_namelist *name_list;
    struct mlua_ast_name *member;
};
struct mlua_ast_funcname *mlua_ast_funcname_new(void);
int mlua_ast_funcname_destroy(struct mlua_ast_funcname *funcname);

struct mlua_ast_statement_fundef
{
    struct mlua_ast_funcname *funcname;
    struct mlua_ast_par_list *parameters;
    struct mlua_ast_statement_list *body;
    int local;
};
struct mlua_ast_statement_fundef *mlua_ast_statement_fundef_new(void);
int mlua_ast_statement_fundef_destroy(struct mlua_ast_statement_fundef *stmt_fundef);


/* Statement : return */
/* stat ::= return [explist] [‘;’] */

struct mlua_ast_statement_return
{
    struct mlua_ast_expression_list *explist;
};
struct mlua_ast_statement_return *mlua_ast_statement_return_new(void);
int mlua_ast_statement_return_destroy(struct mlua_ast_statement_return *stmt_return);


/* Statement */

enum mlua_ast_statement_type
{
    MLUA_AST_STATEMENT_TYPE_UNKNOWN = 0,
    /*MLUA_AST_STATEMENT_TYPE_BLOCKS,*/
    /*MLUA_AST_STATEMENT_TYPE_CHUNKS,*/
    MLUA_AST_STATEMENT_TYPE_ASSIGNMENT,
    MLUA_AST_STATEMENT_TYPE_EXPR, 
    MLUA_AST_STATEMENT_TYPE_IF,
    MLUA_AST_STATEMENT_TYPE_WHILE,
    MLUA_AST_STATEMENT_TYPE_REPEAT,
    MLUA_AST_STATEMENT_TYPE_BREAK,
    MLUA_AST_STATEMENT_TYPE_LABEL,
    MLUA_AST_STATEMENT_TYPE_GOTO,
    MLUA_AST_STATEMENT_TYPE_DO,
    MLUA_AST_STATEMENT_TYPE_FOR,
    MLUA_AST_STATEMENT_TYPE_LOCAL,
    MLUA_AST_STATEMENT_TYPE_FUNCALL, 
    MLUA_AST_STATEMENT_TYPE_FUNDEF, 
    MLUA_AST_STATEMENT_TYPE_RETURN,
};

struct mlua_ast_statement
{
    enum mlua_ast_statement_type type;

    union
    {
        struct mlua_ast_expression_funcall *funcall;
        struct mlua_ast_statement_assignment *stmt_assignment;
        struct mlua_ast_statement_expr *stmt_expr;
        struct mlua_ast_statement_if *stmt_if;
        struct mlua_ast_statement_while *stmt_while;
        struct mlua_ast_statement_repeat *stmt_repeat;
        struct mlua_ast_statement_do *stmt_do;
        struct mlua_ast_statement_for *stmt_for;
        struct mlua_ast_statement_label *stmt_label;
        struct mlua_ast_statement_goto *stmt_goto;
        struct mlua_ast_statement_local *stmt_local;
        struct mlua_ast_statement_fundef *stmt_fundef;
        struct mlua_ast_statement_return *stmt_return;
    } u;

    struct mlua_ast_statement *prev;
    struct mlua_ast_statement *next;
};
struct mlua_ast_statement *mlua_ast_statement_new(enum mlua_ast_statement_type type);
int mlua_ast_statement_destroy(struct mlua_ast_statement *stmt);


/* Statement List (Block) */

struct mlua_ast_statement_list
{
    struct mlua_ast_statement *begin;
    struct mlua_ast_statement *end;
};
struct mlua_ast_statement_list *mlua_ast_statement_list_new(void);
int mlua_ast_statement_list_destroy(struct mlua_ast_statement_list *list);
int mlua_ast_statement_list_append(struct mlua_ast_statement_list *list, \
        struct mlua_ast_statement *new_stmt);


/* Program */

struct mlua_ast_program
{
    struct mlua_ast_statement_list *stmts;
};
struct mlua_ast_program *mlua_ast_program_new(void);
int mlua_ast_program_destroy(struct mlua_ast_program *program);


#endif

