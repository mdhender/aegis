//
//	aegis - project change supervisor
//	Copyright (C) 1991-1995, 2002-2006, 2008 Peter Miller
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
//	along with this program; if not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/mem.h>
#include <common/trace.h>
#include <libaegis/commit.h>
#include <libaegis/dir.h>
#include <libaegis/dir/functor/rm_dir_tree.h>
#include <libaegis/interrupt.h>
#include <libaegis/os.h>
#include <libaegis/undo.h>


//
// This file contains "commit" functions.
//
// The idea is that aegis can be interrupted, or fail from errors, and
// behave as if "nothing" had happened.  That is, no user discernable
// difference to their environment, and certainly no changes to aegis'
// data base.
//
// To do this, many database functions write updates to temporary files
// "near" where they are to go, eventually.  A commit and an abort
// function are both issued, one to put the new file where it really
// goes, and one to remove it.  Exactly one option will be exercised.
//
// Commit actions will be performed with the user-id set the same as was
// set at the time the commit call was issued.
//

enum what_ty
{
    what_rename,
    what_unlink_errok,
    what_rmdir_errok,
    what_rmdir_tree_bg,
    what_rmdir_tree_errok,
    what_hard_link,
    what_symlink
};

struct action_ty
{
    what_ty         what;
    nstring         path1;
    nstring         path2;
    action_ty       *next;
    int             uid;
    int             gid;
    int             umask;

    action_ty() :
	what((what_ty)-1),
	next(0),
	uid(0),
	gid(0),
	umask(0)
    {
	os_become_query(&uid, &gid, &umask);
    }
};

static action_ty *head1;
static action_ty *tail1;
static action_ty *head2;


/**
  * The link1 function is used to add an new item to the head of chain1.
  *
  * @param action
  *     what to do
  * @param path1
  *     mandatory argument
  * @param path2
  *     optional argument
  */

static void
link1(what_ty what, const nstring &path1, const nstring &path2)
{
    trace(("commit::link1(what = %d, path1 = %08lX, path2 = %08lX)\n{\n",
	what, (long)path1, (long)path2));
    action_ty *new_thing = new action_ty;
    new_thing->what = what;
    new_thing->path1 = path1;
    new_thing->path2 = path2;
    if (head1)
    {
	tail1->next = new_thing;
	tail1 = new_thing;
    }
    else
	head1 = tail1 = new_thing;
    trace(("}\n"));
}


/**
  * The link2 function is used to add an new item to the head of chain2
  *
  * @param action
  *     what to do
  * @param path1
  *     mandatory argument
  * @param path2
  *     optional argument
  */

static void
link2(what_ty what, const nstring &path1, const nstring &path2)
{
    trace(("commit::link2(what = %d, path1 = %08lX, path2 = %08lX)\n{\n",
	what, (long)path1, (long)path2));
    action_ty *new_thing = new action_ty;
    new_thing->what = what;
    new_thing->path1 = path1;
    new_thing->path2 = path2;
    new_thing->next = head2;
    head2 = new_thing;
    trace(("}\n"));
}


void
commit_rename(string_ty *from, string_ty *to)
{
    commit_rename(nstring(from), nstring(to));
}


void
commit_rename(const nstring &from, const nstring &to)
{
    trace(("commit_rename(from = \"%s\", to = \"%s\")\n{\n", from.c_str(),
	to.c_str()));
    link1(what_rename, from, to);
    trace(("}\n"));
}


void
commit_symlink(string_ty *from, string_ty *to)
{
    commit_symlink(nstring(from), nstring(to));
}


void
commit_symlink(const nstring &from, const nstring &to)
{
    trace(("commit_symlink(from = \"%s\", to = \"%s\")\n{\n", from.c_str(),
	to.c_str()));
    link1(what_symlink, from, to);
    trace(("}\n"));
}


void
commit_hard_link(string_ty *from, string_ty *to)
{
    commit_hard_link(nstring(from), nstring(to));
}


void
commit_hard_link(const nstring &from, const nstring &to)
{
    trace(("commit_hard_link(from = \"%s\", to = \"%s\")\n{\n", from.c_str(),
	to.c_str()));
    link1(what_hard_link, from, to);
    trace(("}\n"));
}


void
commit_unlink_errok(string_ty *path)
{
    commit_unlink_errok(nstring(path));
}


void
commit_unlink_errok(const nstring &path)
{
    trace(("commit_unlink_errok(path = \"%s\")\n{\n", path.c_str()));
    link2(what_unlink_errok, path, "");
    trace(("}\n"));
}


void
commit_rmdir_errok(string_ty *path)
{
    commit_rmdir_errok(nstring(path));
}


void
commit_rmdir_errok(const nstring &path)
{
    trace(("commit_rmdir_errok(path = \"%s\")\n{\n", path.c_str()));
    link2(what_rmdir_errok, path, "");
    trace(("}\n"));
}


void
commit_rmdir_tree_bg(string_ty *path)
{
    commit_rmdir_tree_bg(nstring(path));
}


void
commit_rmdir_tree_bg(const nstring &path)
{
    trace(("commit_rmdir_tree_bg(path = \"%s\")\n{\n", path.c_str()));
    link2(what_rmdir_tree_bg, path, "");
    trace(("}\n"));
}


void
commit_rmdir_tree_errok(string_ty *path)
{
    commit_rmdir_tree_errok(nstring(path));
}


void
commit_rmdir_tree_errok(const nstring &path)
{
    trace(("commit_rmdir_tree_errok(path = \"%s\")\n{\n", path.c_str()));
    link2(what_rmdir_tree_errok, path, "");
    trace(("}\n"));
}


void
commit(void)
{
    //
    // Disable interrupts (such as ^C) for the duration.  Note that
    // commit consists solely of file renames and removes.  No long
    // writes are performed at this time.  Sometimes there is a lot
    // to do.
    //
    trace(("commit()\n{\n"));
    interrupt_disable();

    //
    // Perform the queued actions.
    //
    while (head1 || head2)
    {
	//
	// Take the first item off the list.
	// Note that actions may append more items to the list.
	//
	action_ty *action = 0;
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

	//
	// Do the action.
	//
	os_become(action->uid, action->gid, action->umask);
	switch (action->what)
	{
	case what_rename:
	    os_rename(action->path1, action->path2);
	    undo_rename(action->path2, action->path1);
	    break;

	case what_symlink:
	    os_symlink(action->path1, action->path2);
	    undo_unlink_errok(action->path2);
	    break;

	case what_hard_link:
	    os_link(action->path1, action->path2);
	    undo_unlink_errok(action->path2);
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
	    {
		dir_functor_rm_dir_tree eraser;
		dir_walk(action->path1, eraser);
	    }
	    break;
	}
	os_become_undo();

	//
	// Delete the list element.
	//
	delete action;
    }

    //
    // it's all committed, nothing left to undo.
    //
    undo_cancel();

    //
    // Enable interrupts once more.
    //
    interrupt_enable();
    trace(("}\n"));
}
