//
//	aegis - project change supervisor
//	Copyright (C) 1994-1999, 2001-2008 Peter Miller
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
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>

#include <common/error.h>
#include <common/gettime.h>
#include <common/progname.h>
#include <common/quit.h>
#include <common/trace.h>
#include <libaegis/ael/project/history.h>
#include <libaegis/arglex2.h>
#include <libaegis/arglex/change.h>
#include <libaegis/arglex/project.h>
#include <libaegis/change/file.h>
#include <libaegis/change/identifier.h>
#include <libaegis/commit.h>
#include <libaegis/file/event.h>
#include <libaegis/help.h>
#include <libaegis/lock.h>
#include <libaegis/pconf.h>
#include <libaegis/project/file.h>
#include <libaegis/project/file/roll_forward.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>
#include <libaegis/zero.h>

#include <aegis/aedn.h>


static void
delta_name_usage(void)
{
    const char      *progname;

    progname = progname_get();
    fprintf
    (
	stderr,
	"usage: %s -Delta_Name [ <option>... ][ <delta_number> ] <string>...\n",
	progname
    );
    fprintf(stderr, "       %s -Delta_Name -List [ <option>... ]\n", progname);
    fprintf(stderr, "       %s -Delta_Name -Help\n", progname);
    quit(1);
}


static void
delta_name_help(void)
{
    help("aedn", delta_name_usage);
}


static void
delta_name_list(void)
{
    trace(("delta_name_list()\n{\n"));
    arglex();
    change_identifier cid;
    cid.command_line_parse_rest(delta_name_usage);
    list_project_history(cid, 0);
    trace(("}\n"));
}


static void
delta_name_main(void)
{
    sub_context_ty  *scp;
    string_ty       *project_name;
    long            change_number;
    long            delta_number;
    string_ty       *delta_name1;
    string_ty       *delta_name2;
    int             stomp;
    project_ty      *pp;
    user_ty::pointer up;
    pconf_ty        *pconf_data;
    time_t          delta_date;

    trace(("delta_name_main()\n{\n"));
    arglex();
    project_name = 0;
    delta_number = 0;
    delta_name1 = 0;
    delta_name2 = 0;
    change_number = 0;
    stomp = 0;
    delta_date = (time_t)(-1);
    while (arglex_token != arglex_token_eoln)
    {
	switch (arglex_token)
	{
	default:
	    generic_argument(delta_name_usage);
	    continue;

	case arglex_token_overwriting:
	    if (stomp)
		duplicate_option(delta_name_usage);
	    stomp = 1;
	    break;

	case arglex_token_string:
	    if (delta_name2)
	    {
		error_intl(0, i18n("too many delta names"));
		delta_name_usage();
	    }
	    delta_name2 = str_from_c(arglex_value.alv_string);
	    break;

	case arglex_token_delta:
	    switch (arglex())
	    {
	    default:
		option_needs_number(arglex_token_delta, delta_name_usage);
		// NOTREACHED

	    case arglex_token_string:
		if (delta_name1 || delta_number)
		{
		    duplicate_option_by_name
		    (
			arglex_token_delta,
			delta_name_usage
		    );
		    // NOTREACHED
		}
		delta_name1 = str_from_c(arglex_value.alv_string);
		break;

	    case arglex_token_number:
		process_delta_number:
		if (delta_name1 || delta_number)
		{
		    duplicate_option_by_name
		    (
			arglex_token_delta,
			delta_name_usage
		    );
		    // NOTREACHED
		}
		delta_number = arglex_value.alv_number;
		if (delta_number < 1)
		{
		    scp = sub_context_new();
		    sub_var_set_long(scp, "Number", delta_number);
		    fatal_intl(scp, i18n("delta $number out of range"));
		    // NOTREACHED
		    sub_context_delete(scp);
		}
		break;
	    }
	    break;

	case arglex_token_number:
	    goto process_delta_number;

	case arglex_token_change:
	case arglex_token_delta_from_change:
	    arglex();
	    arglex_parse_change
	    (
		&project_name,
		&change_number,
		delta_name_usage
	    );
	    continue;

	case arglex_token_delta_date:
	    if (delta_date != (time_t)-1)
		duplicate_option(delta_name_usage);
	    if (arglex() != arglex_token_string)
	    {
		option_needs_string(arglex_token_delta_date, delta_name_usage);
		// NOTREACHED
	    }
	    delta_date = date_scan(arglex_value.alv_string);
	    if (delta_date == (time_t)-1)
		fatal_date_unknown(arglex_value.alv_string);
	    break;

	case arglex_token_project:
	    arglex();
	    arglex_parse_project(&project_name, delta_name_usage);
	    continue;

	case arglex_token_wait:
	case arglex_token_wait_not:
	    user_ty::lock_wait_argument(delta_name_usage);
	    break;
	}
	arglex();
    }
    if
    (
	(
	    (delta_name1 || delta_number > 0)
	+
	    !!change_number
	+
	    (delta_date != (time_t)-1)
	)
    >
	1
    )
    {
	mutually_exclusive_options3
	(
	    arglex_token_delta,
	    arglex_token_delta_date,
	    arglex_token_delta_from_change,
	    delta_name_usage
	);
    }
    if (!delta_name2)
    {
	error_intl(0, i18n("no delta name"));
	delta_name_usage();
    }

    //
    // locate project data
    //
    if (!project_name)
    {
        nstring n = user_ty::create()->default_project();
	project_name = str_copy(n.get_ref());
    }
    pp = project_alloc(project_name);
    str_free(project_name);
    pp->bind_existing();

    //
    // locate user data
    //
    up = user_ty::create();

    //
    // lock the project file
    //
    pp->pstate_lock_prepare();
    lock_take();

    //
    // it is an error if the user is not a project administrator
    //
    if (!project_administrator_query(pp, up->name()))
	project_fatal(pp, 0, i18n("not an administrator"));

    //
    // Convert change to delta, if necessary.
    //
    if (!delta_number && delta_name1)
	delta_number = project_history_delta_by_name(pp, delta_name1, 0);
    if (!delta_number && delta_date != (time_t)(-1))
    {
	delta_number = project_history_timestamp_to_delta(pp, delta_date);
	if (delta_number == 0)
	{
	    scp = sub_context_new();
	    sub_var_set_format(scp, "Number", "%.24s", ctime(&delta_date));
	    project_fatal(pp, scp, i18n("no delta $number"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
    }
    if (!delta_number && change_number)
	delta_number = project_change_number_to_delta_number(pp, change_number);

    //
    // it is an error if the delta does not exist
    //
    if (delta_number)
    {
	if (!project_history_delta_validate(pp, delta_number))
	{
	    scp = sub_context_new();
	    sub_var_set_long(scp, "Number", delta_number);
	    project_fatal(pp, scp, i18n("no delta $number"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
    }
    else
    {
	delta_number = project_history_delta_latest(pp);
	if (delta_number <= 0)
	    project_fatal(pp, 0, i18n("no delta yet"));
    }

    if (!stomp)
    {
	long            other;

	other = project_history_delta_by_name(pp, delta_name2, 1);
	if (other && other != delta_number)
	{
	    scp = sub_context_new();
	    sub_var_set_string(scp, "Name", delta_name2);
	    sub_var_set_long(scp, "Number", delta_number);
	    sub_var_optional(scp, "Number");
	    sub_var_set_long(scp, "Other", other);
	    sub_var_optional(scp, "Other");
	    project_fatal(pp, scp, i18n("delta $name in use"));
	    // NOTREACHED
	    sub_context_delete(scp);
	}
    }
    project_history_delta_name_delete(pp, delta_name2);

    //
    // add the name to the selected history entry
    //
    project_history_delta_name_add(pp, delta_number, delta_name2);

    //
    // If the history label command is defined,
    // label each of the project files.
    //
    pconf_data = project_pconf_get(pp);
    if (pconf_data->history_label_command)
    {
	size_t          j;

	delta_date = project_history_delta_to_timestamp(pp, delta_number);
	project_file_roll_forward historian(pp, delta_date, 0);
	for (j = 0;; j++)
	{
	    fstate_src_ty   *src;
	    file_event   *fep;

	    src = pp->file_nth(j, view_path_simple);
	    if (!src)
		break;
	    fep = historian.get_last(src->file_name);
	    if (!fep)
	    {
		//
		// File not yet created at this delta.
		//
		continue;
	    }
	    src = fep->get_src();
	    assert(src);
	    switch (src->action)
	    {
	    case file_action_remove:
		//
		// File removed before this delta.
		//
		continue;

	    case file_action_create:
	    case file_action_modify:
	    case file_action_insulate:
	    case file_action_transparent:
		break;
	    }

	    //
	    // Label everything else.
	    //
	    change_run_history_label_command
	    (
		fep->get_change(),
		src,
		delta_name2
	    );
	}
    }

    //
    // release the locks
    //
    pp->pstate_write();
    commit();
    lock_release();

    //
    // verbose success message
    //
    scp = sub_context_new();
    sub_var_set_string(scp, "Name", delta_name2);
    sub_var_optional(scp, "Name");
    sub_var_set_long(scp, "Number", delta_number);
    sub_var_optional(scp, "Number");
    project_verbose(pp, scp, i18n("delta name complete"));
    sub_context_delete(scp);
    project_free(pp);
    str_free(delta_name2);
    trace(("}\n"));
}


void
delta_name_assignment(void)
{
    static arglex_dispatch_ty dispatch[] =
    {
	{ arglex_token_help, delta_name_help, 0 },
	{ arglex_token_list, delta_name_list, 0 },
    };

    trace(("delta_name_assignment()\n{\n"));
    arglex_dispatch(dispatch, SIZEOF(dispatch), delta_name_main);
    trace(("}\n"));
}
