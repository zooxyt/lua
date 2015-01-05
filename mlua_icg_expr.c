/* Multiple Lua Programming Language : Intermediate Code Generator
 * Expression
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


int mlua_icodegen_expression(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression *exp);


int mlua_icodegen_args(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_args *args)
{
    int ret = 0;
    char *buffer_str = NULL;
    size_t buffer_str_len;
    uint32_t id;
    
    switch (args->type)
    {
        case MLUA_AST_ARGS_TYPE_STRING:

            /* String */
            buffer_str_len = args->u.str->len;
            buffer_str = (char *)malloc(sizeof(char) * (buffer_str_len + 1));
            if (buffer_str == NULL)
            {
                MULTIPLE_ERROR_MALLOC();
                ret = -MULTIPLE_ERR_MALLOC;
                goto fail; 
            }
            memcpy(buffer_str, args->u.str->str, args->u.str->len);
            buffer_str[buffer_str_len] = '\0';
            multiply_replace_escape_chars(buffer_str, &buffer_str_len);

            if ((ret = multiply_resource_get_str( \
                            err, \
                            context->icode, \
                            context->res_id, \
                            &id, \
                            buffer_str, \
                            buffer_str_len)) != 0)
            { goto fail; }
            if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_PUSH, id)) != 0) 
            { goto fail; }

            free(buffer_str);
            buffer_str = NULL;

            /* Arguments count */
            if ((ret = multiply_resource_get_int( \
                            err, \
                            context->icode, \
                            context->res_id, \
                            &id, \
                            1)) != 0)
            { goto fail; }
            if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_PUSH, id)) != 0) 

            break;

        case MLUA_AST_ARGS_TYPE_EXPLIST:

            /* Arguments with count */
            if ((ret = mlua_icodegen_explist(err, \
                            context, \
                            icg_fcb_block, \
                            args->u.explist)) != 0)
            { goto fail; }

            /* Reverse the arguments */
            if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_REVERSEP, 0)) != 0)
            { goto fail; }

            break;

        case MLUA_AST_ARGS_TYPE_TBLCTOR:
        case MLUA_AST_ARGS_TYPE_UNKNOWN:
            MULTIPLE_ERROR_INTERNAL();
            ret = -MULTIPLE_ERR_INTERNAL;
            goto fail;
    }

    goto done;
fail:
    if (buffer_str != NULL) free(buffer_str);
done:
    return ret;
}


static int mlua_icodegen_expression_prefix(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_prefix *exp_prefix)
{
    int ret = 0;
    struct multiply_text_precompiled *new_text_precompiled = NULL;

    switch (exp_prefix->type)
    {
        case MLUA_AST_PREFIX_EXP_TYPE_VAR:

            if ((ret = mlua_icg_customizable_built_in_procedure_list_called( \
                            context->customizable_built_in_procedure_list, \
                            exp_prefix->u.var->str, \
                            exp_prefix->u.var->len)) != 0)
            { goto fail; }

            if ((ret = multiply_asm_precompile(err, \
                            context->icode, \
                            context->res_id, \
                            &new_text_precompiled, \

                            MULTIPLY_ASM_OP_INT , OP_PUSH    , 1          , 
                            MULTIPLY_ASM_OP_ID  , OP_PUSH    , exp_prefix->u.var->str,
                            MULTIPLY_ASM_OP     , OP_SLV     , 
                            MULTIPLY_ASM_OP     , OP_FUNCMK  , 
                            MULTIPLY_ASM_OP     , OP_CALLC   , 
                            MULTIPLY_ASM_OP     , OP_DROP    , 

                            MULTIPLY_ASM_FINISH)) != 0)
            { goto fail; }

            if ((ret = mlua_icg_fcb_block_append_from_precompiled_pic_text( \
                            icg_fcb_block, \
                            new_text_precompiled)) != 0)
            { goto fail; }

            break;

        case MLUA_AST_PREFIX_EXP_TYPE_FUNCALL:
        case MLUA_AST_PREFIX_EXP_TYPE_EXP:
        case MLUA_AST_PREFIX_EXP_TYPE_UNKNOWN:
            MULTIPLE_ERROR_INTERNAL();
            ret = -MULTIPLE_ERR_INTERNAL;
            goto fail;
    }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    return ret;
}


static int mlua_icodegen_expression_suffixed(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_suffixed *exp_suffixed)
{
    int ret = 0;
    struct multiply_text_precompiled *new_text_precompiled = NULL;
    uint32_t id;
    const int LBL_HASKEY = 0, LBL_TAIL = 1;

    switch (exp_suffixed->type)
    {
        case MLUA_AST_EXPRESSION_SUFFIXED_TYPE_INDEX:

            /* Index */
            if ((ret = mlua_icodegen_expression(err, \
                            context, \
                            icg_fcb_block, \
                            exp_suffixed->u.exp)) != 0)
            { goto fail; }
            if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_SLV, 0)) != 0) 
            { goto fail; }

            /* Sub Object */
            if ((ret = mlua_icodegen_expression(err, \
                            context, \
                            icg_fcb_block, \
                            exp_suffixed->sub)) != 0)
            { goto fail; }

            break;

        case MLUA_AST_EXPRESSION_SUFFIXED_TYPE_MEMBER:

            /* Standard Library */
            if (exp_suffixed->sub->type == MLUA_AST_EXPRESSION_TYPE_PRIMARY)
            {
                if (exp_suffixed->sub->u.primary->type == MLUA_AST_EXPRESSION_PRIMARY_TYPE_NAME)
                {
                    if ((mlua_icg_stdlib_table_list_register(context->stdlibs, \
                                    exp_suffixed->sub->u.primary->u.name->str, \
                                    exp_suffixed->sub->u.primary->u.name->len, \
                                    exp_suffixed->u.name->str, 
                                    exp_suffixed->u.name->len)) != 0)
                    {
                        MULTIPLE_ERROR_INTERNAL();
                        ret = -MULTIPLE_ERR_INTERNAL;
                        goto fail;
                    }
                }
            }

            /* Index */
            if ((ret = multiply_resource_get_str( \
                            err,
                            context->icode, \
                            context->res_id, \
                            &id, \
                            exp_suffixed->u.name->str, 
                            exp_suffixed->u.name->len)) != 0)
            { goto fail; }
            if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                            OP_PUSH, id)) != 0) { goto fail; }

            /* Sub Object */
            if ((ret = mlua_icodegen_expression(err, \
                            context, \
                            icg_fcb_block, \
                            exp_suffixed->sub)) != 0)
            { goto fail; }

            break;

        case MLUA_AST_EXPRESSION_SUFFIXED_TYPE_UNKNOWN:
            MULTIPLE_ERROR_INTERNAL();
            ret = -MULTIPLE_ERR_INTERNAL;
            goto fail;
    }


    /* Can not just simply get the member but test it if exist */

    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled, \

                    /* State : <bottom> index, hash <top> */
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 2          , 
                    MULTIPLY_ASM_OP     , OP_PICKCP  , 
                    MULTIPLY_ASM_OP_INT , OP_PUSH    , 2          , 
                    MULTIPLY_ASM_OP     , OP_PICKCP  , 

                    /* State : <bottom> index, hash, index, hash <top> */
                    MULTIPLY_ASM_OP     , OP_HASHHASKEY, 
                    MULTIPLY_ASM_OP_LBLR, OP_JMPCR   , LBL_HASKEY ,

                    /* Has no key, push a none */
                    MULTIPLY_ASM_OP     , OP_DROP    , 
                    MULTIPLY_ASM_OP     , OP_DROP    , 
                    MULTIPLY_ASM_OP_NONE, OP_PUSH    , 
                    MULTIPLY_ASM_OP_LBLR, OP_JMPR    , LBL_TAIL ,

                    /* Has key */
                    MULTIPLY_ASM_LABEL  , LBL_HASKEY , 
                    MULTIPLY_ASM_OP     , OP_REFGET    , 

                    MULTIPLY_ASM_LABEL  , LBL_TAIL   , 

                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; }

    if ((ret = mlua_icg_fcb_block_append_from_precompiled_pic_text( \
                    icg_fcb_block, \
                    new_text_precompiled)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    return ret;
}


int mlua_icodegen_expression_funcall(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_funcall *exp_funcall)
{
    int ret = 0;
    struct multiply_text_precompiled *new_text_precompiled = NULL;

    /* Arguments */
    if ((ret = mlua_icodegen_args(err, \
                    context, \
                    icg_fcb_block, \
                    exp_funcall->args)) != 0)
    { goto fail; }

    /* Function */
    if ((ret = mlua_icodegen_expression(err, \
                    context, \
                    icg_fcb_block, \
                    exp_funcall->prefixexp)) != 0)
    { goto fail; }

    /* Call */
    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled, \

                    MULTIPLY_ASM_OP     , OP_FUNCMK  , 
                    MULTIPLY_ASM_OP     , OP_CALLC   , 

                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; }

    if ((ret = mlua_icg_fcb_block_append_from_precompiled_pic_text( \
                    icg_fcb_block, \
                    new_text_precompiled)) != 0)
    { goto fail; }


    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    return ret;
}


static int mlua_icodegen_expression_factor(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_factor *exp_factor)
{
    int ret = 0;
    uint32_t id;
    char *buffer_str = NULL;
    size_t buffer_str_len;
    int value_int;
    double value_float;
    
    switch (exp_factor->type)
    {
        case MLUA_AST_EXP_FACTOR_TYPE_NIL:
            if ((ret = multiply_resource_get_none(err, context->icode, context->res_id, &id)) != 0) 
            { goto fail; }
            break;

        case MLUA_AST_EXP_FACTOR_TYPE_FALSE:
            if ((ret = multiply_resource_get_false(err, context->icode, context->res_id, &id)) != 0) 
            { goto fail; }
            break;

        case MLUA_AST_EXP_FACTOR_TYPE_TRUE:
            if ((ret = multiply_resource_get_true(err, context->icode, context->res_id, &id)) != 0) 
            { goto fail; }
            break;

        case MLUA_AST_EXP_FACTOR_TYPE_INTEGER:
            if (multiply_convert_str_to_int(&value_int, \
                        exp_factor->token->str, exp_factor->token->len) != 0)
            {
                multiple_error_update(err, -MULTIPLE_ERR_ICODEGEN, \
                        "\'%s\' is an invalid integer", \
                        exp_factor->token->str);
                ret = -MULTIPLE_ERR_ICODEGEN; 
                goto fail; 
            }
            if ((ret = multiply_resource_get_int(err, context->icode, context->res_id, &id, \
                            value_int)) != 0) 
            { goto fail; }
            break;

        case MLUA_AST_EXP_FACTOR_TYPE_FLOAT:
            if (multiply_convert_str_to_float(&value_float, \
                        exp_factor->token->str, exp_factor->token->len) != 0)
            {
                multiple_error_update(err, -MULTIPLE_ERR_ICODEGEN, \
                        "\'%s\' is an invalid float", \
                        exp_factor->token->str);
                ret = -MULTIPLE_ERR_ICODEGEN; 
                goto fail; 
            }
            if ((ret = multiply_resource_get_float(err, context->icode, context->res_id, &id, \
                            value_float)) != 0) 
            { goto fail; }
            break;

        case MLUA_AST_EXP_FACTOR_TYPE_STRING:

            /* String */
            buffer_str_len = exp_factor->token->len;
            buffer_str = (char *)malloc(sizeof(char) * (buffer_str_len + 1));
            if (buffer_str == NULL)
            {
                MULTIPLE_ERROR_MALLOC();
                ret = -MULTIPLE_ERR_MALLOC;
                goto fail; 
            }
            memcpy(buffer_str, exp_factor->token->str, exp_factor->token->len);
            buffer_str[buffer_str_len] = '\0';
            multiply_replace_escape_chars(buffer_str, &buffer_str_len);

            if ((ret = multiply_resource_get_str(err, \
                            context->icode, \
                            context->res_id, \
                            &id, \
                            buffer_str, \
                            buffer_str_len)) != 0)
            { goto fail; }

            free(buffer_str);
            buffer_str = NULL;
            break;

        case MLUA_AST_EXP_FACTOR_TYPE_UNKNOWN:
            MULTIPLE_ERROR_INTERNAL();
            ret = -MULTIPLE_ERR_INTERNAL;
            goto fail;
    }

    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_PUSH, id)) != 0) 
    { goto fail; }

    goto done;
fail:
    if (buffer_str != NULL) free(buffer_str);
done:
    return ret;
}


static int mlua_icodegen_expression_tblctor(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_tblctor *exp_tblctor)
{
    int ret = 0;
    uint32_t id;
    struct mlua_ast_fieldlist *fieldlist;
    struct mlua_ast_field *field_cur;
    int idx = 1;

    fieldlist = exp_tblctor->fieldlist;

    field_cur = fieldlist->begin; 
    while (field_cur != NULL)
    {

        switch (field_cur->type)
        {
            case MLUA_AST_FIELD_TYPE_ARRAY:
                /* Key */
                if ((ret = mlua_icodegen_expression(err, \
                                context, \
                                icg_fcb_block, \
                                field_cur->u.array->index)) != 0)
                { goto fail; }

                /* Value */
                if ((ret = mlua_icodegen_expression(err, \
                                context, \
                                icg_fcb_block, \
                                field_cur->u.array->value)) != 0)
                { goto fail; }

                break;

            case MLUA_AST_FIELD_TYPE_PROPERTY:
                /* Key */
                if ((ret = multiply_resource_get_str( \
                                err, \
                                context->icode, \
                                context->res_id, \
                                &id, \
                                field_cur->u.property->name->name->str, \
                                field_cur->u.property->name->name->len)) != 0)
                { goto fail; }
                if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, \
                                OP_PUSH, id)) != 0) { goto fail; }

                /* Value */
                if ((ret = mlua_icodegen_expression(err, \
                                context, \
                                icg_fcb_block, \
                                field_cur->u.property->value)) != 0)
                { goto fail; }

                break;

            case MLUA_AST_FIELD_TYPE_EXP:
                /* Key */
                if ((ret = multiply_resource_get_int( \
                                err, \
                                context->icode, \
                                context->res_id, \
                                &id, \
                                idx)) != 0)
                { goto fail; }
                if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_PUSH, id)) != 0) 
                { goto fail; }

                /* Value */
                if ((ret = mlua_icodegen_expression(err, \
                                context, \
                                icg_fcb_block, \
                                field_cur->u.exp->value)) != 0)
                { goto fail; }

                break;

            case MLUA_AST_FIELD_TYPE_UNKNOWN:
                MULTIPLE_ERROR_INTERNAL();
                ret = -MULTIPLE_ERR_INTERNAL;
                goto fail;
        }

        idx += 1;
        field_cur = field_cur->next;
    }

    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_HASHMK, (uint32_t)fieldlist->size)) != 0) 
    { goto fail; }

    goto done;
fail:
done:
    return ret;
}


static int mlua_icodegen_expression_unop_not(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_unop *exp_unop)
{
    int ret = 0;
    uint32_t id_false, id_true, id_nil;
    uint32_t offset_true;
    uint32_t offset_to_tail, offset_tail;
    uint32_t offset_jmpcr_to_eq_to_nil, offset_jmpcr_to_eq_to_false;

    /* not always returns true or false */

    if ((ret = multiply_resource_get_false( \
                    err, \
                    context->icode, \
                    context->res_id, \
                    &id_false)) != 0)
    { goto fail; }
    if ((ret = multiply_resource_get_true( \
                    err, \
                    context->icode, \
                    context->res_id, \
                    &id_true)) != 0)
    { goto fail; }
    if ((ret = multiply_resource_get_none( \
                    err, \
                    context->icode, \
                    context->res_id, \
                    &id_nil)) != 0)
    { goto fail; }

    /* Exp */
    if ((ret = mlua_icodegen_expression(err, \
                    context, \
                    icg_fcb_block, \
                    exp_unop->sub)) != 0)
    { goto fail; }
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_DUP, 0)) != 0)
    { goto fail; }

    /* <bottom> exp, exp <top> */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_PUSH, id_nil)) != 0)
    { goto fail; }
    /* <bottom> exp, exp, nil <top> */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_EQ, 0)) != 0)
    { goto fail; }
    /* <bottom> epx, (exp == nil) <top> */
    offset_jmpcr_to_eq_to_nil = (uint32_t)icg_fcb_block->size;
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_JMPCR, 0)) != 0)
    { goto fail; }
    /* Not equals to nil, try false now */
    /* <bottom> exp <top> */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_DUP, 0)) != 0)
    { goto fail; }
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_PUSH, id_false)) != 0)
    { goto fail; }
    /* <bottom> exp, exp, false <top> */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_EQ, 0)) != 0)
    { goto fail; }
    /* <bottom> exp, (exp == false) <top> */
    offset_jmpcr_to_eq_to_false = (uint32_t)icg_fcb_block->size;
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_JMPCR, 0)) != 0)
    { goto fail; }
    /* The left operand is not false, 
     * regard it as true and return not(true) = false */
    /* <bottom> exp <top> */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_DROP, 0)) != 0)
    { goto fail; }
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_PUSH, id_false)) != 0)
    { goto fail; }
    offset_to_tail = (uint32_t)icg_fcb_block->size;
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_JMPR, 0)) != 0)
    { goto fail; }
    offset_true = (uint32_t)icg_fcb_block->size;
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_DROP, 0)) != 0)
    { goto fail; }
    /* <bottom> false <top> */
    /* regard it as false and return not(false) = true */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_PUSH, id_true)) != 0)
    { goto fail; }
    offset_tail = (uint32_t)icg_fcb_block->size;

    if ((ret = mlua_icg_fcb_block_link_relative(icg_fcb_block, offset_jmpcr_to_eq_to_nil, offset_true)) != 0) { goto fail; }
    if ((ret = mlua_icg_fcb_block_link_relative(icg_fcb_block, offset_jmpcr_to_eq_to_false, offset_true)) != 0) { goto fail; }
    if ((ret = mlua_icg_fcb_block_link_relative(icg_fcb_block, offset_to_tail, offset_tail)) != 0) { goto fail; }

    goto done;
fail:
done:
    return ret;
}


static int mlua_icodegen_expression_unop(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_unop *exp_unop)
{
    int ret = 0;
    uint32_t op;

    if (exp_unop->op->value == TOKEN_KEYWORD_NOT)
    {
        return mlua_icodegen_expression_unop_not(err, \
                context, \
                icg_fcb_block, \
                exp_unop);
    }

    if ((ret = mlua_icodegen_expression(err, \
                    context, \
                    icg_fcb_block, \
                    exp_unop->sub)) != 0)
    { goto fail; }

    /* Operation */
    switch (exp_unop->op->value)
    {
        case TOKEN_KEYWORD_NOT:
            op = OP_NOTL;
            break;
        case '-':
            op = OP_NEG;
            break;
        case '#':
            op = OP_SIZE;
            break;
        default:
            MULTIPLE_ERROR_INTERNAL();
            ret = -MULTIPLE_ERR_INTERNAL;
            goto fail;
    }
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, op, 0)) != 0) 
    { goto fail; }

    goto done;
fail:
done:
    return ret;
}

static int mlua_icodegen_expression_primary(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_primary *exp_primary)
{
    int ret = 0;
    uint32_t id;
    struct multiply_text_precompiled *new_text_precompiled = NULL;
    const int LBL_HAS_VAR = 0, LBL_TAIL = 1;

    switch (exp_primary->type)
    {
        case MLUA_AST_EXPRESSION_PRIMARY_TYPE_NAME:

            if ((ret = multiply_resource_get_id( \
                            err, \
                            context->icode, \
                            context->res_id, \
                            &id, \
                            exp_primary->u.name->str, \
                            exp_primary->u.name->len)) != 0)
            { goto fail; }

            if ((ret = multiply_asm_precompile(err, \
                            context->icode, \
                            context->res_id, \
                            &new_text_precompiled, \

                            /* if (var exists) goto lbl_hasvar; */
                            MULTIPLY_ASM_OP_RAW , OP_PUSH    , id, 
                            MULTIPLY_ASM_OP     , OP_TRYSLV  , 
                            MULTIPLY_ASM_OP_LBLR, OP_JMPCR   , LBL_HAS_VAR,

                            /* no var */
                            MULTIPLY_ASM_OP_NONE, OP_PUSH    , 
                            /* goto lbl_tail */
                            MULTIPLY_ASM_OP_LBLR, OP_JMPR    , LBL_TAIL,

                            /* lbl_hasvar: */
                            MULTIPLY_ASM_LABEL  , LBL_HAS_VAR ,
                            MULTIPLY_ASM_OP_RAW , OP_PUSH    , id, 

                            /* lbl_tail: */
                            MULTIPLY_ASM_LABEL  , LBL_TAIL ,

                            MULTIPLY_ASM_FINISH)) != 0)
            { goto fail; }
            if ((ret = mlua_icg_fcb_block_append_from_precompiled_pic_text( \
                            icg_fcb_block, \
                            new_text_precompiled)) != 0)
            { goto fail; }


            if ((ret = mlua_icg_customizable_built_in_procedure_list_called( \
                            context->customizable_built_in_procedure_list, \
                            exp_primary->u.name->str, \
                            exp_primary->u.name->len)) != 0)
            { goto fail; }

            break;

        case MLUA_AST_EXPRESSION_PRIMARY_TYPE_EXPR:
            if ((ret = mlua_icodegen_expression(err, \
                            context, \
                            icg_fcb_block, \
                            exp_primary->u.exp)) != 0)
            { goto fail; }

            break;

        case MLUA_AST_EXPRESSION_PRIMARY_TYPE_UNKNOWN:
            MULTIPLE_ERROR_INTERNAL();
            ret = -MULTIPLE_ERR_INTERNAL;
            goto fail;
    }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    return ret;
}


static int mlua_icodegen_expression_binop_and(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_binop *exp_binop)
{
    int ret = 0;
    uint32_t id;
    uint32_t offset_jmpcr_to_eq_to_nil, offset_jmpcr_to_eq_to_false;
    uint32_t offset_tail;

    /* Use Short-circuit evaluation */
    /* Regard nil and false are 'false' */

    /* For 'and' operator, if the left operand is false, 
     * then return the left operand, 
     * or return the right operand */

    /* Left */
    if ((ret = mlua_icodegen_expression(err, \
                    context, \
                    icg_fcb_block, \
                    exp_binop->left)) != 0)
    { goto fail; }
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_DUP, 0)) != 0)
    { goto fail; }

    /* <bottom> left, left <top> */
    if ((ret = multiply_resource_get_none(err, \
                    context->icode, \
                    context->res_id, \
                    &id)) != 0)
    { goto fail; }
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_PUSH, id)) != 0)
    { goto fail; }
    /* <bottom> left, left, nil <top> */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_EQ, 0)) != 0)
    { goto fail; }
    /* <bottom> left, (left == nil) <top> */
    offset_jmpcr_to_eq_to_nil = (uint32_t)icg_fcb_block->size;
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_JMPCR, 0)) != 0)
    { goto fail; }
    /* Not equals to nil, try false now */
    /* <bottom> left <top> */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_DUP, 0)) != 0)
    { goto fail; }
    if ((ret = multiply_resource_get_false( \
                    err, \
                    context->icode,
                    context->res_id, \
                    &id)) != 0)
    { goto fail; }
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_PUSH, id)) != 0)
    { goto fail; }
    /* <bottom> left, left, false <top> */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_EQ, 0)) != 0)
    { goto fail; }
    /* <bottom> left, (left == false) <top> */
    offset_jmpcr_to_eq_to_false = (uint32_t)icg_fcb_block->size;
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_JMPCR, 0)) != 0)
    { goto fail; }
    /* The left operand is not false, return the right one  */
    /* <bottom> left <top> */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_DROP, 0)) != 0)
    { goto fail; }
    /* Right */
    if ((ret = mlua_icodegen_expression(err, \
                    context, \
                    icg_fcb_block, \
                    exp_binop->right)) != 0)
    { goto fail; }
    /* <bottom> right <top> */

    offset_tail = (uint32_t)icg_fcb_block->size;

    if ((ret = mlua_icg_fcb_block_link_relative(icg_fcb_block, offset_jmpcr_to_eq_to_nil, offset_tail)) != 0) { goto fail; }
    if ((ret = mlua_icg_fcb_block_link_relative(icg_fcb_block, offset_jmpcr_to_eq_to_false, offset_tail)) != 0) { goto fail; }

    goto done;
fail:
done:
    return ret;
}


static int mlua_icodegen_expression_binop_or(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_binop *exp_binop)
{
    int ret = 0;
    uint32_t id;
    uint32_t offset_jmpcr_to_eq_to_nil, offset_jmpcr_to_eq_to_false;
    uint32_t offset_to_tail;
    uint32_t offset_right;
    uint32_t offset_tail;

    /* Use Short-circuit evaluation */
    /* Regard nil and false are 'false' */

    /* For 'or' operator, if the left operand is false, 
     * then return the right operand, 
     * or return the left operand */

    /* Left */
    if ((ret = mlua_icodegen_expression(err, \
                    context, \
                    icg_fcb_block, \
                    exp_binop->left)) != 0)
    { goto fail; }
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_DUP, 0)) != 0)
    { goto fail; }

    /* <bottom> left, left <top> */
    if ((ret = multiply_resource_get_none( \
                    err, \
                    context->icode, \
                    context->res_id, \
                    &id)) != 0)
    { goto fail; }
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_PUSH, id)) != 0)
    { goto fail; }
    /* <bottom> left, left, nil <top> */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_EQ, 0)) != 0)
    { goto fail; }
    /* <bottom> left, (left == nil) <top> */
    offset_jmpcr_to_eq_to_nil = (uint32_t)icg_fcb_block->size;
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_JMPCR, 0)) != 0)
    { goto fail; }
    /* Not equals to nil, try false now */
    /* <bottom> left <top> */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_DUP, 0)) != 0)
    { goto fail; }
    if ((ret = multiply_resource_get_false( \
                    err, \
                    context->icode, \
                    context->res_id, \
                    &id)) != 0)
    { goto fail; }
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_PUSH, id)) != 0)
    { goto fail; }
    /* <bottom> left, left, false <top> */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_EQ, 0)) != 0)
    { goto fail; }
    /* <bottom> left, (left == false) <top> */
    offset_jmpcr_to_eq_to_false = (uint32_t)icg_fcb_block->size;
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_JMPCR, 0)) != 0)
    { goto fail; }
    /* The left operand is not false, return the right one  */
    /* <bottom> left <top> */
    offset_to_tail = (uint32_t)icg_fcb_block->size;
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_JMPR, 0)) != 0)
    { goto fail; }
    offset_right = (uint32_t)icg_fcb_block->size;
    /* Right */
    /* <bottom> left <top> */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_DROP, 0)) != 0)
    { goto fail; }
    if ((ret = mlua_icodegen_expression(err, \
                    context, \
                    icg_fcb_block, \
                    exp_binop->right)) != 0)
    { goto fail; }
    /* <bottom> right <top> */
    offset_tail = (uint32_t)icg_fcb_block->size;

    if ((ret = mlua_icg_fcb_block_link_relative(icg_fcb_block, offset_jmpcr_to_eq_to_nil, offset_right)) != 0) { goto fail; }
    if ((ret = mlua_icg_fcb_block_link_relative(icg_fcb_block, offset_jmpcr_to_eq_to_false, offset_right)) != 0) { goto fail; }
    if ((ret = mlua_icg_fcb_block_link_relative(icg_fcb_block, offset_to_tail, offset_tail)) != 0) { goto fail; }

    goto done;
fail:
done:
    return ret;
}


static int mlua_icodegen_expression_binop_str_concat(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_binop *exp_binop)
{
    int ret = 0;
    uint32_t type_id;

    ret = virtual_machine_object_type_name_to_id(&type_id, "str", 3);
    if (ret != 0) 
    {
        multiple_error_update(err, -MULTIPLE_ERR_ICODEGEN, "\'str\' isn't a valid type name");
        ret = -MULTIPLE_ERR_ICODEGEN; 
        goto fail; 
    }

    /* Left */
    if ((ret = mlua_icodegen_expression(err, \
                    context, \
                    icg_fcb_block, \
                    exp_binop->left)) != 0)
    { goto fail; }
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_CONVERT, type_id)) != 0) 
    { goto fail; }

    /* Right */
    if ((ret = mlua_icodegen_expression(err, \
                    context, \
                    icg_fcb_block, \
                    exp_binop->right)) != 0)
    { goto fail; }
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_CONVERT, type_id)) != 0) 
    { goto fail; }

    /* Concat */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_ADD, 0)) != 0) 
    { goto fail; }


    goto done;
fail:
done:
    return ret;
}

static int mlua_icodegen_expression_binop_div_mod(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_binop *exp_binop)
{
    int ret = 0;
    uint32_t op = 0;
    const int LBL_RIGHT_ZERO = 0, LBL_LEFT_ZERO = 1, LBL_TAIL = 2; 
    struct multiply_text_precompiled *new_text_precompiled = NULL;

    switch (exp_binop->op->value)
    {
        case '/':
            op = OP_DIV;
            break;
        case '%':
            op = OP_MOD;
            break;
        default:
            MULTIPLE_ERROR_INTERNAL();
            ret = -MULTIPLE_ERR_INTERNAL;
            goto fail;
    }

    /*
     * Every nan in lua are "negative nans"
     * -----------------------------------
       > print(1/0)
       inf
       > print(0/0)
       -nan
       > print(-(1/0))
       nan
     * -----------------------------------
     */

    /* Left */
    if ((ret = mlua_icodegen_expression(err, \
                    context, \
                    icg_fcb_block, \
                    exp_binop->left)) != 0)
    { goto fail; }

    /* Right */
    if ((ret = mlua_icodegen_expression(err, \
                    context, \
                    icg_fcb_block, \
                    exp_binop->right)) != 0)
    { goto fail; }

    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_TYPEUP, 0)) != 0) 
    { goto fail; }

    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled, \

                    /* <bottom> left, right <top> */
                    MULTIPLY_ASM_OP      , OP_DUP       , 
                    MULTIPLY_ASM_OP_INT  , OP_PUSH      , 0,
                    MULTIPLY_ASM_OP      , OP_EQ        , 

                    /* <bottom> left, right, right == 0 <top> */
                    MULTIPLY_ASM_OP_LBLR , OP_JMPCR     ,  LBL_RIGHT_ZERO,

                    MULTIPLY_ASM_OP      , OP_DUP       , 
                    MULTIPLY_ASM_OP_FLOAT, OP_PUSH      , (double)0.0,
                    MULTIPLY_ASM_OP      , OP_EQ        , 

                    /* <bottom> left, right, right == 0.0 <top> */
                    MULTIPLY_ASM_OP_LBLR , OP_JMPCR     ,  LBL_RIGHT_ZERO,

                    /* <bottom> left, right <top> */
                    MULTIPLY_ASM_OP      , op           , 
                    /* <bottom> answer <top> */
                    MULTIPLY_ASM_OP_LBLR , OP_JMPR      ,  LBL_TAIL,
                    /* right_zero: */
                    MULTIPLY_ASM_LABEL   , LBL_RIGHT_ZERO,

                    MULTIPLY_ASM_OP_INT  , OP_PUSH      , 2,
                    MULTIPLY_ASM_OP      , OP_PICKCP    , 
                    /* <bottom> left, right, left <top> */
                    MULTIPLY_ASM_OP_INT  , OP_PUSH      , 0,
                    MULTIPLY_ASM_OP      , OP_EQ        , 
                    /* <bottom> left, right, left == 0 <top> */
                    MULTIPLY_ASM_OP_LBLR , OP_JMPCR     ,  LBL_LEFT_ZERO,
                    MULTIPLY_ASM_OP_INT  , OP_PUSH      , 2,
                    MULTIPLY_ASM_OP      , OP_PICKCP    , 
                    /* <bottom> left, right, left <top> */
                    MULTIPLY_ASM_OP_FLOAT, OP_PUSH      , (double)0.0,
                    MULTIPLY_ASM_OP      , OP_EQ        , 
                    /* <bottom> left, right, left == 0.0 <top> */
                    MULTIPLY_ASM_OP_LBLR , OP_JMPCR     ,  LBL_LEFT_ZERO,

                    /* non-zero / 0 -> inf*/
                    MULTIPLY_ASM_OP      , OP_DROP      ,  
                    MULTIPLY_ASM_OP      , OP_DROP      ,  
                    MULTIPLY_ASM_OP_INF  , OP_PUSH      , 0,
                    MULTIPLY_ASM_OP_LBLR , OP_JMPR      ,  LBL_TAIL,

                    /* left_zero: */
                    MULTIPLY_ASM_LABEL  , LBL_LEFT_ZERO,

                    /* 0 / 0 -> -nan */
                    MULTIPLY_ASM_OP      , OP_DROP      ,  
                    MULTIPLY_ASM_OP      , OP_DROP      ,  
                    MULTIPLY_ASM_OP_NAN  , OP_PUSH      , 1,

                    /* tail: */
                    MULTIPLY_ASM_LABEL   , LBL_TAIL,

                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; }

    if ((ret = mlua_icg_fcb_block_append_from_precompiled_pic_text( \
                    icg_fcb_block, \
                    new_text_precompiled)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    return ret;
}

static int mlua_icodegen_expression_binop(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_binop *exp_binop)
{
    int ret = 0;
    uint32_t op;

    if (exp_binop->op->value == TOKEN_KEYWORD_AND)
    {
        return mlua_icodegen_expression_binop_and(err, \
                context, \
                icg_fcb_block, \
                exp_binop);
    }
    else if (exp_binop->op->value == TOKEN_KEYWORD_OR)
    {
        return mlua_icodegen_expression_binop_or(err, \
                context, \
                icg_fcb_block, \
                exp_binop);
    }
    else if (exp_binop->op->value == TOKEN_OP_DBL_DOT)
    {
        return mlua_icodegen_expression_binop_str_concat(err, \
                context, \
                icg_fcb_block, \
                exp_binop);
    }
    else if ((exp_binop->op->value == '/') || \
            (exp_binop->op->value == '%'))
    {
        return mlua_icodegen_expression_binop_div_mod(err, \
                context, \
                icg_fcb_block, \
                exp_binop);
    }

    /* Left */
    if ((ret = mlua_icodegen_expression(err, \
                    context, \
                    icg_fcb_block, \
                    exp_binop->left)) != 0)
    { goto fail; }

    /* Right */
    if ((ret = mlua_icodegen_expression(err, \
                    context, \
                    icg_fcb_block, \
                    exp_binop->right)) != 0)
    { goto fail; }

    /* Operation */
    switch (exp_binop->op->value)
    {
        case '+':
            op = OP_ADD;
            break;
        case '-': 
            op = OP_SUB;
            break;
        case '*':
            op = OP_MUL;
            break;
        case '/':
            op = OP_DIV;
            break;
        case '%':
            op = OP_MOD;
            break;
        case TOKEN_OP_EQ: 
            op = OP_EQ;
            break;
        case TOKEN_OP_NE: 
            op = OP_NE;
            break;
        case '<': 
            op = OP_L;
            break;
        case '>': 
            op = OP_G;
            break;
        case TOKEN_OP_LE: 
            op = OP_LE;
            break;
        case TOKEN_OP_GE: 
            op = OP_GE;
            break;
        case '^': 
            MULTIPLE_ERROR_NOT_IMPLEMENTED();
            ret = -MULTIPLE_ERR_NOT_IMPLEMENTED;
            goto fail;
        default:
            MULTIPLE_ERROR_INTERNAL();
            ret = -MULTIPLE_ERR_INTERNAL;
            goto fail;
    }

    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, op, 0)) != 0) 
    { goto fail; }

    goto done;
fail:
done:
    return ret;
}


static int mlua_icodegen_expression_fundef(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_fundef *exp_fundef)
{
    int ret = 0;
    struct mlua_icg_fcb_block *new_icg_fcb_block = NULL;
    struct multiple_ir_export_section_item *new_export_section_item = NULL;
    struct mlua_map_offset_label_list *new_map_offset_label_list = NULL;

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
                    exp_fundef->pars)) != 0)
    { goto fail; }

    /* Body */
    if ((ret = mlua_icodegen_statement_list(err, context, \
                    new_icg_fcb_block, \
                    new_map_offset_label_list, \
                    exp_fundef->body, \
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
    if ((exp_fundef->body->end != NULL) && \
            (exp_fundef->body->end->type == MLUA_AST_STATEMENT_TYPE_RETURN))
    {
        /* Do nothing */
    }
    else
    {
        /* Return an empty list */
        if ((ret = mlua_icg_fcb_block_append_with_configure(new_icg_fcb_block, \
                        OP_LSTMK, 0)) != 0) { goto fail; }
        if ((ret = mlua_icg_fcb_block_append_with_configure(new_icg_fcb_block, \
                        OP_RETURN, 0)) != 0) { goto fail; }
    }

    /* Make Lambda */
    if ((ret = mlua_icg_fcb_block_append_with_configure_type(icg_fcb_block, \
                    OP_LAMBDAMK, (uint32_t)(context->icg_fcb_block_list->size), MLUA_ICG_FCB_LINE_TYPE_LAMBDA_MK)) != 0)
    { goto fail; }

    /* Append block */
    if ((ret = mlua_icg_fcb_block_list_append(context->icg_fcb_block_list, new_icg_fcb_block)) != 0)
    {
        MULTIPLE_ERROR_INTERNAL();
        goto fail;
    }
    /* Append blank export section */
    if ((ret = multiple_ir_export_section_append(context->icode->export_section, new_export_section_item)) != 0)
    {
        MULTIPLE_ERROR_INTERNAL();
        goto fail;
    }

    goto done;
fail:
    if (new_icg_fcb_block != NULL) mlua_icg_fcb_block_destroy(new_icg_fcb_block);
    if (new_export_section_item != NULL) multiple_ir_export_section_item_destroy(new_export_section_item);
done:
    if (new_map_offset_label_list != NULL) mlua_map_offset_label_list_destroy(new_map_offset_label_list);
    return ret;
}


static int mlua_icodegen_expression_raw(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression *exp)
{
    int ret = 0;

    switch (exp->type)
    {
        case MLUA_AST_EXPRESSION_TYPE_FACTOR:
            if ((ret = mlua_icodegen_expression_factor(err, \
                            context, \
                            icg_fcb_block, \
                            exp->u.factor)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_EXPRESSION_TYPE_TBLCTOR:
            if ((ret = mlua_icodegen_expression_tblctor(err, \
                            context, \
                            icg_fcb_block, \
                            exp->u.tblctor)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_EXPRESSION_TYPE_BINOP:
            if ((ret = mlua_icodegen_expression_binop(err, \
                            context, \
                            icg_fcb_block, \
                            exp->u.binop)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_EXPRESSION_TYPE_UNOP:
            if ((ret = mlua_icodegen_expression_unop(err, \
                            context, \
                            icg_fcb_block, \
                            exp->u.unop)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_EXPRESSION_TYPE_PRIMARY:
            if ((ret = mlua_icodegen_expression_primary(err, \
                            context, \
                            icg_fcb_block, \
                            exp->u.primary)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_EXPRESSION_TYPE_FUNDEF:
            if ((ret = mlua_icodegen_expression_fundef(err, \
                            context, \
                            icg_fcb_block, \
                            exp->u.fundef)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_EXPRESSION_TYPE_FUNCALL:
            if ((ret = mlua_icodegen_expression_funcall(err, \
                            context, \
                            icg_fcb_block, \
                            exp->u.funcall)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_EXPRESSION_TYPE_PREFIX:
            if ((ret = mlua_icodegen_expression_prefix(err, \
                            context, \
                            icg_fcb_block, \
                            exp->u.prefix)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_EXPRESSION_TYPE_SUFFIXED:
            if ((ret = mlua_icodegen_expression_suffixed(err, \
                            context, \
                            icg_fcb_block, \
                            exp->u.suffixed)) != 0)
            { goto fail; }
            break;

        case MLUA_AST_EXPRESSION_TYPE_UNKNOWN:
            MULTIPLE_ERROR_INTERNAL();
            ret = -MULTIPLE_ERR_INTERNAL;
            goto fail;
    }

    goto done;
fail:
done:
    return ret;
}

int mlua_icodegen_expression(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression *exp)
{
    int ret = 0;

    if ((ret = mlua_icodegen_expression_raw(err, \
                    context, \
                    icg_fcb_block, \
                    exp)) != 0)
    { goto fail; }

    if ((ret = mlua_icodegen_expression_fix_explist(err, \
                    context, \
                    icg_fcb_block)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    return ret;
}

int mlua_icodegen_expression_non_fix(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression *exp)
{
    int ret = 0;

    if ((ret = mlua_icodegen_expression_raw(err, \
                    context, \
                    icg_fcb_block, \
                    exp)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    return ret;
}

/* 'mlua_icodegen_expression' returns explist sometimes, 
 * this function fix the explist result to one exp */
int mlua_icodegen_expression_fix_explist(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block)
{
    int ret = 0;
    struct multiply_text_precompiled *new_text_precompiled = NULL;
    const int LBL_NOT_LIST = 0;
    const int LBL_ZERO_SIZE = 1;
    const int LBL_TAIL = 2;

    if ((ret = multiply_asm_precompile(err, \
                    context->icode, \
                    context->res_id, \
                    &new_text_precompiled, \

                    /* Solve it */
                    MULTIPLY_ASM_OP     , OP_SLV      , 
                    /* if type is not list then goto lbl_not_list */
                    MULTIPLY_ASM_OP     , OP_DUP      , 
                    MULTIPLY_ASM_OP     , OP_TYPE     , 
                    MULTIPLY_ASM_OP_RAW , OP_LSTMK    , 0,
                    MULTIPLY_ASM_OP     , OP_TYPE     , 
                    MULTIPLY_ASM_OP     , OP_NE       , 
                    MULTIPLY_ASM_OP_LBLR, OP_JMPCR    ,  LBL_NOT_LIST,

                    /* List */
                    MULTIPLY_ASM_OP     , OP_DUP      , 
                    MULTIPLY_ASM_OP     , OP_SIZE     , 
                    MULTIPLY_ASM_OP_INT , OP_PUSH     , 0,
                    MULTIPLY_ASM_OP     , OP_EQ       , 
                    MULTIPLY_ASM_OP_LBLR, OP_JMPCR    ,  LBL_ZERO_SIZE,

                    MULTIPLY_ASM_OP_INT , OP_PUSH     , 0,
                    MULTIPLY_ASM_OP_INT , OP_PUSH     , 2,
                    MULTIPLY_ASM_OP     , OP_PICK     , 
                    /* State: <bottom> list, 0 <top> */
                    MULTIPLY_ASM_OP     , OP_REFGET   , 

                    MULTIPLY_ASM_OP_LBLR, OP_JMPR     ,  LBL_TAIL,

                    /* lbl_zero_size: */
                    MULTIPLY_ASM_LABEL  , LBL_ZERO_SIZE,
                    MULTIPLY_ASM_OP_NONE, OP_PUSH     , 
                    MULTIPLY_ASM_OP_LBLR, OP_JMPR     ,  LBL_TAIL,

                    /* lbl_not_list: */
                    MULTIPLY_ASM_LABEL  , LBL_NOT_LIST,

                    /* lbl_zero_size: */
                    MULTIPLY_ASM_LABEL  , LBL_TAIL,

                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; }

    if ((ret = mlua_icg_fcb_block_append_from_precompiled_pic_text( \
                    icg_fcb_block, \
                    new_text_precompiled)) != 0)
    { goto fail; }

    goto done;
fail:
done:
    if (new_text_precompiled != NULL)
    { multiply_text_precompiled_destroy(new_text_precompiled); }
    return ret;
}

