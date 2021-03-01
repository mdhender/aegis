//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2006, 2008, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <common/str.h>
#include <common/str_list.h>
#include <libaegis/ael/column_width.h>
#include <libaegis/col.h>
#include <libaegis/cstate.fmtgen.h>
#include <libaegis/help.h>
#include <libaegis/input/cpio.h>
#include <libaegis/meta_parse.h>
#include <libaegis/os.h>
#include <libaegis/output.h>
#include <libaegis/sub.h>

#include <aedist/arglex3.h>
#include <aedist/list.h>
#include <aedist/open.h>
#include <aedist/usage.h>


void
list_main(void)
{
    cstate_ty       *change_set;
    size_t          j;
    string_ty       *ofn = 0;
    output::pointer head_col;
    output::pointer body_col;
    int             left;
    output::pointer usage_col;
    output::pointer action_col;
    output::pointer file_name_col;
    arglex();
    nstring ifn;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            continue;

        case arglex_token_file:
            if (!ifn.empty())
                duplicate_option(usage);
            switch (arglex())
            {
            default:
                option_needs_file(arglex_token_file, usage);
                // NOTREACHED

            case arglex_token_string:
                ifn = arglex_value.alv_string;
                break;

            case arglex_token_stdio:
                ifn = "-";
                break;
            }
            break;

        case arglex_token_output:
            if (ofn)
                duplicate_option(usage);
            switch (arglex())
            {
            default:
                option_needs_file(arglex_token_file, usage);

            case arglex_token_string:
                ofn = str_from_c(arglex_value.alv_string);
                break;

            case arglex_token_stdio:
                ofn = str_from_c("");
                break;
            }
            break;
        }
        arglex();
    }

    //
    // Open the input file and verify the format.
    //
    nstring s;
    input_cpio *cpio_p = aedist_open(ifn, &s);
    assert(cpio_p);
    assert(s);

    //
    // Set the listing title from the change set subject line.
    //
    col::pointer colp = col::open(ofn);
    head_col = colp->create(0, 0, (const char *)0);
    body_col = colp->create(INDENT_WIDTH, 0, (const char *)0);
    colp->title("Distribution Change Set", s.c_str());

    //
    // read the project name from the archive,
    // and use it to default the project if not given
    //
    os_become_orig();
    nstring archive_name;
    input ifp = cpio_p->child(archive_name);
    if (!ifp.is_open())
        cpio_p->fatal_error("file missing");
    if (archive_name != "etc/project-name")
        ifp->fatal_error("wrong file");
    archive_name.clear();

    nstring pname;
    if (!ifp->one_line(pname) || pname.empty())
        ifp->fatal_error("short file");
    ifp.close();
    os_become_undo();

    head_col->fputs("PROJECT");
    colp->eoln();
    body_col->fputs(pname);

    //
    // read the change number from the archive, and use it to default
    // the change number if not given, and if possible.
    //
    os_become_orig();
    archive_name.clear();
    ifp = cpio_p->child(archive_name);
    if (!ifp.is_open())
        cpio_p->fatal_error("file missing");
    if (archive_name == "etc/change-number")
    {
        nstring chnum;
        ifp->one_line(chnum);
        ifp.close();
        os_become_undo();
        body_col->fputs(", change ");
        body_col->fputs(chnum);

        os_become_orig();
        archive_name.clear();
        ifp = cpio_p->child(archive_name);
        if (!ifp.is_open())
            cpio_p->fatal_error("file missing");
    }
    colp->eoln();

    //
    // get the change details from the input
    //
    if (archive_name != "etc/change-set")
        ifp->fatal_error("wrong file");
    change_set = (cstate_ty *)parse_input(ifp, &cstate_type);
    ifp.close();
    os_become_undo();

    //
    // Make sure we like the change set at a macro level.
    //
    if
    (
        !change_set->brief_description
    ||
        !change_set->description
    ||
        !change_set->src
    ||
        !change_set->src->length
    )
        cpio_p->fatal_error("bad change set");
    for (j = 0; j < change_set->src->length; ++j)
    {
        cstate_src_ty   *src_data;

        src_data = change_set->src->list[j];
        if
        (
            !src_data->file_name
        ||
            !src_data->file_name->str_length
        ||
            !(src_data->mask & cstate_src_action_mask)
        ||
            !(src_data->mask & cstate_src_usage_mask)
        )
            cpio_p->fatal_error("bad file info");
    }

    colp->need(3);
    head_col->fputs("SUMMARY");
    colp->eoln();
    body_col->fputs(change_set->brief_description);
    colp->eoln();

    colp->need(5);
    head_col->fputs("DESCRIPTION");
    colp->eoln();
    body_col->fputs(change_set->description);
    colp->eoln();

    //
    // cause
    //
    colp->need(5);
    head_col->fputs("CAUSE");
    colp->eoln();
    body_col->fprintf
    (
        "This change was caused by %s.",
        change_cause_ename(change_set->cause)
    );
    colp->eoln();

    //
    // files
    //
    colp->need(5);
    head_col->fputs("FILES");
    colp->eoln();

    left = INDENT_WIDTH;
    usage_col = colp->create(left, left + USAGE_WIDTH, "Type\n-------");
    left += USAGE_WIDTH + 1;

    action_col =
        colp->create(left, left + ACTION_WIDTH, "Action\n--------");
    left += ACTION_WIDTH + 1;

    file_name_col = colp->create(left, 0, "File Name\n-----------");

    for (j = 0; j < change_set->src->length; ++j)
    {
        cstate_src_ty *src_data = change_set->src->list[j];
        assert(src_data->file_name);
        usage_col->fputs(file_usage_ename(src_data->usage));
        action_col->fputs(file_action_ename(src_data->action));
        file_name_col->fputs(src_data->file_name);
        if (src_data->move)
        {
            switch (src_data->action)
            {
            case file_action_create:
                file_name_col->fputs("\nMoved from ");
                file_name_col->fputs(src_data->move);
                break;

            case file_action_remove:
                file_name_col->fputs("\nMoved to ");
                file_name_col->fputs(src_data->move);
                break;

            case file_action_modify:
            case file_action_insulate:
            case file_action_transparent:
                assert(0);
                break;
            }
        }
        colp->eoln();
    }

    delete cpio_p;
    cstate_type.free(change_set);
}


// vim: set ts=8 sw=4 et :
