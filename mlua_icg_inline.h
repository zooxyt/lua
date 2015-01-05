/* Multiple Lua Programming Language : Intermediate Code Generator
 * Inline
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

#ifndef _MLUA_ICG_INLINE_H_
#define _MLUA_ICG_INLINE_H_

#include <stdio.h>

#include "multiple_err.h"
#include "mlua_icg_context.h"
#include "mlua_icg_fcb.h"

struct mlua_icg_inline_handler
{
    const char *name;
    const size_t name_len;

    int (*func)(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_ast_args *args);
};

struct mlua_icg_inline_handler *mlua_icg_inline_handler_lookup(char *name, size_t name_len);

#endif

