/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001 Peter Miller;
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
 * MANIFEST: functions to manipulate symlinkss
 */

#include <change.h>
#include <change/file.h>
#include <dir_stack.h>
#include <error.h>	/* for assert */
#include <gmatch.h>
#include <os.h>
#include <project.h>
#include <project/file.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


typedef struct slink_info_ty slink_info_ty;
struct slink_info_ty
{
    string_list_ty  stack;
    change_ty       *cp;
    pconf           pconf_data;
    int             minimum;
    user_ty         *up;
};

static string_ty *dot;


/*
 * NAME
 *	maintain
 *
 * SYNOPSIS
 *	void maintain(void *arg, dir_stack_walk_message_t msg, string_ty *path,
 *		struct stat *st, int depth);
 *
 * DESCRIPTION
 *	The maintain function is used to maintain symbolic links in the
 *	development directory.	It is called once for each file by the
 *	dir_stack_walk function.
 *
 *	arg	The argument passed to dir_stack_walk
 *	msg	The message indicating what the file is
 *	path	The RELATIVE path name of the file, relative to the
 *		directory stack.
 *	st	The stat structure describing the file.
 *	depth	The depth down the directory stack.  Zero means in the
 *		development directory (or integration directory).
 */

static void maintain _((void *, dir_stack_walk_message_t, string_ty *,
    struct stat *, int));

static void
maintain(p, msg, path, st, depth)
    void             *p;
    dir_stack_walk_message_t msg;
    string_ty        *path;
    struct stat      *st;
    int              depth;
{
    slink_info_ty    *sip;
    string_ty        *dd_abs;
    long             j;
    pconf_symlink_exceptions_list lp;
    fstate_src       c_src;
    fstate_src       p_src;
    int              p_src_set;
    string_ty        *s;

    trace(("maintain(path = \"%s\")\n{\n", path->str_text));
    sip = p;
    dd_abs = os_path_cat(sip->stack.string[0], path);
    p_src = 0;
    p_src_set = 0;
    switch (msg)
    {
    case dir_stack_walk_dir_before:
	if (depth)
	    os_mkdir(dd_abs, 02755);
	break;

    case dir_stack_walk_dir_after:
	break;

    case dir_stack_walk_symlink:
	/*
	 * Ignore symlinks that aren't in the development
	 * directory.  They can't be source files, so the build
	 * created them, and we will let it create them again.
	 */
	if (depth)
	    break;

	/*
	 * The minimum option says to only maintain links to
	 * project source files.  A symlink can't ever be a
	 * project source file, so don't maintain it.
	 */
	if (sip->minimum)
	    break;

	/*
	 * Remove the symlink if the file has been removed.
	 */
	user_become_undo();
	c_src = change_file_find(sip->cp, path);
	user_become(sip->up);
	if (c_src && (c_src->deleted_by || c_src->about_to_be_created_by))
	{
	    os_unlink(dd_abs);
	    break;
	}
	user_become_undo();
	p_src = project_file_find(sip->cp->pp, path);
	p_src_set = 1;
	user_become(sip->up);
	if
	(
	    p_src
	&&
	    (
	        p_src->action == file_action_remove
	    ||
	        p_src->deleted_by
	    ||
		p_src->about_to_be_created_by
	    )
	)
	{
	    os_unlink(dd_abs);
	    break;
	}

	/*
	 * Read the link, and make sure it points to the right
	 * place.  We only maintain source files.  Derived files
	 * will need to fixed by the build system.
	 */
	if (!c_src && p_src)
	{
	    string_ty *dest_wanted;
	    string_ty *dest_actual;

	    dest_wanted = project_file_path(sip->cp->pp, path);
	    dest_actual = os_readlink(dd_abs);
	    if (!str_equal(dest_wanted, dest_actual))
	    {
		os_unlink(dd_abs);
		trace(("os_symlink(\"%s\", \"%s\")\n", dest_wanted->str_text,
		    dd_abs->str_text));
		os_symlink(dest_wanted, dd_abs);
	    }
	    str_free(dest_wanted);
	    str_free(dest_actual);
	    break;
	}

	/*
	 * Read the non-source link and make sure it points to
	 * the right place.
	 */
	if (!sip->minimum && !c_src && !p_src)
	{
	    string_ty       *dest_actual;
	    string_ty       *rel_dest;

	    dest_actual = os_readlink(dd_abs);
	    rel_dest = dir_stack_relative(&sip->stack, dest_actual);
	    if (rel_dest)
	    {
		string_ty       *dest_wanted;

		dest_wanted = dir_stack_find(&sip->stack, 1, rel_dest, 0, 0);
		if (dest_wanted)
		{
		    if (!str_equal(dest_wanted, dest_actual))
		    {
			os_unlink(dd_abs);
			os_symlink(dest_wanted, dd_abs);
		    }
		    str_free(dest_wanted);
		}
		str_free(rel_dest);
	    }
	    str_free(dest_actual);
	}
	break;

    case dir_stack_walk_special:
    case dir_stack_walk_file:
	/*
	 * Files in the development directory require no further
	 * attention.
	 */
	if (!depth)
	    break;

	/*
	 * Don't make symlinks for files which are (supposed to
	 * be) in the change.
	 */
	if (change_file_find(sip->cp, path))
	    break;

	/*
	 * The minimum option says to only link project source
	 * files.  This simulates -minimum integration builds.
	 */
	if (sip->minimum)
	{
	    user_become_undo();
	    p_src = project_file_find(sip->cp->pp, path);
	    p_src_set = 1;
	    user_become(sip->up);
	    if
	    (
		p_src
	    &&
		(
		    p_src->action == file_action_remove
		||
		    p_src->about_to_be_created_by
		||
		    p_src->deleted_by
		)
	    )
		break;
	}

	/*
	 * avoid the symlink exceptions
	 */
	lp = sip->pconf_data->symlink_exceptions;
	assert(lp);
	for (j = 0; j < lp->length; ++j)
	{
	    if (gmatch(lp->list[j]->str_text, path->str_text))
		break;
	}
	if (j < lp->length)
	    break;

	/*
	 * avoid removed files
	 */
	if (!p_src_set)
	{
	    user_become_undo();
	    p_src = project_file_find(sip->cp->pp, path);
	    user_become(sip->up);
	    p_src_set = 1;
	}
	if
	(
	    p_src
	&&
	    (
		p_src->action == file_action_remove
	    ||
		p_src->about_to_be_created_by
	    ||
		p_src->deleted_by
	    )
	)
	    break;

	/*
	 * make the symbolic link
	 */
	s = str_format("%S/%S", sip->stack.string[depth], path);
	trace(("os_symlink(\"%s\", \"%s\")\n", s->str_text, dd_abs->str_text));
	os_symlink(s, dd_abs);
	break;
    }
    str_free(dd_abs);
    trace(("}\n"));
}


/*
 * NAME
 *	change_create_symlinks_to_baseline
 *
 * SYNOPSIS
 *	void change_create_symlinks_to_baseline(change_ty *cp, project_ty *pp,
 *		user_ty *up, int minimum);
 *
 * DESCRIPTION
 *	The change_create_symlinks_to_baseline function is used to create
 *	symbolic links between a development directory (or integration
 *	directory) and the baseline (and ancestor baselines).  It does
 *	this in two passes, the first creates the links, and the second
 *	removes dead links.
 */

void
change_create_symlinks_to_baseline(cp, pp, up, minimum)
    change_ty       *cp;
    project_ty      *pp;
    user_ty         *up;
    int             minimum;
{
    slink_info_ty   si;

    trace(("change_create_symlinks_to_baseline(cp = %8.8lX)\n{\n" /*} */ ,
	(long)cp));
    assert(cp->reference_count >= 1);
    change_verbose(cp, 0, i18n("creating symbolic links to baseline"));

    /*
     * Work out the search path.
     */
    string_list_constructor(&si.stack);
    change_search_path_get(cp, &si.stack, 0);

    /*
     * For each ancestor, create symlinks from the development
     * directory to that ancestor's baseline if the file does not
     * already exist.
     */
    si.cp = cp;
    si.pconf_data = change_pconf_get(cp, 0);
    si.minimum = minimum;
    si.up = up;
    if (!dot)
	dot = str_from_c(".");
    user_become(up);
    dir_stack_walk(&si.stack, dot, maintain, &si);
    user_become_undo();
    string_list_destructor(&si.stack);
    trace(("}\n"));
}


static void rsltbl _((void *, dir_stack_walk_message_t, string_ty *,
    struct stat *, int));

static void
rsltbl(p, msg, path, st, depth)
    void            *p;
    dir_stack_walk_message_t msg;
    string_ty       *path;
    struct stat     *st;
    int             depth;
{
    slink_info_ty   *sip;
    string_ty       *dest;
    string_ty       *dest_rel;
    string_ty       *path_abs;

    /*
     * remove symlinks in the development directory
     * which point to their counterpart in the baseline
     */
    trace(("rsltbl(path = \"%s\", msg = %d, depth = %d)\n{\n", path->str_text,
	msg, depth));
    if (msg != dir_stack_walk_symlink)
    {
	trace(("}\n"));
	return;
    }
    if (depth)
    {
	trace(("}\n"));
	return;
    }
    sip = p;
    path_abs = str_format("%S/%S", sip->stack.string[0], path);
    dest = os_readlink(path_abs);
    dest_rel = dir_stack_relative(&sip->stack, dest);
    str_free(dest);
    if (dest_rel)
    {
	if (str_equal(path, dest_rel))
	    os_unlink(path_abs);
	str_free(dest_rel);
    }
    str_free(path_abs);
    trace(("}\n"));
}


void
change_remove_symlinks_to_baseline(cp, pp, up)
    change_ty       *cp;
    project_ty      *pp;
    user_ty         *up;
{
    slink_info_ty   si;

    trace(("change_remove_symlinks_to_baseline(cp = %8.8lX)\n{\n" /*} */ ,
	(long)cp));
    assert(cp->reference_count >= 1);
    change_verbose(cp, 0, i18n("removing symbolic links to baseline"));

    /*
     * Work out the search path.
     */
    string_list_constructor(&si.stack);
    change_search_path_get(cp, &si.stack, 0);

    /*
     * walk the tree
     */
    si.cp = 0;
    si.pconf_data = 0;
    user_become(up);
    if (!dot)
	dot = str_from_c(".");
    dir_stack_walk(&si.stack, dot, rsltbl, &si);
    user_become_undo();
    string_list_destructor(&si.stack);
    trace(("}\n"));
}
