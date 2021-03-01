/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994, 1999, 2001 Peter Miller;
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
#include <ac/string.h>

#include <input/crlf.h>
#include <input/env.h>
#include <input/private.h>
#include <str.h>

typedef struct input_env_ty input_env_ty;
struct input_env_ty
{
	input_ty	inherited;
	string_ty	*base;
	size_t		pos;
	string_ty	*name;
};


static void input_env_destructor _((input_ty *));

static void
input_env_destructor(p)
	input_ty	*p;
{
	input_env_ty	*this;

	this = (input_env_ty *)p;
	str_free(this->base);
	str_free(this->name);
}


static long input_env_read _((input_ty *, void *, size_t));

static long
input_env_read(p, data, len)
	input_ty	*p;
	void		*data;
	size_t		len;
{
	input_env_ty	*this;
	size_t		nbytes;

	this = (input_env_ty *)p;
	if (this->pos >= this->base->str_length)
		return 0;
	nbytes = this->base->str_length - this->pos;
	if (nbytes > len)
		nbytes = len;
	memcpy(data, this->base->str_text + this->pos, nbytes);
	this->pos += nbytes;
	return nbytes;
}


static long input_env_ftell _((input_ty *));

static long
input_env_ftell(p)
	input_ty	*p;
{
	input_env_ty	*this;

	this = (input_env_ty *)p;
	return this->pos;
}


static string_ty *input_env_name _((input_ty *));

static string_ty *
input_env_name(p)
	input_ty	*p;
{
	input_env_ty	*this;

	this = (input_env_ty *)p;
	return this->name;
}


static long input_env_length _((input_ty *));

static long
input_env_length(p)
	input_ty	*p;
{
	input_env_ty	*this;

	this = (input_env_ty *)p;
	return this->base->str_length;
}


static input_vtbl_ty vtbl =
{
	sizeof(input_env_ty),
	input_env_destructor,
	input_env_read,
	input_env_ftell,
	input_env_name,
	input_env_length,
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
	this->base = str_from_c(cp);
	this->pos = 0;
	this->name = str_from_c(s);

	/*
	 * You need the CRLF filter, otherwise bizzare things happen on
	 * DOS (or DOS-like) operating systems.
	 */
	return input_crlf(result, 1);
}
