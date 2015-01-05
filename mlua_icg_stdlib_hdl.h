/* Multiple Lua Programming Language : Intermediate Code Generator
 * Standard Library : Handler
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

#ifndef _MLUA_ICG_STDLIB_HDL_H_
#define _MLUA_ICG_STDLIB_HDL_H_

#include <stdio.h>

#include "multiple_err.h"
#include "multiple_ir.h"

#include "multiply.h"

/* Field Handler */

enum mlua_icg_add_built_in_field_handler_type
{
    MLUA_BUILT_IN_METHOD = 0,
    MLUA_BUILT_IN_PROPERTY,
    MLUA_BUILT_IN_FINISH,
};

struct mlua_icg_add_built_in_field_handler
{
    enum mlua_icg_add_built_in_field_handler_type type;
    const char *name;
    const size_t name_len;

    int (*func)(struct multiple_error *err, \
            struct multiple_ir *multiple_ir, \
            struct multiply_resource_id_pool *res_id);
};

struct mlua_icg_add_built_in_field_handler *mlua_icg_add_built_in_field_handler_lookup( \
        struct mlua_icg_add_built_in_field_handler *field_handler_start, \
        char *name, size_t len);


/* Table Handler */

struct mlua_icg_add_built_in_table_handler
{
    const char *name;
    const size_t name_len;
    struct mlua_icg_add_built_in_field_handler *field_handler;
};

struct mlua_icg_add_built_in_table_handler *mlua_icg_add_built_in_table_handler_lookup( \
        struct mlua_icg_add_built_in_table_handler *table_handler_start, \
        char *name, size_t len);


#endif

