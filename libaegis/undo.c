/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991, 1992, 1993, 1994, 1995, 1999 Peter Miller;
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
 * MANIFEST: functions to store and enact file operations on transaction abort
 */

#include <error.h>
#include <mem.h>
#include <os.h>
#include <sub.h>
#include <trace.h>
#include <undo.h>


enum what_ty
{
	what_rename,
	what_chmod,
	what_chmod_errok,
	what_unlink_errok,
	what_rmdir_bg,
	what_rmdir_errok,
	what_message
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
	os_interrupt_cope();
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
	os_interrupt_cope();
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
	os_interrupt_cope();
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
	os_interrupt_cope();
	trace((/*{*/"}\n"));
}


void
undo_message(path)
	string_ty	*path;
{
	action_ty	*new;

	trace(("undo_message(path = %08lX)\n{\n"/*}*/, path));
	trace_string(path->str_text);
	new = newlink(what_message);
	new->path1 = str_copy(path);
	trace((/*{*/"}\n"));
}


void
undo_rmdir_bg(path)
	string_ty	*path;
{
	action_ty	*new;

	trace(("undo_rmdir_bg(path = %08lX)\n{\n"/*}*/, path));
	trace_string(path->str_text);
	new = newlink(what_rmdir_bg);
	new->path1 = str_copy(path);
	os_interrupt_cope();
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
	os_interrupt_cope();
	trace((/*{*/"}\n"));
}


void
undo()
{
	sub_context_ty	*scp;
	static int	count;
	action_ty	*ap;

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
	
			case what_rmdir_bg:
				os_rmdir_bg(ap->path1);
				break;
	
			case what_rmdir_errok:
				os_rmdir_errok(ap->path1);
				break;
	
			case what_message:
				scp = sub_context_new();
				sub_var_set_string(scp, "Message", ap->path1);
				error_intl(scp, i18n("$message"));
				sub_context_delete(scp);
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
		scp = sub_context_new();
		error_intl(scp, i18n("fatal error during fatal error recovery"));
		sub_context_delete(scp);
		while (head)
		{
			ap = head;
			head = ap->next;
			switch (ap->what)
			{
			case what_rename:
				scp = sub_context_new();
				sub_var_set_string(scp, "File_Name1", ap->path1);
				sub_var_set_string(scp, "File_Name2", ap->path2);
				error_intl(scp, i18n("unfinished: mv $filename1 $filename2"));
				sub_context_delete(scp);
				break;

			case what_chmod:
			case what_chmod_errok:
				scp = sub_context_new();
				sub_var_set_format(scp, "Argument", "%05o", ap->arg1);
				sub_var_set_string(scp, "File_Name", ap->path1);
				error_intl(scp, i18n("unfinished: chmod $arg $filename"));
				sub_context_delete(scp);
				break;

			case what_unlink_errok:
				scp = sub_context_new();
				sub_var_set_string(scp, "File_Name", ap->path1);
				error_intl(scp, i18n("unfinished: rm $filename"));
				sub_context_delete(scp);
				break;

			case what_rmdir_bg:
			case what_rmdir_errok:
				scp = sub_context_new();
				sub_var_set_string(scp, "File_Name", ap->path1);
				error_intl(scp, i18n("unfinished: rmdir $filename"));
				sub_context_delete(scp);
				break;

			case what_message:
				scp = sub_context_new();
				sub_var_set_string(scp, "Message", ap->path1);
				error_intl(scp, i18n("$message"));
				sub_context_delete(scp);
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
