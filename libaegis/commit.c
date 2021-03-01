/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-1995, 2002, 2003 Peter Miller;
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
 * MANIFEST: functions to store and enact file manipulations for commit
 */

#include <commit.h>
#include <dir.h>
#include <interrupt.h>
#include <mem.h>
#include <os.h>
#include <trace.h>
#include <undo.h>

/*
 * This file contains "commit" functions.
 *
 * The idea is that aegis can be interrupted, or fail from errors, and
 * behave as if "nothing" had happened.  That is, no user discernable
 * difference to their environment, and certainly no changes to aegis'
 * data base.
 *
 * To do this, many database functions write updates to temporary files
 * "near" where they are to go, eventually.  A commit and an abort
 * function are both issued, one to put the new file where it really
 * goes, and one to remove it.  Exactly one option will be exercised.
 *
 * Commit actions will be performed with the user-id set the same as was
 * set at the time the commit call was issued.
 */

enum what_ty
{
    what_rename,
    what_unlink_errok,
    what_rmdir_errok,
    what_rmdir_tree_bg,
    what_rmdir_tree_errok
};
typedef enum what_ty what_ty;

typedef struct action_ty action_ty;
struct action_ty
{
    what_ty         what;
    string_ty       *path1;
    string_ty       *path2;
    action_ty       *next;
    int             uid;
    int             gid;
    int             umask;
};

static action_ty *head1;
static action_ty *tail1;
static action_ty *head2;


/*
 * NAME
 *	link1
 *
 * SYNOPSIS
 *	void link1(what_ty action, string_ty *path1, string_ty *path2);
 *
 * DESCRIPTION
 *	The link1 function is used to
 *	add an new item to the head of chain1.
 *
 * ARGUMENTS
 *	action	- what to do
 *	path1	- mandatory argument
 *	path2	- optional argument (NULL if not used)
 */

static void
link1(what_ty what, string_ty *path1, string_ty *path2)
{
    action_ty       *new_thing;

    trace(("commit::link1(what = %d, path1 = %08lX, path2 = %08lX)\n{\n",
	what, (long)path1, (long)path2));
    new_thing = (action_ty *) mem_alloc(sizeof(action_ty));
    new_thing->what = what;
    os_become_query(&new_thing->uid, &new_thing->gid, &new_thing->umask);
    new_thing->path1 = str_copy(path1);
    if (path2)
	new_thing->path2 = str_copy(path2);
    else
	new_thing->path2 = 0;
    new_thing->next = 0;
    if (head1)
    {
	tail1->next = new_thing;
	tail1 = new_thing;
    }
    else
	head1 = tail1 = new_thing;
    trace(("}\n"));
}


/*
 * NAME
 *	link2
 *
 * SYNOPSIS
 *	void link2(what_ty action, string_ty *path1, string_ty *path2);
 *
 * DESCRIPTION
 *	The link2 function is used to
 *	add an new item to the head of chain2
 *
 * ARGUMENTS
 *	action	- what to do
 *	path1	- mandatory argument
 *	path2	- optional argument (NULL if not used)
 */

static void
link2(what_ty what, string_ty *path1, string_ty *path2)
{
    action_ty      *new_thing;

    trace(("commit::link2(what = %d, path1 = %08lX, path2 = %08lX)\n{\n",
	what, (long)path1, (long)path2));
    new_thing = (action_ty *) mem_alloc(sizeof(action_ty));
    new_thing->what = what;
    os_become_query(&new_thing->uid, &new_thing->gid, &new_thing->umask);
    new_thing->path1 = str_copy(path1);
    if (path2)
	new_thing->path2 = str_copy(path2);
    else
	new_thing->path2 = 0;
    new_thing->next = head2;
    head2 = new_thing;
    trace(("}\n"));
}


/*
 * NAME
 *	commit_rename
 *
 * SYNOPSIS
 *	void commit_rename(void);
 *
 * DESCRIPTION
 *	The commit_rename function is used to
 *	submit a commit request to rename a file.
 *
 * ARGUMENTS
 *	from	- path of file now
 *	to	- path of file after commit
 */

void
commit_rename(string_ty *from, string_ty *to)
{
    trace(("commit_rename(from = %08lX, to = %08lX)\n{\n", (long)from,
	(long)to));
    trace_string(from->str_text);
    trace_string(to->str_text);
    link1(what_rename, from, to);
    trace(("}\n"));
}


/*
 * NAME
 *	commit_unlink_errok
 *
 * SYNOPSIS
 *	void commit_unlink_errok(void);
 *
 * DESCRIPTION
 *	The commit_unlink_errok function is used to unlink a file on
 *	commit.  It will not be an error if the file does not exist.
 *
 * ARGUMENTS
 *	path	- path of file to be unlinked
 */

void
commit_unlink_errok(string_ty *path)
{
    trace(("commit_unlink_errok(path = %08lX)\n{\n", (long)path));
    trace_string(path->str_text);
    link2(what_unlink_errok, path, (string_ty *) 0);
    trace(("}\n"));
}


/*
 * NAME
 *	commit_rmdir_errok
 *
 * SYNOPSIS
 *	void commit_rmdir_errok(string_ty *path);
 *
 * DESCRIPTION
 *	The commit_rmdir_errok function is used to delete an empty
 *	directory on commit.  It will not be an error if the directory
 *	does not exist.  It will not be an error if the directory is not
 *	empty.
 *
 * ARGUMENTS
 *	path	- path of directory to be deleted
 */

void
commit_rmdir_errok(string_ty *path)
{
    trace(("commit_rmdir_errok(path = %08lX)\n{\n", (long)path));
    trace_string(path->str_text);
    link2(what_rmdir_errok, path, (string_ty *) 0);
    trace(("}\n"));
}


/*
 * NAME
 *	commit_rmdir_tree_bg
 *
 * SYNOPSIS
 *	void commit_rmdir_tree_bg(string_ty *path);
 *
 * DESCRIPTION
 *	The commit_rmdir_tree_bg function is used to delete a
 *	directory tree on commit.  It will not be an error if the
 *	directory does not exist.  It will not be an error if the
 *	directory, or any subtree, is not empty.
 *
 * ARGUMENTS
 *	path	- path of directory to be deleted
 */

void
commit_rmdir_tree_bg(string_ty *path)
{
    trace(("commit_rmdir_tree_bg(path = %08lX)\n{\n", (long)path));
    trace_string(path->str_text);
    link2(what_rmdir_tree_bg, path, (string_ty *) 0);
    trace(("}\n"));
}


/*
 * NAME
 *	commit_rmdir_tree_errok
 *
 * SYNOPSIS
 *	void commit_rmdir_tree_errok(string_ty *path);
 *
 * DESCRIPTION
 *	The commit_rmdir_tree_errok function is used to delete a
 *	directory tree on commit.  It will not be an error if the
 *	directory does not exist.  It will not be an error if the
 *	directory, or any subtree, is not empty.
 *
 * ARGUMENTS
 *	path	- path of directory to be deleted
 */

void
commit_rmdir_tree_errok(string_ty *path)
{
    trace(("commit_rmdir_tree_errok(path = %08lX)\n{\n", (long)path));
    trace_string(path->str_text);
    link2(what_rmdir_tree_errok, path, (string_ty *) 0);
    trace(("}\n"));
}


/*
 * NAME
 *	rmdir_tree_callback
 *
 * SYNOPSIS
 *	void rmdir_tree_callback(void * arg, dir_walk_message_ty message,
 *		string_ty *path, struct stat *st);
 *
 * DESCRIPTION
 *	The rmdir_tree_callback function is used to perform an action
 *	while walking a directory tree's structure.
 *
 *	This function is the one that actually deletes things out of the
 *	directory tree as it is walked.  Note that the directory should
 *	be deleted last, after all contents have been nuked.
 *
 *	Some sleight-of-hand is involved here, as this function pushes
 *	extra stuff onto the lists of things to be deleted, rather than
 *	really doing it itself.
 *
 * ARGUMENTS
 *	arg	- argument given to dir_walk for us
 *	message	- what sort of file system entity we are looking at
 *	path	- the absolute path of the file system entity
 *	st	- pointer to stat structure describing file system entity
 */

static void
rmdir_tree_callback(void *arg, dir_walk_message_ty message, string_ty *path,
    struct stat *theStat)
{
    trace(("rmdir_tree_callback(message = %d, path = %08lX, "
           "theStat = %08lX)\n{\n",
           message, (long)path, (long)theStat));
    trace_string(path->str_text);
    switch (message)
    {
    case dir_walk_dir_before:
	commit_rmdir_errok(path);
	os_chmod_errok(path, 0750);
	break;

    case dir_walk_dir_after:
	break;

    case dir_walk_file:
    case dir_walk_special:
    case dir_walk_symlink:
	commit_unlink_errok(path);
	break;
    }
    trace(("}\n"));
}


/*
 * NAME
 *	commit
 *
 * SYNOPSIS
 *	void commit(void);
 *
 * DESCRIPTION
 *	The commit function is used to
 *	perform all the actions queued using the commit_* functions.
 *
 *	After it has completed successfully, further calls to commit()
 *	will be NOPs, until new commit_* functions are used.
 *
 *	When the commit has succeeded, the undo list is cancelled,
 *	since there is now no reason to undo anything.
 */

void
commit(void)
{
    /*
     * Disable interrupts (such as ^C) for the duration.  Note that
     * commit cosists solely of file renames and removes.  No long
     * writes are perfomed at this time.  Sometimes there is a lot
     * to do.
     */
    trace(("commit()\n{\n"));
    interrupt_disable();

    /*
     * Perform the queued actions.
     */
    while (head1 || head2)
    {
	action_ty      *action;

	/*
	 * Take the first item off the list.
	 * Note that actions may append more items to the list.
	 */
	if (head1)
	{
	    action = head1;
	    head1 = action->next;
	    if (!head1)
		tail1 = 0;
	}
	else
	{
	    action = head2;
	    head2 = action->next;
	}

	/*
	 * Do the action.
	 */
	os_become(action->uid, action->gid, action->umask);
	switch (action->what)
	{
	case what_rename:
	    os_rename(action->path1, action->path2);
	    undo_rename(action->path2, action->path1);
	    break;

	case what_unlink_errok:
	    os_unlink_errok(action->path1);
	    break;

	case what_rmdir_errok:
	    os_rmdir_errok(action->path1);
	    break;

	case what_rmdir_tree_bg:
	    os_rmdir_bg(action->path1);
	    break;

	case what_rmdir_tree_errok:
	    if (os_exists(action->path1))
		dir_walk(action->path1, rmdir_tree_callback, 0);
	    break;
	}
	os_become_undo();

	/*
	 * Free the list element.
	 */
	str_free(action->path1);
	if (action->path2)
	    str_free(action->path2);
	mem_free((char *)action);
    }

    /*
     * it's all committed, nothing left to undo.
     */
    undo_cancel();

    /*
     * Enable interrupts once more.
     */
    interrupt_enable();
    trace(("}\n"));
}
