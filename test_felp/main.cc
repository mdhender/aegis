//
// aegis - project change supervisor
// Copyright (C) 2007, 2008 Walter Franzini
// Copyright (C) 2009, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/error.h>
#include <common/progname.h>

#include <libaegis/arglex2.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/identifi_sub.h>
#include <libaegis/file/event.h>
#include <libaegis/file/event/list.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/project/identifi_sub/branch.h>
#include <libaegis/project/identifi_sub/plain.h>


static void
usage(void)
{
    const char *progname;

    progname = progname_get();
    fprintf(stderr,
            "%s -PROJect P [ -Delta d | -BaseLine ] filename\n", progname);
    exit(EXIT_FAILURE);
}

int
main(int argc, char **argv)
{
    os_become_init_mortal();

    arglex2_init(argc, argv);
    project_identifier_subset_plain pid;
    project_identifier_subset_branch branch_id(pid);
    change_identifier_subset change_id(branch_id);
    nstring filename;

    arglex();
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            // NOTREACHED

        case arglex_token_baseline:
        case arglex_token_delta:
        case arglex_token_project:
            change_id.command_line_parse(usage);
            continue;

        case arglex_token_string:
            filename = nstring(arglex_value.alv_string);
            break;
        }
        arglex();
    }

    if (filename.empty())
        usage();
    if (!change_id.set())
        usage();

    change::pointer cp = change_id.get_cp();
    project_file_roll_forward historian;
    historian.set(change_id.get_pp(), cp->completion_timestamp(), true);

    file_event_list::pointer felp = historian.get(filename);
    if (!felp)
        return 0;

    for (size_t j = 0; j < felp->size(); ++j)
    {
        file_event *fep = felp->get(j);
        fstate_src_ty *fstate_src = fep->get_src();
        change::pointer cp2 = fep->get_change();
        nstring version = cp2->version_get();

        printf("%s|", version.c_str());
        printf("%s|", file_action_ename(fstate_src->action));
        printf("%s|", fstate_src->uuid ? fstate_src->uuid->str_text : "");
        printf("%s|", fstate_src->file_name->str_text);
        switch (fstate_src->action)
        {
        case file_action_create:
        case file_action_remove:
            if (fstate_src->move)
                printf("%s", fstate_src->move->str_text);
            break;

        case file_action_insulate:
        case file_action_modify:
        case file_action_transparent:
            break;
        }
        puts("");
    }
    return 0;
}


// vim: set ts=8 sw=4 et :
