/* Multiple Lua Programming Language : Intermediate Code Generator
 * Standard Library
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

#ifndef _MLUA_ICG_STDLIB_H_
#define _MLUA_ICG_STDLIB_H_

#include <stdio.h>

#include "multiple_ir.h"

#include "mlua_icg_stdlib_hdl.h"
#include "mlua_icg_stdlib_math.h"

struct mlua_icg_stdlib_field
{
    char *name;
    size_t len;

    int used;

    struct mlua_icg_stdlib_field *next;
};

struct mlua_icg_stdlib_field_list
{
    struct mlua_icg_stdlib_field *begin;
    struct mlua_icg_stdlib_field *end;

    size_t size;
};


struct mlua_icg_stdlib_table
{
    char *name;
    size_t len;

    struct mlua_icg_stdlib_field_list *fields;

    struct mlua_icg_stdlib_table *next;
};


struct mlua_icg_stdlib_table_list
{
    struct mlua_icg_stdlib_table *begin;
    struct mlua_icg_stdlib_table *end;

    size_t size;
};
struct mlua_icg_stdlib_table_list *mlua_icg_stdlib_table_list_new(void);
int mlua_icg_stdlib_table_list_destroy(struct mlua_icg_stdlib_table_list *table_list);
int mlua_icg_stdlib_table_list_register(struct mlua_icg_stdlib_table_list *table_list, \
        char *table_name, size_t table_name_len, \
        char *field_name, size_t field_name_len);


extern struct mlua_icg_add_built_in_table_handler mlua_icg_add_built_in_table_handlers[];

#endif

