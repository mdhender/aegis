//
//	aegis - project change supervisor
//	Copyright (C) 1991-1995, 1998, 1999, 2001-2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/time.h>

#include <common/error.h>
#include <libaegis/gram.h>
#include <libaegis/input.h>
#include <common/mem.h>
#include <libaegis/meta_lex.h>
#include <libaegis/meta_parse.h>
#include <libaegis/sub.h>
#include <common/trace.h>

static sem_ty	*sem_root;


void *
parse(string_ty *filename, meta_type *type)
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
parse_env(const char *name, meta_type *type)
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
parse_input(input &ifp, meta_type *type)
{
    trace(("parse_input(ifp = *%08lX, type = %08lx)\n{\n", (long)&ifp,
	(long)type));
    lex_open_input(ifp);
    assert(type);
    assert(type->alloc);
    assert(type->struct_parse);
    void *addr = type->alloc();
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
sem_push(meta_type *type, void *addr)
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
	// do nothing
;
    else if (sem_root->type == &integer_type)
	*(long *)sem_root->addr = n;
    else if (sem_root->type == &real_type)
	*(double *)sem_root->addr = n;
    else if (sem_root->type == &time_type)
    {
	//
	// Time is always arithmetic, never a structure.
	// This works on every system the author has seen,
	// without loss of precision.
	//
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
	// do nothing
;
    else if (sem_root->type == &integer_type)
    {
	// Precision may be lost.
	*(long *)sem_root->addr = (long int)n;
    }
    else if (sem_root->type == &real_type)
	*(double *)sem_root->addr = n;
    else if (sem_root->type == &time_type)
    {
	//
	// Time is always arithmetic, never a structure.
	// Precision may be lost.
	//
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
	// do nothing
;
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
    {
	// do nothing
    }
    else if (!sem_root->type->enum_parse)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_charstar(scp, "Name", sem_root->type->name);
	lex_error(scp, i18n("value of type $name required"));
	sub_context_delete(scp);
    }
    else if (!sem_root->type->enum_parse(s, sem_root->addr))
    {
	sub_context_ty  *scp;
	string_ty       *suggest;

	assert(sem_root->type->fuzzy);
	suggest = sem_root->type->fuzzy(s);
	if (suggest)
	{
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", s);
	    sub_var_set_string(scp, "Guess", suggest);
	    lex_error(scp, i18n("no \"$name\", guessing \"$guess\""));
	    sub_context_delete(scp);
	    sem_root->type->enum_parse(suggest, sem_root->addr);
	}
	else
	{
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", s);
	    lex_error(scp, i18n("the name \"$name\" is undefined"));
	    sub_context_delete(scp);
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
	meta_type		*type;
	void		*addr;

	addr = sem_root->type->list_parse(sem_root->addr, &type);

	//
	// allocate the storage if necessary
	//
	if (type->alloc)
	{
	    void	    *contents;

	    contents = type->alloc();
	    *(generic_struct **)addr = (generic_struct *)contents;
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
	meta_type		*type;
	void		*addr;
	unsigned long	mask;
	generic_struct *gsp;
	int             redefok;

	gsp = (generic_struct *)sem_root->addr;
	redefok = 0;
	addr = sem_root->type->struct_parse(gsp, name, &type, &mask, &redefok);
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
		addr =
		    sem_root->type->struct_parse
		    (
			gsp,
			suggest,
			&type,
			&mask,
			&redefok
		    );
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
	    //
	    // The first element of all the generated
	    // structures is the mask field.
	    //
	    use_suggestion:
	    trace(("mask = 0x%08lX;\n", mask));
	    if (mask ? (gsp->mask & mask) : type->is_set(addr))
	    {
		sub_context_ty	*scp;

		if (redefok)
		{
		    gsp->mask |= mask;
		    trace(("gsp->mask == 0x%08lX;\n", gsp->mask));
		    if (type->alloc)
			addr = (void *)*(generic_struct **)addr;
		    sem_push(type, addr);
		    trace(("}\n"));
		    return;
		}

		scp = sub_context_new();
		sub_var_set_string(scp, "Name", name);
		lex_error(scp, i18n("field \"$name\" redefined"));
		sub_context_delete(scp);
	    }
	    gsp->mask |= mask;
	    trace(("gsp->mask == 0x%08lX;\n", gsp->mask));

	    //
	    // allocate the storage if necessary
	    //
	    if (type->alloc)
	    {
		void		*contents;

		contents = type->alloc();
		*(generic_struct **)addr = (generic_struct *)contents;
		addr = contents;
	    }

	    sem_push(type, addr);
	}
    }
    trace(("}\n"));
}
