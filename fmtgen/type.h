/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1994, 1998, 2002, 2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: interface definition for fmtgen/type.c
 */

#ifndef TYPE_H
#define TYPE_H

#include <main.h>
#include <lex.h>
#include <str.h>

#define TYPE_TY \
    struct type_method_ty *method; \
    string_ty       *name; \
    int             is_a_typedef; \
    int             included_flag;

typedef struct type_ty type_ty;
struct type_ty
{
    TYPE_TY
};

typedef struct type_method_ty type_method_ty;
struct type_method_ty
{
    long            size;
    const char      *name;
    int             has_a_mask;
    void            (*constructor)(type_ty *);
    void            (*destructor)(type_ty *);
    void            (*gen_include)(type_ty *);
    void            (*gen_include_declarator)(type_ty *, string_ty *, int);
    void            (*gen_code)(type_ty *);
    void            (*gen_code_declarator)(type_ty *, string_ty *, int, int);
    void            (*gen_code_call_xml)(type_ty *type, string_ty *form_name,
			string_ty *member_name, int show);
    void            (*gen_free_declarator)(type_ty *, string_ty *, int);
    void            (*member_add)(type_ty *, string_ty *, type_ty *, int);
    void            (*in_include_file)(type_ty *);
};

void type_gen_include(type_ty *);
void type_gen_include_declarator(type_ty *, string_ty *, int);
void type_gen_code(type_ty *);
void type_gen_code_declarator(type_ty *, string_ty *, int, int);
void type_gen_code_call_xml(type_ty *type, string_ty *form_name,
    string_ty *member_name, int show);
void type_gen_free_declarator(type_ty *, string_ty *, int);
void type_member_add(type_ty *, string_ty *, type_ty *, int);
void type_in_include_file(type_ty *);

type_ty *type_new(type_method_ty *, string_ty *);
void type_delete(type_ty *);

#endif /* TYPE_H */
