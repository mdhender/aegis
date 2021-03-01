/*
 *	aegis - project change supervisor
 *	Copyright (C) 2004 Peter Miller;
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
 * MANIFEST: interface definition for aecvsserver/module/private.c
 */

#ifndef AECVSSERVER_MODULE_PRIVATE_H
#define AECVSSERVER_MODULE_PRIVATE_H

#include <input.h>
#include <module.h>
#include <server.h>
#include <str.h>

typedef struct module_method_ty module_method_ty;
struct module_method_ty
{
    int             size;
    void            (*destructor)(module_ty *);
    void            (*modified)(module_ty *, server_ty *, string_ty *,
			struct file_info_ty *, input_ty *);
    string_ty       *(*canonical_name)(module_ty *);
    int             (*update)(module_ty *, server_ty *, string_ty *,
                        string_ty *, module_options_ty *);
    int             (*checkin)(module_ty *, server_ty *, string_ty *,
                        string_ty *);
    int             (*add)(module_ty *, server_ty *, string_ty *,
                        string_ty *, module_options_ty *);
    int             (*remove)(module_ty *, server_ty *, string_ty *,
                        string_ty *, module_options_ty *);
    int             bogus;
    const char      *name;
};

module_ty *module_new(const module_method_ty *);

#endif /* AECVSSERVER_MODULE_PRIVATE_H */
