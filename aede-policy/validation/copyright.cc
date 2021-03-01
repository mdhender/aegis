//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006 Peter Miller;
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
// MANIFEST: implementation of the validation_copyright class
//

#include <common/ac/string.h>
#include <common/ac/time.h>

#include <common/error.h>
#include <common/now.h>
#include <common/nstring/accumulator.h>
#include <libaegis/attribute.h>
#include <libaegis/change/branch.h>
#include <libaegis/change/file.h>
#include <libaegis/cstate.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>

#include <aede-policy/validation/copyright.h>


validation_copyright::~validation_copyright()
{
}


static nstring
calc_year()
{
    time_t when = now();
    struct tm *tmp = localtime(&when);
    char buffer[10];
    strftime(buffer, sizeof(buffer), "%Y", tmp);
    return nstring(buffer);
}


validation_copyright::validation_copyright() :
    year(calc_year())
{
}


/**
  * The was_downloaded function is used to determine whether or not a
  * change was downloaded via aedist.  This is determined by looking for
  * the "Download" comment inserted by aedist.
  *
  * @param cp
  *     The change in question.
  * @returns
  *     bool; true is was downloaded, false if not.
  */
static bool
was_downloaded(change_ty *cp)
{
    cstate_history_list_ty *hlp = change_cstate_get(cp)->history;
    assert(hlp);
    if (!hlp)
	return false;
    for (size_t j = 0; j < hlp->length; ++j)
    {
	cstate_history_ty *hp = hlp->list[j];
	if
       	(
	    hp->what == cstate_history_what_develop_begin
	&&
	    (hp->why && 0 == memcmp(hp->why->str_text, "Downloaded", 10))
	)
	    return true;
    }
    return false;
}


bool
validation_copyright::run(change_ty *cp)
{
    //
    // Don't check branches, only individual changes have control over
    // the copyright year in their source files.
    //
    if (change_was_a_branch(cp))
	return true;

    //
    // Don't perform this check for changes downloaded and applied by
    // aedist, because the original developer is no longer in control.
    //
    if (was_downloaded(cp))
	return true;

    //
    // Don't perform this check for change sets marked as owning a
    // foreign copyright.
    //
    if (cp->attributes_get_boolean("foreign-copyright"))
	return true;

    //
    // We can only know the copyright owner one we know the change set,
    // because it be changed by a change set.
    //
    who = cp->pconf_copyright_owner_get();
    suggest = "Copyright (C) " + year + " " + who;

    //
    // Check each file in the change set.
    //
    bool result = true;
    for (size_t j = 0; ; ++j)
    {
	fstate_src_ty *src = change_file_nth(cp, j, view_path_first);
	if (!src)
	    break;

	//
	// Don't perform this check for files marked as owning a
	// foreign copyright.
	//
	if (attributes_list_find_boolean(src->attribute, "foreign-copyright"))
	    continue;

	switch (src->action)
	{
	case file_action_create:
	case file_action_modify:
	    if (!check(cp, src))
		result = false;
	    break;

	case file_action_remove:
	case file_action_insulate:
	case file_action_transparent:
	    break;
	}
    }
    return result;
}


bool
validation_copyright::check(change_ty *cp, fstate_src_ty *src)
    const
{
    string_ty *path = change_file_path(cp, src);
    assert(path);
    if (!path)
	return true;

    os_become_orig();
    bool result = true;
    bool copyright_seen = false;
    bool public_domain_seen = false;
    input ip = input_file_open(path);
    nstring_accumulator sa;
    for (;;)
    {
	sa.clear();
	for (;;)
	{
	    int c = ip->getch();
	    if (c < 0)
	    {
		if (sa.empty())
		    goto end_of_file;
		break;
	    }
	    if (c == 0)
	    {
		//
                // Binary files are exempt from the copyright check.
                // Image formats usually have a place to put a copyright
                // notice, but there is no good way to check this.
		//
		ip.close();
		os_become_undo();
		return true;
	    }
	    if (c == '\n')
		break;
	    sa.push_back((unsigned char)c);
	}
	nstring line = sa.mkstr();
	const char *s = strstr(line.c_str(), "Copyright (C)");
	if
       	(
	    s
	&&
	    (
		(strstr(s, year.c_str()) && strstr(s, who.c_str()))
	    ||
		strstr(s, "${date %Y}")
	    )
	)
	    copyright_seen = true;
	if (strstr(line.c_str(), "Public Domain"))
	    public_domain_seen = true;
    }
end_of_file:
    ip.close();
    os_become_undo();
    if (!copyright_seen && !public_domain_seen)
    {
	sub_context_ty sc;
	sc.var_set_string("File_Name", src->file_name);
	sc.var_set_string("Suggest", suggest);
	sc.var_optional("Suggest");
	change_error(cp, &sc, i18n("$filename: no current copyright notice"));
	result = false;
    }
    return result;
}
