//
//      aegis - project change supervisor
//      Copyright (C) 2006-2008, 2011, 2012 Peter Miller
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
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/nstring.h>
#include <common/progname.h>
#include <libaegis/arglex2.h>
#include <libaegis/attribute.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/os.h>
#include <libaegis/pconf.fmtgen.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/project/identifi_sub/plain.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aelock/lock.h>
#include <aelock/usage.h>


void
lock_main()
{
    //
    // Parse the command line arguments.
    //
    arglex();
    project_identifier_subset_plain pid;
    const char *command_name = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(lock_usage);
            continue;

        case arglex_token_project:
        case arglex_token_trunk:
            pid.command_line_parse(lock_usage);
            continue;

        case arglex_token_string:
            if (command_name)
                lock_usage();
            command_name = arglex_value.alv_string;
            break;
        }
        arglex();
    }
    if (!command_name)
    {
        lock_usage();
    }

    //
    // Locate the command.
    //
    // We do this by looking in the given project for the appropriately
    // named attribute.
    //
    pconf_ty *pconf_data = project_pconf_get(pid.get_pp());
    nstring attribute_name =
        nstring::format("%s:%s", progname_get(), command_name);
    attributes_ty *ap =
        attributes_list_find
        (
            pconf_data->project_specific,
            attribute_name.c_str()
        );
    if (!ap)
    {
        sub_context_ty sc;
        sc.var_set_string("Name", attribute_name);
        project_fatal(pid.get_pp(), &sc, i18n("attribute $name unknown"));
        // NOTREACHED
    }
    sub_context_ty sc;
    string_ty *command = sc.substitute_p(pid.get_pp(), ap->value);

    //
    // Only project adminstrators are allowed to do this.
    //
    if (!project_administrator_query(pid.get_pp(), pid.get_up()->name()))
    {
        project_fatal(pid.get_pp(), 0, i18n("not an administrator"));
    }

    //
    // Take the lock and run the command.
    //
    project *trunk = pid.get_pp()->trunk_get();
    trunk->lock_prepare_everything();
    lock_take();
    project_become(trunk);
    os_execute(command, 0, trunk->home_path_get());
    project_become_undo(trunk);
    lock_release();
}


// vim: set ts=8 sw=4 et :
