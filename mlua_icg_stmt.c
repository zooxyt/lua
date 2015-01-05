/* Multiple Lua Programming Language : Intermediate Code Generator
 * Statement
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
#include <stdlib.h>
#include <string.h>

#include "multiple_ir.h"
#include "multiple_misc.h" 
#include "multiple_err.h"

#include "multiply.h"
#include "multiply_assembler.h"
#include "multiply_str_aux.h"
#include "multiply_num.h"

#include "vm_predef.h"
#include "vm_opcode.h"
#include "vm_types.h"

#include "mlua_lexer.h"
#include "mlua_ast.h"
#include "mlua_icg.h"

#include "mlua_icg_fcb.h"
#include "mlua_icg_context.h"
#include "mlua_icg_aux.h"

#include "mlua_icg_expr.h"
#include "mlua_icg_stmt.h"


/* Generate icode for Expression List and put the count of result
 * on the top of stack */
int mlua_icodegen_explist(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_list *explist)
{
    int ret = 0;
    struct mlua_ast_expression *exp_cur;
    uint32_t id_zero;
    struct multiply_text_precompiled *new_text_precompiled_1 = NULL;
    struct multiply_text_precompiled *new_text_precompiled_2 = NULL;
    const int LBL_TAIL = 0, LBL_1 = 1;

    /* the last (or the only) */
    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled_1, \

                    /* Preserve the value */
                    MULTIPLY_ASM_OP     , OP_DUP       ,

                    /* if (type(obj) != type(list)) then goto lbl_1; */
                    MULTIPLY_ASM_OP     , OP_TYPE      ,
                    MULTIPLY_ASM_OP_RAW , OP_LSTMK     , 0          , 
                    MULTIPLY_ASM_OP     , OP_TYPE      ,
                    MULTIPLY_ASM_OP     , OP_NE        ,
                    MULTIPLY_ASM_OP_LBLR, OP_JMPCR     , LBL_1 ,

                    /* <list> */
                    /* Unpack the list */ 
                    MULTIPLY_ASM_OP     , OP_LSTUNPACK , 

                    /* State : <bottom> ... count, elements_of_list, count_of_list  <top> */
                    MULTIPLY_ASM_OP     , OP_DUP       , 
                    MULTIPLY_ASM_OP_INT , OP_PUSH      , 2,
                    MULTIPLY_ASM_OP     , OP_ADD       , 

                    /* State : <bottom> ... count, elements_of_list, count_of_list, count_of_list+2 <top> */
                    /* Pick 'count' up */
                    MULTIPLY_ASM_OP     , OP_PICK      , 

                    /* State : <bottom> ... elements_of_list, count_of_list, count <top> */
                    MULTIPLY_ASM_OP     , OP_ADD       , 

                    /* State : <bottom> ... elements_of_list, count_of_list + count <top> */

                    /* goto lbl_tail; */
                    MULTIPLY_ASM_OP_LBLR, OP_JMPR      , LBL_TAIL ,

                    /* lbl_1: */
                    MULTIPLY_ASM_LABEL  , LBL_1        ,     

                    /* State : <bottom> ... new_exp <top> */
                    MULTIPLY_ASM_OP_INT , OP_PUSH      , 2,
                    MULTIPLY_ASM_OP     , OP_PICK      , 
                    MULTIPLY_ASM_OP_INT , OP_PUSH      , 1,
                    MULTIPLY_ASM_OP     , OP_ADD       , 

                    /* State : <bottom> ... new_exp, count + 1 <top> */

                    /* lbl_tail: */
                    MULTIPLY_ASM_LABEL  , LBL_TAIL     ,     

                    MULTIPLY_ASM_FINISH)) != 0)
                    { goto fail; }

    /* Not the last one */
    /* If the value is a list, get the first element */
    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled_2, \

                    /* Preserve the value */
                    MULTIPLY_ASM_OP     , OP_DUP       ,

                    /* if (type(obj) != type(list)) then goto lbl_tail; */
                    MULTIPLY_ASM_OP     , OP_TYPE      ,
                    MULTIPLY_ASM_OP_RAW , OP_LSTMK     , 0          , 
                    MULTIPLY_ASM_OP     , OP_TYPE      ,
                    MULTIPLY_ASM_OP     , OP_NE        ,
                    MULTIPLY_ASM_OP_LBLR, OP_JMPCR     , LBL_TAIL,

                    MULTIPLY_ASM_OP     , OP_DUP       ,
                    MULTIPLY_ASM_OP     , OP_SIZE      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH      , 0   ,
                    MULTIPLY_ASM_OP     , OP_EQ        , 

                    /* if (sizeof(obj) == 0) then goto lbl_1; */
                    MULTIPLY_ASM_OP_LBLR, OP_JMPCR     , LBL_1,

                    /* Get the last one */
                    MULTIPLY_ASM_OP     , OP_DUP       ,
                    MULTIPLY_ASM_OP     , OP_SIZE      ,
                    MULTIPLY_ASM_OP_INT , OP_PUSH      , 1   ,
                    MULTIPLY_ASM_OP     , OP_SUB       ,

                    MULTIPLY_ASM_OP_INT , OP_PUSH      , 2   ,
                    MULTIPLY_ASM_OP     , OP_PICK      , 

                    MULTIPLY_ASM_OP     , OP_REFGET    , 

                    /* goto lbl_tail */
                    MULTIPLY_ASM_OP_LBLR, OP_JMPR      , LBL_TAIL,

                    /* lbl_1: */
                    MULTIPLY_ASM_LABEL  , LBL_1        ,     
                    MULTIPLY_ASM_OP     , OP_DROP      , 
                    MULTIPLY_ASM_OP_NONE, OP_PUSH      , 

                    /* lbl_tail: */
                    MULTIPLY_ASM_LABEL  , LBL_TAIL     ,     

                    /* State : <bottom> ... count, new_exp <top> */
                    MULTIPLY_ASM_OP_INT , OP_PUSH      , 2,
                    MULTIPLY_ASM_OP     , OP_PICK      , 
                    MULTIPLY_ASM_OP_INT , OP_PUSH      , 1,
                    MULTIPLY_ASM_OP     , OP_ADD       , 

                    /* State : <bottom> ... new_exp, count + 1 <top> */

                    MULTIPLY_ASM_FINISH)) != 0)
                    { goto fail; }

    /* zero */
    if ((ret = multiply_resource_get_int(err, context->icode, context->res_id, \
                    &id_zero, 0)) != 0)
    { goto fail; }
    /* Push the initial count of return value size */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                    OP_PUSH, id_zero)) != 0) { goto fail; }

    exp_cur = explist->begin;
    while (exp_cur != NULL)
    {
        /* Evaluate the expression */
        if ((ret = mlua_icodegen_expression_non_fix(err, \
                        context, \
                        icg_fcb_block, \
                        exp_cur)) != 0)
        { goto fail; }


        /* State : <bottom> ... count, new_exp <top> */

        /* Function call returns a list */
        /* When a function call is the last (or the only) argument to another call,
         * all results from the first call go as arguments */
        if (exp_cur->next == NULL)
        {
            if ((ret = mlua_icg_fcb_block_append_from_precompiled_pic_text( \
                            icg_fcb_block, \
                            new_text_precompiled_1)) != 0)
            { goto fail; }
        }
        else
        {

            if ((ret = mlua_icg_fcb_block_append_from_precompiled_pic_text( \
                            icg_fcb_block, \
                            new_text_precompiled_2)) != 0)
            { goto fail; }
        }

        exp_cur = exp_cur->next; 
    }

    goto done;
fail:
done:
    if (new_text_precompiled_1 != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled_1); }
    if (new_text_precompiled_2 != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled_2); }
    return ret;
}


/* Trim explist result with specified number 
 * Call after 'mlua_icodegen_explist' */
int mlua_icodegen_trim_explist(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        size_t maximum)
{
    int ret = 0;
    struct multiply_text_precompiled *new_text_precompiled_drop_extra_exp = NULL;
    const int LBL_TAIL = 0, LBL_REPEAT = 1;

    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled_drop_extra_exp, \

                    /* Preserve the result count */
                    MULTIPLY_ASM_OP_INT   , OP_PUSH       , (int)maximum,

                    /* State : <bottom> elements, count_of_elements, count_of_vars <top> */
                    MULTIPLY_ASM_LABEL    , LBL_REPEAT    ,

                    MULTIPLY_ASM_OP       , OP_DUP        ,
                    MULTIPLY_ASM_OP_INT   , OP_PUSH       , 3,
                    MULTIPLY_ASM_OP       , OP_PICKCP     ,

                    /* State : <bottom> elements, count_of_elements, count_of_vars
                     * count_of_vars, count_of_elements <top> */

                    MULTIPLY_ASM_OP       , OP_GE         ,
                    MULTIPLY_ASM_OP_LBLR  , OP_JMPCR      , LBL_TAIL,

                    /* State : <bottom> elements, count_of_elements, count_of_vars */
                    MULTIPLY_ASM_OP_INT   , OP_PUSH       , 3,
                    MULTIPLY_ASM_OP       , OP_PICK       ,
                    MULTIPLY_ASM_OP       , OP_DROP       ,

                    MULTIPLY_ASM_OP_INT   , OP_PUSH       , 2,
                    MULTIPLY_ASM_OP       , OP_PICK       ,
                    MULTIPLY_ASM_OP_INT   , OP_PUSH       , 1,
                    MULTIPLY_ASM_OP       , OP_SUB        ,
                    MULTIPLY_ASM_OP_INT   , OP_PUSH       , 2,
                    MULTIPLY_ASM_OP       , OP_PICK       ,

                    MULTIPLY_ASM_OP_LBLR  , OP_JMPR       , LBL_REPEAT,

                    MULTIPLY_ASM_LABEL    , LBL_TAIL      ,

                    /* Drop count of vars */
                    MULTIPLY_ASM_OP       , OP_DROP       ,

                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; }

    if ((ret = mlua_icg_fcb_block_append_from_precompiled_pic_text( \
                    icg_fcb_block, \
                    new_text_precompiled_drop_extra_exp)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    if (new_text_precompiled_drop_extra_exp != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled_drop_extra_exp); }
    return ret;
}


/* Generate icode for Parameter List */
int mlua_icodegen_parlist(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_par_list *parlist)
{
    int ret = 0;
    struct mlua_ast_par *par_cur;
    struct multiply_text_precompiled *new_text_precompiled = NULL;
    const int LBL_HAS_ARG = 0, LBL_TAIL = 1, LBL_HEAD = 2;

    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled, \

                    /* arg = {"n"=0} */
                    /* Key */
                    MULTIPLY_ASM_OP_STR , OP_PUSH    , "n",
                    /* Value */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 0,
                    /* Count */
                    MULTIPLY_ASM_OP_RAW , OP_HASHMK  , 1,
                    /* 'arg' */
                    MULTIPLY_ASM_OP_ID  , OP_POPCL   , "arg",


                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; }
    if ((ret = mlua_icg_fcb_block_append_from_precompiled_pic_text( \
                    icg_fcb_block, \
                    new_text_precompiled)) != 0)
    { goto fail; }
    if (new_text_precompiled != NULL)
    {
        multiply_text_precompiled_destroy(new_text_precompiled);
        new_text_precompiled = NULL; 
    }

    /* Parameter */
    par_cur = parlist->begin;
    while (par_cur != NULL)
    {
        switch (par_cur->name->value)
        {
            case TOKEN_OP_TRI_DOT:
                /* Rest */
                /* Push rest arguments into a table called 'arg' \
                 * with a counter named 'n' */
                if ((ret = multiply_asm_precompile(err, \
                                context->icode, \
                                context->res_id, \
                                &new_text_precompiled, \

                                /* lbl_head: */
                                MULTIPLY_ASM_LABEL  , LBL_HEAD   ,

                                /* if no arg the goto lbl_tail */
                                MULTIPLY_ASM_OP     , OP_ARGP    , 
                                MULTIPLY_ASM_OP     , OP_NOTL    , 
                                MULTIPLY_ASM_OP_LBLR, OP_JMPCR   ,   LBL_TAIL,

                                /* arg[n] = Value */
                                /* Value */
                                MULTIPLY_ASM_OP     , OP_ARGCS   , 
                                /* Key */
                                MULTIPLY_ASM_OP_STR , OP_PUSH    , "n",
                                MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arg",
                                MULTIPLY_ASM_OP     , OP_REFGET  , 
                                /* 'arg' */
                                MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arg",
                                MULTIPLY_ASM_OP     , OP_HASHADD , 
                                MULTIPLY_ASM_OP     , OP_DROP    ,

                                MULTIPLY_ASM_OP_STR , OP_PUSH    , "n",
                                /* <bottom> "n" <top> */
                                MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arg",
                                /* <bottom> "n", arg <top> */
                                MULTIPLY_ASM_OP     , OP_REFGET  , 
                                /* <bottom> n <top> */
                                MULTIPLY_ASM_OP_INT , OP_PUSH    , 1,
                                MULTIPLY_ASM_OP     , OP_ADD     , 
                                /* <bottom> n + 1 <top> */
                                MULTIPLY_ASM_OP_STR , OP_PUSH    , "n",
                                MULTIPLY_ASM_OP_ID  , OP_PUSH    , "arg",
                                /* <bottom> n + 1, "n", arg <top> */
                                MULTIPLY_ASM_OP     , OP_HASHADD , 

                                /* goto lbl_head */
                                MULTIPLY_ASM_OP_LBLR, OP_JMPR    ,   LBL_HEAD,

                                /* lbl_tail */
                                MULTIPLY_ASM_LABEL  , LBL_TAIL   ,

                                MULTIPLY_ASM_FINISH)) != 0)
                { goto fail; }

                break;

            default:
                /* Normal */
                if ((ret = multiply_asm_precompile(err, \
                                context->icode, \
                                context->res_id, \
                                &new_text_precompiled, \

                                MULTIPLY_ASM_OP     , OP_ARGP    , 
                                MULTIPLY_ASM_OP_LBLR, OP_JMPCR   ,   LBL_HAS_ARG,

                                MULTIPLY_ASM_OP_NONE, OP_PUSH    , 
                                MULTIPLY_ASM_OP_ID  , OP_POPC    ,   par_cur->name->str,
                                MULTIPLY_ASM_OP_LBLR, OP_JMPR    ,   LBL_TAIL,

                                MULTIPLY_ASM_LABEL  , LBL_HAS_ARG,
                                MULTIPLY_ASM_OP_ID  , OP_ARGC    ,   par_cur->name->str,

                                MULTIPLY_ASM_LABEL  , LBL_TAIL   ,

                                MULTIPLY_ASM_FINISH)) != 0)
                { goto fail; }
                break;
        }

        if ((ret = mlua_icg_fcb_block_append_from_precompiled_pic_text( \
                        icg_fcb_block, \
                        new_text_precompiled)) != 0)
        { goto fail; }

        if (new_text_precompiled != NULL)
        {
            multiply_text_precompiled_destroy(new_text_precompiled);
            new_text_precompiled = NULL; 
        }

        par_cur = par_cur->next; 
    }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    return ret;
}


static int mlua_icodegen_statement_assignment_raw(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_list *varlist, \
        struct mlua_ast_expression_list *explist)
{
    int ret = 0;
    struct mlua_ast_expression *exp_cur;
    uint32_t id;
    size_t idx;
    struct multiply_text_precompiled *new_text_precompiled = NULL;
    const int LBL_TAIL = 0, LBL_1 = 1;

    /* Right values */
    /* Push the arguments in order 
     * Because of the post processing rely on the explist size that 
     * can only confirmed on run time (after all expressions evaluated),
     * we must keep the "size" value on the top of the stack */
    if ((ret = mlua_icodegen_explist(err, \
                    context, \
                    icg_fcb_block, \
                    explist)) != 0)
    { goto fail; }

    /* If the var count less than the exp count, then we need to drop some */
    if ((ret = mlua_icodegen_trim_explist(err, \
                    context, \
                    icg_fcb_block, \
                    varlist->size)) != 0)
    { goto fail; }


    /* Left Values */
    /* Push the arguments in reverse order and followed with an "assign" */
    idx = varlist->size;
    exp_cur = varlist->end;
    while (exp_cur != NULL)
    {
        switch (exp_cur->type)
        {
            case MLUA_AST_EXPRESSION_TYPE_PRIMARY:
                switch (exp_cur->u.primary->type)
                {
                    case MLUA_AST_EXPRESSION_PRIMARY_TYPE_NAME: 
                        if ((ret = multiply_resource_get_id( \
                                        err, 
                                        context->icode, \
                                        context->res_id, \
                                        &id, \
                                        exp_cur->u.primary->u.name->str, \
                                        exp_cur->u.primary->u.name->len)) != 0)
                        { goto fail; }

                        if ((ret = multiply_asm_precompile(err, \
                                        context->icode, \
                                        context->res_id, \
                                        &new_text_precompiled, \

                                        /* State : <bottom> elements, count <top> */
                                        /* Preserve the value */
                                        MULTIPLY_ASM_OP     , OP_DUP        ,
                                        /* Push var count */ 
                                        MULTIPLY_ASM_OP_INT , OP_PUSH       , idx,

                                        /* State : <bottom> elements, count, count, var_count <top> */
                                        MULTIPLY_ASM_OP     , OP_GE         ,
                                        MULTIPLY_ASM_OP_LBLR, OP_JMPCR      , LBL_1,

                                        /* Should push nil */
                                        MULTIPLY_ASM_OP_NONE, OP_PUSH       ,
                                        MULTIPLY_ASM_OP_RAW , OP_POP        , id, 
                                        MULTIPLY_ASM_OP_LBLR, OP_JMPR       , LBL_TAIL,

                                        MULTIPLY_ASM_LABEL  , LBL_1         ,

                                        /* State : <bottom> elements, count - 1 <top> */
                                        MULTIPLY_ASM_OP_INT , OP_PUSH       , 2,
                                        MULTIPLY_ASM_OP     , OP_PICK       , 

                                        MULTIPLY_ASM_OP_RAW , OP_POP        , id, 

                                        /* lbl_tail: */
                                        MULTIPLY_ASM_LABEL  , LBL_TAIL      ,


                                        MULTIPLY_ASM_FINISH)) != 0)
                                        { goto fail; }

                        if ((ret = mlua_icg_fcb_block_append_from_precompiled_pic_text( \
                                        icg_fcb_block, \
                                        new_text_precompiled)) != 0)
                        { goto fail; }

                        multiply_text_precompiled_destroy(new_text_precompiled);
                        new_text_precompiled = NULL;

                        break;

                    case MLUA_AST_EXPRESSION_PRIMARY_TYPE_EXPR: 
                    case MLUA_AST_EXPRESSION_PRIMARY_TYPE_UNKNOWN:
                        MULTIPLE_ERROR_INTERNAL();
                        ret = -MULTIPLE_ERR_INTERNAL;
                        goto fail;
                }
                break;

            case MLUA_AST_EXPRESSION_TYPE_SUFFIXED:

                /* State : <bottom> elements, count <top> */
                if ((ret = multiply_asm_precompile(err, \
                                context->icode, \
                                context->res_id, \
                                &new_text_precompiled, \

                                MULTIPLY_ASM_OP_INT , OP_PUSH       , 1,
                                MULTIPLY_ASM_OP     , OP_SUB        ,

                                MULTIPLY_ASM_OP_INT , OP_PUSH       , 2,
                                MULTIPLY_ASM_OP     , OP_PICK       ,

                                MULTIPLY_ASM_FINISH)) != 0)
                { goto fail; }
                if ((ret = mlua_icg_fcb_block_append_from_precompiled_pic_text( \
                                icg_fcb_block, \
                                new_text_precompiled)) != 0)
                { goto fail; }
                multiply_text_precompiled_destroy(new_text_precompiled);
                new_text_precompiled = NULL;

                /* State : <bottom> elements, count - 1, exp <top> */

                /* Index */
                switch (exp_cur->u.suffixed->type)
                {
                    case MLUA_AST_EXPRESSION_SUFFIXED_TYPE_MEMBER:
                        if ((ret = multiply_resource_get_str( \
                                        err, \
                                        context->icode, \
                                        context->res_id, \
                                        &id, \
                                        exp_cur->u.suffixed->u.name->str, 
                                        exp_cur->u.suffixed->u.name->len)) != 0)
                        { goto fail; }
                        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                                        OP_PUSH, id)) != 0) { goto fail; }
                        break;

                    case MLUA_AST_EXPRESSION_SUFFIXED_TYPE_INDEX:
                        if ((ret = mlua_icodegen_expression(err, \
                                        context, \
                                        icg_fcb_block, \
                                        exp_cur->u.suffixed->u.exp)) != 0)
                        { goto fail; }

                        /* Solve */
                        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                                        OP_SLV, 0)) != 0) { goto fail; }
                        break;

                    case MLUA_AST_EXPRESSION_SUFFIXED_TYPE_UNKNOWN:
                        MULTIPLE_ERROR_INTERNAL();
                        ret = -MULTIPLE_ERR_INTERNAL;
                        goto fail;
                }

                /* Sub Object */
                if ((ret = mlua_icodegen_expression(err, \
                                context, \
                                icg_fcb_block, \
                                exp_cur->u.suffixed->sub)) != 0)
                { goto fail; }

                /* State : <bottom> elements, count - 1, exp, index, sub <top> */
                if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                                OP_HASHADD, 0)) != 0) { goto fail; }

                break;

            case MLUA_AST_EXPRESSION_TYPE_FACTOR:
            case MLUA_AST_EXPRESSION_TYPE_PREFIX:
            case MLUA_AST_EXPRESSION_TYPE_TBLCTOR:
            case MLUA_AST_EXPRESSION_TYPE_BINOP:
            case MLUA_AST_EXPRESSION_TYPE_UNOP:
            case MLUA_AST_EXPRESSION_TYPE_FUNCALL:
            case MLUA_AST_EXPRESSION_TYPE_FUNDEF:
            case MLUA_AST_EXPRESSION_TYPE_UNKNOWN:
                MULTIPLE_ERROR_INTERNAL();
                ret = -MULTIPLE_ERR_INTERNAL;
                goto fail;
        }

        idx -= 1;
        exp_cur = exp_cur->prev; 
    }

    /* Drop the count */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                    OP_DROP, 0)) != 0) { goto fail; }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    return ret;
}


static int mlua_icodegen_statement_assignment(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_statement_assignment *stmt_assignment)
{
    return mlua_icodegen_statement_assignment_raw(err, \
            context, \
            icg_fcb_block, \
            stmt_assignment->varlist, \
            stmt_assignment->explist);
}


static int mlua_icodegen_statement_local_raw(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_namelist *varlist, \
        struct mlua_ast_expression_list *explist)
{
    int ret = 0;
    struct mlua_ast_name *name_cur;
    struct mlua_ast_expression *exp_cur;
    uint32_t id;

    /* Right values */
    /* Push the arguments in order */
    exp_cur = explist->begin;
    while (exp_cur != NULL)
    {
        if ((ret = mlua_icodegen_expression(err, \
                        context, \
                        icg_fcb_block, \
                        exp_cur)) != 0)
        { goto fail; }

        exp_cur = exp_cur->next; 
    }

    /* Left Values */
    /* Push the arguments in reverse order and followed with an "assign" */
    name_cur = varlist->end;
    while (name_cur != NULL)
    {
        if ((ret = multiply_resource_get_id( \
                        err, \
                        context->icode, \
                        context->res_id, \
                        &id, \
                        name_cur->name->str, \
                        name_cur->name->len)) != 0)
        { goto fail; }
        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                        OP_POPC, id)) != 0) { goto fail; }

        name_cur = name_cur->prev; 
    }

    goto done;
fail:
done:
    return ret;
}


static int mlua_icodegen_statement_local(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_statement_local *stmt_local)
{
    return mlua_icodegen_statement_local_raw(err, \
            context, \
            icg_fcb_block, \
            stmt_local->namelist, \
            stmt_local->explist);
}


static int mlua_icodegen_statement_expr(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_statement_expr *stmt_expr)
{
    int ret = 0;

    if ((ret = mlua_icodegen_expression(err, \
                    context, \
                    icg_fcb_block, \
                    stmt_expr->expr)) != 0)
    { goto fail; }
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_DROP, 0)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    return ret;
}


static int mlua_icodegen_statement_if(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_map_offset_label_list *map_offset_label_list, \
        struct mlua_ast_statement_if *stmt_if, \
        struct multiply_offset_item_pack *offset_pack_break)
{
    int ret = 0;
    uint32_t offset;
    struct mlua_ast_statement_elseif *stmt_elseif_tmp = NULL;

    struct mlua_ast_statement_elseif *stmt_elseif_cur = NULL;
    struct multiply_offset_item_pack *offset_item_pack_jmpc = NULL;
    struct multiply_offset_item_pack *offset_item_pack_jmp_to_end = NULL;
    struct multiply_offset_item_pack *offset_item_pack_condition = NULL;
    struct multiply_offset_item *cur_item_1 = NULL, *cur_item_2 = NULL;

    if ((offset_item_pack_jmpc = multiply_offset_item_pack_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    if ((offset_item_pack_jmp_to_end = multiply_offset_item_pack_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    if ((offset_item_pack_condition = multiply_offset_item_pack_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    for (;;)
    {

        offset = (uint32_t)icg_fcb_block->size;
        if ((ret = multiply_offset_item_pack_push_back(offset_item_pack_condition, offset)) != 0) { goto fail; }

        /* Condition Expression */
        if ((ret = mlua_icodegen_expression(err, \
                        context, \
                        icg_fcb_block, \
                        stmt_elseif_cur == NULL ? stmt_if->exp : stmt_elseif_cur->exp)) != 0)
        { goto fail; }
        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_NOTL, 0)) != 0)
        { goto fail; }

        /* Conditional Jump */
        /* If false, jump to next condition */
        /* Record position of jmpc, 
         * will fill the operand after confirmed the target position of jump */
        offset = (uint32_t)icg_fcb_block->size;
        if ((ret = multiply_offset_item_pack_push_back(offset_item_pack_jmpc, offset)) != 0) { goto fail; }

        /* Conditional Jump */
        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_JMPCR, 0)) != 0)
        { goto fail; }

        /* "THEN" Statements */
        if ((ret = mlua_icodegen_statement_list(err, \
                        context, \
                        icg_fcb_block, \
                        map_offset_label_list, \
                        stmt_elseif_cur == NULL ? stmt_if->block_then : stmt_elseif_cur->block_then, \
                        offset_pack_break)) != 0)

        { goto fail; }

        /* Jump to "END" */
        offset = (uint32_t)icg_fcb_block->size;
        if ((ret = multiply_offset_item_pack_push_back(offset_item_pack_jmp_to_end, offset)) != 0) { goto fail; }

        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_JMPR, 0)) != 0)
        { goto fail; }

        stmt_elseif_tmp = ((stmt_elseif_cur == NULL) ? stmt_if->elseif : stmt_elseif_cur->elseif);
        if (stmt_elseif_tmp != NULL)
        {
            /* "ELIF" */
            stmt_elseif_cur = stmt_elseif_tmp;
        }
        else if ((stmt_elseif_cur == NULL) && (stmt_if->block_else))
        {
            /* offset of else */
            offset = (uint32_t)icg_fcb_block->size;
            if ((ret = multiply_offset_item_pack_push_back(offset_item_pack_condition, offset)) != 0) { goto fail; }

            /* "ELSE" Statements */
            if ((ret = mlua_icodegen_statement_list(err, \
                            context, \
                            icg_fcb_block, \
                            map_offset_label_list, \
                            stmt_if->block_else, \
                            offset_pack_break)) != 0)
            { goto fail; }

            break;
        }
        else
        {
            break;
        }
    }

    /* offset of end */
    offset = (uint32_t)icg_fcb_block->size;
    if ((ret = multiply_offset_item_pack_push_back(offset_item_pack_condition, offset)) != 0) { goto fail; }

    /* Return back to fill the target position of jump */
    /* offset of end */
    offset = (uint32_t)icg_fcb_block->size;
    cur_item_1 = offset_item_pack_jmp_to_end->begin;
    while (cur_item_1 != NULL)
    {
        if ((ret = mlua_icg_fcb_block_link_relative(icg_fcb_block, cur_item_1->offset, offset)) != 0) { goto fail; }
        cur_item_1 = cur_item_1->next;
    }

    cur_item_1 = offset_item_pack_jmpc->begin;
    cur_item_2 = offset_item_pack_condition->begin;
    if (cur_item_2 != NULL && cur_item_2->next != NULL) cur_item_2 = cur_item_2->next;
    /* First condition (the one after if) should be skip */
    while (cur_item_1 != NULL)
    {
        if ((ret = mlua_icg_fcb_block_link_relative(icg_fcb_block, cur_item_1->offset, cur_item_2 != NULL ? cur_item_2->offset : offset)) != 0) { goto fail; }
        cur_item_1 = cur_item_1->next;
        if (cur_item_2 != NULL) cur_item_2 = cur_item_2->next;
    }

    goto done;
fail:
done:
    if (offset_item_pack_jmpc != NULL) multiply_offset_item_pack_destroy(offset_item_pack_jmpc);
    if (offset_item_pack_jmp_to_end != NULL) multiply_offset_item_pack_destroy(offset_item_pack_jmp_to_end);
    if (offset_item_pack_condition != NULL) multiply_offset_item_pack_destroy(offset_item_pack_condition);
    return ret;
}


static int mlua_icodegen_statement_while(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_map_offset_label_list *map_offset_label_list, \
        struct mlua_ast_statement_while *stmt_while)
{
    int ret = 0;
    uint32_t offset_condition;
    uint32_t offset_jmpc;
    uint32_t offset_end;

    struct multiply_offset_item_pack *offset_item_pack_break = NULL;
    struct multiply_offset_item *cur_item_1 = NULL;
    uint32_t offset;

    offset_condition = (uint32_t)icg_fcb_block->size;

    /* Condition Expression */
    if ((ret = mlua_icodegen_expression(err, \
                        context, \
                        icg_fcb_block, \
                        stmt_while->exp)) != 0)
    { goto fail; }
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_NOTL, 0)) != 0)
    { goto fail; }

    /* Conditional Jump */
    /* If false, jump to next condition */
    /* Record position of jmpc, 
     * will fill the operand after confirmed the target position of jump */
    offset_jmpc = (uint32_t)icg_fcb_block->size;

    /* Conditional Jump */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_JMPCR, 0)) != 0)
    { goto fail; }

    /* Record offsets of break */
    if ((offset_item_pack_break = multiply_offset_item_pack_new()) == NULL) 
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    /* "DO" Statements */
    if ((ret = mlua_icodegen_statement_list(err, \
                    context, \
                    icg_fcb_block, \
                    map_offset_label_list, \
                    stmt_while->block, \
                    offset_item_pack_break)) != 0)
    { goto fail; }

    /* Jump back to condition */
    offset = (uint32_t)icg_fcb_block->size;
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                    OP_JMPR, snr_sam_to_cmp((int32_t)offset_condition - (int32_t)offset))) != 0)
    { goto fail; }

    /* Fill back offsets of breaks */
    offset = (uint32_t)icg_fcb_block->size;
    cur_item_1 = offset_item_pack_break->begin;
    while (cur_item_1 != NULL)
    {
        if ((ret = mlua_icg_fcb_block_link_relative(icg_fcb_block, cur_item_1->offset, offset)) != 0) { goto fail; }
        cur_item_1 = cur_item_1->next;
    }

    offset_end = (uint32_t)icg_fcb_block->size;
    if ((ret = mlua_icg_fcb_block_link_relative(icg_fcb_block, offset_jmpc, offset_end)) != 0) { goto fail; }

    ret = 0;
    goto done;
fail:
done:
    if (offset_item_pack_break != NULL) multiply_offset_item_pack_destroy(offset_item_pack_break);
    return ret;
}


static int mlua_icodegen_statement_repeat(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_map_offset_label_list *map_offset_label_list, \
        struct mlua_ast_statement_repeat *stmt_repeat)
{
    int ret = 0;
    uint32_t offset_head;
    uint32_t offset_jmpc;

    struct multiply_offset_item_pack *offset_item_pack_break = NULL;
    struct multiply_offset_item *cur_item_1 = NULL;
    uint32_t offset;

    /* Record offsets of break */
    if ((offset_item_pack_break = multiply_offset_item_pack_new()) == NULL) 
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    offset_head = (uint32_t)icg_fcb_block->size;

    /* Statements in body */
    if ((ret = mlua_icodegen_statement_list(err, \
                    context, \
                    icg_fcb_block, \
                    map_offset_label_list, \
                    stmt_repeat->block, \
                    offset_item_pack_break)) != 0)
    { goto fail; }

    /* Condition Expression */
    if ((ret = mlua_icodegen_expression(err, \
                        context, \
                        icg_fcb_block, \
                        stmt_repeat->exp)) != 0)
    { goto fail; }

    /* Conditional Jump */
    offset_jmpc = (uint32_t)icg_fcb_block->size;
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_JMPCR, 0)) != 0)
    { goto fail; }

    /* Fill back offsets of breaks */
    offset = (uint32_t)icg_fcb_block->size;
    cur_item_1 = offset_item_pack_break->begin;
    while (cur_item_1 != NULL)
    {
        if ((ret = mlua_icg_fcb_block_link_relative(icg_fcb_block, cur_item_1->offset, offset)) != 0) 
        { goto fail; }
        cur_item_1 = cur_item_1->next;
    }

    /* Jump back to head */
    if ((ret = mlua_icg_fcb_block_link_relative(icg_fcb_block, offset_jmpc, offset_head)) != 0) 
    { goto fail; }

    ret = 0;
    goto done;
fail:
done:
    if (offset_item_pack_break != NULL) multiply_offset_item_pack_destroy(offset_item_pack_break);
    return ret;
}


static int mlua_icodegen_statement_do(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_map_offset_label_list *map_offset_label_list, \
        struct mlua_ast_statement_do *stmt_do, \
        struct multiply_offset_item_pack *offset_item_pack_break)
{
    int ret = 0;

    /* Statements in body */
    if ((ret = mlua_icodegen_statement_list(err, \
                    context, \
                    icg_fcb_block, \
                    map_offset_label_list, \
                    stmt_do->block, \
                    offset_item_pack_break)) != 0)
    { goto fail; }

    ret = 0;
    goto done;
fail:
done:
    return ret;
}


static int mlua_icodegen_statement_break(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct multiply_offset_item_pack *offset_pack_break)
{
    int ret = 0;
    uint32_t offset;

    (void)err;
    (void)context;

    offset = (uint32_t)icg_fcb_block->size;

    if ((ret = multiply_offset_item_pack_push_back(offset_pack_break, offset)) != 0)
    { goto fail; }

    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_JMPR, 0)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    return ret;
}

static int mlua_icodegen_statement_label(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_statement_label *stmt_label)
{
    int ret = 0;
    uint32_t offset;

    (void)err;
    (void)context;

    offset = (uint32_t)icg_fcb_block->size;

    if ((ret = multiply_offset_item_pack_push_back_label(context->offset_item_pack_stack->top, \
                    offset, \
                    stmt_label->name->str, \
                    stmt_label->name->len, \
                    icg_fcb_block)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    return ret;
}


static int mlua_icodegen_statement_goto(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_map_offset_label_list *map_offset_label_list, \
        struct mlua_ast_statement_goto *stmt_goto)
{
    int ret = 0;
    uint32_t offset;

    (void)err;
    (void)context;

    offset = (uint32_t)icg_fcb_block->size;

    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                    OP_JMPR, 0)) != 0)
    { goto fail; }

    if (mlua_map_offset_label_list_append_with_configure( \
                map_offset_label_list, \
                offset, \
                stmt_goto->name->str, \
                stmt_goto->name->len, \
                stmt_goto->name->pos_ln, \
                stmt_goto->name->pos_col) != 0)
    {
        MULTIPLE_ERROR_INTERNAL();
        ret = -MULTIPLE_ERR_INTERNAL;
        goto fail;
    }

    goto done;
fail:
done:
    return ret;
}


static int mlua_icodegen_statement_fundef(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_statement_fundef *stmt_fundef)
{
    int ret = 0;
    struct mlua_icg_fcb_block *new_icg_fcb_block = NULL;
    struct multiple_ir_export_section_item *new_export_section_item = NULL;
    struct mlua_map_offset_label_list *new_map_offset_label_list = NULL;
    uint32_t id;

    /* New function */
    new_icg_fcb_block = mlua_icg_fcb_block_new();
    if (new_icg_fcb_block == NULL)
    {
        MULTIPLE_ERROR_MALLOC();
        ret = -MULTIPLE_ERR_MALLOC;
        goto fail;
    }
    new_export_section_item = multiple_ir_export_section_item_new();
    if (new_export_section_item == NULL)
    {
        MULTIPLE_ERROR_MALLOC();
        ret = -MULTIPLE_ERR_MALLOC;
        goto fail;
    }
    new_export_section_item->blank = 1;
    new_map_offset_label_list = mlua_map_offset_label_list_new();
    if (new_map_offset_label_list == NULL)
    {
        MULTIPLE_ERROR_MALLOC();
        ret = -MULTIPLE_ERR_MALLOC;
        goto fail;
    }

    /* Parameter */
    if ((ret = mlua_icodegen_parlist(err, \
                    context, \
                    new_icg_fcb_block, \
                    stmt_fundef->parameters)) != 0)
    { goto fail; }

    /* Body */
    if ((ret = mlua_icodegen_statement_list(err, context, \
                    new_icg_fcb_block, \
                    new_map_offset_label_list, \
                    stmt_fundef->body, \
                    NULL)) != 0)
    { goto fail; }
    /* Apply goto to label */
    if ((ret = mlua_icodegen_statement_list_apply_goto(err, \
                    context, \
                    new_icg_fcb_block, \
                    new_map_offset_label_list)) != 0)
    { goto fail; }
    /* Pop a label offset pack */
    multiply_offset_item_pack_stack_pop(context->offset_item_pack_stack);

    /* Return */
    if ((stmt_fundef->body->end != NULL) && \
            (stmt_fundef->body->end->type == MLUA_AST_STATEMENT_TYPE_RETURN))
    {
        /* Do nothing */
    }
    else
    {
        if ((ret = mlua_icg_fcb_block_append_with_configure(new_icg_fcb_block, \
                        OP_LSTMK, 0)) != 0) { goto fail; }
        if ((ret = mlua_icg_fcb_block_append_with_configure(new_icg_fcb_block, \
                        OP_RETURN, 0)) != 0) { goto fail; }
    }

    /* Make Lambda */
    if ((ret = mlua_icg_fcb_block_append_with_configure_type(icg_fcb_block, \
                    OP_LAMBDAMK, (uint32_t)(context->icg_fcb_block_list->size), MLUA_ICG_FCB_LINE_TYPE_LAMBDA_MK)) != 0)
    { goto fail; }

    if ((stmt_fundef->funcname->name_list->size == 1) && \
            (stmt_fundef->funcname->member == NULL))
    {
        if ((ret = multiply_resource_get_id( \
                        err, 
                        context->icode, \
                        context->res_id, \
                        &id, \
                        stmt_fundef->funcname->name_list->begin->name->str, \
                        stmt_fundef->funcname->name_list->begin->name->len)) != 0)
        { goto fail; }
        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                        (stmt_fundef->local != 0) ? OP_POPC : OP_POPG, id)) != 0) 
        { goto fail; }
    }
    else
    {
        MULTIPLE_ERROR_INTERNAL();
        ret = -MULTIPLE_ERR_INTERNAL;
        goto fail;
    }

    /* Append block */
    if ((ret = mlua_icg_fcb_block_list_append(context->icg_fcb_block_list, new_icg_fcb_block)) != 0)
    {
        MULTIPLE_ERROR_INTERNAL();
        ret = -MULTIPLE_ERR_INTERNAL;
        goto fail;
    }
    /* Append blank export section */
    if ((ret = multiple_ir_export_section_append(context->icode->export_section, new_export_section_item)) != 0)
    {
        MULTIPLE_ERROR_INTERNAL();
        ret = -MULTIPLE_ERR_INTERNAL;
        goto fail;
    }


    goto done;
fail:
done:
    if (new_map_offset_label_list != NULL) mlua_map_offset_label_list_destroy(new_map_offset_label_list);
    return ret;
}


static int mlua_icodegen_statement_return(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_map_offset_label_list *map_offset_label_list, \
        struct mlua_ast_statement_return *stmt_return)
{
    int ret = 0;
    uint32_t id;
    struct mlua_ast_expression_funcall *exp_funcall;
    int args_count = 0;

    (void)map_offset_label_list;

    if ((0)&&(stmt_return->explist->size == 1) && \
            (stmt_return->explist->begin->type == MLUA_AST_EXPRESSION_TYPE_FUNCALL))
    {
        /* Tail call */
        exp_funcall = stmt_return->explist->begin->u.funcall;

        /* Disable GC */
        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                        OP_IDGC, 0)) != 0)
        { goto fail; }

        /* Push arguments in reverse order */
        if ((ret = mlua_icodegen_args(err, \
                        context, \
                        icg_fcb_block, \
                        exp_funcall->args)) != 0)
        { goto fail; }
        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                        OP_REVERSE, 0)) != 0)
        { goto fail; }

        switch (exp_funcall->args->type)
        {
            case MLUA_AST_ARGS_TYPE_STRING:
                args_count = 1;
                break;

            case MLUA_AST_ARGS_TYPE_EXPLIST:
                args_count = (int)exp_funcall->args->u.explist->size;
                break;

            case MLUA_AST_ARGS_TYPE_TBLCTOR:
                args_count = 1;
                break;

            case MLUA_AST_ARGS_TYPE_UNKNOWN:
                MULTIPLE_ERROR_INTERNAL();
                ret = -MULTIPLE_ERR_INTERNAL;
                goto fail;
        }

        /* Lift */
        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                        OP_LIFT, (uint32_t)args_count )) != 0)
        { goto fail; }

        /* Re-enable GC */
        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                        OP_IEGC, 0)) != 0)
        { goto fail; }

        /* Argument count */
        if ((ret = multiply_resource_get_int( \
                        err, \
                        context->icode, \
                        context->res_id, \
                        &id, \
                        (int)args_count)) != 0)
        { goto fail; }
        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                        OP_PUSH, id)) != 0)
        { goto fail; }

        /* Function */
        if ((ret = mlua_icodegen_expression(err, \
                        context, \
                        icg_fcb_block, \
                        exp_funcall->prefixexp)) != 0)
        { goto fail; }

        /* Perform Tail-call */
        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                        OP_SLV, 0)) != 0)
        { goto fail; }
        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                        OP_FUNCMK, 0)) != 0)
        { goto fail; }
        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                        OP_TAILCALLC, 0)) != 0)
        { goto fail; }

    }
    else
    {
        /* Normal call */

        if ((ret = mlua_icodegen_explist(err, \
                        context, \
                        icg_fcb_block, \
                        stmt_return->explist)) != 0)
        { goto fail; }

        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                        OP_DROP, 0)) != 0)
        { goto fail; }

        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                        OP_LSTMK, (uint32_t)(stmt_return->explist->size))) != 0)
        { goto fail; }
        if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                        OP_RETURN, 0)) != 0)
        { goto fail; }
    }

    goto done;
fail:
done:
    return ret;
}


static int mlua_icodegen_statement(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_map_offset_label_list *map_offset_label_list, \
        struct mlua_ast_statement *stmt, \
        struct multiply_offset_item_pack *offset_pack_break)
{
    int ret = 0;

    switch (stmt->type)
    {
        case MLUA_AST_STATEMENT_TYPE_ASSIGNMENT:
            if ((ret = mlua_icodegen_statement_assignment(err, context, \
                            icg_fcb_block, stmt->u.stmt_assignment)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_STATEMENT_TYPE_LOCAL:
            if ((ret = mlua_icodegen_statement_local(err, context, \
                            icg_fcb_block, stmt->u.stmt_local)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_STATEMENT_TYPE_EXPR:
            if ((ret = mlua_icodegen_statement_expr(err, context, \
                            icg_fcb_block, stmt->u.stmt_expr)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_STATEMENT_TYPE_FUNCALL:
            /* Call as an expression */
            if ((ret = mlua_icodegen_expression_funcall(err, context, \
                            icg_fcb_block, \
                            stmt->u.funcall)) != 0)
            { goto fail; }
            /* Drop the return value */
            if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                            OP_DROP, 0)) != 0) { goto fail; }

            break;

        case MLUA_AST_STATEMENT_TYPE_IF:
            if ((ret = mlua_icodegen_statement_if(err, context, \
                            icg_fcb_block, map_offset_label_list, \
                            stmt->u.stmt_if, offset_pack_break)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_STATEMENT_TYPE_WHILE:
            if ((ret = mlua_icodegen_statement_while(err, context, \
                            icg_fcb_block, map_offset_label_list, \
                            stmt->u.stmt_while)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_STATEMENT_TYPE_REPEAT:
            if ((ret = mlua_icodegen_statement_repeat(err, context, \
                            icg_fcb_block, map_offset_label_list, \
                            stmt->u.stmt_repeat)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_STATEMENT_TYPE_DO:
            if ((ret = mlua_icodegen_statement_do(err, context, \
                            icg_fcb_block, map_offset_label_list, \
                            stmt->u.stmt_do, offset_pack_break)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_STATEMENT_TYPE_BREAK:
            if ((ret = mlua_icodegen_statement_break(err, context, \
                            icg_fcb_block, \
                            offset_pack_break)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_STATEMENT_TYPE_LABEL:
            if ((ret = mlua_icodegen_statement_label(err, context, \
                            icg_fcb_block, \
                            stmt->u.stmt_label)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_STATEMENT_TYPE_GOTO:
            if ((ret = mlua_icodegen_statement_goto(err, context, \
                            icg_fcb_block, \
                            map_offset_label_list, stmt->u.stmt_goto)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_STATEMENT_TYPE_FUNDEF:
            if ((ret = mlua_icodegen_statement_fundef(err, context, \
                            icg_fcb_block, \
                            stmt->u.stmt_fundef)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_STATEMENT_TYPE_RETURN:
            if ((ret = mlua_icodegen_statement_return(err, context, \
                            icg_fcb_block, \
                            map_offset_label_list, stmt->u.stmt_return)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_STATEMENT_TYPE_FOR:
        case MLUA_AST_STATEMENT_TYPE_UNKNOWN:
            MULTIPLE_ERROR_NOT_IMPLEMENTED();
            ret = -MULTIPLE_ERR_NOT_IMPLEMENTED;
            goto fail;
    }

    goto done;
fail:
done:
    return ret;
}


int mlua_icodegen_statement_list(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_map_offset_label_list *map_offset_label_list, \
        struct mlua_ast_statement_list *list, \
        struct multiply_offset_item_pack *offset_pack_break)
{
    int ret = 0;
    struct mlua_ast_statement *stmt_cur;
    struct multiply_offset_item_pack *new_offset_item_pack = NULL;

    /* For recording labels */
    new_offset_item_pack = multiply_offset_item_pack_new();
    if (new_offset_item_pack == NULL) 
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }
    multiply_offset_item_pack_stack_push(context->offset_item_pack_stack, \
            new_offset_item_pack);
    new_offset_item_pack = NULL;

    /* Statements */
    stmt_cur = list->begin;
    while (stmt_cur != NULL)
    {
        if ((ret = mlua_icodegen_statement(err, \
                        context, \
                        icg_fcb_block, \
                        map_offset_label_list, \
                        stmt_cur, \
                        offset_pack_break)) != 0)
        { goto fail; }
        stmt_cur = stmt_cur->next; 
    }

    goto done;
fail:
done:
    return ret;
}

