/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * MANIFEST: functions to parse aegis' data files
 */

#include <error.h>
#include <gram.h>
#include <lex.h>
#include <mem.h>
#include <parse.h>
#include <trace.h>

static sem_ty *sem_root;


void *
parse(filename, type)
	char	*filename;
	type_ty	*type;
{
	void	*addr;

	trace(("parse(filename = \"%s\", type = %08lx)\n{\n"/*}*/,
		filename, type));
	lex_open(filename);
	sem_push(type, &addr);

	trace(("gram_parse()\n{\n"/*}*/));
	gram_parse();
	trace((/*{*/"}\n"));

	while (sem_root)
		sem_pop();
	lex_close();
	trace(("return %08lX;\n", addr));
	trace((/*{*/"}\n"));
	return addr;
}


void
sem_push(type, addr)
	type_ty		*type;
	void		*addr;
{
	sem_ty		*sp;

	trace(("sem_push(type = %08lX, addr = %08lX)\n{\n"/*}*/, type, addr));
	trace(("type->class == %s;\n", type ? type_class_name(type) : "void"));
	sp = (sem_ty *)mem_alloc_clear(sizeof(sem_ty));
	sp->type = type;
	sp->addr = addr;
	sp->next = sem_root;
	sem_root = sp;
	if (type && type->alloc)
	{
		*(void **)sp->addr = type->alloc();
		trace
		((
			"sp->addr = %08lX->%08lX\n",
			sp->addr,
			*(void **)sp->addr
		));
	}
	trace((/*{*/"}\n"));
}


void
sem_pop()
{
	sem_ty *x;

	trace(("sem_pop()\n{\n"/*}*/));
	x = sem_root;
	sem_root = x->next;
	mem_free((char *)x);
	trace((/*{*/"}\n"));
}


void
sem_integer(n)
	long	n;
{
	trace(("sem_integer(n = %ld)\n{\n"/*}*/, n));
	if (!sem_root->type)
		goto done;
	if (sem_root->type->class != type_class_integer)
	{
		gram_error
		(
			"value of type %s required",
			type_class_name(sem_root->type)
		);
	}
	else
		*(long *)sem_root->addr = n;

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}


void
sem_string(s)
	string_ty *s;
{
	trace(("sem_string(s = %08lX)\n{\n"/*}*/, s));
	trace_string(s->str_text);
	if (!sem_root->type)
		goto done;
	if (sem_root->type->class != type_class_string)
	{
		gram_error
		(
			"value of type %s required",
			type_class_name(sem_root->type)
		);
	}
	else
	{
		trace
		((
			"addr = %08lX->%08lX",
			sem_root->addr,
			*(void **)sem_root->addr
		));
		*(string_ty **)sem_root->addr = s;
	}
	
	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}


void
sem_enum(s)
	string_ty *s;
{
	trace(("sem_enum(s = %08lX)\n{\n"/*}*/, s));
	trace_string(s->str_text);
	if (!sem_root->type)
		goto done;
	if (sem_root->type->class != type_class_enum)
	{
		gram_error
		(
			"value of type %s required",
			type_class_name(sem_root->type)
		);
	}
	else
	{
		if (sem_root->type->enum_parse(s, sem_root->addr))
		{
			gram_error
			(
				"the name \"%s\" is not a valid enumerator",
				s->str_text
			);
		}
	}

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}


void
sem_list()
{
	trace(("sem_list()\n{\n"/*}*/));
	if (!sem_root->type)
	{
		sem_push(0, 0);
		goto done;
	}
	if (sem_root->type->class != type_class_list)
	{
		gram_error
		(
			"value of type %s required",
			type_class_name(sem_root->type)
		);
		sem_push(0, 0);
	}
	else
	{
		type_ty	*type;
		void	*addr;

		sem_root->type->list_parse
		(
			*(void **)sem_root->addr,
			&type,
			&addr
		);
		sem_push(type, addr);
	}

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}


void
sem_field(name)
	string_ty	*name;
{
	trace(("sem_field(name = %08lX)\n{\n"/*}*/, name));
	trace_string(name->str_text);
	trace(("sem_root == %08lX;\n", sem_root));
	trace(("sem_root->type == %08lX;\n", sem_root->type));
	if (!sem_root->type)
	{
		sem_push(0, 0);
		goto done;
	}
	if (sem_root->type->class != type_class_struct)
	{
		gram_error
		(
			"value of type %s required",
			type_class_name(sem_root->type)
		);
		sem_push(0, 0);
	}
	else
	{
		type_ty		*type;
		void		*addr;
		unsigned long	mask;

		if (!sem_root->type->struct_parse)
			error("no struct parse");
		if
		(
			sem_root->type->struct_parse
			(
				*(void **)sem_root->addr,
				name,
				&type,
				&addr,
				&mask
			)
		)
		{
			gram_error("field name \"%s\" not valid", name->str_text);
			sem_push(0, 0);
		}
		else
		{
			unsigned long	*rmask;

			/*
			 * The first element of all the generated
			 * structures is the mask field.
			 */
			trace(("mask = 0x%08lX;\n", mask));
			rmask = *(unsigned long **)sem_root->addr;
			if (*rmask & mask)
			{
				gram_error
				(
					"field \"%s\" redefined",
					name->str_text
				);
			}
			*rmask |= mask;
			trace(("*rmask == 0x%08lX;\n", *rmask));
			sem_push(type, addr);
		}
	}

	/*
	 * here for all exits
	 */
	done:
	trace((/*{*/"}\n"));
}
