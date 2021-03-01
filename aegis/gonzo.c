/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
 * MANIFEST: functions for manipulating global state data
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <commit.h>
#include <conf.h>
#include <error.h>
#include <gonzo.h>
#include <gstate.h>
#include <lock.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <undo.h>
#include <user.h>
#include <ustate.h>
#include <word.h>

typedef struct gonzo_ty gonzo_ty;
struct gonzo_ty
{
	string_ty	*dir;
	string_ty	*gstate_filename;
	gstate		gstate_data;
	int		is_a_new_file;
	int		modified;
	long		lock_magic;
	int		temporary;
};

static	size_t		ngonzos;
static	gonzo_ty	**gonzo;
static	int		done_tail;


static user_ty *gonzo_user _((void));

static user_ty *
gonzo_user()
{
	static user_ty	*u;

	if (!u)
	{
		string_ty	*s;

		s = str_from_c(AEGIS_USER);
		u = user_symbolic((project_ty *)0, s);
		u->umask = 022;
		str_free(s);
	}
	return u;
}


static int is_temporary _((string_ty *));

static int
is_temporary(s)
	string_ty	*s;
{
	return
	(
		!strncmp(s->str_text, "/tmp/", 5)
	||
		!strncmp(s->str_text, "/usr/tmp/", 9)
	||
		!strncmp(s->str_text, "/var/tmp/", 9)
	);
}


void
gonzo_library_append(s)
	char		*s;
{
	gonzo_ty	*gp;
	string_ty	*tmp;
	string_ty	*dir;

	/*
	 * resolve the pathname
	 */
	trace(("gonzo_library_append(s = \"%s\")\n{\n"/*}*/, s));
	assert(s);
	assert(!done_tail);
	tmp = str_from_c(s);
	os_become_orig();
	dir = os_pathname(tmp, 1);
	os_become_undo();
	str_free(tmp);

	/*
	 * append the new entry to the end of the list
	 */
	gp = (gonzo_ty *)mem_alloc_clear(sizeof(gonzo_ty));
	gp->dir = dir;
	gp->temporary = is_temporary(dir);
	gp->gstate_filename = str_format("%S/state", gp->dir);
	*(gonzo_ty **)
	enlarge
	(
		&ngonzos,
		(char **)&gonzo, sizeof(gonzo_ty *)
	) =
		gp;
	trace((/*{*/"}\n"));
}


static void lock_sync _((gonzo_ty *));

static void
lock_sync(gp)
	gonzo_ty	*gp;
{
	long		n;

	trace(("lock_sync(gp = %08lX)\n{\n"/*}*/, gp));
	n = lock_magic();
	if (gp->lock_magic == n)
		goto ret;
	gp->lock_magic = n;

	if (gp->gstate_data && !gp->is_a_new_file)
	{
		gstate_type.free(gp->gstate_data);
		gp->gstate_data = 0;
	}
	ret:
	trace((/*{*/"}\n"));
}



static gstate gonzo_gstate_get _((gonzo_ty *));

static gstate
gonzo_gstate_get(gp)
	gonzo_ty	*gp;
{
	trace(("gonzo_gstate_get(gp = %08lX)\n{\n"/*}*/, gp));
	lock_sync(gp);
	if (!gp->gstate_data)
	{
		gonzo_become();
		gp->is_a_new_file = !os_exists(gp->gstate_filename);
		if (!gp->is_a_new_file)
		{
			os_chown_check
			(
				gp->gstate_filename,
				0644,
				user_id(gonzo_user()),
				user_gid(gonzo_user())
			);
			gp->gstate_data =
				gstate_read_file(gp->gstate_filename->str_text);
		}
		else
			gp->gstate_data = (gstate)gstate_type.alloc();
		if (!gp->gstate_data->where)
			gp->gstate_data->where =
				(gstate_where_list)
				gstate_where_list_type.alloc();
		gonzo_become_undo();
	}
	trace(("return %08lX;\n", gp->gstate_data));
	trace((/*{*/"}\n"));
	return gp->gstate_data;
}


/*
 * NAME
 *	construct_library_directory
 *
 * SYNOPSIS
 *	void construct_library_directory(gonzo_ty *gp);
 *
 * DESCRIPTION
 *	The construct_library_directory function is used to
 *	construct missing elements of the aegis library search path.
 *	These elements must be constructed with great care so that
 *	they may work on NFS mounted disks of "data-less" clients.
 *
 *	The path up to, but not including, the library directory must
 *	be owned by the executing user.  The actions will fail if the
 *	executing user does not have sufficient permissions, as
 *	one would expect.
 *
 *	The library directory itself must be owned by AEGIS_USER.
 *	This is to defend against tampering and ignorance.
 *	Having this change of ownership requires some fast footwork
 *	with chmod's to allow AEGIS_USER temporary access.
 *
 * ARGUMENTS
 *	gp	- library directory to act upon
 *
 * CAVEAT
 *	The chown() system call will not work on data-less clients,
 *	because the remote hosts which owns the NFS mounted disks
 *	will rarely trust the local "root" user, and map all
 *	foreign "root" users to the "unknown" user instead.
 */

static void construct_library_directory _((gonzo_ty *));

static void
construct_library_directory(gp)
	gonzo_ty	*gp;
{
	int		exists;
	string_ty	*above;
	string_ty	*above2;
	string_ty	*root;
	int		mode;
	
	/*
	 * If the library directory already exists,
	 * then we need do nothing.
	 */
	os_become_orig();
	exists = os_exists(gp->dir);
	if (exists)
	{
		os_become_undo();
		return;
	}

	/*
	 * Construct all directories up to,
	 * but not including, the library directory,
	 * if they do not already exist.
	 * Construct these directories as the original user,
	 * possibly discovering relevant permission problems.
	 */
	above = os_dirname(gp->dir);
	above2 = str_n_from_c(above->str_text + 1, above->str_length - 1);
	root = str_from_c("/");
	os_mkdir_between(root, above2, 0755); /* NOT setgid */
	str_free(root);
	str_free(above2);

	/*
	 * Get the mode of the directory containing the library directory,
	 * so that we may restore it later.  Change the mode to world
	 * writable, so that the library directory itself may be created
	 * belonging to gonzo.
	 */
	mode = os_chmod_query(above);
	mode |= 0111; /* minimum: all may search */
	undo_chmod_errok(above, mode);
	os_chmod(above, mode | 0777);
	os_become_undo();

	/*
	 * Create the library directory itself belonging to gonzo,
	 * and make sure it is world accessable.
	 *	(must be world writable in testing situations)
	 */
	gonzo_become();
	os_mkdir(gp->dir, 0755);
	if (gp->temporary)
		os_chmod(gp->dir, 0777);
	gonzo_become_undo();

	/*
	 * Restore permissions for the containing directory.
	 */
	os_become_orig();
	os_chmod(above, mode);
	os_become_undo();
	str_free(above);
}


static void gonzo_gstate_write_sub _((gonzo_ty *));

static void
gonzo_gstate_write_sub(gp)
	gonzo_ty	*gp;
{
	string_ty	*filename_new;
	string_ty	*filename_old;
	static int	count;

	if (!gp->modified)
		return;
	trace(("gonzo_gstate_write_sub()\n{\n"/*}*/));
	assert(gp->gstate_data);
	assert(gp->gstate_filename);
	filename_new = str_format("%S,%d", gp->gstate_filename, ++count);
	filename_old = str_format("%S,%d", gp->gstate_filename, ++count);
	if (gp->is_a_new_file)
	{
		construct_library_directory(gp);
		gonzo_become();
		undo_unlink_errok(filename_new);
		gstate_write_file(filename_new->str_text, gp->gstate_data);
		commit_rename(filename_new, gp->gstate_filename);
		os_chmod(filename_new, 0644);
		gonzo_become_undo();
	}
	else
	{
		gonzo_become();
		undo_unlink_errok(filename_new);
		gstate_write_file(filename_new->str_text, gp->gstate_data);
		commit_rename(gp->gstate_filename, filename_old);
		commit_rename(filename_new, gp->gstate_filename);
		commit_unlink_errok(filename_old);
		os_chmod(filename_new, 0644);
		gonzo_become_undo();
	}
	str_free(filename_new);
	str_free(filename_old);
	gp->modified = 0;
	trace((/*{*/"}\n"));
}


static void do_tail _((void));

static void
do_tail()
{
	string_ty	*s1;
	string_ty	*s2;
	char		*cp;
	wlist		path;
	size_t		j;

	/*
	 * only do this once
	 */
	if (done_tail)
		return;

	/*
	 * fetch the environment variable
	 */
	trace(("do_tail()\n{\n"/*}*/));
	s1 = str_from_c(option_get_progname());
	s2 = str_upcase(s1);
	str_free(s1);
	cp = getenv(s2->str_text);
	str_free(s2);
	if (cp)
	{
		s1 = str_from_c(cp);
		str2wl(&path, s1, ":");
		str_free(s1);
		for (j = 0; j < path.wl_nwords; ++j)
		{
			s1 = path.wl_word[j];
			if (!os_testing_mode() || is_temporary(s1))
				gonzo_library_append(s1->str_text);
		}
		wl_free(&path);
	}

	if (os_testing_mode())
	{
		if (!ngonzos)
		{
			fatal
			(
"There was no -LIBrary option specified.  This is mandatory in \"test\" mode."
			);
		}
	}
	else
	{
		/*
		 * always have the system one last
		 *	(this is where locks are taken)
		 */
		gonzo_library_append(LIB);
	}
	done_tail = 1;
	trace((/*{*/"}\n"));
}


static gonzo_ty *gonzo_nth _((size_t));

static gonzo_ty *
gonzo_nth(j)
	size_t		j;
{
	gonzo_ty	*result;

	trace(("gonzo_nth(j = %ld)\n{\n"/*}*/, j));
	do_tail();
	if (j >= ngonzos)
		result = 0;
	else
		result = gonzo[j];
	trace(("return %08lX;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


void
gonzo_gstate_write()
{
	size_t		j;
	gonzo_ty	*gp;

	trace(("gonzo_gstate_write()\n{\n"/*}*/));
	for (j = 0; ; ++j)
	{
		gp = gonzo_nth(j);
		if (!gp)
			break;
		gonzo_gstate_write_sub(gp);
	}
	trace((/*{*/"}\n"));
}


static string_ty *gonzo_project_home_path_sub _((gonzo_ty *, string_ty *));

static string_ty *
gonzo_project_home_path_sub(gp, name)
	gonzo_ty	*gp;
	string_ty	*name;
{
	gstate		gstate_data;
	size_t		j;
	string_ty	*result;

	/*
	 * find the project in the gstate
	 */
	trace(("gonzo_project_home_path_sub(gp = %08lX, name = \"%s\")\n{\n"/*}*/, gp, name->str_text));
	gstate_data = gonzo_gstate_get(gp);
	assert(gstate_data->where);
	result = 0;
	for (j = 0; j < gstate_data->where->length; ++j)
	{
		gstate_where	addr;

		addr = gstate_data->where->list[j];
		if (str_equal(addr->project_name, name))
		{
			result = addr->directory;
			break;
		}
	}
	trace(("return \"%s\";\n", (result ? result->str_text : "")));
	trace((/*{*/"}\n"));
	return result;
}


string_ty *
gonzo_project_home_path_from_name(name)
	string_ty	*name;
{
	gonzo_ty	*gp;
	size_t		j;
	string_ty	*result;
	string_ty	*s;

	/*
	 * find the project in the gstate list
	 */
	trace(("gonzo_project_home_path_from_name(name = \"%s\")\n{\n"/*}*/, name->str_text));
	result = 0;
	for (j = 0; ; ++j)
	{
		gp = gonzo_nth(j);
		if (!gp)
			break;
		s = gonzo_project_home_path_sub(gp, name);
		if (s)
		{
			result = s;
			break;
		}
	}
	trace(("return \"%s\";\n", (result ? result->str_text : "")));
	trace((/*{*/"}\n"));
	return result;
}


void
gonzo_project_list(result)
	wlist		*result;
{
	size_t		n;
	size_t		j;
	gonzo_ty	*gp;
	gstate		gstate_data;

	trace(("gonzo_project_list(result = %08lX)\n{\n"/*}*/, result));
	wl_zero(result);
	for (n = 0; ; ++n)
	{
		gp = gonzo_nth(n);
		if (!gp)
			break;

		/*
		 * read the gstate file
		 */
		gstate_data = gonzo_gstate_get(gp);
	
		/*
		 * list the projects
		 */
		assert(gstate_data->where);
		for (j = 0; j < gstate_data->where->length; ++j)
		{
			wl_append_unique
			(
				result,
				gstate_data->where->list[j]->project_name
			);
		}
	}
	trace((/*{*/"}\n"));
}


void
gonzo_project_list_user(uname, result)
	string_ty	*uname;
	wlist		*result;
{
	size_t		n;
	size_t		j;

	trace(("gonzo_project_list_user(uname = \"%s\", \
result = %08lX)\n{\n"/*}*/, uname->str_text, result));
	wl_zero(result);
	for (n = 0; ; ++n)
	{
		string_ty	*ustate_path;
		ustate		ustate_data;
		gonzo_ty	*gp;

		gp = gonzo_nth(n);
		if (!gp)
			break;

		/*
		 * check out the ustate
		 */
		ustate_path = str_format("%S/user/%S", gp->dir, uname);
		trace(("ustate_path = \"%s\";\n", ustate_path->str_text));
		gonzo_become();
		if (!os_exists(ustate_path))
		{
			gonzo_become_undo();
			str_free(ustate_path);
			continue;
		}
		ustate_data = ustate_read_file(ustate_path->str_text);
		gonzo_become_undo();
		if (!ustate_data->own)
			ustate_data->own =
				(ustate_own_list)
				ustate_own_list_type.alloc();
		str_free(ustate_path);

		/*
		 * collect all projects this user owns changes in
		 */
		for (j = 0; j < ustate_data->own->length; ++j)
		{
			trace(("remember \"%s\";\n",
			    ustate_data->own->list[j]->project_name->str_text));
			wl_append_unique
			(
				result,
				ustate_data->own->list[j]->project_name
			);
		}
		ustate_type.free(ustate_data);
	}
	trace(("found %d items\n", result->wl_nwords));
	trace((/*{*/"}\n"));
}


void
gonzo_project_add(pp)
	project_ty	*pp;
{
	gstate_where	*addr_p;
	gstate_where	addr;
	gonzo_ty	*gp;
	type_ty		*type_p;

	trace(("gonzo_project_add(pp = %08lX)\n{\n"/*}*/, pp));
	gp = gonzo_nth(0);
	gstate_where_list_type.list_parse
	(
		gp->gstate_data->where,
		&type_p,
		(void **)&addr_p
	);
	addr = (gstate_where)gstate_where_type.alloc();
	*addr_p = addr;
	trace_pointer(addr);
	addr->project_name = str_copy(project_name_get(pp));
	addr->directory = str_copy(project_home_path_get(pp));
	gp->modified = 1;
	trace((/*{*/"}\n"));
}


static int gonzo_project_delete_sub _((gonzo_ty *, project_ty *));

static int 
gonzo_project_delete_sub(gp, pp)
	gonzo_ty	*gp;
	project_ty	*pp;
{
	gstate		gstate_data;
	size_t		j;
	int		result;

	/*
	 * find the project in the gstate
	 */
	trace(("gonzo_project_delete_sub(gp = %08lX, pp = %08lX)\n{\n"/*}*/,
		gp, pp));
	gstate_data = gonzo_gstate_get(gp);
	assert(gstate_data->where);
	result = 0;
	for (j = 0; j < gstate_data->where->length; ++j)
	{
		gstate_where	addr;
		long		k;

		addr = gstate_data->where->list[j];
		if (!str_equal(addr->project_name, project_name_get(pp)))
			continue;

		/*
		 * delete the item from the list
		 */
		for (k = j + 1; k < gstate_data->where->length; ++k)
			gstate_data->where->list[k - 1] =
				gstate_data->where->list[k];
		gstate_data->where->length--;

		/*
		 * free the item
		 */
		gstate_where_type.free(addr);

		/*
		 * mark this gstate file as modified
		 */
		gp->modified = 1;
		result = 1;
		break;
	}
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


void
gonzo_project_delete(pp)
	project_ty	*pp;
{
	gonzo_ty	*gp;
	long		j;

	trace(("gonzo_project_delete(pp = %08lX)\n{\n"/*}*/, pp));
	for (j = 0; ; ++j)
	{
		gp = gonzo_nth(j);
		if (!gp)
			break;
		if (gonzo_project_delete_sub(gp, pp))
			break;
	}
	trace((/*{*/"}\n"));
}


void
gonzo_gstate_lock_prepare_new()
{
	trace(("gonzo_gstate_lock_prepare_new()\n{\n"/*}*/));
	lock_prepare_gstate();
	trace((/*{*/"}\n"));
}


string_ty *
gonzo_lockpath_get()
{
	static string_ty *path;
	gonzo_ty	*gp;

	trace(("gonzo_lockpath_get()\n{\n"/*}*/));
	if (!path)
	{
		do_tail();
		gp = gonzo[ngonzos - 1];
		construct_library_directory(gp);
		path = str_format("%S/lockfile", gp->dir);
	}
	trace(("return \"%s\";\n", path->str_text));
	trace((/*{*/"}\n"));
	return path;
}


static int gonzo_ustate_path_sub _((gonzo_ty *, string_ty *));

static int
gonzo_ustate_path_sub(gp, project_name)
	gonzo_ty	*gp;
	string_ty	*project_name;
{
	gstate		gstate_data;
	size_t		j;
	int		result;

	/*
	 * find the project in the gstate
	 */
	trace(("gonzo_ustate_path_sub(gp = %08lX)\n{\n"/*}*/, gp));
	gstate_data = gonzo_gstate_get(gp);
	assert(gstate_data->where);
	result = 0;
	for (j = 0; j < gstate_data->where->length; ++j)
	{
		gstate_where	addr;

		addr = gstate_data->where->list[j];
		if (str_equal(addr->project_name, project_name))
		{
			result = 1;
			break;
		}
	}
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


string_ty *
gonzo_ustate_path(project_name, user_name)
	string_ty	*project_name;
	string_ty	*user_name;
{
	gonzo_ty	*gp;
	size_t		j;
	string_ty	*dir;
	string_ty	*result;

	/*
	 * find the project in the gstate list
	 *	the user state file contains an index into the project files
	 *	and is thus kept in the same directory
	 */
	trace(("gonzo_ustate_path(project_name = \"%s\", user_name = \"%s\")\n{\n"/*}*/, project_name->str_text, user_name->str_text));
	for (j = 0; ; ++j)
	{
		gp = gonzo_nth(j);
		if (!gp)
		{
			fatal
			(
				"project \"%s\" unknown (bug)",
				project_name->str_text
			);
		}
		if (gonzo_ustate_path_sub(gp, project_name))
			break;
	}

	/*
	 * make sure that the directory for the user state files exists
	 *	(must be world writable in testing situations)
	 */
	gonzo_become();
	assert(os_exists(gp->dir));
	dir = str_format("%S/user", gp->dir);
	if (!os_exists(dir))
	{
		os_mkdir(dir, 0755);
		if (gp->temporary)
			os_chmod(dir, 0777);
	}
	str_free(dir);
	gonzo_become_undo();

	/*
	 * build the user state file name
	 */
	result = str_format("%S/user/%S", gp->dir, user_name);
	trace(("return \"%s\";\n", result->str_text));
	trace((/*{*/"}\n"));
	return result;
}


void
gonzo_become()
{
	trace(("gonzo_become()\n{\n"/*}*/));
	user_become(gonzo_user());
	trace((/*{*/"}\n"));
}


void
gonzo_become_undo()
{
	trace(("gonzo_become_undo()\n{\n"/*}*/));
	user_become_undo();
	trace((/*{*/"}\n"));
}
