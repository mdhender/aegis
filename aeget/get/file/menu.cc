//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate menus
//

#include <ac/stdio.h>

#include <change.h>
#include <change/file.h>
#include <error.h>
#include <get/file/menu.h>
#include <fstate.h>
#include <http.h>
#include <project.h>
#include <project/file.h>
#include <str_list.h>


void
get_file_menu(change_ty *cp, string_ty *filename, string_list_ty *modifier)
{
    fstate_src_ty   *src;
    int             baseline_file_exists;
    int             hold_bogus;

    html_header(0, cp);
    printf("<title>Project\n");
    html_encode_string(project_name_get(cp->pp));
    if (!cp->bogus)
	printf(", Change %ld\n", magic_zero_decode(cp->number));
    printf(", File ");
    html_encode_string(filename);
    printf("</title></head><body>\n");
    html_header_ps(0, cp);
    printf("<h1 align=center>\n");
    emit_change(cp);
    printf(",<br>File &ldquo;");
    html_encode_string(filename);
    printf("&rdquo;</h1>\n");

    //
    // Make sure the file exists.
    //
    if (!cp->bogus)
    {
	src = change_file_find(cp, filename, view_path_first);
	if (!src)
	{
	    printf("This file is not part of this change.\n");
	    if (project_file_find(cp->pp, filename, view_path_simple))
	    {
		printf("You may like to see the ");
		cp->bogus = 1;
		emit_file_href(cp, filename, "menu");
		cp->bogus = 0;
		printf("project file menu</a> instead.\n");
	    }
	    else
	    {
		printf("It may be created by the build; ");
		emit_file_href(cp, filename, 0);
		printf("if it exists</a> it is unmanaged.");
	    }
	    html_footer(0, cp);
	    return;
	}
    }
    else
    {
	src = project_file_find(cp->pp, filename, view_path_simple);
	if (!src)
	{
	    printf("This file is not controlled by Aegis.\n");
	    printf("It may have been created by the build; ");
	    emit_file_href(cp, filename, 0);
	    printf("if it exists</a> it is unmanaged.");
	    html_footer(0, cp);
	    return;
	}
    }
    assert(src);

    printf("<div class=\"information\">\n");
    printf("<dl>\n");
    if (src->uuid)
    {
	printf("<td>UUID<dd><tt>");
	html_escape_string(src->uuid);
	printf("</tt><p>\n");
    }

    if (!cp->bogus)
    {
	printf("<dt>\n");
	cp->bogus = 1;
	emit_file_href(cp, filename, "menu");
	cp->bogus = 0;
	printf("Project File</a>\n");
	printf("<dd>\n");
	printf("This item directs you to the corresponding file menu\n");
	printf("for the project.\n");
	printf("<p>\n");
    }

    printf("<dt>\n");
    emit_file_href(cp, filename, "activity");
    printf("File Activity</a>\n");
    printf("<dd>\n");
    printf("This item will provide you with a listing of changes which\n");
    printf("are actively modifying this file.  The list includes who\n");
    printf("is working on the changes, and a brief description of each\n");
    printf("change.  Where a file is common to more than one change,\n");
    printf("all changes are listed against the file.\n");
    printf("<p>\n");

    printf("<dt>\n");
    emit_file_href(cp, filename, "conflict");
    printf("File Conflict</a>\n");
    printf("<dd>\n");
    printf("This item will provide you with a listing of changes which are\n");
    printf("actively modifying this file <strong>if</strong> more than\n");
    printf("<em>one</em> change is modifying the file.\n");
    printf("<p>\n");

    printf("<dt>\n");
    hold_bogus = cp->bogus;
    cp->bogus = 1;
    emit_file_href(cp, filename, "file@history");
    cp->bogus = hold_bogus;
    printf("File History</a>\n");
    printf("<dd>\n");
    printf("This item will provide you with a listing of all completed\n");
    printf("changes which affected this file in the past.  This report\n");
    printf("can take a long time to generate.\n");
    printf("<p>\n");

    baseline_file_exists = 0;
    if (cp->bogus)
    {
	if (src->deleted_by)
	{
	    printf("<dt>\n");
	    printf("Baseline\n");
	    printf("<dd>\n");
	    printf("The file no longer exists in the baseline.\n");
	    // FIXME: add link.
	    printf("It was deleted by change %ld.", src->deleted_by);
	    printf("<p>\n");
	}
	else
	{
	    printf("<dt>\n");
	    emit_file_href(cp, filename, 0);
	    printf("Baseline</a>\n");
	    printf("<dd>\n");
	    printf("This item gives access to the file contents as it\n");
	    printf("exists in the project baseline.\n");
	    printf("<p>\n");
	    baseline_file_exists = 1;
	}
    }
    else
    {
	switch (src->action)
	{
	case file_action_create:
	    if (!change_is_completed(cp))
	    {
		printf("<dt>\n");
		printf("Baseline\n");
		printf("<dd>\n");
		printf("The file does not yet exist in the baseline.\n");
		printf("It is being created by this change, which is not\n");
		printf("yet integrated\n");
		printf("<p>\n");
		break;
	    }
	    // fall through...

	case file_action_modify:
	case file_action_remove:
	case file_action_insulate:
	case file_action_transparent:
#ifndef DEBUG
	default:
#endif
	    printf("<dt>\n");
	    cp->bogus = 1;
	    emit_file_href(cp, filename, 0);
	    cp->bogus = 0;
	    printf("Baseline</a>\n");
	    printf("<dd>\n");
	    printf("This item will show you the contents of the file in\n");
	    printf("the project baseline, the <strong>current</strong>\n");
	    printf("master version.\n");
	    printf("<p>\n");
	    baseline_file_exists = 1;
	    break;
	}
    }

#if 0
    if (!cp->bogus)
    {
	if (!change_is_completed(cp))
	{
	    printf("<dt>\n");
	    href = script_name ## "?file@pre+" ## quote_url(fn);
	    href ##= "+project@" ## quote_url(pn);
	    href ##= "+change@" ## cn;
	    print("<a href=\"" ## href ## "\">Change " ##
		change_number() ## "</a>");
	    printf("<dd>\n");
	    printf("This item will show you the contents of the file\n");
	    printf("in change " ## cn ## ".\n");
	}
	else
	{
	    auto delta;
	    delta=cs.delta_number;

	    switch (action)
	    {
	    case file_action_create:
		printf("<dt>\n");
		print("Pre Change " ## cn);
		printf("<dd>\n");
		printf("The file did not exist prior to this change.\n");
		break;

	    case file_action_modify:
	    case file_action_remove:
	    case file_action_insulate:
	    case file_action_transparent:
#ifndef DEBUG
	    default:
#endif
		auto d;
		d = delta -1;
		printf("<dt>\n");
		href = script_name ## "?file@pre+" ## "delta@" ## d ## "+";
		href ##= quote_url(fn);
		href ##= "+project@" ## quote_url(pn);
		print("<a href=\"" ## href ## "\">Pre Change " ##
		    change_number() ## "</a>");
		printf("<dd>\n");
		printf("This item will show you the how the contents\n");
		printf("of the file in the project baseline looked\n");
		printf("<strong>before</strong> change " ## cn ## "\n");
		printf("was integrated.\n");
		break;
	    }

	    printf("<dt>\n");
	    switch (action)
	    {
	    case file_action_modify:
	    case file_action_create:
	    case file_action_insulate:
	    case file_action_transparent:
#ifndef DEBUG
	    default:
#endif
		href = script_name ## "?file@pre+" ## "delta@" ## delta ## "+";
		href ##= quote_url(fn);
		href ##= "+project@" ## quote_url(pn);
		print("<a href=\"" ## href ## "\">Post Change " ##
		    change_number() ## "</a>");
		printf("<dd>\n");
		printf("This item will show you the how the contents\n");
		printf("of the file in the project baseline looked\n");
		printf("<strong>after</strong> change " ## cn ## "\n");
		printf("was integrated.\n");
		break;

	    case file_action_remove:
		print("Post Change " ## change_number());
		printf("<dd>The file was removed from the project in\n");
		printf("this change.\n");
		break;
	    }
	}
	if (cs.state != "awaiting_development")
	{
	    auto pre_d, post_d;
	    pre_d = delta -1;
	    post_d = delta;

	    printf("<dt>File Differences</dt>\n");
	    printf("<dd>This item will show you differences between the\n");
	    printf("various versions of the file.\n");
	    printf("<table align=center>\n");
	    printf("<tr>\n");
	    printf("<td></td>\n");

	    if (cs.state == "completed")
	    {
		switch (action)
		{
		case file_action_remove:
		    printf("<td width=\"100\" align=\"center\">Before</td>\n");
		    break;

		case file_action_create:
		    printf("<td width=\"100\" align=\"center\">After</td>\n");
		    break;

		case file_action_modify:
		case file_action_insulate:
		case file_action_transparent:
		    printf("<td width=\"100\" align=\"center\">Before</td>\n");
		    printf("<td width=\"100\" align=\"center\">After</td>\n");
		    break;
		}
	    }
	    else
		printf("<td width=\"100\" align=\"center\">Change</td>\n");

	    if (baseline_file_exists)
	    {
		printf("<td width=\"100\" align=\"center\">Baseline</td>\n");
	    }

	    printf("</tr>\n");

	    if (cs.state == "completed")
	    {
		switch (action)
		{
		case file_action_create:
		    break;

		case file_action_remove:
		    printf("<tr>\n");
		    printf("<td>Before</td>\n");
		    printf("<td align=\"center\">*</td>\n");

		    if (baseline_file_exists)
		    {
			href = script_name ## "?file@diff+";
			href ##= "delta1@" ## pre_d ## "+";
			href ##= "delta2@" ## "baseline" ## "+";
			href ##= quote_url(fn);
			href ##= "+project@" ## quote_url(pn);
			print("<td align=\"center\"><a href=\"" ## href ##
			    "\">diff</a></td>");
		    }
		    printf("</tr>\n");
		    break;

		case file_action_modify:
		case file_action_insulate:
		case file_action_transparent:
#ifndef DEBUG
		default:
#endif
		    printf("<tr>\n");
		    printf("<td>Before</td>\n");
		    printf("<td align=\"center\">*</td>\n");

		    href = script_name ## "?file@diff+";
		    href ##= "delta1@" ## pre_d ## "+";
		    href ##= "delta2@" ## post_d ## "+";
		    href ##= quote_url(fn);
		    href ##= "+project@" ## quote_url(pn);

		    print("<td align=\"center\"><a href=\"" ## href ##
			"\">diff</a></td>");
		    if (baseline_file_exists)
		    {
			href = script_name ## "?file@diff+";
			href ##= "delta1@" ## pre_d ## "+";
			href ##= "delta2@" ## "baseline" ## "+";
			href ##= quote_url(fn);
			href ##= "+project@" ## quote_url(pn);
			print("<td align=\"center\"><a href=\"" ## href ##
			    "\">diff</a></td>");
		    }
		    printf("</tr>\n");
		    break;
		}
		switch (action)
		{
		case file_action_remove:
		    break;

		case file_action_create:
		    printf("<tr>\n");
		    printf("<td>After</td>\n");
		    printf("<td align=\"center\">*</td>\n");
		    if (baseline_file_exists)
		    {
			href = script_name ## "?file@diff+";
			href ##= "delta1@" ## post_d ## "+";
			href ##= "delta2@" ## "baseline" ## "+";
			href ##= quote_url(fn);
			href ##= "+project@" ## quote_url(pn);
			print("<td align=\"center\"><a href=\"" ## href ##
			    "\">diff</a></td>");
		    }
		    printf("</tr>\n");
		    break;

		case file_action_modify:
		case file_action_insulate:
		case file_action_transparent:
#ifndef DEBUG
		default:
#endif
		    printf("<tr>\n");
		    printf("<td>After</td>\n");
		    href = script_name ## "?file@diff+";
		    href ##= "delta1@" ## post_d ## "+";
		    href ##= "delta2@" ## pre_d ## "+";
		    href ##= quote_url(fn);
		    href ##= "+project@" ## quote_url(pn);
		    print("<td align=\"center\"><a href=\"" ## href ##
			"\">diff</a></td>");
		    printf("<td align=\"center\">*</td>\n");
		    if (baseline_file_exists)
		    {
			href = script_name ## "?file@diff+";
			href ##= "delta1@" ## post_d ## "+";
			href ##= "delta2@" ## "baseline" ## "+";
			href ##= quote_url(fn);
			href ##= "+project@" ## quote_url(pn);
			print("<td align=\"center\"><a href=\"" ## href ##
			    "\">diff</a></td>");
		    }
		    printf("</tr>\n");
		    break;
		}
	    }
	    if (cs.state != "completed")
	    {
		printf("<tr>\n");
		printf("<td>Change</td>\n");
		printf("<td align=\"center\">*</td>\n");

		if (baseline_file_exists)
		{
		    href = script_name ## "?file@diff+";
		    href ##= "delta1@" ## "change" ## "+";
		    href ##= "delta2@" ## "baseline"  ## "+";
		    href ##= quote_url(fn);
		    href ##= "+project@" ## quote_url(pn);
		    href ##= "+change@" ## change_number();

		    print("<td align=\"center\"><a href=\"" ## href ##
			"\">diff</a></td>");
		}
		printf("</tr>\n");
	    }

	    if (baseline_file_exists)
	    {
		printf("<tr>\n");
		printf("<td>Baseline</td>\n");

		if (cs.state == "completed")
		{
		    switch (action)
		    {
		    case file_action_create:
			break;

		    case file_action_modify:
		    case file_action_remove:
		    case file_action_insulate:
		    case file_action_transparent:
#ifndef DEBUG
		    default:
#endif
			href = script_name ## "?file@diff+";
			href ##= "delta1@" ## "baseline" ## "+";
			href ##= "delta2@" ## pre_d  ## "+";
			href ##= quote_url(fn);
			href ##= "+project@" ## quote_url(pn);
			print("<td align=\"center\"><a href=\"" ## href ##
			    "\">diff</a></td>");
			break;
		    }
		    href = script_name ## "?file@diff+";
		    href ##= "delta1@" ## "baseline" ## "+";
		    href ##= "delta2@" ## post_d ## "+";
		    href ##= quote_url(fn);
		    href ##= "+project@" ## quote_url(pn);
		    print("<td align=\"center\"><a href=\"" ## href ##
			"\">diff</a></td>");
		}
		else
		{
		    href = script_name ## "?file@diff+";
		    href ##= "delta1@" ## "baseline" ## "+";
		    href ##= "delta2@" ## "change"  ## "+";
		    href ##= quote_url(fn);
		    href ##= "+project@" ## quote_url(pn);
		    href ##= "+change@" ## change_number();

		    print("<td align=\"center\"><a href=\"" ## href ##
			"\">diff</a></td>");
		}

		printf("<td align=\"center\">*</td>\n");
		printf("</tr>\n");
	    }
	    printf("</table>\n");
	    printf("</dd>\n");
	}
    }
#endif

    printf("</dl>\n");
    printf("</div>\n");

    printf("<hr>\n");
    printf("<p align=center class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(cp->pp, "menu");
    printf("Project Menu</a> |\n");
    if (!cp->bogus)
    {
	emit_change_href(cp, "menu");
	printf("Change Menu</a> |\n");
    }
    emit_change_href(cp, "files");
    printf("File List</a>\n");
    printf("]</p>\n");

    html_footer(0, cp);
}
