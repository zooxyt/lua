/* Multiple Lua Programming Language : Intermediate Code Generator
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

#include "multiply.h"
#include "multiply_assembler.h"
#include "multiply_str_aux.h"

#include "multiple_misc.h" 
#include "multiple_err.h"

#include "vm_predef.h"
#include "vm_opcode.h"
#include "vm_types.h"

#include "mlua_lexer.h"
#include "mlua_ast.h"
#include "mlua_icg.h"
#include "mlua_icg_aux.h"

#include "mlua_icg_fcb.h"
#include "mlua_icg_context.h"

#include "mlua_icg_expr.h"
#include "mlua_icg_stmt.h"

#include "mlua_icg_built_in_proc.h"
#include "mlua_icg_built_in_table.h"

/* Declaration */
int mlua_icodegen_statement_list(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_map_offset_label_list *map_offset_label_list, \
        struct mlua_ast_statement_list *list, \
        struct multiply_offset_item_pack *offset_pack_break);


static int mlua_icodegen_merge_blocks(struct multiple_error *err, \
        struct mlua_icg_context *context)
{
    int ret = 0;
    struct mlua_icg_fcb_block *icg_fcb_block_cur;
    struct mlua_icg_fcb_line *icg_fcb_line_cur;

    uint32_t instrument_number;
    struct multiple_ir_export_section_item *export_section_item_cur;
    struct multiple_ir_text_section_item *text_section_item_cur;

    uint32_t offset_start;
    uint32_t fcb_size = 0;

    uint32_t count;

	/* Do not disturb the instrument produced by other way */
	offset_start = (uint32_t)(context->icode->text_section->size);
	
	export_section_item_cur = context->icode->export_section->begin;
    icg_fcb_block_cur = context->icg_fcb_block_list->begin;
    while (icg_fcb_block_cur != NULL)
    {
        icg_fcb_line_cur = icg_fcb_block_cur->begin;

        /* Record the absolute instrument number */
        instrument_number = (uint32_t)context->icode->text_section->size;
        if (export_section_item_cur == NULL)
        {
            MULTIPLE_ERROR_INTERNAL();
            ret = -MULTIPLE_ERR_INTERNAL;
            goto fail;
        }
        export_section_item_cur->instrument_number = instrument_number;

        while (icg_fcb_line_cur != NULL)
        {
            switch (icg_fcb_line_cur->type)
            {
                case MLUA_ICG_FCB_LINE_TYPE_NORMAL:
                    if ((ret = multiply_icodegen_text_section_append(err, \
                                    context->icode, \
                                    icg_fcb_line_cur->opcode, icg_fcb_line_cur->operand)) != 0)
                    { goto fail; }
                    break;
                case MLUA_ICG_FCB_LINE_TYPE_PC:
                    if ((ret = multiply_icodegen_text_section_append(err, \
                                    context->icode, \
                                    icg_fcb_line_cur->opcode, instrument_number + icg_fcb_line_cur->operand)) != 0)
                    { goto fail; }
                    break;
                case MLUA_ICG_FCB_LINE_TYPE_LAMBDA_MK:
                    /* Operand of this instrument here is the index number of lambda */
                    if ((ret = multiply_icodegen_text_section_append(err, \
                                    context->icode, \
                                    icg_fcb_line_cur->opcode, icg_fcb_line_cur->operand)) != 0)
                    { goto fail; }
                    break;
                case MLUA_ICG_FCB_LINE_TYPE_BLTIN_PROC_MK:
                    if ((ret = multiply_icodegen_text_section_append(err, \
                                    context->icode, \
                                    icg_fcb_line_cur->opcode, icg_fcb_line_cur->operand)) != 0)
                    { goto fail; }
                    break;
            }

            fcb_size += 1;
            icg_fcb_line_cur = icg_fcb_line_cur->next;
        }

        icg_fcb_block_cur = icg_fcb_block_cur->next;
        export_section_item_cur = export_section_item_cur->next;
    }

    /* 2nd pass, dealing with lambdas */
    icg_fcb_block_cur = context->icg_fcb_block_list->begin;
    /* Skip text body of built-in procedures at the beginning part */
    text_section_item_cur = context->icode->text_section->begin;
    while (offset_start-- > 0)
    {
        text_section_item_cur = text_section_item_cur->next; 
    }
    /* Process lambda mks */
    while (icg_fcb_block_cur != NULL)
    {
        icg_fcb_line_cur = icg_fcb_block_cur->begin;
        while (icg_fcb_line_cur != NULL)
        {
            if (icg_fcb_line_cur->type == MLUA_ICG_FCB_LINE_TYPE_LAMBDA_MK)
            {
                /* Locate to the export section item */
                count = icg_fcb_line_cur->operand;
                export_section_item_cur = context->icode->export_section->begin;
                while ((export_section_item_cur != NULL) && (count != 0))
                {
                    count--;
                    export_section_item_cur = export_section_item_cur->next;
                }
                if (export_section_item_cur == NULL)
                {
                    MULTIPLE_ERROR_INTERNAL();
                    ret = -MULTIPLE_ERR_INTERNAL;
                    goto fail;
                }
                text_section_item_cur->operand = export_section_item_cur->instrument_number; 
            }
            text_section_item_cur = text_section_item_cur->next; 
            icg_fcb_line_cur = icg_fcb_line_cur->next;
        }

        icg_fcb_block_cur = icg_fcb_block_cur->next;
    }

    goto done;
fail:
done:
    return ret;
}

static int mlua_icodegen_special(struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id, \
        struct mlua_icg_fcb_block_list *icg_fcb_block_list, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        const char *name, const size_t name_len)
{
    int ret = 0;
    uint32_t id;
    struct multiple_ir_export_section_item *new_export_section_item = NULL;

    new_export_section_item = multiple_ir_export_section_item_new();
    if (new_export_section_item == NULL)
    {
        MULTIPLE_ERROR_MALLOC();
        ret = -MULTIPLE_ERR_MALLOC;
        goto fail;
    }

    new_export_section_item->args_count = 0;
    new_export_section_item->args = NULL;
    new_export_section_item->args_types = NULL;

    /* Return */
    if ((ret = mlua_icg_fcb_block_append_with_configure(icg_fcb_block, OP_RETNONE, 0)) != 0)
    { goto fail; }

    /* Append block */
    if ((ret = mlua_icg_fcb_block_list_append(icg_fcb_block_list, icg_fcb_block)) != 0)
    {
        MULTIPLE_ERROR_INTERNAL();
        goto fail;
    }

    /* Append export section item */
    if ((ret = multiply_resource_get_id( \
                    err, \
                    icode, \
                    res_id, \
                    &id, \
                    name, name_len)) != 0)
    { goto fail; }

    new_export_section_item->name = id;
    new_export_section_item->instrument_number = (uint32_t)icode->export_section->size;
    multiple_ir_export_section_append(icode->export_section, new_export_section_item);

    goto done;
fail:
    if (new_export_section_item != NULL) multiple_ir_export_section_item_destroy(new_export_section_item);
done:
    return ret;
}


static int mlua_icodegen_program(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_ast_program *program)
{
    int ret = 0;
    struct mlua_icg_fcb_block *new_icg_fcb_block_autorun = NULL;
    struct mlua_map_offset_label_list *new_map_offset_label_list = NULL;
    uint32_t id;
    struct multiple_ir_export_section_item *new_export_section_item = NULL;
    uint32_t instrument_number_insert_point_built_in_proc;
    uint32_t instrument_count_built_in_proc;

    new_icg_fcb_block_autorun = mlua_icg_fcb_block_new();
    if (new_icg_fcb_block_autorun == NULL) 
    {
        MULTIPLE_ERROR_MALLOC();
        ret = -MULTIPLE_ERR_MALLOC;
        goto fail;
    }
    new_map_offset_label_list = mlua_map_offset_label_list_new();
    if (new_map_offset_label_list == NULL)
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

    /* def in .text section */
    if ((ret = multiply_resource_get_id( \
                    err,  \
                    context->icode, \
                    context->res_id, \
                    &id, \
                    VM_PREDEF_MODULE_AUTORUN, \
                    VM_PREDEF_MODULE_AUTORUN_LEN)) != 0)
    { goto fail; }
    if ((ret = mlua_icg_fcb_block_append_with_configure(new_icg_fcb_block_autorun, OP_DEF, id)) != 0) { goto fail; }
    new_export_section_item->name = id;
    new_export_section_item->args_count = 0;
    new_export_section_item->args = NULL;
    new_export_section_item->args_types = NULL;

    instrument_number_insert_point_built_in_proc = mlua_icg_fcb_block_get_instrument_number(new_icg_fcb_block_autorun);

    /* Statements of top level */
    if ((ret = mlua_icodegen_statement_list(err, context, \
                    new_icg_fcb_block_autorun, \
                    new_map_offset_label_list, \
                    program->stmts, NULL)) != 0)
    { goto fail; }

    /* Apply goto to label */
    if ((ret = mlua_icodegen_statement_list_apply_goto(err, \
                    context, \
                    new_icg_fcb_block_autorun, \
                    new_map_offset_label_list)) != 0)
    { goto fail; }
    /* Pop a label offset pack */
    multiply_offset_item_pack_stack_pop(context->offset_item_pack_stack);

    /* Put built-in procedures directly into icode,
     * and put initialize code into '__autorun__' */
    if ((ret = mlua_icg_add_built_in_procs(err, \
                    context->icode, \
                    context->res_id, \
                    new_icg_fcb_block_autorun, \
                    context->customizable_built_in_procedure_list, \
                    instrument_number_insert_point_built_in_proc,
                    &instrument_count_built_in_proc)) != 0)
    { goto fail; }

    /* Put built-in 'tables' directly into icode,
     * and put initialize code into '__autorun__' */
    if ((ret = mlua_icg_add_built_in_tables(err, \
                    context->icode, \
                    context->res_id, \
                    new_icg_fcb_block_autorun, \
                    context->stdlibs, \
                    instrument_number_insert_point_built_in_proc,
                    &instrument_count_built_in_proc)) != 0)
    { goto fail; }

    /* '__autorun__' subroutine */
    if ((ret = mlua_icodegen_special( \
                    err, \
                    context->icode, \
                    context->res_id, \
                    context->icg_fcb_block_list, new_icg_fcb_block_autorun, \
                    VM_PREDEF_MODULE_AUTORUN, VM_PREDEF_MODULE_AUTORUN_LEN)) != 0)
    { goto fail; }
    new_icg_fcb_block_autorun = NULL;

    /* Append export section item */
    if ((ret = multiply_resource_get_id( \
                    err, \
                    context->icode, \
                    context->res_id, \
                    &id, \
                    VM_PREDEF_MODULE_AUTORUN, \
                    VM_PREDEF_MODULE_AUTORUN_LEN)) != 0)
    { goto fail; }

    new_export_section_item->name = id;

    goto done;
fail:
    if (new_icg_fcb_block_autorun != NULL) mlua_icg_fcb_block_destroy(new_icg_fcb_block_autorun);
done:
    if (new_export_section_item != NULL) multiple_ir_export_section_item_destroy(new_export_section_item);
    if (new_map_offset_label_list != NULL) mlua_map_offset_label_list_destroy(new_map_offset_label_list);
    return ret;
}

int mlua_irgen(struct multiple_error *err, \
        struct multiple_ir **icode_out, \
        struct mlua_ast_program *program, \
        int verbose)
{
    int ret = 0;
    struct mlua_icg_context context;
    struct mlua_icg_fcb_block_list *new_icg_fcb_block_list = NULL;
    struct multiple_ir *new_icode = NULL;
    struct multiply_resource_id_pool *new_res_id = NULL;
    struct mlua_icg_customizable_built_in_procedure_list *new_customizable_built_in_procedure_list = NULL;
    struct multiply_offset_item_pack_stack *new_offset_item_pack_stack = NULL;
    struct mlua_icg_stdlib_table_list *new_table_list = NULL;

    (void)verbose;

    if ((new_customizable_built_in_procedure_list = mlua_icg_customizable_built_in_procedure_list_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    if ((new_icg_fcb_block_list = mlua_icg_fcb_block_list_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    if ((new_offset_item_pack_stack = multiply_offset_item_pack_stack_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    if ((new_table_list = mlua_icg_stdlib_table_list_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    if ((new_icode = multiple_ir_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    if ((new_res_id = multiply_resource_id_pool_new()) == NULL)
    { MULTIPLE_ERROR_MALLOC(); ret = -MULTIPLE_ERR_MALLOC; goto fail; }

    mlua_icg_context_init(&context);
    context.icg_fcb_block_list = new_icg_fcb_block_list;
    context.icode = new_icode;
    context.res_id = new_res_id;
    context.customizable_built_in_procedure_list = new_customizable_built_in_procedure_list;
    context.offset_item_pack_stack = new_offset_item_pack_stack;
    context.stdlibs = new_table_list;

    /* Generating icode for '__init__' */
    if ((ret = mlua_icodegen_program(err, \
                    &context, \
                    program)) != 0)
    { goto fail; }

    /* Merge blocks */
    if ((ret = mlua_icodegen_merge_blocks(err, \
                    &context)) != 0)
    { goto fail; }

    *icode_out = new_icode;

    ret = 0;
    goto done;
fail:
    if (new_icode != NULL) multiple_ir_destroy(new_icode);
done:
    if (new_res_id != NULL) multiply_resource_id_pool_destroy(new_res_id);
    if (new_icg_fcb_block_list != NULL) mlua_icg_fcb_block_list_destroy(new_icg_fcb_block_list);
    if (new_customizable_built_in_procedure_list != NULL) \
    { mlua_icg_customizable_built_in_procedure_list_destroy(new_customizable_built_in_procedure_list); }
    if (new_offset_item_pack_stack != NULL)
    { multiply_offset_item_pack_stack_destroy(new_offset_item_pack_stack); }
    if (new_table_list != NULL)
    { mlua_icg_stdlib_table_list_destroy(new_table_list); }
    return ret;
}

