/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1999 Peter Miller;
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
 * MANIFEST: functions for reading input from environment variables
 */

#include <ac/stdio.h>
#include <ac/stdlib.h>

#include <input/crlf.h>
#include <input/env.h>
#include <input/private.h>
#include <mem.h>

typedef struct input_env_ty input_env_ty;
struct input_env_ty
{
	input_ty	inherited;
	char		*base;
	char		*pos;
	char		*name;
};


static void destruct _((input_ty *));

static void
destruct(p)
	input_ty	*p;
{
	input_env_ty	*this;

	this = (input_env_ty *)p;
	mem_free(this->base);
	mem_free(this->name);
}


static int get _((input_ty *));

static int
get(p)
	input_ty	*p;
{
	input_env_ty	*this;
	int		c;

	this = (input_env_ty *)p;
	c = (unsigned char)*this->pos++;
	if (!c)
	{
		c = EOF;
		this->pos--;
	}
	return c;
}


static long itell _((input_ty *));

static long
itell(p)
	input_ty	*p;
{
	input_env_ty	*this;

	this = (input_env_ty *)p;
	return (this->pos - this->base);
}


static const char *name _((input_ty *));

static const char *
name(p)
	input_ty	*p;
{
	input_env_ty	*this;

	this = (input_env_ty *)p;
	return this->name;
}


static long length _((input_ty *));

static long
length(p)
	input_ty	*p;
{
	input_env_ty	*this;

	this = (input_env_ty *)p;
	return strlen(this->base);
}


static input_vtbl_ty vtbl =
{
	sizeof(input_env_ty),
	destruct,
	input_generic_read,
	get,
	itell,
	name,
	length,
};


input_ty *
input_env_open(s)
	const char	*s;
{
	input_ty	*result;
	input_env_ty	*this;
	char		*cp;

	cp = getenv(s);
	if (!cp)
		cp = "";

	result = input_new(&vtbl);
	this = (input_env_ty *)result;
	this->base = mem_copy_string(cp);
	this->pos = this->base;
	this->name = mem_copy_string(s);
	return input_crlf(result, 1);
}
