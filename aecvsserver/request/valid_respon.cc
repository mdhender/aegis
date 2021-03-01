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
// MANIFEST: functions to manipulate valid_respons
//
//
// Valid-responses <list-of-names>
//
// Tell the server what responses the client will accept.
// The <list-of-names> is a space separated list of tokens.
//
// Response expected: no.
// Root required: no.
//

#include <ac/string.h>

#include <output.h>
#include <server.h>
#include <str_list.h>
#include <request/valid_respon.h>
#include <response/private.h>


static int
find_response_code(const char *name)
{
    struct table_ty
    {
	const char      *name;
	response_code_ty code;
    };

    static const table_ty table[] =
    {
	{ "Checked-in", response_code_Checked_in },
	{ "Checksum", response_code_Checksum },
	{ "Clear-static-directory", response_code_Clear_static_directory },
	{ "Clear-sticky", response_code_Clear_sticky },
	{ "Copy-file", response_code_Copy_file },
	{ "Created", response_code_Created },
	{ "E", response_code_E },
	{ "error", response_code_error },
	{ "F", response_code_F },
	{ "Mbinary", response_code_Mbinary },
	{ "Merged", response_code_Merged },
	{ "Mode", response_code_Mode },
	{ "Mod-time", response_code_Mod_time },
	{ "Module-expansion", response_code_Module_expansion },
	{ "M", response_code_M },
	{ "MT", response_code_MT },
	{ "New-entry", response_code_New_entry },
	{ "Notified", response_code_Notified },
	{ "ok", response_code_ok },
	{ "Patched", response_code_Patched },
	{ "Rcs-diff", response_code_Rcs_diff },
	{ "Removed", response_code_Removed },
	{ "Remove-entry", response_code_Remove_entry },
	{ "Set-checkin-prog", response_code_Set_checkin_prog },
	{ "Set-static-directory", response_code_Set_static_directory },
	{ "Set-sticky", response_code_Set_sticky },
	{ "Set-update-prog", response_code_Set_update_prog },
	{ "Template", response_code_Template },
	{ "Updated", response_code_Updated },
	{ "Update-existing", response_code_Update_existing },
	{ "Valid-requests", response_code_Valid_requests },
	{ "Wrapper-rcsOption", response_code_Wrapper_rcsOption },
    };

    const table_ty  *tp;

    for (tp = table; tp < ENDOF(table); ++tp)
    {
	if (0 == strcmp(tp->name, name))
	    return tp->code;
    }
    return -1;
}


static void
run(server_ty *sp, string_ty *arg)
{
    string_list_ty  wl;
    size_t          j;

    str2wl(&wl, arg, 0, 0);
    for (j = 0; j < wl.nstrings; ++j)
    {
	int             code;
	const char      *name;

	name = wl.string[j]->str_text;
	code = find_response_code(name);
	if (code >= 0)
	    sp->np->response_valid[code] = 1;
    }
    string_list_destructor(&wl);
}


const request_ty request_valid_responses =
{
    "Valid-responses",
    run,
    0, // no reset
};
