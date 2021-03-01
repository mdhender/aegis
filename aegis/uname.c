/*
 *	aegis - project change supervisor
 *	Copyright (C) 1993, 1994 Peter Miller.
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
 * MANIFEST: functions to manipulate unames
 */

#include <ctype.h>

#include <sys/utsname.h>

#include <error.h>
#include <trace.h>
#include <uname.h>

static string_ty *uname_variant;
static string_ty *uname_node;


static void uname_init _((void));

static void
uname_init()
{
	struct utsname	u;

	if (uname_variant)
		return;
	trace(("uname_init()\n{\n"/*}*/));
	if (uname(&u) < 0)
		nfatal("uname");
	uname_variant =
		str_format
		(
			"%s-%s-%s-%s",
			u.sysname,
			u.release,
			u.version,
			u.machine
		);
	trace_string(uname_variant->str_text);
	uname_node = str_from_c(u.nodename);
	trace_string(uname_node->str_text);
	trace((/*{*/"}\n"));
}


string_ty *
uname_node_get()
{
	trace(("uname_node_get()\n{\n"/*}*/));
	uname_init();
	trace(("return %08lX;\n", (long)uname_node));
	trace((/*{*/"}\n"));
	return uname_node;
}


string_ty *
uname_variant_get()
{
	trace(("uname_default()\n{\n"/*}*/));
	uname_init();
	trace(("return %08lX;\n", (long)uname_variant));
	trace((/*{*/"}\n"));
	return uname_variant;
}
