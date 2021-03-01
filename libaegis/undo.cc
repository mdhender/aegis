//
//      aegis - project change supervisor
//      Copyright (C) 1991-1995, 1999, 2002-2006, 2008, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/error.h>
#include <common/mem.h>
#include <libaegis/os.h>
#include <libaegis/quit/action/undo.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/undo.h>
#include <libaegis/undo/item/chmod_errok.h>
#include <libaegis/undo/item/chmod.h>
#include <libaegis/undo/item/message.h>
#include <libaegis/undo/item/rename.h>
#include <libaegis/undo/item/rmdir_bg.h>
#include <libaegis/undo/item/rmdir_errok.h>
#include <libaegis/undo/item/unlink_errok.h>


quit_action_undo undo_quitter;

#include <list>
typedef std::list<undo_item *> jobs_t;
static jobs_t jobs;


void
undo_rename(string_ty *from, string_ty *to)
{
    trace(("undo_rename(from = %p, to = %p)\n{\n", from, to));
    undo_rename(nstring(from), nstring(to));
    trace(("}\n"));
}


void
undo_rename(const nstring &from, const nstring &to)
{
    trace(("undo_rename(from = \"%s\", to = \"%s\")\n{\n",
        from.c_str(), to.c_str()));
    undo_item *ip = new undo_item_rename(from, to);
    jobs.push_back(ip);
    os_interrupt_cope();
    trace(("}\n"));
}


void
undo_rename_cancel(string_ty *from, string_ty *to)
{
    undo_rename_cancel(nstring(from), nstring(to));
}


void
undo_rename_cancel(const nstring &from, const nstring &to)
{
    trace(("undo_rename_cancel(\"%s\", \"%s\")\n{\n",
        from.c_str(), to.c_str()));
    undo_item_rename dummy(from, to);
    for (jobs_t::iterator it = jobs.begin(); it != jobs.end(); ++it)
    {
        undo_item *ip = *it;
        // downcast and check
        undo_item_rename *irp = dynamic_cast<undo_item_rename *>(ip);
        if (irp && *irp == dummy)
        {
            jobs.erase(it);
            trace(("}\n"));
            return;
        }
    }

    //
    // It is a bug if we try to cancel a rename never requested.
    //
    this_is_a_bug();
    trace(("}\n"));
}


void
undo_chmod(string_ty *path, int mode)
{
    undo_chmod(nstring(path), mode);
}


void
undo_chmod(const nstring &path, int mode)
{
    mode &= 07777;
    trace(("undo_chmod(path = \"%s\", mode = %05o)\n{\n", path.c_str(), mode));
    undo_item *ip = new undo_item_chmod(path, mode);
    jobs.push_back(ip);
    os_interrupt_cope();
    trace(("}\n"));
}


void
undo_chmod_errok(string_ty *path, int mode)
{
    undo_chmod_errok(nstring(path), mode);
}


void
undo_chmod_errok(const nstring &path, int mode)
{
    mode &= 07777;
    trace(("undo_chmod_errok(path = \"%s\", mode = %05o)\n{\n", path.c_str(),
        mode));
    undo_item *ip = new undo_item_chmod_errok(path, mode);
    jobs.push_back(ip);
    os_interrupt_cope();
    trace(("}\n"));
}


void
undo_unlink_errok(string_ty *path)
{
    undo_unlink_errok(nstring(path));
}


void
undo_unlink_errok(const nstring &path)
{
    trace(("undo_unlink_errok(path = \"%s\")\n{\n", path.c_str()));
    undo_item *ip = new undo_item_unlink_errok(path);
    jobs.push_back(ip);
    os_interrupt_cope();
    trace(("}\n"));
}


void
undo_message(string_ty *path)
{
    undo_message(nstring(path));
}


void
undo_message(const nstring &msg)
{
    trace(("undo_message(msg = \"%s\")\n{\n", msg.c_str()));
    undo_item *ip = new undo_item_message(msg);
    jobs.push_back(ip);
    trace(("}\n"));
}


void
undo_rmdir_bg(string_ty *path)
{
    undo_rmdir_bg(nstring(path));
}


void
undo_rmdir_bg(const nstring &path)
{
    trace(("undo_rmdir_bg(path = \"%s\")\n{\n", path.c_str()));
    undo_item *ip = new undo_item_rmdir_bg(path);
    jobs.push_back(ip);
    os_interrupt_cope();
    trace(("}\n"));
}


void
undo_rmdir_errok(string_ty *path)
{
    undo_rmdir_errok(nstring(path));
}


void
undo_rmdir_errok(const nstring &path)
{
    trace(("undo_rmdir_errok(path = \"%s\")\n{\n", path.c_str()));
    undo_item *ip = new undo_item_rmdir_errok(path);
    jobs.push_back(ip);
    os_interrupt_cope();
    trace(("}\n"));
}


void
undo()
{
    trace(("undo()\n{\n"));
    static int recursion_depth;
    ++recursion_depth;
    switch (recursion_depth)
    {
    case 1:
        while (os_become_active())
            os_become_undo();
        while (!jobs.empty())
        {
            undo_item *ip = jobs.back();
            jobs.pop_back();
            trace(("ip = %p;\n", ip));
            ip->act();
            delete ip;
        }
        break;

    case 2:
        {
            sub_context_ty *scp = sub_context_new();
            error_intl(scp, i18n("fatal error during fatal error recovery"));
            sub_context_delete(scp);
            while (!jobs.empty())
            {
                undo_item *ip = jobs.back();
                jobs.pop_back();
                ip->unfinished();
                delete ip;
            }
        }
        break;

    default:
        // probably an error writing stderr
        break;
    }
    --recursion_depth;
    trace(("}\n"));
}


void
undo_cancel()
{
    trace(("undo_cancel()\n{\n"));
    while (!jobs.empty())
    {
        undo_item *ip = jobs.back();
        jobs.pop_back();
        delete ip;
    }
    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
