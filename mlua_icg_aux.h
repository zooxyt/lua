/* Multiple Lua Programming Language : Intermediate Code Generator
 * Auxiliary
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

#ifndef _MLUA_ICG_AUX_H_
#define _MLUA_ICG_AUX_H_

#include <stdio.h>
#include <stdint.h>

#include "multiple_ir.h"

#include "mlua_icg_fcb.h"

int mlua_icg_fcb_block_append_from_precompiled_pic_text( \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct multiply_text_precompiled *text_precompiled);


/* A data structure for connecting goto's offset and label */

struct mlua_map_offset_label
{
    uint32_t offset;

    char *str;
    size_t len;

    uint32_t pos_ln;
    uint32_t pos_col;

    struct mlua_map_offset_label *next;
    struct mlua_map_offset_label *prev;
};
struct mlua_map_offset_label *mlua_map_offset_label_new( \
        uint32_t offset, \
        char *str, size_t len, \
        uint32_t pos_ln, uint32_t pos_col);
int mlua_map_offset_label_destroy(struct mlua_map_offset_label *mlua_map_offset_label);

struct mlua_map_offset_label_list
{
    struct mlua_map_offset_label *begin;
    struct mlua_map_offset_label *end;
};
struct mlua_map_offset_label_list *mlua_map_offset_label_list_new(void);
int mlua_map_offset_label_list_destroy(struct mlua_map_offset_label_list *list);
int mlua_map_offset_label_list_append(struct mlua_map_offset_label_list *list, \
        struct mlua_map_offset_label *new_mlua_map_offset_label);
int mlua_map_offset_label_list_append_with_configure( \
        struct mlua_map_offset_label_list *list, \
        uint32_t offset, \
        char *str, size_t len, \
        uint32_t pos_ln, uint32_t pos_col);

int mlua_icodegen_statement_list_apply_goto(struct multiple_error *err, \
        struct mlua_icg_context *context, \
        struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_map_offset_label_list *map_offset_label_list);

#endif

