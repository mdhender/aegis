//
//	aegis - project change supervisor
//	Copyright (C) 1991-2005 Peter Miller;
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
// MANIFEST: functions to manage information about users
//

#include <ac/ctype.h>
#include <ac/stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/unistd.h>

#include <arglex2.h>
#include <attribute.h>
#include <commit.h>
#include <error.h>
#include <file.h>
#include <fstrcmp.h>
#include <getgr_cache.h>
#include <getpw_cache.h>
#include <gonzo.h>
#include <help.h>
#include <lock.h>
#include <libdir.h>
#include <mem.h>
#include <os.h>
#include <os/domain_name.h>
#include <project.h>
#include <project/history.h>
#include <stracc.h>
#include <sub.h>
#include <trace.h>
#include <user.h>
#include <undo.h>
#include <str_list.h>
#include <zero.h>
#include <change.h>


static size_t	nusers;
static user_ty	**user;


//
// NAME
//	user_set_project - set the project associated with a user
//
// SYNOPSIS
//	void user_set_project(user_ty *, project_ty *);
//
// DESCRIPTION
//	The user_set_project is used to set the project associated
//	with each user.	 Should only be called for uses without a project
//	associated.
//
// CAVEATS
//	Only do this AFTER adding the user structure to the list,
//	because the project may construct the same user
//	when it reads it's state file to find the umask.
//

static void
user_set_project(user_ty *up, project_ty *pp)
{
    struct group    *gr;

    //
    // set the project
    //
    trace(("user_set_project(up = %08lX, pp = %08lX)\n{\n",
	(long)up, (long)pp));
    assert(up);
    assert(pp);
    up->pp = project_copy(pp);

    //
    // set the group from the project
    //	    (cannonical name is first in /etc/group file)
    //
    up->gid = project_gid_get(pp);
    gr = getgrgid_cached(up->gid);
    if (!gr)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_long(scp, "Number", up->gid);
	fatal_intl(scp, i18n("gid $number unknown"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    assert(up->group);
    str_free(up->group);
    up->group = str_from_c(gr->gr_name);

    //
    // set the umask from the project
    //
    up->umask = project_umask_get(pp);
    trace(("}\n"));
}


//
// NAME
//	user_numeric
//
// SYNOPSIS
//	user_ty *user_numeric(project_ty *pp, int uid);
//
// DESCRIPTION
//	The user_numeric function is used to
//	create a user structure from a uid.
//
//	The default group is derived from the project group,
//	or the password file if a NULL project pointer is supplied.
//
// ARGUMENTS
//	pp	- project user is associated with
//	uid	- system user id
//
// RETURNS
//	pointer to user structure in dynamic memory
//
// CAVEAT
//	Release the user structure with the user_free() function when done.
//

user_ty *
user_numeric(project_ty *pp, int uid)
{
    size_t	    j;
    user_ty	    *up;
    struct passwd   *pw;
    struct group    *gr;
    size_t	    nbytes;
    char	    *full_name;
    char	    *comma;

    //
    // see if we already know her
    //
    trace(("user_numeric(pp = %08lX, uid = %d)\n{\n", (long)pp, uid));
    for (j = 0; j < nusers; ++j)
    {
	up = user[j];
	if (up->uid == uid)
	{
	    up = user_copy(up);
	    if (!up->pp && pp)
		user_set_project(up, pp);
	    goto ret;
	}
    }

    //
    // first time we have met her
    //	    group always binds to the project
    //
    // Always use the number as the primary reference:
    // system treats the first entry as cannonical, so do we.
    //
    pw = getpwuid_cached(uid);
    if (!pw)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_long(scp, "Number", uid);
	fatal_intl(scp, i18n("uid $number unknown"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    up = (user_ty *)mem_alloc_clear(sizeof(user_ty));
    up->reference_count = 1;
    up->uid = uid;
    up->gid = pw->pw_gid;
    up->umask = DEFAULT_UMASK;
    up->name = str_from_c(pw->pw_name);
    up->home = str_from_c(pw->pw_dir);
    up->pp = 0;
    if (pw->pw_gecos && pw->pw_gecos[0])
	full_name = pw->pw_gecos;
#ifdef HAVE_pw_comment
    else if (pw->pw_comment && pw->pw_comment[0])
	full_name = pw->pw_comment;
#endif
    else
	full_name = pw->pw_name;

    //
    // Some systems add lots of other stuff to the full name field
    // in the passwd file.  We are only interested in the name.
    //
    comma = strchr(full_name, ',');
    if (comma)
	up->full_name = str_n_from_c(full_name, comma - full_name);
    else
	up->full_name = str_from_c(full_name);

    gr = getgrgid_cached(up->gid);
    if (!gr)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_long(scp, "Number", up->gid);
	fatal_intl(scp, i18n("gid $number unknown"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    up->group = str_from_c(gr->gr_name);

    //
    // add it to the list
    //
    nbytes = (nusers + 1) * sizeof(user_ty *);
    user = (user_ty **)mem_change_size(user, nbytes);
    user[nusers++] = up;

    //
    // Set the user's project, if known.
    //
    // Only do this AFTER adding it to the list,
    // because the project may construct the same user
    // when it reads it's state file to find the umask.
    //
    if (pp)
	user_set_project(up, pp);

    //
    // here for all exits
    //
    ret:
    trace(("return %08lX;\n", (long)up));
    trace(("}\n"));
    return up;
}


//
// NAME
//	user_numeric2
//
// SYNOPSIS
//	user_ty *user_numeric2(int uid, int gid);
//
// DESCRIPTION
//	The user_numeric2 function is used to
//	create a user structure for the aegis/gonzo.c file.
//	It has a known uid and gid, and will succeed, even
//	if there are no relvant entriesin system tables.
//
// ARGUMENTS
//	uid	- system user id
//	gid	- system group id
//
// RETURNS
//	pointer to user structure in dynamic memory
//
// CAVEAT
//	Release the user structure with the user_free() function when done.
//
//	It is essential this function only be called once;
//	unlike user_numeric, it does not check for multiple invokations.
//

user_ty *
user_numeric2(int uid, int gid)
{
    user_ty	    *up;
    struct passwd   *pw;
    struct group    *gr;
    size_t	    nbytes;

    //
    // craete the user structure
    //
    trace(("user_numeric2(uid = %d, gid = %d)\n{\n", uid, gid));
    up = (user_ty *)mem_alloc_clear(sizeof(user_ty));
    up->reference_count = 1;
    up->uid = uid;
    up->gid = gid;
    up->umask = 022;
    up->pp = 0;

    //
    // try the passwd file,
    //	    otherwise default to something sensable
    //
    pw = getpwuid_cached(uid);
    if (pw)
    {
	up->name = str_from_c(pw->pw_name);
	up->home = str_from_c(pw->pw_dir);
    }
    else
    {
	up->name = str_format("uid %d", uid);
	up->home = str_from_c("/");
    }
    up->full_name = str_copy(up->name);

    //
    // try the group file,
    //	    otherwise default to something sensable
    //
    gr = getgrgid_cached(up->gid);
    if (!gr)
	up->group = str_format("gid %d", gid);
    else
	up->group = str_from_c(gr->gr_name);

    //
    // add it to the list
    //
    nbytes = (nusers + 1) * sizeof(user_ty *);
    user = (user_ty **)mem_change_size(user, nbytes);
    user[nusers++] = up;

    //
    // here for all exits
    //
    trace(("return %08lX;\n", (long)up));
    trace(("}\n"));
    return up;
}


//
// NAME
//	user_symbolic
//
// SYNOPSIS
//	user_ty *user_symbolic(project_ty *pp, string_ty *name);
//
// DESCRIPTION
//	The user_symbolic function is used to
//	create a user structure from a login name.
//
//	The login name is mapped to a uid.  The password file is searched
//	from beginning to end, so the cannonical name is the first found
//	in the password file.  The cannonical name is used even for user
//	structures created with this function.
//
// ARGUMENTS
//	pp	- project user is associated with
//	name	- user's login name
//
// RETURNS
//	pointer to user structure in dynamic memory
//
// CAVEAT
//	Release the user structure with the user_free() function when done.
//

user_ty *
user_symbolic(project_ty *pp, string_ty *name)
{
    struct passwd   *pw;
    user_ty	    *result;

    trace(("user_symbolic(pp = %08lX, name = \"%s\")\n{\n", (long)pp,
	name->str_text));
    pw = getpwnam_cached(name);
    if (!pw)
    {
	string_ty	*best;
	double		best_weight;
	double		weight;

	best = 0;
	best_weight = 0.6;
	setpwent();
	for (;;)
	{
	    pw = getpwent();
	    if (!pw)
		break;
	    weight = fstrcmp(name->str_text, pw->pw_name);
	    if (weight > best_weight)
	    {
		if (best)
		    str_free(best);
		best = str_from_c(pw->pw_name);
		best_weight = weight;
	    }
	}
	if (best)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", name);
	    sub_var_set_string(scp, "Guess", best);
	    fatal_intl(scp, i18n("user $name unknown, closest is $guess"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
	else
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", name);
	    fatal_intl(scp, i18n("user $name unknown"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
    }
    result = user_numeric(pp, pw->pw_uid);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


//
// NAME
//	user_executing
//
// SYNOPSIS
//	user_ty *user_executing(project_ty *pp);
//
// DESCRIPTION
//	The user_executing function is used to
//	create a user structure based on the user who invoked
//	the currently executing program.
//
// ARGUMENTS
//	pp	- project user is operating on
//
// RETURNS
//	pointer to user structure in dynamic memory
//
// CAVEAT
//	Release the user structure with the user_free() function when done.
//

user_ty *
user_executing(project_ty *pp)
{
    user_ty	    *result;
    int		    uid;
    int		    gid;

    trace(("user_executing(pp = %08lX)\n{\n", (long)pp));
    os_become_orig_query(&uid, &gid, (int *)0);
    result = user_numeric(pp, uid);
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


//
// NAME
//	user_free
//
// SYNOPSIS
//	void user_free(user_ty *up);
//
// DESCRIPTION
//	The user_free function is used to
//	release memory when a user structure is done with.
//
// ARGUMENTS
//	up	- pointer to user structure in dynamic memory
//

void
user_free(user_ty *up)
{
    trace(("user_free(up = %08lX)\n{\n", (long)up));
    up->reference_count--;
    if (up->reference_count <= 0)
    {
	size_t		j;

	//
	// remove from cache
	//
	for (j = 0; j < nusers; ++j)
	{
	    if (user[j] == up)
	    {
		user[j] = user[--nusers];
		break;
	    }
	}

	//
	// release memory references
	//
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
    trace(("}\n"));
}


//
// NAME
//	user_copy
//
// SYNOPSIS
//	user_ty *user_copy(user_ty *up);
//
// DESCRIPTION
//	The user_copy function is used to
//	make a logical copy of a user structure.
//
// ARGUMENTS
//	up	- pointer to user structure in dynamic memory.
//
// RETURNS
//	pointer to user structure in dynamic memory
//
// CAVEAT
//	Release the user structure with the user_free() function when done.
//

user_ty *
user_copy(user_ty *up)
{
    trace(("user_copy(up = %08lX)\n{\n", (long)up));
    up->reference_count++;
    trace(("return %08lX;\n", (long)up));
    trace(("}\n"));
    return up;
}


//
// NAME
//	user_name
//
// SYNOPSIS
//	string_ty *user_name(user_ty *);
//
// DESCRIPTION
//	The user_name function is used to
//	return the login name of the user described by the user structure.
//
// ARGUMENTS
//	up	- user structure describing user
//
// RETURNS
//	pointer to string containing user's login name
//

string_ty *
user_name(user_ty *up)
{
    trace(("user_name(up = %08lX)\n{\n", (long)up));
    trace(("return \"%s\";\n", up->name->str_text));
    trace(("}\n"));
    return up->name;
}


string_ty *
user_name2(user_ty *up)
{
    trace(("user_name2(up = %08lX)\n{\n", (long)up));
    trace(("return \"%s\";\n", up->name->str_text));
    trace(("}\n"));
    return up->full_name;
}


//
// NAME
//	user_id
//
// SYNOPSIS
//	int user_id(user_ty *up);
//
// DESCRIPTION
//	The user_id function is used to
//	determine the uid of the user described by the user structure.
//
// ARGUMENTS
//	up	- user structure describing user
//
// RETURNS
//	the system uid of the user
//

int
user_id(user_ty *up)
{
    trace(("user_id(up = %08lX)\n{\n", (long)up));
    trace(("return %d;\n", up->uid));
    trace(("}\n"));
    return up->uid;
}


//
// NAME
//	user_group
//
// SYNOPSIS
//	string_ty *user_group(user_ty *);
//
// DESCRIPTION
//	The user_group function is used to
//	determine the name of the default group of a user.
//
// ARGUMENTS
//	up	- pointer to user structure
//
// RETURNS
//	pointer to string naming default group
//

string_ty *
user_group(user_ty *up)
{
    trace(("user_group(up = %08lX)\n{\n", (long)up));
    trace(("return \"%s\";\n", up->group->str_text));
    trace(("}\n"));
    return up->group;
}


//
// NAME
//	user_gid
//
// SYNOPSIS
//	int user_gid(user_ty *up);
//
// DESCRIPTION
//	The user_gid function is used to
//	determine the system gid of the user.
//
// ARGUMENTS
//	up	- pointer to user structure
//
// RETURNS
//	the default system gid of the user
//

int
user_gid(user_ty *up)
{
    trace(("user_gid(up = %08lX)\n{\n", (long)up));
    trace(("return %d;\n", up->gid));
    trace(("}\n"));
    return up->gid;
}


//
// NAME
//	user_umask
//
// SYNOPSIS
//	int user_umask(user_ty *up);
//
// DESCRIPTION
//	The user_umask function is used to
//	determine the umask (file creation mask) of the user.
//
// ARGUMENTS
//	up	- pointer to user structure
//
// RETURNS
//	the default system umask of the user
//

int
user_umask(user_ty *up)
{
    trace(("user_umask(up = %08lX)\n{\n", (long)up));
    trace(("return %05o;\n", up->umask));
    trace(("}\n"));
    return up->umask;
}


//
// NAME
//	lock_sync
//
// SYNOPSIS
//	void lock_sync(user_ty *up);
//
// DESCRIPTION
//	The lock_sync function is used to
//	flush any out-of-date data caching
//	associated with the user structure.
//
// ARGUMENTS
//	up	- pointer to user structure
//

static void
lock_sync(user_ty *up)
{
    long	    n;

    trace(("lock_sync(up = %08lX)\n{\n", (long)up));
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
    trace(("}\n"));
}


//
// NAME
//	user_ustate_get
//
// SYNOPSIS
//	ustate_ty *user_ustate_get(user_ty *up);
//
// DESCRIPTION
//	The user_ustate_get function is used to
//	fetch the ``ustate'' file for this user,
//	caching for future reference.
//
// ARGUMENTS
//	up	-pointer to user structure
//
// RETURNS
//	pointer to ustate structure in dynamic memory
//

static ustate_ty *
user_ustate_get(user_ty *up)
{
    trace(("user_ustate_get(up = %08lX)\n{\n", (long)up));
    lock_sync(up);
    if (!up->ustate_path)
    {
	project_ty	*ppp;

	for (ppp = up->pp; ppp->parent; ppp = ppp->parent)
	    ;
	up->ustate_path = gonzo_ustate_path(project_name_get(ppp), up->name);
    }
    if (!up->ustate_data)
    {
	gonzo_become();
	if (os_exists(up->ustate_path))
	{
	    up->ustate_data = ustate_read_file(up->ustate_path);
	}
	else
	{
	    up->ustate_data = (ustate_ty *)ustate_type.alloc();
	    up->ustate_is_new = 1;
	}
	gonzo_become_undo();
	if (!up->ustate_data->own)
	    up->ustate_data->own =
		(ustate_own_list_ty *)ustate_own_list_type.alloc();
    }
    trace(("return %08lX;\n", (long)up->ustate_data));
    trace(("}\n"));
    return up->ustate_data;
}


static void
fix_default_change(uconf_ty *uconf_data)
{
    if
    (
	(uconf_data->mask & uconf_default_change_number_mask)
    &&
	uconf_data->default_change_number == 0
    )
	uconf_data->default_change_number = MAGIC_ZERO;
}


//
// NAME
//	merge_uconf
//
// SYNOPSIS
//	void merge_uconf(curdata, defdata);
//
// DESCRIPTION
//	The merge_uconf function is used internally by
//	user_uconf_get to merge two sets of user
//	configuration parameters.
//
// ARGUMENTS
//	curdata -data set that is operated on
//	defdata -default data set. used if some fields
//		 in curdata is not filled.
// RETURNS
//	nothing
//
//

static void
merge_uconf(uconf_ty *data, uconf_ty *tmp)
{
    if (!data->default_project_name && tmp->default_project_name)
    {
	data->default_project_name = str_copy(tmp->default_project_name);
    }
    if (!data->default_change_number && tmp->default_change_number)
    {
	data->default_change_number = tmp->default_change_number;
    }
    if
    (
	!data->default_development_directory
    &&
	tmp->default_development_directory
    )
    {
	data->default_development_directory =
	    str_copy(tmp->default_development_directory);
    }
    if (!data->default_project_directory && tmp->default_project_directory)
    {
	data->default_project_directory =
	    str_copy(tmp->default_project_directory);
    }
    if
    (
	!(data->mask & uconf_delete_file_preference_mask)
    &&
	(tmp->mask & uconf_delete_file_preference_mask)
    )
    {
	data->delete_file_preference = tmp->delete_file_preference;
	data->mask |= uconf_delete_file_preference_mask;
    }
    if
    (
	!(data->mask & uconf_pager_preference_mask)
    &&
	(tmp->mask & uconf_pager_preference_mask)
    )
    {
	data->pager_preference = tmp->pager_preference;
	data->mask |= uconf_pager_preference_mask;
    }
    if
    (
	!(data->mask & uconf_persevere_preference_mask)
    &&
	(tmp->mask & uconf_persevere_preference_mask)
    )
    {
	data->persevere_preference = tmp->persevere_preference;
	data->mask |= uconf_persevere_preference_mask;
    }
    if
    (
	!(data->mask & uconf_log_file_preference_mask)
    &&
	(tmp->mask & uconf_log_file_preference_mask)
    )
    {
	data->log_file_preference = tmp->log_file_preference;
	data->mask |= uconf_log_file_preference_mask;
    }
    if
    (
	!(data->mask & uconf_lock_wait_preference_mask)
    &&
	(tmp->mask & uconf_lock_wait_preference_mask)
    )
    {
	data->lock_wait_preference = tmp->lock_wait_preference;
	data->mask |= uconf_lock_wait_preference_mask;
    }
    if (!data->email_address && tmp->email_address)
    {
	data->email_address = str_copy(tmp->email_address);
    }
    if
    (
	!(data->mask & uconf_whiteout_preference_mask)
    &&
	(tmp->mask & uconf_whiteout_preference_mask)
    )
    {
	data->whiteout_preference = tmp->whiteout_preference;
	data->mask |= uconf_whiteout_preference_mask;
    }
    if (!data->editor_command && tmp->editor_command)
    {
	data->editor_command = str_copy(tmp->editor_command);
    }
    if (!data->visual_command && tmp->visual_command)
    {
	data->visual_command = str_copy(tmp->visual_command);
    }
    if (!data->pager_command && tmp->pager_command)
    {
	data->pager_command = str_copy(tmp->pager_command);
    }

    //
    // Merge the attribute lists.
    //
    if (tmp->attribute && tmp->attribute->length)
    {
	if (!data->attribute)
	{
	    data->attribute =
		    (attributes_list_ty *)attributes_list_type.alloc();
	}
	for (size_t j = 0; j < tmp->attribute->length; ++j)
	{
	    attributes_ty *ap = tmp->attribute->list[j];
	    if
	    (
		ap->name
	    &&
		ap->value
	    &&
		!attributes_list_find(data->attribute, ap->name->str_text)
	    )
	    {
		attributes_list_append
	       	(
		    data->attribute,
		    ap->name->str_text,
		    ap->value->str_text
		);
	    }
	}
    }
}


static void
read_and_merge(uconf_ty *data, string_ty *filename)
{
    uconf_ty	    *uconf_new;

    //
    // Read the file.
    //
    if (os_readable(filename) == 0)
	uconf_new = uconf_read_file(filename);
    else
	uconf_new = (uconf_ty *)uconf_type.alloc();

    //
    // Merge the two.
    //
    merge_uconf(data, uconf_new);
    uconf_type.free(uconf_new);
}


//
// NAME
//	user_uconf_get
//
// SYNOPSIS
//	uconf_ty *user_uconf_get(user_ty *up);
//
// DESCRIPTION
//	The user_uconf_get function is used to
//	fetch the ``uconf'' file for this user,
//	caching for future reference.
//
// ARGUMENTS
//	up	-pointer to user structure
//
// RETURNS
//	pointer to uconf structure in dynamic memory
//

uconf_ty *
user_uconf_get(user_ty *up)
{
    trace(("user_uconf_get(up = %08lX)\n{\n", (long)up));
    lock_sync(up);
    if (!up->uconf_path)
	up->uconf_path = str_format("%s/.aegisrc", up->home->str_text);

    //
    // Read in the user preferences.  There are several sources of
    // preferences, in the following priority order:
    //
    // $AEGIS_FLAGS
    // $HOME/.aegisrc
    // $(datadir)/aegisrc
    // $(libdir)/aegisrc
    //
    if (!up->uconf_data)
    {
	static string_ty *libdir_aegisrc;
	static string_ty *datadir_aegisrc;

	//
	// read the environment variable
	//
	up->uconf_data = (uconf_ty *)parse_env("AEGIS_FLAGS", &uconf_type);

	//
	// Read the user's $HOME/.aegisrc file.
	//
	user_become(up);
	read_and_merge(up->uconf_data, up->uconf_path);

	//
	// read system architecture-neutral file
	//
	if (!datadir_aegisrc)
	    datadir_aegisrc = str_format("%s/aegisrc", configured_datadir());
	read_and_merge(up->uconf_data, datadir_aegisrc);

	//
	// read system architecture-specific file
	//
	if (!libdir_aegisrc)
	    libdir_aegisrc = str_format("%s/aegisrc", configured_libdir());
	read_and_merge(up->uconf_data, libdir_aegisrc);
	user_become_undo();

	fix_default_change(up->uconf_data);
    }
    trace(("return %08lX;\n", (long)up->uconf_data));
    trace(("}\n"));
    return up->uconf_data;
}


//
// NAME
//	user_home
//
// SYNOPSIS
//	string_ty *user_home(user_ty *up);
//
// DESCRIPTION
//	The user_home function is used to
//	fetch the home directory of the user.
//
// ARGUMENTS
//	up	- pointer to user structure
//
// RETURNS
//	pointer to string containing absolute path
//
// CAVEAT
//	If the path isn't absolute in /etc/passwd,
//	it won't be here, either.
//

string_ty *
user_home(user_ty *up)
{
    trace(("user_home(up = %08lX)\n{\n", (long)up));
    assert(up->home);
    trace(("return \"%s\";\n", up->home->str_text));
    trace(("}\n"));
    return up->home;
}


//
// NAME
//	user_full_name
//
// SYNOPSIS
//	char *user_full_name(string_ty *login);
//
// DESCRIPTION
//	The user_full_name function is used to
//	find the full name of a user, given their login name.
//
// ARGUMENTS
//	login	- pointer to string containing login name
//
// RETURNS
//	pointer to string containing user's full name
//
// CAVEAT
//	Will return the empty string if the user can't be found.
//	NEVER modify the data pointed to by the return value.
//	The returned data is volatile, it will not remain stable for long.
//

const char *
user_full_name(string_ty *name)
{
    struct passwd   *pw;
    const char      *result;
    static char	    *trimmed;
    static size_t   trimmed_max;

    trace(("user_full_name(name = \"%s\")\n{\n", name->str_text));
    pw = getpwnam_cached(name);
    if (!pw)
	result = "";
    else if (pw->pw_gecos && pw->pw_gecos[0])
	result = pw->pw_gecos;
#ifdef HAVE_pw_comment
    else if (pw->pw_comment && pw->pw_comment[0])
	result = pw->pw_comment;
#endif
    else
	result = "";

    //
    // Some systems add lots of other stuff to the full name field
    // in the passwd file.  We are only interested in the name.
    //
    const char *comma = strchr(result, ',');
    if (comma)
    {
	size_t		len;

	len = comma - result;
	if (len + 1 > trimmed_max)
	{
	    trimmed_max = len + 1;
	    trimmed = (char *)mem_change_size(trimmed, trimmed_max);
	}
	memcpy(trimmed, result, len);
	trimmed[len] = 0;
	result = trimmed;
    }
    trace(("return \"%s\";\n", result));
    trace(("}\n"));
    return result;
}


//
// NAME
//	user_ustate_write
//
// SYNOPSIS
//	void user_ustate_write(user_ty *up);
//
// DESCRIPTION
//	The user_ustate_write function is used to
//	write any modified ustate file contents back to disk.
//
// ARGUMENTS
//	up	- pointer to user structure
//

void
user_ustate_write(user_ty *up)
{
    string_ty	    *filename_new;
    string_ty	    *filename_old;
    static int	    count;

    trace(("user_ustate_write(up = %08lX)\n{\n", (long)up));
    assert(up->ustate_data);
    assert(up->ustate_path);
    if (!up->ustate_modified)
	goto ret;

    //
    // write it out
    //
    filename_new = str_format("%s,%d", up->ustate_path->str_text, ++count);
    filename_old = str_format("%s,%d", up->ustate_path->str_text, ++count);
    gonzo_become();
    if (up->ustate_is_new)
    {
	undo_unlink_errok(filename_new);
	ustate_write_file(filename_new, up->ustate_data, 0);
	commit_rename(filename_new, up->ustate_path);
    }
    else
    {
	undo_unlink_errok(filename_new);
	ustate_write_file(filename_new, up->ustate_data, 0);
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
    trace(("}\n"));
}


//
// NAME
//	user_own_add
//
// SYNOPSIS
//	void user_own_add(user_ty *up, string_ty *project_name,
//		long change_number);
//
// DESCRIPTION
//	The user_own_add function is used to
//	add a change to the user's list of owned changes.
//
// ARGUMENTS
//	up		- pointer to user structure
//	project_name	- project of the change
//	change_number	- number of the change
//
// CAVEAT
//	The change is assumed to be unique.
//

void
user_own_add(user_ty *up, string_ty *project_name, long change_number)
{
    ustate_ty	    *ustate_data;
    size_t	    j;
    ustate_own_ty   *own_data =	    0;
    ustate_own_ty   **own_data_p;
    long	    *change_p;
    type_ty	    *type_p;

    trace(("user_own_add(up = %8.8lX, project_name = \"%s\", "
	"change_number = %ld)\n{\n", (long)up, project_name->str_text,
	change_number));
    ustate_data = user_ustate_get(up);
    assert(ustate_data->own);

    //
    // See if the project is already known.
    //
    for (j = 0; j < ustate_data->own->length; ++j)
    {
	own_data = ustate_data->own->list[j];
	if (str_equal(own_data->project_name, project_name))
	    break;
    }

    //
    // If the project isn't known, append it to the list.
    //
    if (j >= ustate_data->own->length)
    {
	own_data_p =
	    (ustate_own_ty **)
            ustate_own_list_type.list_parse(ustate_data->own, &type_p);
	assert(type_p == &ustate_own_type);
	own_data = (ustate_own_ty *)ustate_own_type.alloc();
	*own_data_p = own_data;
	own_data->project_name = str_copy(project_name);
    }

    //
    // Create a changes for the project, if necessary.
    //
    if (!own_data->changes)
	own_data->changes =
	    (ustate_own_changes_list_ty *)ustate_own_changes_list_type.alloc();

    //
    // Add another item to the changes list for the project.
    //
    change_p =
	(long int *)
	ustate_own_changes_list_type.list_parse(own_data->changes, &type_p);
    assert(type_p == &integer_type);
    *change_p = change_number;
    up->ustate_modified = 1;
    trace(("}\n"));
}


//
// NAME
//	user_own_nth
//
// SYNOPSIS
//	int user_own_nth(user_ty *up, string_ty *project_name, long n,
//		long *change_number);
//
// DESCRIPTION
//	The user_own_nth function is used to fetch the n'th
//	change owned by a user.
//	The project name is derived from the user structure.
//
// ARGUMENTS
//	up		- pointer to user structure
//	n		- selector
//	change_number	- pointer to where to put number of the change
//
// RETURNS
//	1 on sucess, 0 if no such n.
//

int
user_own_nth(user_ty *up, string_ty *project_name, long n, long *change_number)
{
    ustate_ty	    *ustate_data;
    size_t	    j;
    int		    result;

    trace(("user_own_nth(up = %08lX, project_name = \"%s\", n = %ld)\n{\n",
	(long)up, project_name->str_text, n));
    result = 0;
    assert(n >= 0);
    if (n < 0)
	goto done;
    ustate_data = user_ustate_get(up);
    assert(ustate_data->own);
    if (!ustate_data->own)
	goto done;

    //
    // find the relevant project
    //	    and extract the n'th change
    //
    for (j = 0; j < ustate_data->own->length; ++j)
    {
	ustate_own_ty   *own_data;

	own_data = ustate_data->own->list[j];
	if (str_equal(project_name, own_data->project_name))
	{
	    if (own_data->changes && n < (long)own_data->changes->length)
	    {
		*change_number = own_data->changes->list[n];
		result = 1;
	    }
	    break;
	}
    }

    //
    // here for all exits
    //
    done:
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


//
// NAME
//	user_own_remove
//
// SYNOPSIS
//	void user_own_remove(user_ty *up, string_ty *project_name,
//		long change_number);
//
// DESCRIPTION
//	The user_own_remove function is used to
//	remove a change from the user's owned change list.
//
// ARGUMENTS
//	up		- pointer to user structure
//	project_name	- project of the change
//	change_number	- number of the change
//
// CAVEAT
//	The change is assumed to be unique.
//

void
user_own_remove(user_ty *up, string_ty *project_name, long change_number)
{
    ustate_ty	    *ustate_data;
    size_t	    j;
    size_t	    k;
    ustate_own_ty   *own_data;

    trace(("usate_own_remove()\n{\n"));
    ustate_data = user_ustate_get(up);
    assert(ustate_data->own);

    //
    // Search for the project in the ``own'' list.
    //
    for (j = 0;; ++j)
    {
	if (j >= ustate_data->own->length)
	    goto ret;
	own_data = ustate_data->own->list[j];
	if (str_equal(own_data->project_name, project_name))
	    break;
    }

    //
    // Create the ``changes'' list for the project, if necessary.
    //
    if (!own_data->changes)
	own_data->changes =
	    (ustate_own_changes_list_ty *)ustate_own_changes_list_type.alloc();

    //
    // Search for the change in the ``changes'' list.
    //
    for (k = 0; k < own_data->changes->length; ++k)
    {
	if (own_data->changes->list[k] == change_number)
	    break;
    }

    //
    // If the change was there, remove it from the list.
    //
    if (k < own_data->changes->length)
    {
	own_data->changes->list[k] =
	    own_data->changes->list[own_data->changes->length - 1];
	own_data->changes->length--;
	up->ustate_modified = 1;
    }

    //
    // If the changes list for the project is now empty,
    // remove the project from the ``own'' list.
    //
    if (!own_data->changes->length)
    {
	ustate_own_type.free(own_data);
	ustate_data->own->list[j] =
	    ustate_data->own->list[ustate_data->own->length - 1];
	ustate_data->own->length--;
	up->ustate_modified = 1;
    }

    //
    // here for all exits
    //
    ret:
    trace(("}\n"));
}


static void
user_error(user_ty *up, sub_context_ty *scp, const char *fmt)
{
    string_ty	    *s;
    int		    need_to_delete;

    if (!scp)
    {
	scp = sub_context_new();
	need_to_delete = 1;
    }
    else
	need_to_delete = 0;

    s = subst_intl(scp, fmt);

    // re-use substitution context
    sub_var_set_string(scp, "MeSsaGe", s);
    str_free(s);
    sub_var_set_string(scp, "Name", up->name);
    error_intl(scp, i18n("user $name: $message"));

    if (need_to_delete)
	sub_context_delete(scp);
}


static void
user_fatal(user_ty *up, sub_context_ty *scp, const char *fmt)
{
    string_ty	    *s;
    int		    need_to_delete;

    if (!scp)
    {
	scp = sub_context_new();
	need_to_delete = 1;
    }
    else
	need_to_delete = 0;

    s = subst_intl(scp, fmt);

    // re-use substitution context
    sub_var_set_string(scp, "MeSsaGe", s);
    str_free(s);
    sub_var_set_string(scp, "Name", up->name);
    fatal_intl(scp, i18n("user $name: $message"));

    if (need_to_delete)
	sub_context_delete(scp);
}


static void
waiting_for_lock(void *p)
{
    user_ty	    *up;

    up = (user_ty *)p;
    if (user_lock_wait((user_ty *)0))
	user_error(up, 0, i18n("waiting for lock"));
    else
	user_fatal(up, 0, i18n("lock not available"));
}


//
// NAME
//	user_ustate_lock_prepare
//
// SYNOPSIS
//	void user_ustate_lock_prepare(user_ty *up);
//
// DESCRIPTION
//	The user_ustate_lock_prepare function is used to
//	notify the lock manager that a ustate lock will be required.
//
// ARGUMENTS
//	up	- pointer to user structure
//

void
user_ustate_lock_prepare(user_ty *up)
{
    trace(("user_ustate_lock_prepare(up = %08lX)\n{\n", (long)up));
    lock_prepare_ustate(up->uid, waiting_for_lock, up);
    trace(("}\n"));
}


//
// NAME
//	user_default_change
//
// SYNOPSIS
//	long user_default_change(user_ty *up);
//
// DESCRIPTION
//	The user_default_change function is used to
//	find the default change number.	 The project is taken from the
//	user structure's notion of the set project.
//
// ARGUMENTS
//	up	- pointer to user structure
//
// RETURNS
//	the change number.
//
// CAVEAT
//	it is a fatal error if there is no default change number
//

static long
is_a_change_number(char *s)
{
    long	    n;

    while (isspace((unsigned char)*s))
	++s;
    if (!isdigit((unsigned char)*s))
	return 0;
    n = 0;
    for (;;)
    {
	n = n * 10 + *s++ - '0';
	if (!isdigit((unsigned char)*s))
	    break;
    }
    while (isspace((unsigned char)*s))
	++s;
    if (*s)
	return 0;
    return magic_zero_encode(n);
}


static long
project_dot_change(string_ty *s, string_ty *p)
{
    char	    *suffix;

    if
    (
	s->str_length <= p->str_length + 1
    ||
	memcmp(s->str_text, p->str_text, p->str_length)
    ||
	!ispunct((unsigned char)s->str_text[p->str_length])
    )
	return 0;
    suffix = s->str_text + p->str_length + 1;
    while (isupper((unsigned char)*suffix))
	++suffix;
    return is_a_change_number(suffix);
}


static int
is_below(string_ty *hi, string_ty *lo)
{
    if (str_equal(hi, lo))
	return 1;
    return
	(
	    lo->str_length > hi->str_length
	&&
	    lo->str_text[hi->str_length] == '/'
	&&
	    !memcmp(hi->str_text, lo->str_text, hi->str_length)
	);
}


long
user_default_change(user_ty *up)
{
    long	    change_number;
    char	    *cp;

    trace(("user_default_change(up = %08lX)\n{\n", (long)up));
    change_number = 0;

    //
    // check the AEGIS_CHANGE environment variable
    //
    cp = getenv("AEGIS_CHANGE");
    if (cp)
    {
	change_number = is_a_change_number(cp);
	if (!change_number)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_charstar(scp, "Name", "AEGIS_CHANGE");
	    fatal_intl(scp, i18n("$$$name must be positive"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
    }

    //
    // check the $HOME/.aegisrc file
    //
    if (!change_number)
    {
	uconf_ty	*uconf_data;

	uconf_data = user_uconf_get(up);
	if (uconf_data->mask & uconf_default_change_number_mask)
	    change_number =
		magic_zero_encode(uconf_data->default_change_number);
    }

    //
    // examine the pathname of the current directory
    // to see if we can extract the change number
    //
    // This only works if the development directory was created
    // by aegis, and not specified by the -DIRectory option.
    // It doesn't work at all for IntDir or BL.
    //
    if (!change_number)
    {
	string_ty	*cwd;
	string_list_ty	part;
	size_t		j;

	//
	// get the current directory
	//
	os_become_orig();
	cwd = os_curdir();
	os_become_undo();
	assert(cwd);

	//
	// break it into file names
	//
	part.split(cwd, "/");
	str_free(cwd);

	//
	// search for <proj>.<num>
	//
	for (j = 0; j < part.nstrings; ++j)
	{
	    change_number =
		project_dot_change(part.string[j], project_name_get(up->pp));
	    if (change_number)
		break;
	}
    }

    //
    // slower method than above, but works with any directory
    //
    if (!change_number)
    {
	string_ty	*cwd;
	string_ty	*d;
	long		j;
	change_ty	*cp2;
	cstate_ty       *cstate_data;

	//
	// get the current directory
	//
	os_become_orig();
	cwd = os_curdir();
	os_become_undo();
	assert(cwd);

	for (j = 0;; ++j)
	{
	    long	    cn;

	    if (!project_change_nth(up->pp, j, &cn))
		break;
	    cp2 = change_alloc(up->pp, cn);
	    change_bind_existing(cp2);
	    cstate_data = change_cstate_get(cp2);
	    switch (cstate_data->state)
	    {
	    case cstate_state_being_integrated:
		d = change_integration_directory_get(cp2, 0);
		if (is_below(d, cwd))
		{
		    change_number = cn;
		    break;
		}
		// fall through...

	    case cstate_state_awaiting_review:
	    case cstate_state_awaiting_integration:
	    case cstate_state_being_reviewed:
	    case cstate_state_being_developed:
		d = change_development_directory_get(cp2, 0);
		if (is_below(d, cwd))
		{
		    change_number = cn;
		}
		break;

	    case cstate_state_awaiting_development:
	    case cstate_state_completed:
		break;
	    }
	    change_free(cp2);
	    if (change_number)
		break;
	}
	str_free(cwd);
    }

    //
    // If the user is only working on one change within the given
    // project, then that is the change.
    //
    if (!change_number)
    {
	ustate_ty	*ustate_data;
	size_t		j;

	ustate_data = user_ustate_get(up);
	assert(ustate_data->own);
	for (j = 0; j < ustate_data->own->length; ++j)
	{
	    ustate_own_ty   *own_data;
	    string_ty	    *project_name;

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

    //
    // It is an error if no change number has been given.
    //
    if (!change_number)
	project_fatal(up->pp, 0, i18n("no change number"));
    trace(("return %ld;\n", change_number));
    trace(("}\n"));
    return change_number;
}

//
// NAME
//	user_default_project_by_user
//
// SYNOPSIS
//	void user_default_project_by_user(user_ty *);
//
// DESCRIPTION
//	The user_default_project_by_user function is used to
//	determine the default project of the specified user.
//
// RETURNS
//	pointer to string containing project name
//
// CAVEAT
//	it is a fatal error if there is no default project name for the user
//

string_ty *
user_default_project_by_user(user_ty *up)
{
    string_ty       *result;
    char	    *cp;

    trace(("user_default_project_by_user()\n{\n"));
    result = 0;

    //
    // from the AEGIS_PROJECT environment variable.
    //
    cp = getenv("AEGIS_PROJECT");
    if (cp && *cp)
	result = str_from_c(cp);

    //
    // From the $HOME/.aegisrc file.
    //
    if (!result)
    {
	uconf_ty	*uconf_data;

	uconf_data = user_uconf_get(up);
	if (uconf_data->default_project_name)
	    result = str_copy(uconf_data->default_project_name);
    }

    //
    // check the search path, see if we use just one
    //
    // else check the current dirctory to see if we are within one
    //
    // This only works if the development directory was created
    // by aegis, and not specified by the -DIRectory option.
    // It doesn't work at all for IntDir or BL.
    //
    if (!result)
    {
	string_list_ty	name;

	gonzo_project_list_user(up->name, &name);
	if (name.nstrings == 1)
	    result = str_copy(name.string[0]);
	else
	{
	    string_ty	    *cwd;
	    string_list_ty  part;
	    size_t	    j;
	    size_t	    k;

	    //
	    // get pathname of the current directory
	    //
	    os_become_orig();
	    cwd = os_curdir();
	    os_become_undo();
	    assert(cwd);

	    //
	    // break into pieces
	    //
	    part.split(cwd, "/");
	    str_free(cwd);

	    //
	    // search the path
	    // looking for <proj>.<num>
	    //
	    for (j = 0; j < part.nstrings && !result; ++j)
	    {
		for (k = 0; k < name.nstrings; ++k)
		{
		    if (project_dot_change(part.string[j], name.string[k]))
		    {
			result = str_copy(name.string[k]);
			break;
		    }
		}
	    }
	}

	//
	// check current directory more intensively.
	// this method is slower than previous, but works for
	// baseline and integration directories.
	//
	if (!result)
	{
	    string_ty	    *cwd;
	    string_ty	    *d;
	    size_t	    j;
	    size_t	    k;
	    project_ty	    *pp;
	    change_ty	    *cp3;
	    cstate_ty       *cstate_data;

	    //
	    // get pathname of the current directory
	    //
	    os_become_orig();
	    cwd = os_curdir();
	    os_become_undo();
	    assert(cwd);

	    for (j = 0; j < name.nstrings; ++j)
	    {
		pp = project_alloc(name.string[j]);
		project_bind_existing(pp);

		//
		// first check if it is in baseline
		//
		d = project_baseline_path_get(pp, 0);
		if (is_below(d, cwd))
		{
		    result = str_copy(name.string[j]);
		    project_free(pp);
		    break;
		}

		//
		// second check for every change
		//
		for (k = 0;; ++k)
		{
		    long	    change_number;

		    if (!project_change_nth(pp, k, &change_number))
			break;
		    cp3 = change_alloc(pp, change_number);
		    change_bind_existing(cp3);
		    cstate_data = change_cstate_get(cp3);
		    switch (cstate_data->state)
		    {
		    case cstate_state_being_integrated:
			d = change_integration_directory_get(cp3, 0);
			if (is_below(d, cwd))
			{
			    result = str_copy(name.string[j]);
			    break;
			}
			// fall through...

		    case cstate_state_awaiting_review:
		    case cstate_state_awaiting_integration:
		    case cstate_state_being_reviewed:
		    case cstate_state_being_developed:
			d = change_development_directory_get(cp3, 0);
			if (is_below(d, cwd))
			{
			    result = str_copy(name.string[j]);
			}
			break;

		    case cstate_state_awaiting_development:
		    case cstate_state_completed:
			break;
		    }
		    change_free(cp3);
		    if (result)
			break;
		}
		project_free(pp);
		if (result)
		    break;
	    }
	    str_free(cwd);
	}
    }

    //
    // It is an error if no project name has been given.
    //
    if (!result)
	fatal_intl(0, i18n("no project name"));

    //
    // clean up and go home
    //
    user_free(up);
    trace(("return \"%s\";\n", result->str_text));
    trace(("}\n"));
    return result;
}


//
// NAME
//	user_default_project
//
// SYNOPSIS
//	void user_default_project(void);
//
// DESCRIPTION
//	The user_default_project function is used to
//	determine the default project of the user who invoked the program.
//
// RETURNS
//	pointer to string containing project name
//
// CAVEAT
//	it is a fatal error if there is no default project name
//

string_ty *
user_default_project()
{
    //
    // build a temporary user
    //
    trace(("user_default_project()\n{\n"));
    return user_default_project_by_user(user_executing((project_ty *)0));
}


//
// NAME
//	user_default_development_directory_get
//
// SYNOPSIS
//	string_ty *user_default_development_directory_get(user_ty *up);
//
// DESCRIPTION
//	The user_default_development_directory_get function is used to
//	determine the absolute path of the user's default development directory.
//
// ARGUMENTS
//	up	- pointer to user structure
//
// RETURNS
//	pointer to string containing path
//
// CAVEAT
//	If the user has not explicitly set one,
//	and the project does not have one set,
//	the user's home directory will be returned.
//

string_ty *
user_default_development_directory(user_ty *up)
{
    uconf_ty	    *uconf_data;
    string_ty	    *path;

    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    uconf_data = user_uconf_get(up);
    path = uconf_data->default_development_directory;
    if (path)
    {
	if (path->str_text[0] == '/')
	    path = str_copy(path);
	else
	    path = os_path_join(user_home(up), path);
    }
    else
    {
	path = project_default_development_directory_get(up->pp);
	if (!path)
	    path = user_home(up);
	path = str_copy(path);
    }
    return path;
}


//
// NAME
//	user_default_project_directory
//
// SYNOPSIS
//	void user_default_project_directory(void);
//
// DESCRIPTION
//	The user_default_project_directory function is used to
//	determine the absolute path for where to place new projects.
//
// ARGUMENTS
//	up	- pointer to user structure
//
// RETURNS
//	pointer to string containing path
//
// CAVEAT
//	if the user has not explicitly set one,
//	the user's home directory will be returned.
//

string_ty *
user_default_project_directory(user_ty *up)
{
    uconf_ty	    *uconf_data;
    string_ty	    *path;

    //
    // To cope with automounters, directories are stored as given,
    // or are derived from the home directory in the passwd file.
    // Within aegis, pathnames have their symbolic links resolved,
    // and any comparison of paths is done on this "system idea"
    // of the pathname.
    //
    trace(("user_default_project_directory(up = %08lX)\n{\n", (long)up));
    uconf_data = user_uconf_get(up);
    path = uconf_data->default_project_directory;
    if (path)
    {
	if (path->str_text[0] == '/')
	    path = str_copy(path);
	else
	    path = os_path_join(user_home(up), path);
    }
    else
	path = str_copy(user_home(up));
    trace(("return \"%s\";\n", path->str_text));
    trace(("}\n"));
    return path;
}


//
// NAME
//	user_uid_check
//
// SYNOPSIS
//	int user_uid_check(char *name);
//
// DESCRIPTION
//	The user_uid_check function is used to
//	see if the named user exists, and if they do,
//	wether they have a non-system uid (i.e. 100 or over).
//
// ARGUMENTS
//	name	- string containing name to check
//
// RETURNS
//	zero if the login name is a system login,
//	non-zero if the login name is a mortal.
//
// CAVEAT
//	it is a fatal error if the user does not exist
//

int
user_uid_check(string_ty *name)
{
    struct passwd   *pw;

    pw = getpwnam_cached(name);
    if (!pw)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "Name", name);
	fatal_intl(scp, i18n("user $name unknown"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    return (pw->pw_uid >= AEGIS_MIN_UID);
}


//
// NAME
//	user_gid_check
//
// SYNOPSIS
//	int user_gid_check(char *name);
//
// DESCRIPTION
//	The user_gid_check function is used to
//	see if the named group exists, and if it does,
//	wether it is a non-system gid (i.e. 10 or over).
//
// ARGUMENTS
//	name	- string containing name to check
//
// RETURNS
//	zero if the name is a system group,
//	non-zero if the group is a mortal.
//
// CAVEAT
//	it is a fatal error if the group does not exist
//

int
user_gid_check(string_ty *name)
{
    struct group    *gr;

    gr = getgrnam_cached(name);
    if (!gr)
    {
	sub_context_ty	*scp;

	scp = sub_context_new();
	sub_var_set_string(scp, "Name", name);
	fatal_intl(scp, i18n("group \"$name\" unknown"));
	// NOTREACHED
	sub_context_delete(scp);
    }
    return (gr->gr_gid >= AEGIS_MIN_GID);
}


//
// NAME
//	user_become
//
// SYNOPSIS
//	void user_become(user_ty *up);
//
// DESCRIPTION
//	The user_become function is used to
//	set user and group to the given user.
//
// ARGUMENTS
//	up	- pointer to user structure
//
// CAVEAT
//	cancel with user_become_undo() when finished imitating this user.
//

void
user_become(user_ty *up)
{
    trace(("user_become(up = %08lX)\n{\n", (long)up));
    os_become(up->uid, up->gid, up->umask);
    trace(("}\n"));
}


void
user_become_undo()
{
    trace(("user_become_undo()\n{\n"));
    os_become_undo();
    trace(("}\n"));
}


enum del_pref
{
    del_pref_unset,
    del_pref_keep_not,
    del_pref_interactive,
    del_pref_keep
};

static del_pref cmd_line_pref = del_pref_unset;


void
user_delete_file_argument(void (*usage)(void))
{
    if (cmd_line_pref != del_pref_unset)
    {
	mutually_exclusive_options3
	(
	    arglex_token_keep_not,
	    arglex_token_interactive,
	    arglex_token_keep,
	    usage
	);
    }
    switch (arglex_token)
    {
    default:
	assert(0);

    case arglex_token_keep:
	cmd_line_pref = del_pref_keep;
	break;

    case arglex_token_interactive:
	cmd_line_pref = del_pref_interactive;
	break;

    case arglex_token_keep_not:
	cmd_line_pref = del_pref_keep_not;
	break;
    }
}


static int
ask(string_ty *filename, int isdir)
{
    struct table_ty
    {
	const char      *name;
	del_pref        set;
	int             result;
    };

    //
    // The order of items in the table needs to be considered
    // carefully.  The ``unset'' items must come before the ``set''
    // items; particularly those with similar names.  Do not sort
    // this table alphabetically.
    //
    static table_ty table[] =
    {
	{ "No", del_pref_unset, 0, },
	{ "False", del_pref_unset, 0, },
	{ "Never", del_pref_keep, 0, },
	{ "None", del_pref_keep, 0, },
	{ "Yes", del_pref_unset, 1, },
	{ "True", del_pref_unset, 1, },
	{ "All", del_pref_keep_not, 1, },
	{ "Always", del_pref_keep_not, 1, },
    };
    table_ty	    *tp;
    char	    buffer[100];
    char	    *bp;
    int		    c;

    for (;;)
    {
	printf
	(
	    "Delete the \"%s\" %s? ",
	    filename->str_text,
	    (isdir ? "directory and everything below it" : "file")
	);
	fflush(stdout);

	bp = buffer;
	for (;;)
	{
	    c = getchar();
	    if (c == '\n' || c == EOF)
		break;
	    if (bp < ENDOF(buffer) - 1)
		*bp++ = c;
	}
	*bp = 0;

	for (tp = table; tp < ENDOF(table); ++tp)
	{
	    if (arglex_compare(tp->name, buffer, 0))
	    {
		if (tp->set != del_pref_unset)
		    cmd_line_pref = tp->set;
		return tp->result;
	    }
	}
	printf("Please answer 'yes', 'no', 'all' or 'none'.\n");
    }
}


int
user_delete_file_query(user_ty *up, string_ty *filename, int isdir)
{
    int		    result;

    //
    // if the preference was not set on the command line,
    // read it fron the user config file
    //
    trace(("user_delete_file_query()\n{\n"));
    if (cmd_line_pref == del_pref_unset)
    {
	uconf_ty	*uconf_data;

	uconf_data = user_uconf_get(up);
	switch (uconf_data->delete_file_preference)
	{
	default:
	    cmd_line_pref = del_pref_keep_not;
	    break;

	case uconf_delete_file_preference_interactive:
	    cmd_line_pref = del_pref_interactive;
	    break;

	case uconf_delete_file_preference_keep:
	    cmd_line_pref = del_pref_keep;
	    break;
	}
    }

    //
    // if the preference is to ask but we are in the background,
    // delete the files without asking.
    //
    if
    (
	cmd_line_pref == del_pref_interactive
    &&
	(!isatty(0) || os_background())
    )
	cmd_line_pref = del_pref_keep_not;

    //
    // figure the result
    //
    switch (cmd_line_pref)
    {
    default:
	result = 1;
	break;

    case del_pref_interactive:
	result = ask(filename, isdir);
	break;

    case del_pref_keep:
	result = 0;
	break;
    }
    trace(("return %d;\n", result));
    trace(("}\n"));
    return result;
}


int
user_diff_preference(user_ty *up)
{
    uconf_ty	    *uconf_data;

    uconf_data = user_uconf_get(up);
    if (!(uconf_data->mask & uconf_diff_preference_mask))
	return uconf_diff_preference_automatic_merge;
    return uconf_data->diff_preference;
}


int
user_pager_preference(user_ty *up)
{
    uconf_ty	    *uconf_data;

    uconf_data = user_uconf_get(up);
    return (uconf_data->pager_preference == uconf_pager_preference_foreground);
}


static int	uconf_persevere_option = -1;


void
user_persevere_argument(void (*usage)(void))
{
    if (uconf_persevere_option >= 0)
	duplicate_option(usage);
    switch (arglex_token)
    {
    default:
	assert(0);
	return;

    case arglex_token_persevere:
	uconf_persevere_option = 1;
	break;

    case arglex_token_persevere_not:
	uconf_persevere_option = 0;
	break;
    }
}


int
user_persevere_preference(user_ty *up, int dflt)
{
    uconf_ty	    *uconf_data;

    if (uconf_persevere_option >= 0)
	return uconf_persevere_option;

    uconf_data = user_uconf_get(up);
    if (uconf_data->mask & uconf_persevere_preference_mask)
    {
	return
	    (
		uconf_data->persevere_preference
	    ==
		uconf_persevere_preference_all
	    );
    }
    return dflt;
}


uconf_log_file_preference_ty
user_log_file_preference(user_ty *up, uconf_log_file_preference_ty dflt)
{
    uconf_ty	    *uconf_data;

    uconf_data = user_uconf_get(up);
    if (uconf_data->mask & uconf_log_file_preference_mask)
	return uconf_data->log_file_preference;
    return dflt;
}


static int	uconf_lock_wait_option = -1;


void
user_lock_wait_argument(void (*usage)(void))
{
    if (uconf_lock_wait_option >= 0)
	duplicate_option(usage);
    switch (arglex_token)
    {
    default:
	assert(0);
	return;

    case arglex_token_wait:
	uconf_lock_wait_option = 1;
	break;

    case arglex_token_wait_not:
	uconf_lock_wait_option = 0;
	break;
    }
}


int
user_lock_wait(user_ty *up)
{
    if (uconf_lock_wait_option < 0)
    {
	uconf_ty	*uconf_data;
	uconf_lock_wait_preference_ty result;

	if (!up)
	    up = user_executing((project_ty *)0);
	uconf_data = user_uconf_get(up);
	result = uconf_data->lock_wait_preference;
	if (result == uconf_lock_wait_preference_background)
	{
	    result =
		(
		    os_background()
		?
		    uconf_lock_wait_preference_always
		:
		    uconf_lock_wait_preference_never
		);
	}
	uconf_lock_wait_option = (result == uconf_lock_wait_preference_always);
    }
    return uconf_lock_wait_option;
}


static int	uconf_whiteout_option = -1;


void
user_whiteout_argument(void (*usage)(void))
{
    if (uconf_whiteout_option >= 0)
	duplicate_option(usage);
    switch (arglex_token)
    {
    default:
	assert(0);
	return;

    case arglex_token_whiteout:
	uconf_whiteout_option = 1;
	break;

    case arglex_token_whiteout_not:
	uconf_whiteout_option = 0;
	break;
    }
}


int
user_whiteout(user_ty *up, int dflt)
{
    if (uconf_whiteout_option < 0)
    {
	if (dflt >= 0)
	{
	    uconf_whiteout_option = dflt;
	}
	else
	{
	    if (!up)
		up = user_executing((project_ty *)0);
	    uconf_ty *uconf_data = user_uconf_get(up);
	    uconf_whiteout_preference_ty result =
		uconf_data->whiteout_preference;
	    uconf_whiteout_option =
		(result == uconf_whiteout_preference_always);
	}
    }
    return uconf_whiteout_option;
}


static int	uconf_symlink_pref_option = -1;


void
user_symlink_pref_argument(void (*usage)(void))
{
    if (uconf_symlink_pref_option >= 0)
	duplicate_option(usage);
    switch (arglex_token)
    {
    default:
	assert(0);
	return;

    case arglex_token_symbolic_links:
	uconf_symlink_pref_option = 1;
	break;

    case arglex_token_symbolic_links_not:
	uconf_symlink_pref_option = 0;
	break;
    }
}


int
user_symlink_pref(user_ty *up, int proj_files_changed)
{
    if (uconf_symlink_pref_option < 0)
    {
	if (proj_files_changed)
	{
	    uconf_symlink_pref_option = 1;
	}
	else
	{
	    uconf_ty	    *uconf_data;
	    uconf_symbolic_link_preference_ty result;

	    if (!up)
		up = user_executing((project_ty *)0);
	    uconf_data = user_uconf_get(up);
	    result = uconf_data->symbolic_link_preference;
	    uconf_symlink_pref_option =
		(result == uconf_symbolic_link_preference_verify);
	}
    }
    return uconf_symlink_pref_option;
}


static int	user_relative_filename_preference_option = -1;


void
user_relative_filename_preference_argument(void (*usage)(void))
{
    if (user_relative_filename_preference_option >= 0)
	duplicate_option(usage);
    switch (arglex_token)
    {
    default:
	assert(0);
	return;

    case arglex_token_base_relative:
	user_relative_filename_preference_option =
	    uconf_relative_filename_preference_base;
	break;

    case arglex_token_current_relative:
	user_relative_filename_preference_option =
	    uconf_relative_filename_preference_current;
	break;
    }
}


uconf_relative_filename_preference_ty
user_relative_filename_preference(user_ty *up,
                                  uconf_relative_filename_preference_ty dflt)
{
    if (user_relative_filename_preference_option < 0)
    {
	uconf_ty	*uconf_data;

	if (!up)
	    up = user_executing((project_ty *)0);
	uconf_data = user_uconf_get(up);
	if (uconf_data->mask & uconf_relative_filename_preference_mask)
	    dflt = uconf_data->relative_filename_preference;
	user_relative_filename_preference_option = dflt;
    }
    return
	(uconf_relative_filename_preference_ty)
        user_relative_filename_preference_option;
}


//
// NAME
//	clean_up
//
// SYNOPSIS
//	string_ty *clean_up(string_ty *);
//
// DESCRIPTION
//	The clean_up function is used to clean up a user's full name
//	into an rfc822-acceptable string, suitable for use within an
//	email address.
//
// RETURNS
//	string_ty *; use str_free when finished with
//

static string_ty *
clean_up(string_ty *s)
{
    stracc_t	    buf;
    const char	    *cp;
    string_ty	    *result;
    unsigned char   prev;

    prev = ' ';
    for (cp = s->str_text; *cp; ++cp)
    {
	unsigned char	c;

	c = *cp;
	if (isspace(c) || strchr("@,<>()[]{}'\"", c) || !isprint(c))
	    c = ' ';
	if (c != ' ' || prev != ' ')
	    buf.push_back(c);
	prev = c;
    }
    while (buf.size() > 0 && buf[buf.size() - 1] == ' ')
	buf.pop_back();
    result = buf.mkstr();
    return result;
}


//
// NAME
//	user_email_address
//
// SYNOPSIS
//	string_ty *user_email_address(user_ty *);
//
// DESCRIPTION
//	The user_email_address is used to read the user's preferences
//	for her email address.	Defaults to full name from /etc/passwd,
//	then login name at hostname (not generally useful).
//
// RETURNS
//	string_ty *; DO NOT use str_free when finished
//

string_ty *
user_email_address(user_ty *up)
{
    uconf_ty	    *uconf_data;

    uconf_data = user_uconf_get(up);
    if (!uconf_data->email_address || !uconf_data->email_address->str_length)
    {
	//
	// Look for the domain name to go on the right hand side of the @ sign.
	//
	nstring domain = os_domain_name();

	//
	// Construct the email address.
	//
	string_ty *full_name = clean_up(user_name2(up));
	uconf_data->email_address =
	    str_format
	    (
		"%s <%s@%s>",
		full_name->str_text,
		user_name(up)->str_text,
		domain.c_str()
	    );
	str_free(full_name);
    }
    else
    {
	//
        // Even if the user has given us a string, we have to make sure
        // that it has all the right bits...
	//
	const char *s = uconf_data->email_address->str_text;
	const char *lt = strchr(s, '<');
	const char *at = strchr(s, '@');
	const char *gt = strchr(s, '>');
	if (lt && at && gt)
	{
	    // this is what we need, do nothing
	    if (lt == s)
	    {
		// Add their full name.
		string_ty *temp = uconf_data->email_address;
		string_ty *full_name = clean_up(user_name2(up));
		uconf_data->email_address =
		    str_format("%s %s", full_name->str_text, temp->str_text);
		str_free(full_name);
		str_free(temp);
	    }
	}
	else if (at)
	{
	    // Add their full name.
	    string_ty *temp = uconf_data->email_address;
	    string_ty *full_name = clean_up(user_name2(up));
	    uconf_data->email_address =
		str_format("%s <%s>", full_name->str_text, temp->str_text);
	    str_free(full_name);
	    str_free(temp);
	}
	else
	{
            //
            // Look for the domain name to go on the right hand side of
            // the @ sign.
	    //
	    nstring domain = os_domain_name();

	    //
	    // Construct the email address.
	    //
	    string_ty *full_name = clean_up(user_name2(up));
	    string_ty *temp = uconf_data->email_address;
	    uconf_data->email_address =
		str_format
		(
		    "%s <%s@%s>",
		    full_name->str_text,
		    temp->str_text,
		    domain.c_str()
		);
	    str_free(full_name);
	    str_free(temp);
	}
    }
    return uconf_data->email_address;
}


//
// NAME
//	user_editor_command
//
// SYNOPSIS
//	string_ty *user_editor_command(user_ty *);
//
// DESCRIPTION
//	The user_editor_command is used to read the user's preferences
//	for editor to use with aegis. Defaults to EDITOR environment
//	variable.
//
// RETURNS
//	string_ty *; DO NOT use str_free when finished
//

string_ty *
user_editor_command(user_ty *up)
{
    uconf_ty	    *uconf_data;
    char	    *editor;

    trace(("user_editor_command()\n{\n"));
    if (!up)
	up = user_executing((project_ty *)0);

    assert(!os_become_active());
    uconf_data = user_uconf_get(up);

    if (!uconf_data->editor_command)
    {
	editor = getenv("EDITOR");
	if (!editor || !*editor)
	{
	    sub_context_ty  *scp;

	    scp = sub_context_new();
	    sub_var_set_charstar(scp, "Name", "EDITOR");
	    fatal_intl(scp, i18n("environment variable $name not set"));
	    // NOTREACHED
	}
	uconf_data->editor_command = str_from_c(editor);
    }
    trace(("return \"%s\";\n", uconf_data->editor_command->str_text));
    trace(("}\n"));
    return uconf_data->editor_command;
}


//
// NAME
//	user_visual_command
//
// SYNOPSIS
//	string_ty *user_visual_command(user_ty *);
//
// DESCRIPTION
//	The user_visual_command is used to read the user's preferences
//	for editor to use with aegis for foreground editing.
//	If 'editor_command' is not set VISUAL environment variable is
//	used. If $VISUAL is not set then EDITOR environment variable is
//	used. If EDITOR is not set then "vi" is used.
//
// RETURNS
//	string_ty *; DO NOT use str_free when finished
//

string_ty *
user_visual_command(user_ty *up)
{
    uconf_ty	    *uconf_data;

    trace(("user_visual_command()\n{\n"));
    if (!up)
	up = user_executing((project_ty *)0);

    assert(!os_become_active());
    uconf_data = user_uconf_get(up);

    if (!uconf_data->visual_command)
    {
	const char      *editor;

	editor = getenv("VISUAL");
	if (!editor || !*editor)
	    editor = getenv("EDITOR");
	if (!editor || !*editor)
	    editor = "vi";
	uconf_data->visual_command = str_from_c(editor);
    }
    trace(("return \"%s\";\n", uconf_data->visual_command->str_text));
    trace(("}\n"));
    return uconf_data->visual_command;
}


//
// NAME
//	user_pager_command
//
// SYNOPSIS
//	string_ty *user_pager_command(user_ty *);
//
// DESCRIPTION
//	The user_pager_command is used to read the user's preferences
//	for pager to use with aegis. Defaults to PAGER environment variable
//	If $PAGER is not set then "more" is used.
//
// RETURNS
//	string_ty *; DO NOT use str_free when finished
//

string_ty *
user_pager_command(user_ty *up)
{
    uconf_ty	    *uconf_data;

    trace(("user_pager_command()\n{\n"));
    if (!up)
	up = user_executing((project_ty *)0);
    assert(!os_become_active());
    uconf_data = user_uconf_get(up);

    if (!uconf_data->pager_command)
    {
	const char      *pager;

	pager = getenv("PAGER");
	if (!pager || !*pager)
	    pager = "more";
	uconf_data->pager_command = str_from_c(pager);
    }
    trace(("return \"%s\";\n", uconf_data->pager_command->str_text));
    trace(("}\n"));
    return uconf_data->pager_command;
}


void
user_uconf_write_xml(user_ty *up, output_ty *op)
{
    uconf_ty	    *uconf_data;

    up = user_executing((project_ty *)0);
    user_email_address(up); // establish email_address field
    uconf_data = user_uconf_get(up);
    uconf_write_xml(op, uconf_data);
}
