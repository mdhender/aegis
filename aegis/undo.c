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
 * MANIFEST: functions to store and enact file operations on transation abort
 */

#include <error.h>
#include <mem.h>
#include <option.h>
#include <os.h>
#include <trace.h>
#include <undo.h>


enum what_ty
{
	what_rename,
	what_chmod,
	what_chmod_errok,
	what_unlink_errok,
	what_rmdir_errok
};
typedef enum what_ty what_ty;

typedef struct action_ty action_ty;
struct action_ty
{
	what_ty		what;
	string_ty	*path1;
	string_ty	*path2;
	int		arg1;
	int		arg2;
	action_ty	*next;
	int		uid;
	int		gid;
	int		umask;
};

static	action_ty	*head;


static action_ty *newlink _((what_ty));

static action_ty *
newlink(what)
	what_ty		what;
{
	action_ty	*new;

	trace(("undo::newlink(what = %d)\n{\n"/*}*/, what));
	new = (action_ty *)mem_alloc(sizeof(action_ty));
	new->what = what;
	new->next = head;
	new->path1 = 0;
	new->path2 = 0;
	new->arg1 = 0;
	new->arg2 = 0;
	os_become_query(&new->uid, &new->gid, &new->umask);
	head = new;
	trace(("return %08lX;\n", new));
	trace((/*{*/"}\n"));
	return new;
}


void
undo_rename(from, to)
	string_ty	*from;
	string_ty	*to;
{
	action_ty	*new;

	trace(("undo_rename(from = %08lX, to = %08lX)\n{\n"/*}*/, from, to));
	trace_string(from->str_text);
	trace_string(to->str_text);
	new = newlink(what_rename);
	new->path1 = str_copy(from);
	new->path2 = str_copy(to);
	trace((/*{*/"}\n"));
}


void
undo_chmod(path, mode)
	string_ty	*path;
	int		mode;
{
	action_ty	*new;

	mode &= 07777;
	trace(("undo_chmod(path = %08lX, mode = %05o)\n{\n"/*}*/, path, mode));
	trace_string(path->str_text);
	new = newlink(what_chmod);
	new->path1 = str_copy(path);
	new->arg1 = mode;
	trace((/*{*/"}\n"));
}


void
undo_chmod_errok(path, mode)
	string_ty	*path;
	int		mode;
{
	action_ty	*new;

	mode &= 07777;
	trace(("undo_chmod_errok(path = %08lX, mode = %05o)\n{\n"/*}*/, path, mode));
	trace_string(path->str_text);
	new = newlink(what_chmod_errok);
	new->path1 = str_copy(path);
	new->arg1 = mode;
	trace((/*{*/"}\n"));
}


void
undo_unlink_errok(path)
	string_ty	*path;
{
	action_ty	*new;

	trace(("undo_unlink_errok(path = %08lX)\n{\n"/*}*/, path));
	trace_string(path->str_text);
	new = newlink(what_unlink_errok);
	new->path1 = str_copy(path);
	trace((/*{*/"}\n"));
}

void
undo_rmdir_errok(path)
	string_ty	*path;
{
	action_ty	*new;

	trace(("undo_rmdir_errok(path = %08lX)\n{\n"/*}*/, path));
	trace_string(path->str_text);
	new = newlink(what_rmdir_errok);
	new->path1 = str_copy(path);
	trace((/*{*/"}\n"));
}


void
undo()
{
	static int	count;
	action_ty	*ap;
	char		*progname;

	trace(("undo()\n{\n"/*}*/));
	++count;
	switch (count)
	{
	case 1:
		while (os_become_active())
			os_become_undo();
		while (head)
		{
			/*
			 * Take the first item off the list.
			 */
			ap = head;
			head = ap->next;
	
			/*
			 * Do the action
			 */
			trace(("ap = %08lX;\n", ap));
			os_become(ap->uid, ap->gid, ap->umask);
			switch (ap->what)
			{
			case what_rename:
				os_rename(ap->path1, ap->path2);
				break;

			case what_chmod:
				os_chmod(ap->path1, ap->arg1);
				break;

			case what_chmod_errok:
				os_chmod_errok(ap->path1, ap->arg1);
				break;

			case what_unlink_errok:
				os_unlink_errok(ap->path1);
				break;
	
			case what_rmdir_errok:
				os_rmdir_errok(ap->path1);
				break;
			}
			os_become_undo();
	
			/*
			 * Free the list element.
			 */
			str_free(ap->path1);
			if (ap->path2)
				str_free(ap->path2);
			mem_free((char *)ap);
		}
		break;

	case 2:
		progname = option_progname_get();
		error
		(
"A fatal error occurred while attempting to recover \
from an earlier fatal error.  \
Some %s data may have been corrupted.  \
Inform the nearest %s guru immediately.",
			progname,
			progname
		);
		while (head)
		{
			ap = head;
			head = ap->next;
			switch (ap->what)
			{
			case what_rename:
				error
				(
					"unfinished recovery: mv %s %s",
					ap->path1->str_text,
					ap->path2->str_text
				);
				break;

			case what_chmod:
			case what_chmod_errok:
				error
				(
					"unfinished recovery: chmod %04o %s",
					ap->arg1,
					ap->path1->str_text
				);
				break;

			case what_unlink_errok:
				error
				(
					"unfinished recovery: rm %s",
					ap->path1->str_text
				);
				break;

			case what_rmdir_errok:
				error
				(
					"unfinished recovery: rmdir %s",
					ap->path1->str_text
				);
				break;
			}
	
			/*
			 * Free the list element.
			 */
			str_free(ap->path1);
			if (ap->path2)
				str_free(ap->path2);
			mem_free((char *)ap);
		}
		break;

	default:
		/* probably an error writing stderr */
		break;
	}
	--count;
	trace((/*{*/"}\n"));
}


void
undo_quitter(n)
	int	n;
{
	if (n)
		undo();
}


void
undo_cancel()
{
	action_ty	*ap;

	trace(("undo_cancel()\n{\n"/*}*/));
	while (head)
	{
		/*
		 * Take the first item off the list.
		 */
		ap = head;
		head = ap->next;

		/*
		 * Free the list element.
		 */
		str_free(ap->path1);
		if (ap->path2)
			str_free(ap->path2);
		mem_free((char *)ap);
	}
	trace((/*{*/"}\n"));
}
