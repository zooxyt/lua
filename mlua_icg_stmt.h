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

#ifndef _MLUA_ICG_STMT_H_
#define _MLUA_ICG_STMT_H_

#include "multiple_ir.h"

#include "multiply_offset.h"

#include "mlua_ast.h"

/* Generate icode for statements */
int mlua_icodegen_statement_list(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_map_offset_label_list *map_offset_label_list, \
        struct mlua_ast_statement_list *list, \
        struct multiply_offset_item_pack *offset_pack_break);

/* Generate icode for Expression List and put the count of result
 * on the top of stack */
int mlua_icodegen_explist(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_list *explist);

/* Trim explist result with specified number 
 * Call after 'mlua_icodegen_explist' */
int mlua_icodegen_trim_explist(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        size_t maximum);

/* Generate icode for Parameter List */
int mlua_icodegen_parlist(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_par_list *parlist);

#endif

