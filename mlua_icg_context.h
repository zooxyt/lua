/* Multiple Lua Programming Language : Intermediate Code Generator
 * Global Context
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

#ifndef _MLUA_ICG_CONTEXT_H_
#define _MLUA_ICG_CONTEXT_H_

#include "multiple_ir.h"

#include "multiply.h"

#include "mlua_icg_fcb.h"
#include "mlua_icg_built_in_proc.h"
#include "mlua_icg_stdlib.h"

struct mlua_icg_context
{
    struct mlua_icg_fcb_block_list *icg_fcb_block_list;
    struct multiple_ir *icode;
    struct multiply_resource_id_pool *res_id;
    struct mlua_icg_customizable_built_in_procedure_list *customizable_built_in_procedure_list;
    struct multiply_offset_item_pack_stack *offset_item_pack_stack;
    struct mlua_icg_stdlib_table_list *stdlibs;
};

int mlua_icg_context_init(struct mlua_icg_context *context);
int mlua_icg_context_uninit(struct mlua_icg_context *context);

#endif



