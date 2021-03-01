/*
 *	aegis - project change supervisor
 *	Copyright (C) 2001-2003 Peter Miller;
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
 * MANIFEST: functions to manipulate getgr_caches
 */

#include <ac/string.h>

#include <getgr_cache.h>
#include <itab.h>
#include <mem.h>
#include <symtab.h>

static symtab_ty *name_table;
static itab_ty *gid_table;


static struct group *
group_null(void)
{
	struct group	*result;

	result = (struct group *)mem_alloc(sizeof(struct group));

	/*
	 * This isn't portable, it assumes that NULL pointers are
	 * all-bits-zero, but this is only to cover all the fields we
	 * don't care about.  We set the ones we -do- care about to
	 * NULL explicitly.
	 */
	memset(result, 0, sizeof(*result));

#ifndef SOURCE_FORGE_HACK
	result->gr_name = 0;
	result->gr_passwd = 0;
	result->gr_gid = (__gid_t)-1;
	result->gr_mem = 0;
#else
	/*
	 * This is used to fake a group, because on SourceForge.net
	 * the Apache servers don't have access to /etc/group.  The fake
	 * information will be further taylored below.
	 */
	result->gr_name = "nogroup";
	result->gr_passwd = "*";
	result->gr_gid = AEGIS_MIN_GID;
	result->gr_mem = 0;
#endif

	/*
	 * All done.
	 */
	return result;
}


static struct group *
group_copy(struct group *gr)
{
	struct group	*result;

	result = group_null();

	/*
	 * Copy values onto the heap, because the next call to getgrnam
	 * will trash the ones in *gr.
	 */
	result->gr_name = mem_copy_string(gr->gr_name);
	result->gr_gid = gr->gr_gid;

	/*
	 * All done.
	 */
	return result;
}


struct group *
getgrnam_cached(string_ty *name)
{
	struct group	*data;

	/*
	 * Create the tables if they don't exist already.
	 */
	if (!name_table)
	{
		name_table = symtab_alloc(5);
		gid_table = itab_alloc(5);
	}

	/*
	 * Look for the data in the name table.
	 */
	data = (struct group *)symtab_query(name_table, name);

	/*
	 * If the data isn't there, ask the system for it.
	 */
	if (!data)
	{
		struct group	*gr;

		gr = getgrnam(name->str_text);
		if (gr)
		{
			data = group_copy(gr);
			itab_assign(gid_table, data->gr_gid, data);
		}
		else
		{
			data = group_null();
#ifdef SOURCE_FORGE_HACK
			data->gr_name = mem_copy_string(name->str_text);
#endif
		}
		symtab_assign(name_table, name, data);
	}

	/*
	 * Negative search results are also cached.
	 * They have NULL pointers for all the fields.
	 */
	if (!data->gr_name)
		return 0;

	/*
	 * Success.
	 */
	return data;
}


struct group *
getgrgid_cached(int gid)
{
	struct group	*data;

	/*
	 * Create the tables if they don't exist already.
	 */
	if (!name_table)
	{
		name_table = symtab_alloc(5);
		gid_table = itab_alloc(5);
	}

	/*
	 * Look for the data in the name table.
	 */
	data = (struct group *)itab_query(gid_table, gid);

	/*
	 * If the data isn't there, ask the system for it.
	 */
	if (!data)
	{
		struct group	*gr;

		gr = getgrgid(gid);
		if (gr)
		{
			string_ty	*name;

			data = group_copy(gr);

			name = str_from_c(gr->gr_name);
			symtab_assign(name_table, name, data);
			str_free(name);
		}
		else
		{
			data = group_null();
#ifdef SOURCE_FORGE_HACK
			data->gr_gid = gid;
#endif
		}
		itab_assign(gid_table, gid, data);
	}

	/*
	 * Negative search results are also cached.
	 * They have NULL pointers for all the fields.
	 */
	if (!data->gr_name)
		return 0;

	/*
	 * Success.
	 */
	return data;
}
