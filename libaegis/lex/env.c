/*
 *	aegis - project change supervisor
 *	Copyright (C) 1994 Peter Miller.
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
 * MANIFEST: functions to get lex input from environment variable
 */

#include <stdio.h>
#include <ac/stdlib.h>

#include <lex/env.h>
#include <mem.h>

typedef struct lex_input_env_ty lex_input_env_ty;
struct lex_input_env_ty
{
	lex_input_method_ty method;
	char		*base;
	char		*pos;
	char		*name;
};


static void destruct _((lex_input_ty *));

static void
destruct(p)
	lex_input_ty	*p;
{
	lex_input_env_ty *this;

	this = (lex_input_env_ty *)p;
	mem_free(this->base);
	mem_free(this->name);
	mem_free(this);
}


static int get _((lex_input_ty *));

static int
get(p)
	lex_input_ty	*p;
{
	lex_input_env_ty *this;
	int		c;

	this = (lex_input_env_ty *)p;
	c = (unsigned char)*this->pos++;
	if (!c)
	{
		c = EOF;
		this->pos--;
	}
	return c;
}


static void unget _((lex_input_ty *, int));

static void
unget(p, c)
	lex_input_ty	*p;
	int		c;
{
	lex_input_env_ty *this;

	this = (lex_input_env_ty *)p;
	if (c != EOF && this->pos > this->base)
		this->pos--;
}


static char *name _((lex_input_ty *));

static char *
name(p)
	lex_input_ty	*p;
{
	lex_input_env_ty *this;

	this = (lex_input_env_ty *)p;
	return this->name;
}


lex_input_ty *
lex_input_env_open(s)
	char		*s;
{
	lex_input_env_ty *result;
	char		*cp;

	cp = getenv(s);
	if (!cp)
		cp = "";

	result = mem_alloc(sizeof(lex_input_env_ty));
	result->method.destruct = destruct;
	result->method.get = get;
	result->method.unget = unget;
	result->method.name = name;
	result->base = mem_copy_string(cp);
	result->pos = result->base;
	result->name = mem_copy_string(s);
	return (lex_input_ty *)result;
}
