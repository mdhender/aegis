//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2000, 2002-2005 Peter Miller;
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
// MANIFEST: functions to manipulate templates
//

#include <libaegis/change.h>
#include <libaegis/change/env_set.h>
#include <libaegis/change/file.h>
#include <common/error.h> // for assert
#include <libaegis/file.h>
#include <common/gmatch.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/user.h>


static pconf_file_template_ty *
find(change_ty *cp, string_ty *file_name)
{
    pconf_file_template_ty *result;
    size_t	    j;
    size_t          k;
    pconf_ty        *pconf_data;

    trace(("change_file_template_string(file_name = \"%s\")\n{\n",
	file_name->str_text));
    assert(cp->reference_count>=1);
    result = 0;
    pconf_data = change_pconf_get(cp, 0);
    if (!pconf_data->file_template)
	goto done;
    for (j = 0; j < pconf_data->file_template->length; ++j)
    {
	pconf_file_template_ty *ftp;

	ftp = pconf_data->file_template->list[j];
	if (!ftp->pattern)
	    continue;
	for (k = 0; k < ftp->pattern->length; ++k)
	{
	    int		    m;
	    string_ty	    *s;

	    s = ftp->pattern->list[k];
	    m = gmatch(s->str_text, file_name->str_text);
	    if (m < 0)
	    {
		sub_context_ty	*scp;

		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", s);
		change_fatal(cp, scp, i18n("bad pattern $filename"));
		// NOTREACHED
		sub_context_delete(scp);
	    }
	    if (m)
	    {
		result = ftp;
		goto done;
	    }
	}
    }

    //
    // here for all exits
    //
    done:
    trace(("return %08lX;\n", (long)result));
    trace(("}\n"));
    return result;
}


void
change_file_template(change_ty *cp, string_ty *filename, user_ty *up,
    int use_template)
{
    string_ty	    *dd;
    string_ty	    *path;
    pconf_file_template_ty *tp;
    sub_context_ty  *scp;

    //
    // figure the absolute path of the file
    //
    trace(("change_file_template(name = \"%s\")\n{\n", filename->str_text));
    assert(cp->reference_count>=1);
    dd = change_development_directory_get(cp, 0);
    path = os_path_join(dd, filename);

    //
    // If the file exists, do not over-write what the user has
    // already written.
    //
    user_become(up);
    os_mkdir_between(dd, filename, 02755);
    bool exists = false;
    if (os_symlink_query(path))
    {
        //
        // Source files can't be symbolic links, and we are about to
        // create a source file, so remove any symlink that may be
        // present.  It's probably left-over from the development
        // directory style.
	//
	os_unlink(path);
    }
    else
    {
	//
        // Test to see if the file exists.
	//
	exists = os_exists(path);
    }

    user_become_undo();
    int keep = 0;
    if (exists)
	keep = user_delete_file_query(up, filename, 0);

    //
    // The logic here is controlled by the following truth table:
    //
    // exists keep use_template | action
    // -------------------------+-------
    //   0      X        0      | create, empty
    //   0      X        !=0    | create, template
    //   1      0        0      | create, empty
    //   1      0        !=0    | create, template
    //   1      1        X      | do nothing
    //
    if (!exists || !keep)
    {
	//
	// Find the template to be used to construct the new file.
	//
	tp = use_template ? find(cp, filename) : 0;
	if (tp && tp->body_command)
	{
	    int		    flags;
	    string_ty	    *the_command;

	    //
	    // Build the command to be executed.
	    //
	    scp = sub_context_new();
	    sub_var_set_string(scp, "File_Name", filename);
	    the_command = substitute(scp, cp, tp->body_command);
	    sub_context_delete(scp);

	    //
	    // Execute the command.
	    //
	    flags = OS_EXEC_FLAG_NO_INPUT;
	    change_env_set(cp, 1);
	    user_become(up);
	    if (exists)
		os_unlink_errok(path);
	    os_execute(the_command, flags, dd);
	    exists = os_exists(path);
	    user_become_undo();
	    str_free(the_command);

	    //
	    // It is an error if the command didn't create
	    // the file.
	    //
	    if (!exists)
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", filename);
		change_fatal(cp, scp, i18n("new file $filename not created"));
		// NOTREACHED
		sub_context_delete(scp);
	    }
	}
	else
	{
	    string_ty	    *body;
	    int		    mode;

	    //
	    // There is no command, it must be a string template.
	    //
	    if (!tp)
		body = str_from_c("");
	    else
	    {
		scp = sub_context_new();
		sub_var_set_string(scp, "File_Name", filename);
		sub_var_optional(scp, "File_Name");
		body = substitute(scp, cp, tp->body);
		sub_context_delete(scp);
	    }

	    //
	    // Now we have the string, write it to the file.
	    //
	    mode = 0644 & ~change_umask(cp);
	    user_become(up);
	    file_from_string(path, body, mode);
	    user_become_undo();
	    str_free(body);
	}
    }
    str_free(path);
    trace(("}\n"));
}
