/*
 *      aegis - project change supervisor
 *      Copyright (C) 1991-1995, 1999, 2002, 2003 Peter Miller;
 *      All rights reserved.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
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
    what_ty         what;
    string_ty       *path1;
    string_ty       *path2;
    int             arg1;
    int             arg2;
    action_ty       *next;
    int             uid;
    int             gid;
    int             umask;
};

static action_ty *head;


static action_ty *
newlink(what_ty what)
{
    action_ty       *new_thing;

    trace(("undo::newlink(what = %d)\n{\n", what));
    new_thing = (action_ty *)mem_alloc(sizeof(action_ty));
    new_thing->what = what;
    new_thing->next = head;
    new_thing->path1 = 0;
    new_thing->path2 = 0;
    new_thing->arg1 = 0;
    new_thing->arg2 = 0;
    os_become_query(&new_thing->uid, &new_thing->gid, &new_thing->umask);
    head = new_thing;
    trace(("return %08lX;\n", (long)new_thing));
    trace(("}\n"));
    return new_thing;
}


void
undo_rename(string_ty *from, string_ty *to)
{
    action_ty       *new_thing;

    trace(("undo_rename(from = %08lX, to = %08lX)\n{\n", (long)from, (long)to));
    trace_string(from->str_text);
    trace_string(to->str_text);
    new_thing = newlink(what_rename);
    new_thing->path1 = str_copy(from);
    new_thing->path2 = str_copy(to);
    os_interrupt_cope();
    trace(("}\n"));
}


void
undo_chmod(string_ty *path, int mode)
{
    action_ty       *new_thing;

    mode &= 07777;
    trace(("undo_chmod(path = %08lX, mode = %05o)\n{\n", (long)path, mode));
    trace_string(path->str_text);
    new_thing = newlink(what_chmod);
    new_thing->path1 = str_copy(path);
    new_thing->arg1 = mode;
    os_interrupt_cope();
    trace(("}\n"));
}


void
undo_chmod_errok(string_ty *path, int mode)
{
    action_ty       *new_thing;

    mode &= 07777;
    trace(("undo_chmod_errok(path = %08lX, mode = %05o)\n{\n", (long)path,
        mode));
    trace_string(path->str_text);
    new_thing = newlink(what_chmod_errok);
    new_thing->path1 = str_copy(path);
    new_thing->arg1 = mode;
    os_interrupt_cope();
    trace(("}\n"));
}


void
undo_unlink_errok(string_ty *path)
{
    action_ty       *new_thing;

    trace(("undo_unlink_errok(path = %08lX)\n{\n", (long)path));
    trace_string(path->str_text);
    new_thing = newlink(what_unlink_errok);
    new_thing->path1 = str_copy(path);
    os_interrupt_cope();
    trace(("}\n"));
}


void
undo_message(string_ty *path)
{
    action_ty       *new_thing;

    trace(("undo_message(path = %08lX)\n{\n", (long)path));
    trace_string(path->str_text);
    new_thing = newlink(what_message);
    new_thing->path1 = str_copy(path);
    trace(("}\n"));
}


void
undo_rmdir_bg(string_ty *path)
{
    action_ty       *new_thing;

    trace(("undo_rmdir_bg(path = %08lX)\n{\n", (long)path));
    trace_string(path->str_text);
    new_thing = newlink(what_rmdir_bg);
    new_thing->path1 = str_copy(path);
    os_interrupt_cope();
    trace(("}\n"));
}


void
undo_rmdir_errok(string_ty *path)
{
    action_ty       *new_thing;

    trace(("undo_rmdir_errok(path = %08lX)\n{\n", (long)path));
    trace_string(path->str_text);
    new_thing = newlink(what_rmdir_errok);
    new_thing->path1 = str_copy(path);
    os_interrupt_cope();
    trace(("}\n"));
}


void
undo()
{
    sub_context_ty  *scp;
    static int      count;
    action_ty       *ap;

    trace(("undo()\n{\n"));
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
            trace(("ap = %08lX;\n", (long)ap));
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
    trace(("}\n"));
}


void
undo_quitter(int n)
{
    if (n)
        undo();
}


void
undo_cancel()
{
    action_ty       *ap;

    trace(("undo_cancel()\n{\n"));
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
    trace(("}\n"));
}
