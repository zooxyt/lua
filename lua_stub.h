/* Multiple Lua Programming Language : Stub
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

#ifndef _LUA_STUB_H_
#define _LUA_STUB_H_

#include <stdio.h>

#include "multiple_ir.h"

#define MLUA_FRONTNAME "lua"
#define MLUA_FULLNAME "Lua"
#define MLUA_EXT "lua"

struct mlua_stub
{
    /* plain text of source code */
    char *code;
    size_t len;

    /* debug info */
    int debug_info;

    /* optimize */
    int optimize;

    /* intermediate data */
    struct token_list *tokens;
    struct mlua_ast_program *program;

    /* options */
    int opt_internal_reconstruct;

    /* pathname */
    char *pathname;
    size_t pathname_len;
};

int mlua_stub_create(struct multiple_error *err, void **stub_out, \
        char *pathname_dst, int type_dst, \
        char *pathname_src, int type_src);
int mlua_stub_destroy(void *stub);
int mlua_stub_debug_info_set(void *stub, int debug_info);
int mlua_stub_optimize_set(void *stub, int optimize);
int mlua_stub_tokens_print(struct multiple_error *err, void *stub);
int mlua_stub_reconstruct(struct multiple_error *err, struct multiple_ir **ir, void *stub);
int mlua_stub_irgen(struct multiple_error *err, struct multiple_ir **ir, void *stub);

#endif

