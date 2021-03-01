//
//	aegis - project change supervisor
//	Copyright (C) 2001-2004 Peter Miller;
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
// MANIFEST: functions to manipulate getpw_caches
//

#include <ac/string.h>

#include <getpw_cache.h>
#include <itab.h>
#include <mem.h>
#include <symtab.h>

static symtab_ty *login_table;
static itab_ty *uid_table;


static struct passwd *
passwd_null(void)
{
	struct passwd	*result;

	result = (struct passwd *)mem_alloc(sizeof(struct passwd));

	//
	// This isn't portable, it assumes that NULL pointers are
	// all-bits-zero, but this is only to cover all the fields we
	// don't care about.  We set the ones we -do- care about to
	// NULL explicitly.
	//
	memset(result, 0, sizeof(*result));

#ifndef SOURCE_FORGE_HACK
	result->pw_name = 0;
	result->pw_passwd = 0;
	result->pw_uid = ~0;
	result->pw_gid = ~0;
	result->pw_gecos = 0;
#ifdef HAVE_pw_comment
	result->pw_comment = 0;
#endif
	result->pw_dir = 0;
	result->pw_shell = 0;
#else
	//
	// This is used to fake a user account, because on SourceForge.net
	// the Apache servers don't have access to /etc/passwd.  The fake
	// information will be further taylored below.
	//
	result->pw_name = "nobody";
	result->pw_passwd = "x";
	result->pw_uid = AEGIS_MIN_UID;
	result->pw_gid = AEGIS_MIN_GID;
	result->pw_gecos = "nobody";
#ifdef HAVE_pw_comment
	result->pw_comment = 0;
#endif
	result->pw_dir = "/home/nobody";
	result->pw_shell = "/bin/sh";
#endif

	//
	// All done.
	//
	return result;
}


static struct passwd *
passwd_copy(struct passwd *pw)
{
	struct passwd	*result;

	result = passwd_null();

	//
	// Copy values onto the heap, because the next call to getpwnam
	// will trash the ones in *pw.
	//
	result->pw_name = mem_copy_string(pw->pw_name);
	result->pw_uid = pw->pw_uid;
	result->pw_gid = pw->pw_gid;
	result->pw_gecos = mem_copy_string(pw->pw_gecos);
#ifdef HAVE_pw_comment
	result->pw_comment =
	    pw->pw_comment ? mem_copy_string(pw->pw_comment) : 0;
#endif
	result->pw_dir = mem_copy_string(pw->pw_dir);
	result->pw_shell = mem_copy_string(pw->pw_shell);

	//
	// All done.
	//
	return result;
}


struct passwd *
getpwnam_cached(string_ty *name)
{
	struct passwd	*data;

	//
	// Create the tables if they don't exist already.
	//
	if (!login_table)
	{
		login_table = symtab_alloc(5);
		uid_table = itab_alloc(5);
	}

	//
	// Look for the data in the name table.
	//
	data = (struct passwd *)symtab_query(login_table, name);

	//
	// If the data isn't there, ask the system for it.
	//
	if (!data)
	{
		struct passwd	*pw;

		pw = getpwnam(name->str_text);
		if (pw)
		{
			data = passwd_copy(pw);
			itab_assign(uid_table, data->pw_uid, data);
		}
		else
		{
			data = passwd_null();
#ifdef SOURCE_FORGE_HACK
			data->pw_name = mem_copy_string(name->str_text);
			data->pw_gecos = data->pw_name;
#endif
		}
		symtab_assign(login_table, name, data);
	}

	//
	// Negative search results are also cached.
	// They have NULL pointers for all the fields.
	//
	if (!data->pw_name)
		return 0;

	//
	// Success.
	//
	return data;
}


struct passwd *
getpwuid_cached(int uid)
{
	struct passwd	*data;

	//
	// Create the tables if they don't exist already.
	//
	if (!login_table)
	{
		login_table = symtab_alloc(5);
		uid_table = itab_alloc(5);
	}

	//
	// Look for the data in the name table.
	//
	data = (struct passwd *)itab_query(uid_table, uid);

	//
	// If the data isn't there, ask the system for it.
	//
	if (!data)
	{
		struct passwd	*pw;

		pw = getpwuid(uid);
		if (pw)
		{
			string_ty	*name;

			data = passwd_copy(pw);

			name = str_from_c(pw->pw_name);
			symtab_assign(login_table, name, data);
			str_free(name);
		}
		else
		{
			data = passwd_null();
#ifdef SOURCE_FORGE_HACK
			data->pw_uid = uid;
#endif
		}
		itab_assign(uid_table, uid, data);
	}

	//
	// Negative search results are also cached.
	// They have NULL pointers for all the fields.
	//
	if (!data->pw_name)
		return 0;

	//
	// Success.
	//
	return data;
}
