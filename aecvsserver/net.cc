//
//	aegis - project change supervisor
//	Copyright (C) 2004, 2005 Peter Miller;
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
#include <input/crop.h>
#include <mem.h>
#include <net.h>
#include <os.h>
#include <output/file.h>
#include <output/prefix.h>
#include <output/stdout.h>
#include <output/tee.h>
#include <response.h>
#include <symtab.h>


net_ty::net_ty() :
    in(input_stdin()),
    out(new output_stdout_ty()),
    log_client(0),
    rooted(0),
    response_queue_length(0),
    response_queue_max(0),
    response_queue_item(0),
    dir_info_cs(0),
    dir_info_ss(0),
    curdir(0),
    file_info(0),
    updating_verbose(0)
{
    //
    // Initialize which responses the client is capable of receiving.
    //
    for (int j = 0; j < response_code_MAX; ++j)
	response_valid[j] = 0;
    response_valid[response_code_E] = 1;
    response_valid[response_code_error] = 1;
    response_valid[response_code_hate] = 1;
    response_valid[response_code_love] = 1;
    response_valid[response_code_ok] = 1;
    response_valid[response_code_Valid_requests] = 1;

    //
    // Set SO_KEEPALIVE on the socket, so that we don't hang forever
    // if the client dies while we are waiting for input.
    //
    input_keepalive(in);
}


net_ty::~net_ty()
{
    input_delete(in);
    in = 0;
    delete out;
    out = 0;

    if (updating_verbose)
    {
	str_free(updating_verbose);
	updating_verbose = 0;
    }

    if (log_client)
    {
	delete log_client;
	log_client = 0;
    }
    if (dir_info_cs)
    {
	delete dir_info_cs;
	dir_info_cs = 0;
    }
    if (dir_info_ss)
    {
	delete dir_info_ss;
	dir_info_ss = 0;
    }
    curdir = 0;
    if (file_info)
    {
	delete file_info;
	file_info = 0;
    }
}


string_ty *
net_ty::getline()
{
    string_ty *s = input_one_line(in);
    if (log_client)
    {
	log_client->fprintf("%s\n", s->str_text);
	log_client->flush();
    }
    return s;
}


void
net_ty::printf(const char *fmt, ...)
{
    va_list ap;
    assert(fmt);
    va_start(ap, fmt);
    out->vfprintf(fmt, ap);
    va_end(ap);
}


void
net_ty::response_queue(response_ty *rp)
{
    //
    // Don't bother queueing responses the client has asked us not to send.
    //
    response_code_ty code = response_code_get(rp);
    if (!response_valid[code])
    {
	response_delete(rp);
	return;
    }

    //
    // Make sure there is enough room in the queue.
    //
    if (response_queue_length >= response_queue_max)
    {
	size_t          nbytes;

	response_queue_max = 4 + 2 * response_queue_max;
	nbytes = sizeof(response_queue_item[0]) * response_queue_max;
	response_queue_item =
	    (response_ty **)
	    mem_change_size(response_queue_item, nbytes);
    }

    //
    // Put the response on the end of the queue.
    //
    response_queue_item[response_queue_length++] = rp;

    //
    // Some codes cause an immediate flush.
    //
    if (response_flushable(rp))
	response_flush();
}


void
net_ty::response_flush()
{
    //
    // Write any pending responses to the client.
    //
    for (size_t j = 0; j < response_queue_length; ++j)
    {
	response_ty     *rp;

	rp = response_queue_item[j];
	response_write(rp, out);
	response_delete(rp);
	if (log_client)
	    out->flush();
    }
    response_queue_length = 0;

    //
    // Make sure the output is written to the client.
    //
    out->flush();
}


void
net_ty::log_to_file(string_ty *filename)
{
    // This only works once
    if (log_client)
	return;

    os_become_orig();
    output_ty *op = output_file_text_open(filename);
    os_become_undo();
    log_client = new output_prefix_ty(op, true, "C: ");
    output_ty *log_server = new output_prefix_ty(op, false, "S: ");
    out = new output_tee_ty(out, true, log_server, true);
}


void
net_ty::log_by_env(const char *envar)
{
    const char      *cp;
    string_ty       *s;

    cp = getenv(envar);
    if (!cp || !*cp)
	return;
    s = str_from_c(cp);
    log_to_file(s);
    str_free(s);
}


void
net_ty::argument(string_ty *s)
{
    assert(s);
    argument_list.push_back(s);
}


void
net_ty::argumentx(string_ty *s)
{
    assert(s);
    if (argument_list.nstrings)
    {
	static string_ty *newline;
	string_ty       **spp;
	string_ty       *s2;

	if (!newline)
	    newline = str_from_c("\n");
	spp = argument_list.string + argument_list.nstrings - 1;
	s2 = str_cat_three(*spp, newline, s);
	str_free(*spp);
	*spp = s2;
    }
    else
	argument_list.push_back(s);
}


void
net_ty::accumulator_reset()
{
    argument_list.clear();
    if (file_info)
	file_info->clear();
    if (dir_info_cs)
	dir_info_cs->clear();
    if (dir_info_ss)
	dir_info_ss->clear();
    curdir = 0;
    if (updating_verbose)
    {
	str_free(updating_verbose);
	updating_verbose = 0;
    }
}


static void
file_info_reaper(void *p)
{
    file_info_delete((file_info_ty *)p);
}


file_info_ty *
net_ty::file_info_find(string_ty *server_side, int auto_alloc)
{
    file_info_ty    *fip;

    if (!file_info)
    {
	file_info = new symtab_ty(5);
	file_info->set_reap(file_info_reaper);
    }
    fip = (file_info_ty *)file_info->query(server_side);
    if (!fip && auto_alloc)
    {
	fip = file_info_new();
	file_info->assign(server_side, fip);
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
net_ty::directory_set(string_ty *client_side, string_ty *server_side)
{
    directory_ty    *dp;

    dp = directory_new(client_side, server_side);
    curdir = dp;
    if (!dir_info_cs)
    {
	dir_info_cs = new symtab_ty(5);
	dir_info_cs->set_reap(dir_reaper);
    }
    dir_info_cs->assign(client_side, dp);
    if (!dir_info_ss)
    {
	dir_info_ss = new symtab_ty(5);
	// NO reaper for this one.
    }
    dir_info_ss->assign(server_side, dp);
}


directory_ty *
net_ty::directory_find_client_side(string_ty *client_side)
{
    if (!dir_info_cs)
	return 0;
    return (directory_ty *)dir_info_cs->query(client_side);
}


directory_ty *
net_ty::directory_find_server_side(string_ty *server_side)
{
    if (!dir_info_ss)
	return 0;
    return (directory_ty *)dir_info_ss->query(server_side);
}


void
net_ty::set_updating_verbose(string_ty *s)
{
    if (updating_verbose)
    {
	str_free(updating_verbose);
	updating_verbose = 0;
    }
    if (s)
	updating_verbose = str_copy(s);
}


input_ty *
net_ty::in_crop(long length)
{
    return input_crop_new(in, 0, length);
}
