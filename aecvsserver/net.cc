//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate nets
//

#include <ac/stdlib.h>

#include <error.h> // for assert
#include <file_info.h>
#include <input/stdin.h>
#include <mem.h>
#include <net.h>
#include <os.h>
#include <output/file.h>
#include <output/prefix.h>
#include <output/stdout.h>
#include <output/tee.h>
#include <response.h>
#include <symtab.h>


net_ty *
net_new(void)
{
    net_ty          *np;
    int             j;

    np = (net_ty *)mem_alloc(sizeof(net_ty));
    np->in = input_stdin();
    np->out = output_stdout();
    np->log_client = 0;
    np->rooted = 0;
    np->updating_verbose = 0;

    //
    // Initialize which responses the client is capable of receiving.
    //
    for (j = 0; j < response_code_MAX; ++j)
	np->response_valid[j] = 0;
    np->response_valid[response_code_E] = 1;
    np->response_valid[response_code_error] = 1;
    np->response_valid[response_code_hate] = 1;
    np->response_valid[response_code_love] = 1;
    np->response_valid[response_code_ok] = 1;
    np->response_valid[response_code_Valid_requests] = 1;

    np->response_queue_length = 0;
    np->response_queue_max = 0;
    np->response_queue_item = 0;

    np->curdir = 0;
    np->dir_info_cs = 0;
    np->dir_info_ss = 0;
    np->file_info = 0;
    string_list_constructor(&np->argument_list);

    //
    // Set SO_KEEPALIVE on the socket, so that we don't hang forever
    // if the client dies while we are waiting for input.
    //
    input_keepalive(np->in);
    return np;
}


void
net_delete(net_ty *np)
{
    assert(np);
    input_delete(np->in);
    output_delete(np->out);
    np->in = 0;
    np->out = 0;

    if (np->updating_verbose)
    {
	str_free(np->updating_verbose);
	np->updating_verbose = 0;
    }

    if (np->log_client)
    {
	output_delete(np->log_client);
	np->log_client = 0;
    }
    if (np->dir_info_cs)
    {
	symtab_free(np->dir_info_cs);
	np->dir_info_cs = 0;
    }
    if (np->dir_info_ss)
    {
	symtab_free(np->dir_info_ss);
	np->dir_info_ss = 0;
    }
    np->curdir = 0;
    if (np->file_info)
	symtab_free(np->file_info);
    string_list_destructor(&np->argument_list);
    mem_free(np);
}


string_ty *
net_getline(net_ty *np)
{
    string_ty       *s;

    s = input_one_line(np->in);
    if (np->log_client)
    {
	output_fprintf(np->log_client, "%s\n", s->str_text);
	output_flush(np->log_client);
    }
    return s;
}


void
net_printf(net_ty *np, const char *fmt, ...)
{
    va_list         ap;

    assert(ap);
    assert(fmt);
    va_start(ap, fmt);
    output_vfprintf(np->out, fmt, ap);
    va_end(ap);
}


void
net_response_queue(net_ty *np, response_ty *rp)
{
    response_code_ty code;

    //
    // Don't bother queueing responses the client has asked us not to send.
    //
    code = response_code_get(rp);
    if (!np->response_valid[code])
    {
	response_delete(rp);
	return;
    }

    //
    // Make sure there is enough room in the queue.
    //
    if (np->response_queue_length >= np->response_queue_max)
    {
	size_t          nbytes;

	np->response_queue_max = 4 + 2 * np->response_queue_max;
	nbytes = sizeof(np->response_queue_item[0]) * np->response_queue_max;
	np->response_queue_item =
	    (response_ty **)
	    mem_change_size(np->response_queue_item, nbytes);
    }

    //
    // Put the response on the end of the queue.
    //
    np->response_queue_item[np->response_queue_length++] = rp;

    //
    // Some codes cause an immediate flush.
    //
    if (response_flushable(rp))
	net_response_flush(np);
}


void
net_response_flush(net_ty *np)
{
    size_t          j;

    //
    // Write any pending responses to the client.
    //
    for (j = 0; j < np->response_queue_length; ++j)
    {
	response_ty     *rp;

	rp = np->response_queue_item[j];
	response_write(rp, np->out);
	response_delete(rp);
	if (np->log_client)
	    output_flush(np->out);
    }
    np->response_queue_length = 0;

    //
    // Make sure the output is written to the client.
    //
    output_flush(np->out);
}


void
net_log_to_file(net_ty *np, string_ty *filename)
{
    output_ty       *op;
    output_ty       *log_server;

    // This only works once
    if (np->log_client)
	return;

    os_become_orig();
    op = output_file_text_open(filename);
    os_become_undo();
    np->log_client = output_prefix(op, 1, "C: ");
    log_server = output_prefix(op, 0, "S: ");
    np->out = output_tee(np->out, 1, log_server, 1);
}


void
net_log_by_env(net_ty *np, const char *envar)
{
    const char      *cp;
    string_ty       *s;

    cp = getenv(envar);
    if (!cp || !*cp)
	return;
    s = str_from_c(cp);
    net_log_to_file(np, s);
    str_free(s);
}


void
net_argument(net_ty *np, string_ty *s)
{
    assert(np);
    assert(s);
    string_list_append(&np->argument_list, s);
}


void
net_argumentx(net_ty *np, string_ty *s)
{
    assert(np);
    assert(s);
    if (np->argument_list.nstrings)
    {
	static string_ty *newline;
	string_ty       **spp;
	string_ty       *s2;

	if (!newline)
	    newline = str_from_c("\n");
	spp = np->argument_list.string + np->argument_list.nstrings - 1;
	s2 = str_cat_three(*spp, newline, s);
	str_free(*spp);
	*spp = s2;
    }
    else
	string_list_append(&np->argument_list, s);
}


void
net_accumulator_reset(net_ty *np)
{
    while (np->argument_list.nstrings)
	str_free(np->argument_list.string[--np->argument_list.nstrings]);
    if (np->file_info)
    {
	symtab_free(np->file_info);
	np->file_info = 0;
    }
    if (np->dir_info_cs)
    {
	symtab_free(np->dir_info_cs);
	np->dir_info_cs = 0;
    }
    if (np->dir_info_ss)
    {
	symtab_free(np->dir_info_ss);
	np->dir_info_ss = 0;
    }
    np->curdir = 0;
    if (np->updating_verbose)
    {
	str_free(np->updating_verbose);
	np->updating_verbose = 0;
    }
}


static void
file_info_reaper(void *p)
{
    file_info_delete((file_info_ty *)p);
}


file_info_ty *
net_file_info_find(net_ty *np, string_ty *server_side, int auto_alloc)
{
    file_info_ty    *fip;

    if (!np->file_info)
    {
	np->file_info = symtab_alloc(5);
	np->file_info->reap = file_info_reaper;
    }
    fip = (file_info_ty *)symtab_query(np->file_info, server_side);
    if (!fip && auto_alloc)
    {
	fip = file_info_new();
	symtab_assign(np->file_info, server_side, fip);
    }
    return fip;
}


static void
dir_reaper(void *p)
{
    directory_ty    *dp;

    dp = (directory_ty *)p;
    directory_delete(dp);
}


void
net_directory_set(net_ty *np, string_ty *client_side, string_ty *server_side)
{
    directory_ty    *dp;

    dp = directory_new(client_side, server_side);
    np->curdir = dp;
    if (!np->dir_info_cs)
    {
	np->dir_info_cs = symtab_alloc(5);
	np->dir_info_cs->reap = dir_reaper;
    }
    symtab_assign(np->dir_info_cs, client_side, dp);
    if (!np->dir_info_ss)
    {
	np->dir_info_ss = symtab_alloc(5);
	// NO reaper for this one.
    }
    symtab_assign(np->dir_info_ss, server_side, dp);
}


directory_ty *
net_directory_find_client_side(net_ty *np, string_ty *client_side)
{
    if (!np->dir_info_cs)
	return 0;
    return (directory_ty *)symtab_query(np->dir_info_cs, client_side);
}


directory_ty *
net_directory_find_server_side(net_ty *np, string_ty *server_side)
{
    if (!np->dir_info_ss)
	return 0;
    return (directory_ty *)symtab_query(np->dir_info_ss, server_side);
}
