//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
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

#include <libaegis/gram.h>
#include <libaegis/input.h>
#include <libaegis/meta_context.h>
#include <libaegis/meta_lex.h>
#include <libaegis/sem.h>


static meta_context *context;


void
sem_integer(long n)
{
    context->integer(n);
}


void
sem_real(double n)
{
    context->real(n);
}


void
sem_string(const nstring &s)
{
    context->string(s);
}


void
sem_enum(const nstring &s)
{
    context->enumeration(s);
}


void
sem_list(void)
{
    context->list();
}


void
sem_list_end(void)
{
    context->list_end();
}


void
sem_field(const nstring &name)
{
    context->field(name);
}


void
sem_field_end()
{
    context->field_end();
}


void
sem_parse(meta_context &ctx, input &ifp)
{
    context = &ctx;
    lex_open_input(ifp);
    gram_parse();
    context->end();
    lex_close();
    context = 0;
}


void
sem_parse_file(meta_context &ctx, const nstring &filename)
{
    input ip = lex_iopen_file(filename);
    sem_parse(ctx, ip);
}


void
sem_parse_env(meta_context &ctx, const nstring &name)
{
    input ip = lex_iopen_env(name);
    sem_parse(ctx, ip);
}
