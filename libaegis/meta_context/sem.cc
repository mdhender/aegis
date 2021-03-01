//
// aegis - project change supervisor
// Copyright (C) 2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/trace.h>

#include <libaegis/input.h>
#include <libaegis/meta_context/sem.h>
#include <libaegis/meta_lex.h>
#include <libaegis/meta_type.h>
#include <libaegis/sem.h>
#include <libaegis/sub.h>


meta_context_sem::~meta_context_sem()
{
}


meta_context_sem::meta_context_sem() :
    root(0)
{
}


void *
meta_context_sem::parse(const nstring &filename, meta_type *type)
{
    trace(("parse(filename = %s, type = %p)\n{\n",
        filename.quote_c().c_str(), type));
    input ifp = lex_iopen_file(filename);
    void *addr = parse_input(ifp, type);
    trace(("return %p;\n", addr));
    trace(("}\n"));
    return addr;
}


void *
meta_context_sem::parse_env(const nstring &name, meta_type *type)
{
    trace(("parse_env(name = %s, type = %08lx)\n{\n", name.quote_c().c_str(),
        (long)type));
    input ifp = lex_iopen_env(name);
    void *addr = parse_input(ifp, type);
    trace(("return %p;\n", addr));
    trace(("}\n"));
    return addr;
}


void *
meta_context_sem::parse_input(input &ifp, meta_type *type)
{
    trace(("parse_input(ifp = *%p, type = %p)\n{\n", &ifp, type));
    assert(type);
    assert(type->alloc);
    assert(type->struct_parse);
    void *addr = type->alloc();
    sem_push(type, addr);

    sem_parse(*this, ifp);

    trace(("return %p;\n", addr));
    trace(("}\n"));
    return addr;
}


void
meta_context_sem::sem_push(meta_type *type, void *addr)
{
    trace(("sem_push(type = %p, addr = %p)\n{\n", type, addr));
    trace(("type->name == \"%s\";\n", type ? type->name : "void"));
    sem_ty *sp = new sem_ty;
    sp->type = type;
    sp->addr = addr;
    sp->next = root;
    root = sp;
    trace(("}\n"));
}


void
meta_context_sem::sem_pop(void)
{
    trace(("sem_pop()\n{\n"));
    sem_ty *x = root;
    root = x->next;
    x->type = 0;
    x->addr = 0;
    x->next = 0;
    delete x;
    trace(("}\n"));
}


void
meta_context_sem::integer(long n)
{
    trace(("meta_context_sem::integer(n = %ld)\n{\n", n));
    if (!root->type)
        // do nothing
        ;
    else if (root->type == &integer_type)
        *(long *)root->addr = n;
    else if (root->type == &real_type)
        *(double *)root->addr = n;
    else if (root->type == &time_type)
    {
        //
        // Time is always arithmetic, never a structure.
        // This works on every system the author has seen,
        // without loss of precision.
        //
        *(time_t *)root->addr = n;
        trace(("time is %s", ctime((time_t *)root->addr)));
    }
    else
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_charstar(scp, "Name", root->type->name);
        lex_error(scp, i18n("value of type $name required"));
        sub_context_delete(scp);
    }
    trace(("}\n"));
}


void
meta_context_sem::real(double n)
{
    trace(("meta_context_sem::real(n = %g)\n{\n", n));
    if (!root->type)
        // do nothing
        ;
    else if (root->type == &integer_type)
    {
        // Precision may be lost.
        *(long *)root->addr = (long int)n;
    }
    else if (root->type == &real_type)
        *(double *)root->addr = n;
    else if (root->type == &time_type)
    {
        //
        // Time is always arithmetic, never a structure.
        // Precision may be lost.
        //
        *(time_t *)root->addr = (long int)n;
    }
    else
    {
        sub_context_ty  *scp;

        scp = sub_context_new();
        sub_var_set_charstar(scp, "Name", root->type->name);
        lex_error(scp, i18n("value of type $name required"));
        sub_context_delete(scp);
    }
    trace(("}\n"));
}


void
meta_context_sem::string(const nstring &s)
{
    trace(("meta_context_sem::string(s = %s)\n{\n", s.quote_c().c_str()));
    if (!root->type)
        // do nothing
        ;
    else if (root->type != &string_type)
    {
        sub_context_ty *scp = sub_context_new();
        sub_var_set_charstar(scp, "Name", root->type->name);
        lex_error(scp, i18n("value of type $name required"));
        sub_context_delete(scp);
    }
    else
    {
        trace(("addr = %p->%p\n", root->addr,
            *(void **)root->addr));
        *(string_ty **)root->addr = s.get_ref_copy();
    }
    trace(("}\n"));
}


void
meta_context_sem::enumeration(const nstring &s)
{
    trace(("sem_enum(s = %s)\n{\n", s.quote_c().c_str()));
    if (!root->type)
    {
        // do nothing
    }
    else if (!root->type->enum_parse)
    {
        sub_context_ty *scp = sub_context_new();
        sub_var_set_charstar(scp, "Name", root->type->name);
        lex_error(scp, i18n("value of type $name required"));
        sub_context_delete(scp);
    }
    else if (!root->type->enum_parse(s.get_ref(), root->addr))
    {
        assert(root->type->fuzzy);
        string_ty *suggest = root->type->fuzzy(s.get_ref());
        if (suggest)
        {
            sub_context_ty *scp = sub_context_new();
            sub_var_set_string(scp, "Name", s);
            sub_var_set_string(scp, "Guess", suggest);
            lex_error(scp, i18n("no \"$name\", guessing \"$guess\""));
            sub_context_delete(scp);
            root->type->enum_parse(suggest, root->addr);
        }
        else
        {
            sub_context_ty *scp = sub_context_new();
            sub_var_set_string(scp, "Name", s);
            lex_error(scp, i18n("the name \"$name\" is undefined"));
            sub_context_delete(scp);
        }
    }
    trace(("}\n"));
}


void
meta_context_sem::list(void)
{
    trace(("meta_context_sem::list()\n{\n"));
    if (!root->type)
        sem_push(0, 0);
    else if (!root->type->list_parse)
    {
        sub_context_ty *scp = sub_context_new();
        sub_var_set_charstar(scp, "Name", root->type->name);
        lex_error(scp, i18n("value of type $name required"));
        sub_context_delete(scp);
        sem_push(0, 0);
    }
    else
    {
        meta_type *type = 0;
        void *addr = root->type->list_parse(root->addr, &type);

        //
        // allocate the storage if necessary
        //
        if (type->alloc)
        {
            void *contents = type->alloc();
            *(generic_struct **)addr = (generic_struct *)contents;
            addr = contents;
        }

        sem_push(type, addr);
    }
    trace(("}\n"));
}


void
meta_context_sem::list_end()
{
    sem_pop();
}


void
meta_context_sem::field(const nstring &name)
{
    trace(("meta_context_sem::field(name = %s)\n{\n", name.quote_c().c_str()));
    trace(("root == %p;\n", root));
    trace(("root->type == %p;\n", root->type));
    trace(("root->addr == %p;\n", root->addr));
    if (!root->type)
        sem_push(0, 0);
    else if (!root->type->struct_parse)
    {
        sub_context_ty *scp = sub_context_new();
        sub_var_set_charstar(scp, "Name", root->type->name);
        lex_error(scp, i18n("value of type $name required"));
        sub_context_delete(scp);
        sem_push(0, 0);
    }
    else
    {
        generic_struct *gsp = (generic_struct *)root->addr;
        meta_type *type = 0;
        unsigned long mask = 0;
        int redefok = 0;
        void *addr =
            root->type->struct_parse
            (
                gsp,
                name.get_ref(),
                &type,
                &mask,
                &redefok
            );
        if (!addr)
        {
            assert(root->type->fuzzy);
            string_ty *suggest = root->type->fuzzy(name.get_ref());
            if (suggest)
            {
                sub_context_ty *scp = sub_context_new();
                sub_var_set_string(scp, "Name", name);
                sub_var_set_string(scp, "Guess", suggest);
                lex_error(scp, i18n("no \"$name\", guessing \"$guess\""));
                sub_context_delete(scp);
                addr =
                    root->type->struct_parse
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
            sub_context_ty *scp = sub_context_new();
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

                sub_context_ty *scp = sub_context_new();
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
                void *contents = type->alloc();
                *(generic_struct **)addr = (generic_struct *)contents;
                addr = contents;
            }

            sem_push(type, addr);
        }
    }
    trace(("}\n"));
}


void
meta_context_sem::field_end()
{
    sem_pop();
}


void
meta_context_sem::end()
{
    while (root)
        sem_pop();
}


// vim: set ts=8 sw=4 et :
