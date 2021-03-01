//
//	aegis - project change supervisor
//	Copyright (C) 2004 Peter Miller;
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
// MANIFEST: functions to manipulate staffs
//

#include <ac/stdio.h>

#include <change.h>
#include <error.h> // for assert
#include <get/project/staff.h>
#include <emit/project.h>
#include <http.h>
#include <mem.h>
#include <project.h>
#include <project/history.h>
#include <str_list.h>
#include <symtab_iter.h>
#include <user.h>


#define BAR_WIDTH 50
#define BAR_HEIGHT 12


typedef struct staff_role_ty staff_role_ty;
struct staff_role_ty
{
    int             enabled;
    long            count;
};

typedef struct staff_member_ty staff_member_ty;
struct staff_member_ty
{
    staff_role_ty   developer;
    staff_role_ty   reviewer;
    staff_role_ty   integrator;
    staff_role_ty   admin;
};


static void
staff_member_constructor(staff_member_ty *smp)
{
    smp->developer.enabled = 0;
    smp->developer.count = 0;
    smp->reviewer.enabled = 0;
    smp->reviewer.count = 0;
    smp->integrator.enabled = 0;
    smp->integrator.count = 0;
    smp->admin.enabled = 0;
    smp->admin.count = 0;
}


static staff_member_ty *
staff_member_new(void)
{
    staff_member_ty *smp;

    smp = (staff_member_ty *)mem_alloc(sizeof(staff_member_ty));
    staff_member_constructor(smp);
    return smp;
}


static void
staff_member_delete(staff_member_ty *smp)
{
    mem_free(smp);
}


static void
reaper(void *p)
{
    staff_member_ty *smp;

    smp = (staff_member_ty *)p;
    staff_member_delete(smp);
}


static const char *
flag(int x)
{
    return (x ? "Yes" : "No");
}


static staff_member_ty *
staff_member_find(symtab_ty *stp, string_ty *key)
{
    staff_member_ty *mp;

    mp = (staff_member_ty *)symtab_query(stp, key);
    if (!mp)
    {
	mp = staff_member_new();
	symtab_assign(stp, key, mp);
    }
    return mp;
}


void
get_project_staff(project_ty *pp, string_ty *fn, string_list_ty *modifier)
{
    double          scale;
    cstate_branch_ty *bp;
    change_ty       *cp;
    long            number_of_changes = 0;
    size_t          n;
    cstate_ty       *cstate_data;
    symtab_ty       *stp;
    staff_member_ty total;
    staff_member_ty max;
    long            tmax;
    string_list_ty  keys;
    string_ty       *key;
    void            *dp;
    symtab_iterator it;
    staff_member_ty *smp;
    size_t          j;

    html_header(pp);
    printf("<title>Project ");
    html_encode_string(project_name_get(pp));
    printf(" Staff</title></head>\n<body><h1 align=center>\n");
    emit_project(pp);
    printf(",<br>\nList of Staff</h1>\n");
    printf("<div class=\"information\">\n");

    printf("This page provides infomation about staff assigned to\n");
    printf("the project, and the permissions they have top perform\n");
    printf("each role.  Statistics are provided about how many\n");
    printf("times these roles have been exersized.\n");

    stp = symtab_alloc(5);
    stp->reap = reaper;

    //
    // Set the enabled flag for active staff members.
    //
    for (j = 0; ; ++j)
    {
	key = project_administrator_nth(pp, j);
	if (!key)
	    break;
	smp = staff_member_find(stp, key);
	smp->admin.enabled = 1;
    }
    for (j = 0; ; ++j)
    {
	key = project_developer_nth(pp, j);
	if (!key)
	    break;
	smp = staff_member_find(stp, key);
	smp->developer.enabled = 1;
    }
    for (j = 0; ; ++j)
    {
	key = project_reviewer_nth(pp, j);
	if (!key)
	    break;
	smp = staff_member_find(stp, key);
	smp->reviewer.enabled = 1;
    }
    for (j = 0; ; ++j)
    {
	key = project_integrator_nth(pp, j);
	if (!key)
	    break;
	smp = staff_member_find(stp, key);
	smp->integrator.enabled = 1;
    }

    //
    // traverse each change
    //
    staff_member_constructor(&total);
    staff_member_constructor(&max);
    cp = project_change_get(pp);
    cstate_data = change_cstate_get(cp);
    bp = cstate_data->branch;
    assert(bp);
    if (!bp->change)
    {
	bp->change =
	    (cstate_branch_change_list_ty *)
	    cstate_branch_change_list_type.alloc();
    }
    for (n = 0; n < bp->change->length; ++n)
    {
	long            change_number;
	cstate_history_list_ty *hlp;

	change_number = bp->change->list[n];
	cp = change_alloc(pp, change_number);
	change_bind_existing(cp);
	cstate_data = change_cstate_get(cp);

	hlp = cstate_data->history;
	if (hlp)
	{
	    size_t          k;

	    for (k = 0; k < hlp->length; ++k)
	    {
		cstate_history_ty *hp;

		hp = hlp->list[k];
		smp = staff_member_find(stp, hp->who);
		switch (hp->what)
		{
		case cstate_history_what_new_change:
		    smp->admin.count++;
		    if (max.admin.count < smp->admin.count)
			max.admin.count = smp->admin.count;
		    total.admin.count++;
		    break;

		case cstate_history_what_develop_begin:
		case cstate_history_what_develop_begin_undo:
		case cstate_history_what_develop_end:
		case cstate_history_what_develop_end_2ar:
		case cstate_history_what_develop_end_2ai:
		case cstate_history_what_develop_end_undo:
		    smp->developer.count++;
		    if (max.developer.count < smp->developer.count)
			max.developer.count = smp->developer.count;
		    total.developer.count++;
		    break;

		case cstate_history_what_review_begin:
		case cstate_history_what_review_begin_undo:
		case cstate_history_what_review_pass:
		case cstate_history_what_review_pass_undo:
		case cstate_history_what_review_fail:
		    smp->reviewer.count++;
		    if (max.reviewer.count < smp->reviewer.count)
			max.reviewer.count = smp->reviewer.count;
		    total.reviewer.count++;
		    break;

		case cstate_history_what_integrate_begin:
		case cstate_history_what_integrate_begin_undo:
		case cstate_history_what_integrate_pass:
		case cstate_history_what_integrate_fail:
		    smp->integrator.count++;
		    if (max.integrator.count < smp->integrator.count)
			max.integrator.count = smp->integrator.count;
		    total.integrator.count++;
		    break;
		}
		smp = 0;
	    }
	}
	number_of_changes++;
	change_free(cp);
	cp = 0;
    }
    tmax = max.admin.count;
    if (tmax < max.developer.count)
	tmax = max.developer.count;
    if (tmax < max.reviewer.count)
	tmax = max.reviewer.count;
    if (tmax < max.integrator.count)
	tmax = max.integrator.count;
    scale = (tmax ? (double)BAR_WIDTH / tmax : 1);

    //
    // Extract the names from the symbol table.
    // Sort them for predictable results.
    //
    symtab_iterator_constructor(&it, stp);
    string_list_constructor(&keys);
    while (symtab_iterator_next(&it, &key, &dp))
	string_list_append(&keys, key);
    string_list_sort(&keys);

    //
    // Print statistics about staff.
    //
    printf("<table align=center>\n");
    printf("<tr class=\"even-group\"><th>User</th>\n");
    printf("<th colspan=3>Developer</th>");
    printf("<th colspan=3>Reviewer</th>\n");
    printf("<th colspan=3>Integrator</th>\n");
    printf("<th colspan=3>Administrator</th></tr>\n");
    for (n = 0; n < keys.nstrings; ++n)
    {
	user_ty         *up;
	int             width;

	key = keys.string[n];
	smp = (staff_member_ty *)symtab_query(stp, key);
	assert(smp);
	if (!smp)
	    continue;

	printf("<tr class=\"%s-group\">\n", (j % 6 < 3 ? "odd" : "even"));
	printf("<td valign=top>\n");
	printf("<a href=\"mailto:");
	up = user_symbolic(pp, key);
	html_escape_string(user_email_address(up));
	user_free(up);
	printf("\">");
	html_encode_string(key);
	printf("</a></td>\n");

	printf("<td valign=top>%s</td>\n", flag(smp->developer.enabled));
	printf("<td valign=top align=right>%ld</td>\n", smp->developer.count);
	printf("<td valign=top width=%d>\n", BAR_WIDTH);
	width = (int)(0.5 + scale * smp->developer.count);
	emit_rect_image(width, BAR_HEIGHT, 0);
	printf("</td>\n");

	printf("<td valign=top>%s</td>\n", flag(smp->reviewer.enabled));
	printf("<td valign=top align=right>%ld</td>\n", smp->reviewer.count);
	printf("<td valign=top width=%d>\n", BAR_WIDTH);
	width = (int)(0.5 + scale * smp->reviewer.count);
	emit_rect_image(width, BAR_HEIGHT, 0);
	printf("</td>\n");

	printf("<td valign=top>%s</td>\n", flag(smp->integrator.enabled));
	printf("<td valign=top align=right>%ld</td>\n", smp->integrator.count);
	printf("<td valign=top width=%d>\n", BAR_WIDTH);
	width = (int)(0.5 + scale * smp->integrator.count);
	emit_rect_image(width, BAR_HEIGHT, 0);
	printf("</td>\n");

	printf("<td valign=top>%s</td>\n", flag(smp->admin.enabled));
	printf("<td valign=top align=right>%ld</td>\n", smp->admin.count);
	printf("<td valign=top width=%d>\n", BAR_WIDTH);
	width = (int)(0.5 + scale * smp->admin.count);
	emit_rect_image(width, BAR_HEIGHT, 0);
	printf("</td></tr>\n");
    }
    printf("<tr class=\"even-group\">\n");
    printf("<td colspan=13>Listed %ld staff.</td>\n", (long)keys.nstrings);
    printf("</tr></table></div>\n");
    symtab_free(stp);

    printf("<hr>\n");
    printf("<div class=\"report-cmd\">\n");
    printf("A similar report may be obtained from the command line, with\n");
    printf("<blockquote><pre>aer proj_staff -p ");
    html_encode_string(project_name_get(pp));
    printf("</pre></blockquote>\n");
    printf("</div>\n");

    printf("<hr>\n");
    printf("<p align=center class=\"navbar\">[\n");
    printf("<a href=\"%s/\">Project List</a> |\n", http_script_name());
    emit_project_href(pp, "menu");
    printf("Project Menu</a>\n");
    printf("]</p>\n");

    html_footer();
}