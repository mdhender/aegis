/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1995, 1998, 1999, 2001-2003 Peter Miller;
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
 * MANIFEST: functions to parse aegis' data files
 */

#include <ac/time.h>

#include <error.h>
#include <gram.h>
#include <input.h>
#include <lex.h>
#include <mem.h>
#include <parse.h>
#include <sub.h>
#include <trace.h>

static sem_ty	*sem_root;


void *
parse(string_ty *filename, type_ty *type)
{
    void	    *addr;

    trace(("parse(filename = \"%s\", type = %08lX)\n{\n", filename->str_text,
	(long)type));
    lex_open(filename);
    assert(type);
    assert(type->alloc);
    assert(type->struct_parse);
    addr = type->alloc();
    sem_push(type, addr);

    trace(("gram_parse()\n{\n"));
    gram_parse();
    trace(("}\n"));

    while (sem_root)
	sem_pop();
    lex_close();
    trace(("return %08lX;\n", (long)addr));
    trace(("}\n"));
    return addr;
}


void *
parse_env(const char *name, type_ty *type)
{
    void	    *addr;

    trace(("parse_env(name = \"%s\", type = %08lx)\n{\n", name, (long)type));
    lex_open_env(name);
    assert(type);
    assert(type->alloc);
    assert(type->struct_parse);
    addr = type->alloc();
    sem_push(type, addr);

    trace(("gram_parse()\n{\n"));
    gram_parse();
    trace(("}\n"));

    while (sem_root)
	sem_pop();
    lex_close();
    trace(("return %08lX;\n", (long)addr));
    trace(("}\n"));
    return addr;
}


void *
parse_input(input_ty *ifp, type_ty *type)
{
    void	    *addr;

    trace(("parse_input(ifp = %08lX, type = %08lx)\n{\n", (long)ifp,
	(long)type));
    lex_open_input(ifp);
    assert(type);
    assert(type->alloc);
    assert(type->struct_parse);
    addr = type->alloc();
    sem_push(type, addr);

    trace(("gram_parse()\n{\n"));
    gram_parse();
    trace(("}\n"));

    while (sem_root)
	sem_pop();
    lex_close();
    trace(("return %08lX;\n", (long)addr));
    trace(("}\n"));
    return addr;
}


void
sem_push(type_ty *type, void *addr)
{
    sem_ty	    *sp;

    trace(("sem_push(type = %08lX, addr = %08lX)\n{\n", (long)type,
	(long)addr));
    trace(("type->name == \"%s\";\n", type ? type->name : "void"));
    sp = (sem_ty *)mem_alloc(sizeof(sem_ty));
    sp->type = type;
    sp->addr = addr;
    sp->next = sem_root;
    sem_root = sp;
    trace(("}\n"));
}


void
sem_pop(void)
{
    sem_ty	    *x;

    trace(("sem_pop()\n{\n"));
    x = sem_root;
    sem_root = x->next;
    x->type = 0;
    x->addr = 0;
    x->next = 0;
    mem_free((char *)x);
    trace(("}\n"));
}


void
sem_integer(long n)
{
    trace(("sem_integer(n = %ld)\n{\n", n));
    if (!sem_root->type)
	/* do nothing */;
    else if (sem_root->type == &integer_type)
	*(long *)sem_root->addr = n;
    else if (sem_root->type == &real_type)
	*(double *)sem_root->addr = n;
    else if (sem_root->type == &time_type)
    {
	/*
	 * Time is always arithmetic, never a structure.
	 * This works on every system the author has seen,
	 * without loss of precision.
	 */
	*(time_t *)sem_root->addr = n;
	trace(("time is %s", ctime((time_t *)sem_root->addr)));
    }
    else
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", sem_root->type->name);
	lex_error(scp, i18n("value of type $name required"));
	sub_context_delete(scp);
    }
    trace(("}\n"));
}


void
sem_real(double n)
{
    trace(("sem_real(n = %g)\n{\n", n));
    if (!sem_root->type)
	/* do nothing */;
    else if (sem_root->type == &integer_type)
    {
	/* Precision may be lost. */
	*(long *)sem_root->addr = (long int)n;
    }
    else if (sem_root->type == &real_type)
	*(double *)sem_root->addr = n;
    else if (sem_root->type == &time_type)
    {
	/*
	 * Time is always arithmetic, never a structure.
	 * Precision may be lost.
	 */
	*(time_t *)sem_root->addr = (long int)n;
    }
    else
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", sem_root->type->name);
	lex_error(scp, i18n("value of type $name required"));
	sub_context_delete(scp);
    }
    trace(("}\n"));
}


void
sem_string(string_ty *s)
{
    trace(("sem_string(s = %08lX)\n{\n", (long)s));
    trace_string(s->str_text);
    if (!sem_root->type)
	/* do nothing */;
    else if (sem_root->type != &string_type)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", sem_root->type->name);
	lex_error(scp, i18n("value of type $name required"));
	sub_context_delete(scp);
    }
    else
    {
	trace(("addr = %08lX->%08lX\n", (long)sem_root->addr,
	    (long)*(void **)sem_root->addr));
	*(string_ty **)sem_root->addr = s;
    }
    trace(("}\n"));
}


void
sem_enum(string_ty *s)
{
    trace(("sem_enum(s = %08lX)\n{\n", (long)s));
    trace_string(s->str_text);
    if (!sem_root->type)
	/* do nothing */;
    else if (!sem_root->type->enum_parse)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", sem_root->type->name);
	lex_error(scp, i18n("value of type $name required"));
	sub_context_delete(scp);
    }
    else
    {
	int		n;

	n = sem_root->type->enum_parse(s);
	if (n < 0)
	{
	    sub_context_ty  *scp;
	    string_ty	    *suggest;

	    assert(sem_root->type->fuzzy);
	    suggest = sem_root->type->fuzzy(s);
	    if (suggest)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "Name", s);
		sub_var_set_string(scp, "Guess", suggest);
		lex_error(scp, i18n("no \"$name\", guessing \"$guess\""));
		sub_context_delete(scp);
		n = sem_root->type->enum_parse(suggest);
		assert(n >= 0);
		goto use_suggestion;
	    }
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", s);
	    lex_error(scp, i18n("the name \"$name\" is undefined"));
	    sub_context_delete(scp);
	}
	else
	{
	    /*
	     * This is a portability problem: if the C
	     * implementation does not always store enums in ints
	     * this will break.
	     */
	    use_suggestion:
	    *(int *)sem_root->addr = n;
	}
    }
    trace(("}\n"));
}


void
sem_list(void)
{
    trace(("sem_list()\n{\n"));
    if (!sem_root->type)
	sem_push(0, 0);
    else if (!sem_root->type->list_parse)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", sem_root->type->name);
	lex_error(scp, i18n("value of type $name required"));
	sub_context_delete(scp);
	sem_push(0, 0);
    }
    else
    {
	type_ty		*type;
	void		*addr;

	addr = sem_root->type->list_parse(sem_root->addr, &type);

	/*
	 * allocate the storage if necessary
	 */
	if (type->alloc)
	{
	    void	    *contents;

	    contents = type->alloc();
	    *(generic_struct_ty **)addr = (generic_struct_ty *)contents;
	    addr = contents;
	}

	sem_push(type, addr);
    }
    trace(("}\n"));
}


void
sem_field(string_ty *name)
{
    trace(("sem_field(name = %08lX)\n{\n", (long)name));
    trace_string(name->str_text);
    trace(("sem_root == %08lX;\n", (long)sem_root));
    trace(("sem_root->type == %08lX;\n", (long)sem_root->type));
    trace(("sem_root->addr == %08lX;\n", (long)sem_root->addr));
    if (!sem_root->type)
	sem_push(0, 0);
    else if (!sem_root->type->struct_parse)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", sem_root->type->name);
	lex_error(scp, i18n("value of type $name required"));
	sub_context_delete(scp);
	sem_push(0, 0);
    }
    else
    {
	type_ty		*type;
	void		*addr;
	unsigned long	mask;
	generic_struct_ty *gsp;

	gsp = (generic_struct_ty *)sem_root->addr;
	addr = sem_root->type->struct_parse(gsp, name, &type, &mask);
	if (!addr)
	{
	    sub_context_ty  *scp;
	    string_ty	    *suggest;

	    assert(sem_root->type->fuzzy);
	    suggest = sem_root->type->fuzzy(name);
	    if (suggest)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "Name", name);
		sub_var_set_string(scp, "Guess", suggest);
		lex_error(scp, i18n("no \"$name\", guessing \"$guess\""));
		sub_context_delete(scp);
		addr = sem_root->type->struct_parse(gsp, suggest, &type, &mask);
		assert(addr);
		goto use_suggestion;
	    }
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", name);
	    lex_error(scp, i18n("the name \"$name\" is undefined"));
	    sub_context_delete(scp);
	    sem_push(0, 0);
	}
	else
	{
	    /*
	     * The first element of all the generated
	     * structures is the mask field.
	     */
	    use_suggestion:
	    trace(("mask = 0x%08lX;\n", mask));
	    if (mask ? (gsp->mask & mask) : type->is_set(addr))
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "Name", name);
		lex_error(scp, i18n("field \"$name\" redefined"));
		sub_context_delete(scp);
	    }
	    gsp->mask |= mask;
	    trace(("gsp->mask == 0x%08lX;\n", gsp->mask));

	    /*
	     * allocate the storage if necessary
	     */
	    if (type->alloc)
	    {
		void		*contents;

		contents = type->alloc();
		*(generic_struct_ty **)addr = (generic_struct_ty *)contents;
		addr = contents;
	    }

	    sem_push(type, addr);
	}
    }
    trace(("}\n"));
}
