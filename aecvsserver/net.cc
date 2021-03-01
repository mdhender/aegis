//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#include <common/ac/stdlib.h>

#include <common/error.h> // for assert
#include <common/symtab.h>
#include <libaegis/input/crop.h>
#include <libaegis/input/stdin.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/output/prefix.h>
#include <libaegis/output/stdout.h>
#include <libaegis/output/tee.h>

#include <aecvsserver/file_info.h>
#include <aecvsserver/net.h>
#include <aecvsserver/response.h>


net_ty::net_ty() :
    in(new input_stdin()),
    out(output_stdout::create()),
    rooted(0),
    response_queue_length(0),
    response_queue_max(0),
    response_queue_item(0),
    dir_info_cs(0),
    dir_info_ss(0),
    curdir(0),
    file_info(0)
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
    in->keepalive();
}


net_ty::~net_ty()
{
    out.reset();

    if (updating_verbose)
    {
	str_free(updating_verbose);
	updating_verbose = 0;
    }

    log_client.reset();
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


bool
net_ty::getline(nstring &s)
{
    bool result = in->one_line(s);
    if (result && log_client)
    {
	log_client->fprintf("%s\n", s.c_str());
	log_client->flush();
    }
    return result;
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
net_ty::response_queue(response *rp)
{
    //
    // Don't bother queueing responses the client has asked us not to send.
    //
    response_code_ty code = rp->code_get();
    if (!response_valid[code])
    {
	delete rp;
	return;
    }

    //
    // Make sure there is enough room in the queue.
    //
    if (response_queue_length >= response_queue_max)
    {
	response_queue_max = response_queue_max * 2 + 4;
	response **new_queue = new response * [response_queue_max];
	for (size_t k = 0; k < response_queue_length; ++k)
	    new_queue[k] = response_queue_item[k];
	delete [] response_queue_item;
	response_queue_item = new_queue;
    }

    //
    // Put the response on the end of the queue.
    //
    response_queue_item[response_queue_length++] = rp;

    //
    // Some codes cause an immediate flush.
    //
    if (rp->flushable())
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
	response     *rp;

	rp = response_queue_item[j];
	rp->write(out);
	delete rp;
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
    output::pointer op = output_file::text_open(filename);
    os_become_undo();
    log_client = output_prefix::create(op, "C: ");
    output::pointer log_server = output_prefix::create(op, "S: ");
    out = output_tee::create(out, log_server);
}


void
net_ty::log_by_env(const char *envar)
{
    const char *cp = getenv(envar);
    if (!cp || !*cp)
	return;
    string_ty *s = str_from_c(cp);
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


input
net_ty::in_crop(long length)
{
    return new input_crop(in, length);
}
