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

#include <stdlib.h>
#include <string.h>

#include "mlua_icg_stdlib.h"
#include "mlua_icg_stdlib_math.h"
#include "mlua_icg_stdlib_bitwise.h"
#include "mlua_icg_stdlib_os.h"


/* Declarations */

static struct mlua_icg_stdlib_field *mlua_icg_stdlib_field_new(char *name, size_t len);
static int mlua_icg_stdlib_field_destroy(struct mlua_icg_stdlib_field *field);
static struct mlua_icg_stdlib_field_list *mlua_icg_stdlib_field_list_new(void);
static int mlua_icg_stdlib_field_list_destroy(struct mlua_icg_stdlib_field_list *field_list);
static int mlua_icg_stdlib_field_list_append(struct mlua_icg_stdlib_field_list *field_list, \
        struct mlua_icg_stdlib_field *new_field);
static struct mlua_icg_stdlib_table *mlua_icg_stdlib_table_new(char *name, size_t len);
static int mlua_icg_stdlib_table_destroy(struct mlua_icg_stdlib_table *table);


/* Implementations */

static struct mlua_icg_stdlib_field *mlua_icg_stdlib_field_new(char *name, size_t len)
{
    struct mlua_icg_stdlib_field *new_field = NULL;

    new_field = (struct mlua_icg_stdlib_field *)malloc(sizeof(struct mlua_icg_stdlib_field));
    if (new_field == NULL) { goto fail; }
    new_field->next = NULL;
    new_field->len = len;
    new_field->name = (char *)malloc(sizeof(char) * (len + 1));
    if (new_field->name == NULL) { goto fail; } 
    memcpy(new_field->name, name, len);
    new_field->name[len] = '\0';

    goto done;
fail:
    if (new_field != NULL)
    {
        mlua_icg_stdlib_field_destroy(new_field);
        new_field = NULL;
    }
done:
    return new_field;
}

static int mlua_icg_stdlib_field_destroy(struct mlua_icg_stdlib_field *field)
{
    if (field->name != NULL) free(field->name);
    free(field);

    return 0;
}

static struct mlua_icg_stdlib_field_list *mlua_icg_stdlib_field_list_new(void)
{
    struct mlua_icg_stdlib_field_list *new_list = NULL;

    new_list = (struct mlua_icg_stdlib_field_list *)malloc(sizeof(struct mlua_icg_stdlib_field_list));
    if (new_list == NULL) { goto fail; }
    new_list->begin = new_list->end = NULL;
    new_list->size = 0;

    goto done;
fail:
    if (new_list != NULL)
    {
        mlua_icg_stdlib_field_list_destroy(new_list);
        new_list = NULL;
    }
done:
    return new_list;
}

static int mlua_icg_stdlib_field_list_append(struct mlua_icg_stdlib_field_list *field_list, \
        struct mlua_icg_stdlib_field *new_field)
{
    if (field_list->begin == NULL)
    {
        field_list->begin = field_list->end = new_field;
    }
    else
    {
        field_list->end->next = new_field;
        field_list->end = new_field;
    }

    return 0;
}

static int mlua_icg_stdlib_field_list_destroy(struct mlua_icg_stdlib_field_list *field_list)
{
    struct mlua_icg_stdlib_field *field_cur, *field_next;

    field_cur = field_list->begin;
    while (field_cur != NULL)
    {
        field_next = field_cur->next; 
        mlua_icg_stdlib_field_destroy(field_cur);
        field_cur = field_next; 
    }
    free(field_list);

    return 0;
}

static struct mlua_icg_stdlib_field *mlua_icg_stdlib_field_list_lookup(struct mlua_icg_stdlib_field_list *field_list, \
        char *field_name, size_t field_name_len)
{
    struct mlua_icg_stdlib_field *field_cur;

    field_cur = field_list->begin;
    while (field_cur != NULL)
    {
        if ((field_cur->len == field_name_len) && \
                (strncmp(field_cur->name, field_name, field_name_len)== 0))
        {
            return field_cur;
        }
        field_cur = field_cur->next;
    }

    return NULL;
}

static struct mlua_icg_stdlib_table *mlua_icg_stdlib_table_new(char *name, size_t len)
{
    struct mlua_icg_stdlib_table *new_table = NULL;

    new_table = (struct mlua_icg_stdlib_table *)malloc(sizeof(struct mlua_icg_stdlib_table));
    if (new_table == NULL) { goto fail; }
    new_table->name = NULL;
    new_table->fields = NULL;
    new_table->next = NULL;
    new_table->len = len;
    new_table->name = (char *)malloc(sizeof(char) * (len + 1));
    if (new_table->name == NULL) { goto fail; } 
    memcpy(new_table->name, name, len);
    new_table->name[len] = '\0';
    if ((new_table->fields = mlua_icg_stdlib_field_list_new()) == NULL)
    { goto fail; }

    goto done;
fail:
    if (new_table != NULL)
    {
        mlua_icg_stdlib_table_destroy(new_table);
        new_table = NULL;
    }
done:
    return new_table;
}

static int mlua_icg_stdlib_table_destroy(struct mlua_icg_stdlib_table *table)
{
    if (table->name != NULL) free(table->name);
    if (table->fields != NULL) mlua_icg_stdlib_field_list_destroy(table->fields);
    free(table);

    return 0;
}

struct mlua_icg_stdlib_table_list *mlua_icg_stdlib_table_list_new(void)
{
    struct mlua_icg_stdlib_table_list *new_list = NULL;

    new_list = (struct mlua_icg_stdlib_table_list *)malloc(sizeof(struct mlua_icg_stdlib_table_list));
    if (new_list == NULL) { goto fail; }
    new_list->begin = NULL;
    new_list->end = NULL;
    new_list->size = 0;

    goto done;
fail:
    if (new_list != NULL)
    {
        mlua_icg_stdlib_table_list_destroy(new_list);
        new_list = NULL;
    }
done:
    return new_list;
}

int mlua_icg_stdlib_table_list_destroy(struct mlua_icg_stdlib_table_list *table_list)
{
    struct mlua_icg_stdlib_table *table_cur, *table_next;

    table_cur = table_list->begin;
    while (table_cur != NULL)
    {
        table_next = table_cur->next;
        mlua_icg_stdlib_table_destroy(table_cur);
        table_cur = table_next;
    }
    free(table_list);

    return 0;
}

static int mlua_icg_stdlib_table_list_append(struct mlua_icg_stdlib_table_list *table_list, \
        struct mlua_icg_stdlib_table *new_table)
{
    if (table_list->begin == NULL)
    {
        table_list->begin = table_list->end = new_table;
    }
    else
    {
        table_list->end->next = new_table;
        table_list->end = new_table;
    }
    table_list->size += 1;

    return 0;
}

static struct mlua_icg_stdlib_table *mlua_icg_stdlib_table_list_lookup( \
        struct mlua_icg_stdlib_table_list *table_list, \
        char *table_name, size_t table_name_len)
{
    struct mlua_icg_stdlib_table *table_cur;

    table_cur = table_list->begin;
    while (table_cur != NULL)
    {
        if ((table_cur->len == table_name_len) && \
                (strncmp(table_cur->name, table_name, table_name_len)== 0))
        {
            return table_cur;
        }
        table_cur = table_cur->next;
    }

    return NULL;
}

int mlua_icg_stdlib_table_list_register(struct mlua_icg_stdlib_table_list *table_list, \
        char *table_name, size_t table_name_len, \
        char *field_name, size_t field_name_len)
{
    struct mlua_icg_stdlib_table *table_target;
    struct mlua_icg_stdlib_table *new_table;
    struct mlua_icg_stdlib_field *field_target;
    struct mlua_icg_stdlib_field *new_field;

    if ((table_target = mlua_icg_stdlib_table_list_lookup( \
            table_list, \
            table_name, table_name_len)) == NULL)
    {
        new_table = mlua_icg_stdlib_table_new(table_name, table_name_len);
        if (new_table == NULL) { return -1; }
        mlua_icg_stdlib_table_list_append(table_list, new_table);
        table_target = new_table; new_table = NULL;
    }

    if ((field_target = mlua_icg_stdlib_field_list_lookup( \
                    table_target->fields, \
                    field_name, field_name_len)) == NULL)
    {
        new_field = mlua_icg_stdlib_field_new(field_name, field_name_len);
        if (new_field == NULL) { return -1; }
        mlua_icg_stdlib_field_list_append(table_target->fields, new_field);
    }

    return 0;
}


struct mlua_icg_add_built_in_table_handler mlua_icg_add_built_in_table_handlers[] =
{
    {"math", 4, mlua_icg_add_built_in_field_handlers_math},
    {"bit32", 4, mlua_icg_add_built_in_field_handlers_bitwise},
    {"os", 2, mlua_icg_add_built_in_field_handlers_os},
    {NULL, 0, NULL},
};

