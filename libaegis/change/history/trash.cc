//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001-2008 Peter Miller
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

#include <libaegis/change.h>
#include <common/str_list.h>
#include <libaegis/sub.h>


//
// NAME
//	change_history_trashed_fingerprints
//
// SYNOPSIS
//	void change_history_trashed_fingerprints(change::pointer cp,
//	    string_list_ty *);
//
// DESCRIPTION
//	Many history tools (e.g. RCS) can modify the contents of the file
//	when it is committed.  While there are usually options to turn
//	this off, they are seldom used.  The problem is: if the commit
//	changes the file, the source in the repository now no longer
//	matches the object file in the repository - i.e. the history
//	tool has compromised the referential integrity of the repository.
//
//	Check here if this is the case, and emit an error message if so.
//	(It could be a fatal error, just a warning, or completely ignored,
//	depending on the history_put_trashes_file field of the project
//	config file.
//

void
change_history_trashed_fingerprints(change::pointer cp, string_list_ty *slp)
{
    pconf_ty        *config;
    size_t	    j;
    sub_context_ty  *scp;

    if (slp->nstrings < 1)
	return;
    config = change_pconf_get(cp, 0);
    if
    (
	config->history_put_trashes_file
    ==
	pconf_history_put_trashes_file_ignore
    )
	return;
    for (j = 0; j < slp->nstrings; ++j)
    {
	string_ty	*fn;

	fn = slp->string[j];
	scp = sub_context_new();
	sub_var_set_string(scp, "File_Name", fn);
	switch (config->history_put_trashes_file)
	{
	case pconf_history_put_trashes_file_ignore:
	    break;

	case pconf_history_put_trashes_file_warn:
	    change_verbose
	    (
	       	cp,
	       	scp,
	       	i18n("warning: $filename modified by history tool")
	    );
	    break;

	case pconf_history_put_trashes_file_fatal:
	    change_error
	    (
	       	cp,
	       	scp,
	       	i18n("$filename modified by history tool")
	    );
	    break;
	}
	sub_context_delete(scp);
    }
    scp = sub_context_new();
    sub_var_set_long(scp, "Number", (long)slp->nstrings);
    sub_var_optional(scp, "Number");
    switch (config->history_put_trashes_file)
    {
    case pconf_history_put_trashes_file_ignore:
	break;

    case pconf_history_put_trashes_file_warn:
	change_verbose
	(
    	    cp,
    	    scp,
    	    i18n("warning: files modified by history tool")
	);
	break;

    case pconf_history_put_trashes_file_fatal:
	change_fatal(cp, scp, i18n("files modified by history tool"));
	break;
    }
    sub_context_delete(scp);
}
