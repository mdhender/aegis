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
 * MANIFEST: functions to manage information about users
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#include <commit.h>
#include <conf.h>
#include <error.h>
#include <gonzo.h>
#include <lock.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <project.h>
#include <trace.h>
#include <user.h>
#include <undo.h>
#include <word.h>


static	size_t	nusers;
static	user_ty	**user;


/*
 * NAME
 *	user_numeric
 *
 * SYNOPSIS
 *	user_ty *user_numeric(project_ty *pp, int uid);
 *
 * DESCRIPTION
 *	The user_numeric function is used to
 *	create a user structure from a uid.
 *
 *	The default group is derived from the project group,
 *	or the password file if a NULL project pointer is supplied.
 *
 * ARGUMENTS
 *	pp	- project user is associated with
 *	uid	- system user id
 *
 * RETURNS
 *	pointer to user structure in dynamic memory
 *
 * CAVEAT
 *	Release the user structure with the user_free() function when done.
 */

user_ty *
user_numeric(pp, uid)
	project_ty	*pp;
	int		uid;
{
	size_t		j;
	user_ty		*up;
	struct passwd	*pw;
	struct group	*gr;

	/*
	 * see if we already know her
	 */
	trace(("user_numeric(pp = %08lX, uid = %d)\n{\n"/*}*/, pp, uid));
	for (j = 0; j < nusers; ++j)
	{
		up = user[j];
		if (up->uid == uid)
		{
			up = user_copy(up);
			goto ret;
		}
	}

	/*
	 * first time we have met her
	 *	group always binds to the project
	 *
	 * Always use the number as the primary reference:
	 * system treats the first entry as cannonical, so do we.
	 */
	pw = getpwuid(uid);
	if (!pw)
		fatal("uid %d unknown", uid);
	up = (user_ty *)mem_alloc_clear(sizeof(user_ty));
	up->reference_count = 1;
	if (pp)
		up->pp = project_copy(pp);
	up->uid = uid;
	up->gid = pw->pw_gid;
	up->umask = DEFAULT_UMASK;
	up->name = str_from_c(pw->pw_name);
	up->home = str_from_c(pw->pw_dir);
	if (pw->pw_gecos && pw->pw_gecos[0])
		up->full_name = str_from_c(pw->pw_gecos);
#ifndef CONF_NO_pw_comment
	else if (pw->pw_comment && pw->pw_comment[0])
		up->full_name = str_from_c(pw->pw_comment);
#endif
	else
		up->full_name = str_from_c(pw->pw_name);
	if (pp)
	{
		gr = getgrnam(project_group(pp)->str_text);
		if (!gr)
		{
			fatal
			(
				"group \"%s\" unknown",
				project_group(pp)->str_text
			);
		}
		up->gid = gr->gr_gid;
	}
	gr = getgrgid(up->gid);
	if (!gr)
		fatal("gid %d unknown", up->gid);
	up->group = str_from_c(gr->gr_name);

	/*
	 * add it to the list
	 */
	*(user_ty **)enlarge(&nusers, (char **)&user, sizeof(user_ty *)) = up;

	/*
	 * Find the umask.
	 *
	 * Only do this AFTER adding it to the list,
	 * because the project may construct the same user
	 * when it reads it's state file to find the umask.
	 */
	if (pp)
		up->umask = project_umask(pp);

	/*
	 * here for all exits
	 */
	ret:
	trace(("return %08lX;\n", up));
	trace((/*{*/"}\n"));
	return up;
}


/*
 * NAME
 *	user_symbolic
 *
 * SYNOPSIS
 *	user_ty *user_symbolic(project_ty *pp, string_ty *name);
 *
 * DESCRIPTION
 *	The user_symbolic function is used to
 *	create a user structure from a login name.
 *
 *	The login name is mapped to a uid.  The password file is searched
 *	from beginning to end, so the cannonical name is the first found
 *	in the password file.  The cannonical name is used even for user
 *	structures created with this function.
 *
 * ARGUMENTS
 *	pp	- project user is associated with
 *	name	- user's login name
 *
 * RETURNS
 *	pointer to user structure in dynamic memory
 *
 * CAVEAT
 *	Release the user structure with the user_free() function when done.
 */

user_ty *
user_symbolic(pp, name)
	project_ty	*pp;
	string_ty	*name;
{
	struct passwd	*pw;
	user_ty		*result;

	trace(("user_symbolic(pp = %08lX, name = \"%s\")\n{\n"/*}*/, pp,
		name->str_text));
	pw = getpwnam(name->str_text);
	if (!pw)
		fatal("user \"%s\" unknown", name->str_text);
	result = user_numeric(pp, pw->pw_uid);
	trace(("return %08lX;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	user_executing
 *
 * SYNOPSIS
 *	user_ty *user_executing(project_ty *pp);
 *
 * DESCRIPTION
 *	The user_executing function is used to
 *	create a user structure based on the user who invoked
 *	the currently executing program.
 *
 * ARGUMENTS
 *	pp	- project user is operating on
 *
 * RETURNS
 *	pointer to user structure in dynamic memory
 *
 * CAVEAT
 *	Release the user structure with the user_free() function when done.
 */

user_ty *
user_executing(pp)
	project_ty	*pp;
{
	user_ty		*result;
	int		uid;
	int		gid;

	trace(("user_executing(pp = %08lX)\n{\n"/*}*/, pp));
	os_become_orig_query(&uid, &gid, (int *)0);
	result = user_numeric(pp, uid);
	trace(("return %08lX;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	user_free
 *
 * SYNOPSIS
 *	void user_free(user_ty *up);
 *
 * DESCRIPTION
 *	The user_free function is used to
 *	release memory when a user structure is done with.
 *
 * ARGUMENTS
 *	up	- pointer to user structure in dynamic memory
 */

void
user_free(up)
	user_ty		*up;
{
	trace(("user_free(up = %08lX)\n{\n"/*}*/, up));
	up->reference_count--;
	if (up->reference_count <= 0)
	{
		size_t	j;

		/*
		 * remove from cache
		 */
		for (j = 0; j < nusers; ++j)
		{
			if (user[j] == up)
			{
				user[j] = user[--nusers];
				break;
			}
		}

		/*
		 * release memory references
		 */
		if (up->pp)
			project_free(up->pp);
		str_free(up->name);
		str_free(up->full_name);
		str_free(up->home);
		str_free(up->group);
		if (up->ustate_path)
			str_free(up->ustate_path);
		if (up->ustate_data)
			ustate_type.free(up->ustate_data);
		if (up->uconf_path)
			str_free(up->uconf_path);
		if (up->uconf_data)
			uconf_type.free(up->uconf_data);
		mem_free((char *)up);
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	user_copy
 *
 * SYNOPSIS
 *	user_ty *user_copy(user_ty *up);
 *
 * DESCRIPTION
 *	The user_copy function is used to
 *	make a logical copy of a user structure.
 *
 * ARGUMENTS
 *	up	- pointer to user structure in dynamic memory.
 *
 * RETURNS
 *	pointer to user structure in dynamic memory
 *
 * CAVEAT
 *	Release the user structure with the user_free() function when done.
 */

user_ty *
user_copy(up)
	user_ty		*up;
{
	trace(("user_copy(up = %08lX)\n{\n"/*}*/, up));
	up->reference_count++;
	trace(("return %08lX;\n", up));
	trace((/*{*/"}\n"));
	return up;
}


/*
 * NAME
 *	user_name
 *
 * SYNOPSIS
 *	string_ty *user_name(user_ty *);
 *
 * DESCRIPTION
 *	The user_name function is used to
 *	return the login name of the user described by the user structure.
 *
 * ARGUMENTS
 *	up	- user structure describing user
 *
 * RETURNS
 *	pointer to string containing user's login name
 */

string_ty *
user_name(up)
	user_ty		*up;
{
	trace(("user_name(up = %08lX)\n{\n"/*}*/, up));
	trace(("return \"%s\";\n", up->name->str_text));
	trace((/*{*/"}\n"));
	return up->name;
}


/*
 * NAME
 *	user_id
 *
 * SYNOPSIS
 *	int user_id(user_ty *up);
 *
 * DESCRIPTION
 *	The user_id function is used to
 *	determine the uid of the user described by the user structure.
 *
 * ARGUMENTS
 *	up	- user structure describing user
 *
 * RETURNS
 *	the system uid of the user
 */

int
user_id(up)
	user_ty		*up;
{
	trace(("user_id(up = %08lX)\n{\n"/*}*/, up));
	trace(("return %d;\n", up->uid));
	trace((/*{*/"}\n"));
	return up->uid;
}


/*
 * NAME
 *	user_group
 *
 * SYNOPSIS
 *	string_ty *user_group(user_ty *);
 *
 * DESCRIPTION
 *	The user_group function is used to
 *	determine the name of the default group of a user.
 *
 * ARGUMENTS
 *	up	- pointer to user structure
 *
 * RETURNS
 *	pointer to string naming default group
 */

string_ty *
user_group(up)
	user_ty		*up;
{
	trace(("user_group(up = %08lX)\n{\n"/*}*/, up));
	trace(("return \"%s\";\n", up->group->str_text));
	trace((/*{*/"}\n"));
	return up->group;
}


/*
 * NAME
 *	user_gid
 *
 * SYNOPSIS
 *	int user_gid(user_ty *up);
 *
 * DESCRIPTION
 *	The user_gid function is used to
 *	determine the system gid of the user.
 *
 * ARGUMENTS
 *	up	- pointer to user structure
 *
 * RETURNS
 *	the default system gid of the user
 */

int
user_gid(up)
	user_ty		*up;
{
	trace(("user_gid(up = %08lX)\n{\n"/*}*/, up));
	trace(("return %d;\n", up->gid));
	trace((/*{*/"}\n"));
	return up->gid;
}


/*
 * NAME
 *	user_umask
 *
 * SYNOPSIS
 *	int user_umask(user_ty *up);
 *
 * DESCRIPTION
 *	The user_umask function is used to
 *	determine the umask (file creation mask) of the user.
 *
 * ARGUMENTS
 *	up	- pointer to user structure
 *
 * RETURNS
 *	the default system umask of the user
 */

int
user_umask(up)
	user_ty		*up;
{
	trace(("user_umask(up = %08lX)\n{\n"/*}*/, up));
	trace(("return %05o;\n", up->umask));
	trace((/*{*/"}\n"));
	return up->umask;
}


/*
 * NAME
 *	lock_sync
 *
 * SYNOPSIS
 *	void lock_sync(user_ty *up);
 *
 * DESCRIPTION
 *	The lock_sync function is used to
 *	flush any out-of-date data caching
 *	associated with the user structure.
 *
 * ARGUMENTS
 *	up	- pointer to user structure
 */

static void lock_sync _((user_ty *));

static void
lock_sync(up)
	user_ty		*up;
{
	long		n;

	trace(("lock_sync(up = %08lX)\n{\n"/*}*/, up));
	n = lock_magic();
	if (up->lock_magic != n)
	{
		up->lock_magic = n;
		if (up->ustate_data && !up->ustate_is_new)
		{
			ustate_type.free(up->ustate_data);
			up->ustate_data = 0;
		}
	}
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	user_ustate_get
 *
 * SYNOPSIS
 *	ustate user_ustate_get(user_ty *up);
 *
 * DESCRIPTION
 *	The user_ustate_get function is used to
 *	fetch the ``ustate'' file for this user,
 *	caching for future reference.
 *
 * ARGUMENTS
 *	up	-pointer to user structure
 *
 * RETURNS
 *	pointer to ustate structure in dynamic memory
 */

static ustate user_ustate_get _((user_ty *));

static ustate
user_ustate_get(up)
	user_ty		*up;
{
	trace(("user_ustate_get(up = %08lX)\n{\n"/*}*/, up));
	lock_sync(up);
	if (!up->ustate_path)
		up->ustate_path =
			gonzo_ustate_path(project_name_get(up->pp), up->name);
	if (!up->ustate_data)
	{
		gonzo_become();
		if (os_exists(up->ustate_path))
		{
			up->ustate_data =
				ustate_read_file(up->ustate_path->str_text);
		}
		else
		{
			up->ustate_data = (ustate)ustate_type.alloc();
			up->ustate_is_new = 1;
		}
		gonzo_become_undo();
		if (!up->ustate_data->own)
			up->ustate_data->own =
				(ustate_own_list)
				ustate_own_list_type.alloc();
	}
	trace(("return %08lX;\n", up->ustate_data));
	trace((/*{*/"}\n"));
	return up->ustate_data;
}


/*
 * NAME
 *	user_uconf_get
 *
 * SYNOPSIS
 *	void user_uconf_get(void);
 *
 * DESCRIPTION
 *	The user_uconf_get function is used to
 *	fetch the ``uconf'' file for this user,
 *	caching for future reference.
 *
 * ARGUMENTS
 *	up	-pointer to user structure
 *
 * RETURNS
 *	pointer to uconf structure in dynamic memory
 */

static uconf user_uconf_get _((user_ty *up));

static uconf
user_uconf_get(up)
	user_ty		*up;
{
	trace(("user_uconf_get(up = %08lX)\n{\n"/*}*/, up));
	lock_sync(up);
	if (!up->uconf_path)
		up->uconf_path =
			str_format("%S/.%src", up->home, option_progname_get());
	if (!up->uconf_data)
	{
		user_become(up);
		if (os_exists(up->uconf_path))
		{
			up->uconf_data =
				uconf_read_file(up->uconf_path->str_text);
		}
		else
			up->uconf_data = (uconf)uconf_type.alloc();
		user_become_undo();
	}
	trace(("return %08lX;\n", up->uconf_data));
	trace((/*{*/"}\n"));
	return up->uconf_data;
}


/*
 * NAME
 *	user_home
 *
 * SYNOPSIS
 *	string_ty *user_home(user_ty *up);
 *
 * DESCRIPTION
 *	The user_home function is used to
 *	fetch the home directory of the user.
 *
 * ARGUMENTS
 *	up	- pointer to user structure
 *
 * RETURNS
 *	pointer to string containing absolute path
 *
 * CAVEAT
 *	If the path isn't absolute in /etc/passwd,
 *	it won't be here, either.
 */

string_ty *
user_home(up)
	user_ty		*up;
{
	trace(("user_home(up = %08lX)\n{\n"/*}*/, up));
	assert(up->home);
	trace(("return \"%s\";\n", up->home->str_text));
	trace((/*{*/"}\n"));
	return up->home;
}


/*
 * NAME
 *	user_full_name
 *
 * SYNOPSIS
 *	char *user_full_name(string_ty *login);
 *
 * DESCRIPTION
 *	The user_full_name function is used to
 *	find the full name of a user, given their login name.
 *
 * ARGUMENTS
 *	login	- pointer to string containing login name
 *
 * RETURNS
 *	pointer to string containing user's full name
 *
 * CAVEAT
 *	Will return the empty string if the user can't be found.
 *	NEVER modify the data pointed to by the return value.
 *	The returned data is volatile, it will not remain stable for long.
 */

char *
user_full_name(name)
	string_ty	*name;
{
	struct passwd	*pw;
	char		*result;

	trace(("user_full_name(name = \"%s\")\n{\n"/*}*/, name->str_text));
	pw = getpwnam(name->str_text);
	if (!pw)
		result = "";
	else if (pw->pw_gecos && pw->pw_gecos[0])
		result = pw->pw_gecos;
#ifndef CONF_NO_pw_comment
	else if (pw->pw_comment && pw->pw_comment[0])
		result = pw->pw_comment;
#endif
	else
		result = "";
	trace(("return \"%s\";\n", result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	user_ustate_write
 *
 * SYNOPSIS
 *	void user_ustate_write(user_ty *up);
 *
 * DESCRIPTION
 *	The user_ustate_write function is used to
 *	write any modified ustate file contents back to disk.
 *
 * ARGUMENTS
 *	up	- pointer to user structure
 */

void
user_ustate_write(up)
	user_ty		*up;
{
	string_ty	*filename_new;
	string_ty	*filename_old;
	static int	count;

	trace(("user_ustate_write(up = %08lX)\n{\n"/*}*/, up));
	assert(up->ustate_data);
	assert(up->ustate_path);
	if (!up->ustate_modified)
		goto ret;

	/*
	 * write it out
	 */
	filename_new = str_format("%S,%d", up->ustate_path, ++count);
	filename_old = str_format("%S,%d", up->ustate_path, ++count);
	gonzo_become();
	if (up->ustate_is_new)
	{
		undo_unlink_errok(filename_new);
		ustate_write_file(filename_new->str_text, up->ustate_data);
		commit_rename(filename_new, up->ustate_path);
	}
	else
	{
		undo_unlink_errok(filename_new);
		ustate_write_file(filename_new->str_text, up->ustate_data);
		commit_rename(up->ustate_path, filename_old);
		commit_rename(filename_new, up->ustate_path);
		commit_unlink_errok(filename_old);
	}
	os_chmod(filename_new, 0644);
	gonzo_become_undo();
	str_free(filename_new);
	str_free(filename_old);
	up->ustate_modified = 0;
	up->ustate_is_new = 0;
	ret:
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	user_own_add
 *
 * SYNOPSIS
 *	void user_own_add(user_ty *up, string_ty *project_name,
 *		long change_number);
 *
 * DESCRIPTION
 *	The user_own_add function is used to
 *	add a change to the user's list of owned changes.
 *
 * ARGUMENTS
 *	up		- pointer to user structure
 *	project_name	- project of the change
 *	change_number	- number of the change
 *
 * CAVEAT
 *	The change is assumed to be unique.
 */

void
user_own_add(up, project_name, change_number)
	user_ty		*up;
	string_ty	*project_name;
	long		change_number;
{
	ustate		ustate_data;
	int		j;
	ustate_own	own_data = 0;
	ustate_own	*own_data_p;
	long		*change_p;
	type_ty		*type_p;

	trace(("user_own_add()\n{\n"/*}*/));
	ustate_data = user_ustate_get(up);
	assert(ustate_data->own);

	/*
	 * See if the project is already known.
	 */
	for (j = 0; j < ustate_data->own->length; ++j)
	{
		own_data = ustate_data->own->list[j];
		if (str_equal(own_data->project_name, project_name))
			break;
	}

	/*
	 * If the project isn't known, append it to the list.
	 */
	if (j >= ustate_data->own->length)
	{
		ustate_own_list_type.list_parse
		(
			ustate_data->own,
			&type_p,
			(void **)&own_data_p
		);
		own_data = (ustate_own)ustate_own_type.alloc();
		*own_data_p = own_data;
		own_data->project_name = str_copy(project_name);
	}

	/*
	 * Create a changes for the project, if necessary.
	 */
	if (!own_data->changes)
		own_data->changes =
			(ustate_own_changes_list)
			ustate_own_changes_list_type.alloc();

	/*
	 * Add another item to the changes list for the project.
	 */
	ustate_own_changes_list_type.list_parse
	(
		own_data->changes,
		&type_p,
		(void **)&change_p
	);
	*change_p = change_number;
	up->ustate_modified = 1;
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	user_own_nth
 *
 * SYNOPSIS
 *	int user_own_nth(user_ty *up, long n, long *change_number);
 *
 * DESCRIPTION
 *	The user_own_nth function is used to fetch the n'th
 *	change owned by a user.
 *	The project name is derived from the user structure.
 *
 * ARGUMENTS
 *	up		- pointer to user structure
 *	n		- selector
 *	change_number	- pointer to where to put number of the change
 *
 * RETURNS
 *	1 on sucess, 0 if no such n.
 */

int
user_own_nth(up, n, change_number)
	user_ty		*up;
	long		n;
	long		*change_number;
{
	ustate		ustate_data;
	int		j;
	int		result;

	trace(("user_own_nth(up = %08lX, n = %ld)\n{\n"/*}*/, (long)up, n));
	result = 0;
	assert(n >= 0);
	assert(up->pp);
	if (n < 0 || !up->pp)
		goto done;
	ustate_data = user_ustate_get(up);
	assert(ustate_data->own);

	/*
	 * find the relevant project
	 *	and extract the n'th change
	 */
	for (j = 0; j < ustate_data->own->length; ++j)
	{
		ustate_own	own_data;

		own_data = ustate_data->own->list[j];
		if (str_equal(project_name_get(up->pp), own_data->project_name))
		{
			if (n < own_data->changes->length)
			{
				*change_number = own_data->changes->list[n];
				result = 1;
			}
			break;
		}
	}

	/*
	 * here for all exits
	 */
	done:
	trace(("return %d;\n", result));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	user_own_remove
 *
 * SYNOPSIS
 *	void user_own_remove(user_ty *up, string_ty *project_name,
 *		long change_number);
 *
 * DESCRIPTION
 *	The user_own_remove function is used to
 *	remove a change from the user's owned change list.
 *
 * ARGUMENTS
 *	up		- pointer to user structure
 *	project_name	- project of the change
 *	change_number	- number of the change
 *
 * CAVEAT
 *	The change is assumed to be unique.
 */

void
user_own_remove(up, project_name, change_number)
	user_ty		*up;
	string_ty	*project_name;
	long		change_number;
{
	ustate		ustate_data;
	int		j, k;
	ustate_own	own_data;

	trace(("usate_own_remove()\n{\n"/*}*/));
	ustate_data = user_ustate_get(up);
	assert(ustate_data->own);

	/*
	 * Search for the project in the ``own'' list.
	 */
	for (j = 0; ; ++j)
	{
		if (j >= ustate_data->own->length)
			goto ret;
		own_data = ustate_data->own->list[j];
		if (str_equal(own_data->project_name, project_name))
			break;
	}

	/*
	 * Create the ``changes'' list for the project, if necessary.
	 */
	if (!own_data->changes)
		own_data->changes =
			(ustate_own_changes_list)
			ustate_own_changes_list_type.alloc();

	/*
	 * Search for the change in the ``changes'' list.
	 */
	for (k = 0; k < own_data->changes->length; ++k)
	{
		if (own_data->changes->list[k] == change_number)
			break;
	}

	/*
	 * If the change was there, remove it from the list.
	 */
	if (k < own_data->changes->length)
	{
		own_data->changes->list[k] =
			own_data->changes->list[own_data->changes->length - 1];
		own_data->changes->length--;
		up->ustate_modified = 1;
	}

	/*
	 * If the changes list for the project is now empty,
	 * remove the project from the ``own'' list.
	 */
	if (!own_data->changes->length)
	{
		ustate_own_type.free(own_data);
		ustate_data->own->list[j] =
			ustate_data->own->list[ustate_data->own->length - 1];
		ustate_data->own->length--;
		up->ustate_modified = 1;
	}

	/*
	 * here for all exits
	 */
	ret:
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	user_ustate_lock_prepare
 *
 * SYNOPSIS
 *	void user_ustate_lock_prepare(user_ty *up);
 *
 * DESCRIPTION
 *	The user_ustate_lock_prepare function is used to
 *	notify the lock manager that a ustate lock will be required.
 *
 * ARGUMENTS
 *	up	- pointer to user structure
 */

void
user_ustate_lock_prepare(up)
	user_ty		*up;
{
	trace(("user_ustate_lock_prepare(up = %08lX)\n{\n"/*}*/, up));
	lock_prepare_ustate(up->uid);
	trace((/*{*/"}\n"));
}


/*
 * NAME
 *	user_default_change
 *
 * SYNOPSIS
 *	long user_default_change(user_ty *up);
 *
 * DESCRIPTION
 *	The user_default_change function is used to
 *	find the default change number.  The project is taken from the
 *	user structure's notion of the set project.
 *
 * ARGUMENTS
 *	up	- pointer to user structure
 *
 * RETURNS
 *	the change number.
 *
 * CAVEAT
 *	it is a fatal error if there is no default change number
 */

static long is_a_change_number _((char *));

static long
is_a_change_number(s)
	char		*s;
{
	long		n;

	while (isspace(*s))
		++s;
	n = 0;
	while (isdigit(*s))
		n = n * 10 + *s++ - '0';
	if (n < 1)
		return 0;
	while (isspace(*s))
		++s;
	if (*s)
		return 0;
	return n;
}


static long project_dot_change _((string_ty *, string_ty *));

static long
project_dot_change(s, p)
	string_ty	*s;
	string_ty	*p;
{
	if
	(
		s->str_length > p->str_length + 1
	&&
		!memcmp(s->str_text, p->str_text, p->str_length)
	&&
		s->str_text[p->str_length] == '.'
	)
		return is_a_change_number(s->str_text + p->str_length + 1);
	return 0;
}


long
user_default_change(up)
	user_ty		*up;
{
	long		change_number;
	string_ty	*s1;
	string_ty	*s2;
	char		*cp;

	trace(("user_default_change(up = %08lX)\n{\n"/*}*/, up));
	change_number = 0;

	/*
	 * check the AEGIS_CHANGE environment variable
	 */
	s1 = str_format("%s_change", option_progname_get());
	s2 = str_upcase(s1);
	str_free(s1);
	cp = getenv(s2->str_text);
	if (cp)
	{
		change_number = is_a_change_number(cp);
		if (!change_number)
		{
			fatal
			(
		"the %s environment variable must be a positive decimal number",
				s2->str_text
			);
		}
	}
	str_free(s2);

	/*
	 * check the $HOME/.aegisrc file
	 */
	if (!change_number)
	{
		uconf		uconf_data;

		uconf_data = user_uconf_get(up);
		if (uconf_data->mask & uconf_default_change_number_mask)
			change_number = uconf_data->default_change_number;
	}

	/*
	 * If the user is only working on one change within the given
	 * project, then that is the change.
	 */
	if (!change_number)
	{
		ustate		ustate_data;
		int		j;

		ustate_data = user_ustate_get(up);
		assert(ustate_data->own);
		for (j = 0; j < ustate_data->own->length; ++j)
		{
			ustate_own	own_data;
			string_ty	*project_name;
	
			own_data = ustate_data->own->list[j];
			project_name = project_name_get(up->pp);
			if (str_equal(own_data->project_name, project_name))
			{
				if (own_data->changes->length == 1)
					change_number = own_data->changes->list[0];
				break;
			}
		}
	}

	/*
	 * examine the pathname of the current directory
	 * to see if we can extract the change number
	 *
	 * This only works if the development directory was created
	 * by aegis, and not specified by the -DIRectory option.
	 * It doesn't work at all for IntDir or BL.
	 */
	if (!change_number)
	{
		string_ty	*cwd;
		wlist		part;
		long		j;

		/*
		 * get the current directory
		 */
		os_become_orig();
		cwd = os_curdir();
		os_become_undo();
		assert(cwd);

		/*
		 * break it into file names
		 */
		str2wl(&part, cwd, "/");
		str_free(cwd);

		/*
		 * search for <proj>.<num>
		 */
		for (j = 0; j < part.wl_nwords; ++j)
		{
			change_number =
				project_dot_change
				(
					part.wl_word[j],
					project_name_get(up->pp)
				);
			if (change_number)
				break;
		}
		wl_free(&part);
	}

	/*
	 * It is an error if no change number has been given.
	 */
	if (!change_number)
		project_fatal(up->pp, "no -Change option specified");
	trace(("return %d;\n", change_number));
	trace((/*{*/"}\n"));
	return change_number;
}


/*
 * NAME
 *	user_default_project
 *
 * SYNOPSIS
 *	void user_default_project(void);
 *
 * DESCRIPTION
 *	The user_default_project function is used to
 *	determine the default project of the user who invoked the program.
 *
 * RETURNS
 *	pointer to string containing project name
 *
 * CAVEAT
 *	it is a fatal error if there is no default project name
 */

string_ty *
user_default_project()
{
	string_ty	*result;
	user_ty		*up;
	string_ty	*s1;
	string_ty	*s2;
	char		*cp;

	/*
	 * build a temporary user
	 */
	trace(("user_default_project()\n{\n"/*}*/));
	up = user_executing((project_ty *)0);
	result = 0;

	/*
	 * from the AEGIS_PROJECT environment variable.
	 */
	s1 = str_format("%s_project", option_progname_get());
	s2 = str_upcase(s1);
	str_free(s1);
	cp = getenv(s2->str_text);
	str_free(s2);
	if (cp && *cp)
		result = str_from_c(cp);

	/*
	 * From the $HOME/.aegisrc file.
	 */
	if (!result)
	{
		uconf		uconf_data;

		uconf_data = user_uconf_get(up);
		if (uconf_data->default_project_name)
			result = str_copy(uconf_data->default_project_name);
	}

	/*
	 * check the search path, see if we use just one
	 *
	 * else check the current dirctory to see if we are within one
	 *
	 * This only works if the development directory was created
	 * by aegis, and not specified by the -DIRectory option.
	 * It doesn't work at all for IntDir or BL.
	 */
	if (!result)
	{
		wlist		name;

		gonzo_project_list_user(up->name, &name);
		if (name.wl_nwords == 1)
			result = str_copy(name.wl_word[0]);
		else
		{
			string_ty	*cwd;
			wlist		part;
			long		j;
			long		k;
		    
			/*
			 * get pathname of the current directory
			 */
			os_become_orig();
			cwd = os_curdir();
			os_become_undo();
			assert(cwd);

			/*
			 * break into pieces
			 */
			str2wl(&part, cwd, "/");
			str_free(cwd);

			/*
			 * search the path
			 * looking for <proj>.<num>
			 */
			for (j = 0; j < part.wl_nwords && !result; ++j)
			{
				for (k = 0; k < name.wl_nwords; ++k)
				{
					if
					(
						project_dot_change
						(
							part.wl_word[j],
							name.wl_word[k]
						)
					)
					{
						result =
						      str_copy(name.wl_word[k]);
						break;
					}
				}
			}
			wl_free(&part);
		}
		wl_free(&name);
	}

	/*
	 * It is an error if no project name has been given.
	 */
	if (!result)
		fatal("no -Project option specified");

	/*
	 * clean up and go home
	 */
	user_free(up);
	trace(("return \"%s\";\n", result->str_text));
	trace((/*{*/"}\n"));
	return result;
}


/*
 * NAME
 *	user_default_development_directory_get
 *
 * SYNOPSIS
 *	string_ty *user_default_development_directory_get(user_ty *up);
 *
 * DESCRIPTION
 *	The user_default_development_directory_get function is used to
 *	determine the absolute path of the user's default development directory.
 *
 * ARGUMENTS
 *	up	- pointer to user structure
 *
 * RETURNS
 *	pointer to string containing path
 *
 * CAVEAT
 *	If the user has not explicitly set one,
 *	and the project does not have one set,
 *	the user's home directory will be returned.
 */

string_ty *
user_default_development_directory(up)
	user_ty		*up;
{
	uconf		uconf_data;
	string_ty	*path;

	/*
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	uconf_data = user_uconf_get(up);
	path = uconf_data->default_development_directory;
	if (path)
	{
		if (path->str_text[0] == '/')
			path = str_copy(path);
		else
			path = str_format("%S/%S", user_home(up), path);
	}
	else
	{
		path = project_default_development_directory(up->pp);
		if (!path)
			path = user_home(up);
		path = str_copy(path);
	}
	return path;
}


/*
 * NAME
 *	user_default_project_directory
 *
 * SYNOPSIS
 *	void user_default_project_directory(void);
 *
 * DESCRIPTION
 *	The user_default_project_directory function is used to
 *	determine the absolute path for where to place new projects.
 *
 * ARGUMENTS
 *	up	- pointer to user structure
 *
 * RETURNS
 *	pointer to string containing path
 *
 * CAVEAT
 *	if the user has not explicitly set one,
 *	the user's home directory will be returned.
 */

string_ty *
user_default_project_directory(up)
	user_ty		*up;
{
	uconf		uconf_data;
	string_ty	*path;

	/*
	 * To cope with automounters, directories are stored as given,
	 * or are derived from the home directory in the passwd file.
	 * Within aegis, pathnames have their symbolic links resolved,
	 * and any comparison of paths is done on this "system idea"
	 * of the pathname.
	 */
	trace(("user_default_project_directory(up = %08lX)\n{\n"/*}*/, up));
	uconf_data = user_uconf_get(up);
	path = uconf_data->default_project_directory;
	if (path)
	{
		if (path->str_text[0] == '/')
			path = str_copy(path);
		else
			path = str_format("%S/%S", user_home(up), path);
	}
	else
		path = str_copy(user_home(up));
	trace(("return \"%s\";\n", path->str_text));
	trace((/*{*/"}\n"));
	return path;
}


/*
 * NAME
 *	user_uid_check
 *
 * SYNOPSIS
 *	int user_uid_check(char *name);
 *
 * DESCRIPTION
 *	The user_uid_check function is used to
 *	see if the named user exists, and if they do,
 *	wether they have a non-system uid (i.e. 100 or over).
 *
 * ARGUMENTS
 *	name	- string containing name to check
 *
 * RETURNS
 *	zero if the login name is a system login,
 *	non-zero if the login name is a mortal.
 *
 * CAVEAT
 *	it is a fatal error if the user does not exist
 */

int
user_uid_check(name)
	string_ty	*name;
{
	struct passwd	*pw;

	pw = getpwnam(name->str_text);
	if (!pw)
		fatal("user \"%s\" unknown", name->str_text);
	return (pw->pw_uid >= AEGIS_MIN_UID);
}


/*
 * NAME
 *	user_gid_check
 *
 * SYNOPSIS
 *	int user_gid_check(char *name);
 *
 * DESCRIPTION
 *	The user_gid_check function is used to
 *	see if the named group exists, and if it does,
 *	wether it is a non-system gid (i.e. 10 or over).
 *
 * ARGUMENTS
 *	name	- string containing name to check
 *
 * RETURNS
 *	zero if the name is a system group,
 *	non-zero if the group is a mortal.
 *
 * CAVEAT
 *	it is a fatal error if the group does not exist
 */

int
user_gid_check(name)
	string_ty	*name;
{
	struct group	*gr;

	gr = getgrnam(name->str_text);
	if (!gr)
		fatal("group \"%s\" unknown", name->str_text);
	return (gr->gr_gid >= AEGIS_MIN_GID);
}


/*
 * NAME
 *	user_become
 *
 * SYNOPSIS
 *	void user_become(user_ty *up);
 *
 * DESCRIPTION
 *	The user_become function is used to
 *	set user and group to the given user.
 *
 * ARGUMENTS
 *	up	- pointer to user structure
 *
 * CAVEAT
 *	cancel with user_become_undo() when finished imitating this user.
 */

void
user_become(up)
	user_ty		*up;
{
	trace(("user_become(up = %08lX)\n{\n"/*}*/, up));
	os_become(up->uid, up->gid, up->umask);
	trace((/*{*/"}\n"));
}

void
user_become_undo()
{
	trace(("user_become_undo()\n{\n"/*}*/));
	os_become_undo();
	trace((/*{*/"}\n"));
}
