//
//	aegis - project change supervisor
//	Copyright (C) 2006 Peter Miller;
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
// MANIFEST: implementation of the validation_printable class
//

#include <common/ac/ctype.h>
#include <common/ac/string.h>

#include <libaegis/attribute.h>
#include <libaegis/change/file.h>
#include <libaegis/fstate.h>
#include <libaegis/input/file.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>

#include <aede-policy/validation/printable.h>


validation_printable::~validation_printable()
{
}


validation_printable::validation_printable()
{
}


bool
validation_printable::run(change_ty *cp)
{
    bool result = true;
    for (size_t j = 0; ; ++j)
    {
	fstate_src_ty *src = change_file_nth(cp, j, view_path_first);
	if (!src)
	    break;
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


static bool
international_character_set(fstate_src_ty *src)
{
    //
    // If the content-type attribute doesn't exist, or no character set
    // is specified by the content-type, plain ascii is assumed.
    //
    attributes_ty *ap = attributes_list_find(src->attribute, "content-type");
    if (!ap || !ap->value)
	return false;
    const char *cp = strstr(ap->value->str_text, "charset=");
    if (!cp)
	return false;
    cp += 8;
    return (0 != strcmp(cp, "ascii") && 0 != strcmp(cp, "us-ascii"));
}


static bool
is_a_text_file(fstate_src_ty *src)
{
    attributes_ty *ap = attributes_list_find(src->attribute, "content-type");
    if (!ap || !ap->value)
	return true;
    const char *cp = ap->value->str_text;
    return (0 == memcmp(cp, "text", 4) && (cp[4] == 0 || cp[4] == '/'));
}


bool
validation_printable::check(change_ty *cp, fstate_src_ty *src)
    const
{
    //
    // Examine the Content-Type and don't check non-text files.
    //
    if (is_a_text_file(src))
	return true;

    //
    // Note: we return true if the file is acceptable to the policy
    // (i.e. has no unprintable characters).  We return false if the file is
    // unacceptable.
    //
    string_ty *path = change_file_path(cp, src);
    // assert(path);
    if (!path)
	return true;

    os_become_orig();
    bool result = true;
    input ip = input_file_open(path);
    if
    (
	nstring(src->file_name).ends_with(".po")
    ||
	international_character_set(src)
    )
    {
	//
        // An obvious exception is the message translation files, which
        // are supposed to contain character sets other than ASCII.
	//
	for (;;)
	{
	    int c = ip->getch();
	    if (c < 0)
		break;
	    if (c == 0)
	    {
		sub_context_ty sc;
		sc.var_set_string("File_Name", src->file_name);
		change_error(cp, &sc, i18n("$filename: is binary"));
		result = false;
		break;
	    }
	}
    }
    else
    {
	for (;;)
	{
	    int c = ip->getch();
	    if (c < 0)
		break;
	    if (c == 0)
	    {
		sub_context_ty sc;
		sc.var_set_string("File_Name", src->file_name);
		change_error(cp, &sc, i18n("$filename: is binary"));
		result = false;
		break;
	    }
	    unsigned char uc = c;
	    // Using the "C" locale.
	    if (!isprint(uc) && !isspace(uc))
	    {
		sub_context_ty sc;
		sc.var_set_string("File_Name", src->file_name);
		change_error(cp, &sc, i18n("$filename: is unprintable"));
		result = false;
		break;
	    }
	}
    }
    ip.close();
    os_become_undo();
    return result;
}
