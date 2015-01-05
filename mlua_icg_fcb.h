/* Multiple Lua Programming Language : Intermediate Code Generator
 * Floating Code Block
   Copyright(C) 2013-2014 Cheryl Natsu

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

#ifndef _MLUA_ICG_FCB_H_
#define _MLUA_ICG_FCB_H_

#include <stdio.h>
#include <stdint.h>

/* Attributes for each line */

struct mlua_icg_fcb_line_attr
{
    uint32_t attr_id;
    uint32_t res_id;
    struct mlua_icg_fcb_line_attr *next;
};

struct mlua_icg_fcb_line_attr *mlua_icg_fcb_line_attr_new(uint32_t attr_id, \
        uint32_t res_id); 
int mlua_icg_fcb_line_attr_destroy(struct mlua_icg_fcb_line_attr *attr);

struct mlua_icg_fcb_line_attr_list
{
    struct mlua_icg_fcb_line_attr *begin;
    struct mlua_icg_fcb_line_attr *end;
    size_t size;
};

struct mlua_icg_fcb_line_attr_list *mlua_icg_fcb_line_attr_list_new(void);
int mlua_icg_fcb_line_attr_list_destroy(struct mlua_icg_fcb_line_attr_list *list);
int mlua_icg_fcb_line_attr_list_append(struct mlua_icg_fcb_line_attr_list *list, \
        struct mlua_icg_fcb_line_attr *new_attr);
int mlua_icg_fcb_line_attr_list_append_with_configure(struct mlua_icg_fcb_line_attr_list *list, \
        uint32_t attr_id, uint32_t res_id);


enum
{
    /* No needed to do anything to operand */
    MLUA_ICG_FCB_LINE_TYPE_NORMAL = 0,        

    /* operand = global_start + operand */
    MLUA_ICG_FCB_LINE_TYPE_PC = 1,            

    /* operand = global_offsets_of_lambda_procs[res_id] */
    MLUA_ICG_FCB_LINE_TYPE_LAMBDA_MK = 2,     

    /* operand = global_offsets_of_built_in_proces[res_id] 
     * At the beginning of __init__ */
    MLUA_ICG_FCB_LINE_TYPE_BLTIN_PROC_MK = 3, 
};

struct mlua_icg_fcb_line
{
    uint32_t opcode;
    uint32_t operand;
    int type;
    struct mlua_icg_fcb_line_attr_list *attrs;

    struct mlua_icg_fcb_line *prev;
    struct mlua_icg_fcb_line *next;
};
struct mlua_icg_fcb_line *mlua_icg_fcb_line_new(void);
int mlua_icg_fcb_line_destroy(struct mlua_icg_fcb_line *icg_fcb_line);
struct mlua_icg_fcb_line *mlua_icg_fcb_line_new_with_configure(uint32_t opcode, uint32_t operand);
struct mlua_icg_fcb_line *mlua_icg_fcb_line_new_with_configure_type(uint32_t opcode, uint32_t operand, int type);

struct mlua_icg_fcb_block
{
    struct mlua_icg_fcb_line *begin;
    struct mlua_icg_fcb_line *end;
    size_t size;

    struct mlua_icg_fcb_block *prev;
    struct mlua_icg_fcb_block *next;
};
struct mlua_icg_fcb_block *mlua_icg_fcb_block_new(void);
int mlua_icg_fcb_block_destroy(struct mlua_icg_fcb_block *icg_fcb_block);
int mlua_icg_fcb_block_append(struct mlua_icg_fcb_block *icg_fcb_block, \
        struct mlua_icg_fcb_line *new_icg_fcb_line);
int mlua_icg_fcb_block_insert(struct mlua_icg_fcb_block *icg_fcb_block, \
        uint32_t instrument_number_insert, \
        struct mlua_icg_fcb_line *new_icg_fcb_line);

int mlua_icg_fcb_block_append_with_configure(struct mlua_icg_fcb_block *icg_fcb_block, \
        uint32_t opcode, uint32_t operand);
int mlua_icg_fcb_block_append_with_configure_type(struct mlua_icg_fcb_block *icg_fcb_block, \
        uint32_t opcode, uint32_t operand, int type);

uint32_t mlua_icg_fcb_block_get_instrument_number(struct mlua_icg_fcb_block *icg_fcb_block);

int mlua_icg_fcb_block_insert_with_configure(struct mlua_icg_fcb_block *icg_fcb_block, \
        uint32_t instrument, uint32_t opcode, uint32_t operand);
int mlua_icg_fcb_block_insert_with_configure_type(struct mlua_icg_fcb_block *icg_fcb_block, \
        uint32_t instrument, uint32_t opcode, uint32_t operand, int type);

int mlua_icg_fcb_block_link(struct mlua_icg_fcb_block *icg_fcb_block, \
        uint32_t instrument_number_from, uint32_t instrument_number_to);
int mlua_icg_fcb_block_link_relative(struct mlua_icg_fcb_block *icg_fcb_block, \
        uint32_t instrument_number_from, uint32_t instrument_number_to);

struct mlua_icg_fcb_block_list
{
    struct mlua_icg_fcb_block *begin;
    struct mlua_icg_fcb_block *end;
    size_t size;
};
struct mlua_icg_fcb_block_list *mlua_icg_fcb_block_list_new(void);
int mlua_icg_fcb_block_list_destroy(struct mlua_icg_fcb_block_list *icg_fcb_block_list);
int mlua_icg_fcb_block_list_append(struct mlua_icg_fcb_block_list *icg_fcb_block_list, \
        struct mlua_icg_fcb_block *new_icg_fcb_block);


struct mlua_icg_customizable_built_in_procedure_write_back
{
    uint32_t instrument_number;
    struct mlua_icg_customizable_built_in_procedure_write_back *next;
};
struct mlua_icg_customizable_built_in_procedure_write_back *mlua_icg_customizable_built_in_procedure_write_back_new(uint32_t instrument_number);
int mlua_icg_customizable_built_in_procedure_write_back_destroy(struct mlua_icg_customizable_built_in_procedure_write_back *write_back);

struct mlua_icg_customizable_built_in_procedure_write_back_list
{
    struct mlua_icg_customizable_built_in_procedure_write_back *begin;
    struct mlua_icg_customizable_built_in_procedure_write_back *end;
    size_t size;
};
struct mlua_icg_customizable_built_in_procedure_write_back_list *mlua_icg_customizable_built_in_procedure_write_back_list_new(void);
int mlua_icg_customizable_built_in_procedure_write_back_list_destroy(struct mlua_icg_customizable_built_in_procedure_write_back_list *write_back_list);
int mlua_icg_customizable_built_in_procedure_write_back_list_append_with_configure(struct mlua_icg_customizable_built_in_procedure_write_back_list *write_back_list, uint32_t instrument_number);

struct mlua_icg_customizable_built_in_procedure
{
    char *name;
    size_t name_len;
    int called;

    /* instrument number in icode */
    uint32_t instrument_number_icode;
    struct mlua_icg_customizable_built_in_procedure_write_back_list *write_backs;

    struct mlua_icg_customizable_built_in_procedure *next;
};
struct mlua_icg_customizable_built_in_procedure *mlua_icg_customizable_built_in_procedure_new(const char *name, const size_t name_len);
int mlua_icg_customizable_built_in_procedure_destroy(struct mlua_icg_customizable_built_in_procedure *customizable_built_in_procedure);

struct mlua_icg_customizable_built_in_procedure_list
{
    struct mlua_icg_customizable_built_in_procedure *begin;
    struct mlua_icg_customizable_built_in_procedure *end;
};
struct mlua_icg_customizable_built_in_procedure_list *mlua_icg_customizable_built_in_procedure_list_new(void);
int mlua_icg_customizable_built_in_procedure_list_destroy(struct mlua_icg_customizable_built_in_procedure_list *list);

struct mlua_icg_customizable_built_in_procedure *mlua_icg_customizable_built_in_procedure_list_lookup(struct mlua_icg_customizable_built_in_procedure_list *customizable_built_in_procedure_list, \
        const char *name, const size_t name_len);
int mlua_icg_customizable_built_in_procedure_list_called( \
        struct mlua_icg_customizable_built_in_procedure_list *customizable_built_in_procedure_list, \
        const char *name, const size_t name_len);
int mlua_icg_customizable_built_in_procedure_list_add_writeback( \
        struct mlua_icg_customizable_built_in_procedure_list *customizable_built_in_procedure_list, \
        const char *name, const size_t name_len, uint32_t instrument_number);

#endif


