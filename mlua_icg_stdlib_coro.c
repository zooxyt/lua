/* Multiple Lua Programming Language : Intermediate Code Generator
 * Standard Library : Coroutine
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "multiple_err.h"
#include "multiple_ir.h"

#include "multiply.h"
#include "multiply_assembler.h"

#include "vm_opcode.h"
#include "vm_types.h"
#include "vm_predef.h"

#include "mlua_lexer.h"
#include "mlua_ast.h"
#include "mlua_icg.h"
#include "mlua_icg_fcb.h"

/* coroutine.create(function) */
static int mlua_icg_add_built_in_procs_coroutine_create( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id)
{
    int ret = 0;

    if ((ret = multiply_asm(err, icode, res_id,
                    MULTIPLY_ASM_OP     , OP_ARGCS   ,
                    MULTIPLY_ASM_OP     , OP_RETURN  , 

                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

/* coroutine.status(co) */
static int mlua_icg_add_built_in_procs_coroutine_status( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id)
{
    int ret = 0;

    if ((ret = multiply_asm(err, icode, res_id,
                    MULTIPLY_ASM_OP     , OP_ARGCS   ,
                    MULTIPLY_ASM_OP     , OP_RETURN  , 

                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

/* coroutine.resume(co, arguments) */
static int mlua_icg_add_built_in_procs_coroutine_resume( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id)
{
    int ret = 0;

    if ((ret = multiply_asm(err, icode, res_id,
                    MULTIPLY_ASM_OP     , OP_ARGCS   ,
                    MULTIPLY_ASM_OP     , OP_RETURN  , 

                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

/* coroutine.yield(co, arguments) */
static int mlua_icg_add_built_in_procs_coroutine_yield( \
        struct multiple_error *err, \
        struct multiple_ir *icode, \
        struct multiply_resource_id_pool *res_id)
{
    int ret = 0;

    if ((ret = multiply_asm(err, icode, res_id,
                    MULTIPLY_ASM_OP     , OP_ARGCS   ,
                    MULTIPLY_ASM_OP     , OP_RETURN  , 

                    MULTIPLY_ASM_FINISH)) != 0)
    { goto fail; } 
    goto done;
fail:
done:
    return ret;
}

extern struct mlua_icg_add_built_in_field_handler mlua_icg_add_built_in_field_handlers_coroutine[];

struct mlua_icg_add_built_in_field_handler mlua_icg_add_built_in_field_handlers_coroutine[] =
{
    { MLUA_BUILT_IN_METHOD, "create", 6, mlua_icg_add_built_in_procs_coroutine_create },
    { MLUA_BUILT_IN_METHOD, "status", 6, mlua_icg_add_built_in_procs_coroutine_status },
    { MLUA_BUILT_IN_METHOD, "resume", 6, mlua_icg_add_built_in_procs_coroutine_resume },
    { MLUA_BUILT_IN_METHOD, "yield", 5, mlua_icg_add_built_in_procs_coroutine_yield },
    { MLUA_BUILT_IN_FINISH, NULL, 0, NULL},
};

