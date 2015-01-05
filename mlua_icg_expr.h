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

#ifndef _MLUA_ICG_EXP_H_
#define _MLUA_ICG_EXP_H_

#include "multiple_ir.h"

#include "mlua_ast.h"

int mlua_icodegen_expression_funcall(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression_funcall *exp_funcall);

int mlua_icodegen_expression(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression *exp);

int mlua_icodegen_expression_non_fix(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_expression *exp);

/* 'mlua_icodegen_expression' returns explist sometimes, 
 * this function fix the explist result to one exp */
int mlua_icodegen_expression_fix_explist(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block);

int mlua_icodegen_args(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_args *args);

#endif

