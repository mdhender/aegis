//
// aegis - project change supervisor
// Copyright (C) 2003-2008, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>

#include <common/env.h>
#include <common/language.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/rsrc_limits.h>
#include <common/trace.h>
#include <libaegis/arglex2.h>
#include <libaegis/change/identifier.h>
#include <libaegis/help.h>
#include <libaegis/os.h>
#include <libaegis/output/file.h>
#include <libaegis/sub.h>
#include <libaegis/version.h>

#include <aexml/xml.h>


static void
xml_usage(void)
{
    const char *progname = progname_get();
    fprintf(stderr, "usage: %s [ <option>... ] <xml-name>\n", progname);
    fprintf
    (
        stderr,
        "       %s [ <option>... ] -File <filename>\n",
        progname
    );
    fprintf(stderr, "       %s -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Help\n", progname);
    quit(1);
}


static void
xml_help(void)
{
    help((char *)0, xml_usage);
}


static void
xml_list(void)
{
    trace(("xml_list()\n{\n"));
    arglex();
    nstring outfile;
    while (arglex_token != arglex_token_eoln)
        generic_argument(xml_usage);

    os_become_orig();
    output::pointer op = output_file::compressed_text_open(outfile);
    os_become_undo();

    //
    // list the available reports
    //
    xml::factory_list(op);
    trace(("}\n"));
}


static void
xml_main(void)
{
    trace(("xml_main()\n{\n"));
    change_identifier cid;
    nstring outfile;
    const char *report_name = 0;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(xml_usage);
            continue;

        case arglex_token_baseline:
        case arglex_token_branch:
        case arglex_token_change:
        case arglex_token_delta:
        case arglex_token_delta_date:
        case arglex_token_grandparent:
        case arglex_token_number:
        case arglex_token_project:
        case arglex_token_trunk:
            cid.command_line_parse(xml_usage);
            continue;

        case arglex_token_output:
            if (!outfile.empty())
                duplicate_option(xml_usage);
            if (arglex() != arglex_token_string)
                option_needs_file(arglex_token_output, xml_usage);
            outfile = nstring(arglex_value.alv_string);
            break;

        case arglex_token_string:
            if (report_name)
            {
                sub_context_ty sc;
                sc.fatal_intl(i18n("too many lists"));
            }
            report_name = arglex_value.alv_string;
            break;
        }
        arglex();
    }
    if (!report_name)
        fatal_intl(0, i18n("no list"));
    cid.command_line_check(xml_usage);

    //
    // Find the report.
    //
    xml::pointer xp = xml::factory(report_name);

    //
    // Open the output file.
    //
    os_become_orig();
    output::pointer op = output_file::compressed_text_open(outfile);
    os_become_undo();

    //
    // run the report
    //
    xp->report(cid, op);
    trace(("}\n"));
}


int
main(int argc, char **argv)
{
    arglex2_init(argc, argv);
    resource_limits_init();
    os_become_init_mortal();
    env_initialize();
    language_init();
    switch (arglex())
    {
    default:
        xml_main();
        break;

    case arglex_token_help:
        xml_help();
        break;

    case arglex_token_list:
        xml_list();
        break;

    case arglex_token_version:
        version();
        break;
    }
    quit(0);
    return 0;
}


// vim: set ts=8 sw=4 et :
