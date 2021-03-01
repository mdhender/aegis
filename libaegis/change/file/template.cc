//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2000, 2002-2006 Peter Miller;
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

#include <common/error.h> // for assert
#include <common/gmatch.h>
#include <common/trace.h>
#include <libaegis/change/env_set.h>
#include <libaegis/change/file.h>
#include <libaegis/change.h>
#include <libaegis/file.h>
#include <libaegis/os.h>
#include <libaegis/project/file.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


static pconf_file_template_ty *
find(change_ty *cp, string_ty *file_name)
{
    trace(("change_file_template_string(file_name = \"%s\")\n{\n",
	file_name->str_text));
    assert(cp->reference_count>=1);
    pconf_file_template_ty *result = 0;
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    if (!pconf_data->file_template)
	goto done;
    for (size_t j = 0; j < pconf_data->file_template->length; ++j)
    {
	pconf_file_template_ty *ftp = pconf_data->file_template->list[j];
	if (!ftp->pattern)
	    continue;
	for (size_t k = 0; k < ftp->pattern->length; ++k)
	{
	    string_ty *s = ftp->pattern->list[k];
	    int m = gmatch(s->str_text, file_name->str_text);
	    if (m < 0)
	    {
		sub_context_ty sc;
		sc.var_set_string("File_Name", s);
		change_fatal(cp, &sc, i18n("bad pattern $filename"));
		// NOTREACHED
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


static bool
painful_development_directory_style(change_ty *cp)
{
    trace(("painful_development_directory_style(cp = %08lX)\n{\n", (long)cp));
    pconf_ty *pconf_data = change_pconf_get(cp, 0);
    bool result =
	(
	    pconf_data
	&&
	    pconf_data->development_directory_style
	&&
	    (
		pconf_data->development_directory_style->source_file_link
	    ||
		pconf_data->development_directory_style->source_file_copy
	    )
	);
    trace(("return %d\n", result));
    trace(("}\n"));
    return result;
}


void
change_file_template(change_ty *cp, string_ty *filename, user_ty *up,
    int use_template)
{
    trace(("change_file_template(cp = %08lX, filename = \"%s\", ip = %08lX, "
	"use_template = %d)\n{\n", (long)cp, filename->str_text, (long)up,
	use_template));

    //
    // figure the absolute path of the file
    //
    trace(("change_file_template(name = \"%s\")\n{\n", filename->str_text));
    assert(cp->reference_count>=1);
    string_ty *dd = change_development_directory_get(cp, 0);
    string_ty *path = os_path_join(dd, filename);

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
	user_become_undo();
    }
    else
    {
	//
        // Test to see if the file exists.
        //
	trace(("path = \"%s\"\n", path->str_text));
	exists = os_exists(path);
	user_become_undo();

	//
        // If the development directory style makes hard links or
        // copies, we have a problem.  We have to distinguish between
        // something the user edited, and wants to keep, from a copy of
        // a baseline file (which may be obsolete), or a hard link to a
        // baseline file (which may be been changed in the baseline, and
        // now the link count is one).
	//
	if
       	(
	    exists
	&&
	    painful_development_directory_style(cp)
	&&
	    project_file_find(cp->pp, filename, view_path_extreme)
	)
	{
	    //
            // At present, we will err on the side of caution, and
            // discard the file in the development directory, if the
            // development directory style permits these kinds of
            // artifacts.
	    //
	    user_become(up);
	    os_unlink(path);
	    user_become_undo();
	    exists = false;
	}
    }
    trace(("exists = %d\n", exists));

    int keep = 0;
    if (exists)
    {
	trace(("Should we keep it?\n"));
	keep = !user_delete_file_query(up, filename, false, false);
    }
    trace(("keep = %d\n", keep));

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
	pconf_file_template_ty *tp = use_template ? find(cp, filename) : 0;
	trace(("tp = %08lX\n", (long)tp));
	if (tp && tp->body_command)
	{
	    //
	    // Build the command to be executed.
	    //
	    sub_context_ty sc;
	    sc.var_set_string("File_Name", filename);
	    string_ty *the_command = sc.substitute(cp, tp->body_command);

	    //
	    // Execute the command.
	    //
	    int flags = OS_EXEC_FLAG_NO_INPUT;
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
		sub_context_ty err;
		err.var_set_string("File_Name", filename);
		change_fatal(cp, &err, i18n("new file $filename not created"));
		// NOTREACHED
	    }
	}
	else
	{
	    //
	    // There is no command, it must be a string template.
	    //
	    string_ty *body = 0;
	    if (!tp)
		body = str_from_c("");
	    else
	    {
		sub_context_ty sc;
		sc.var_set_string("File_Name", filename);
		sc.var_optional("File_Name");
		body = sc.substitute(cp, tp->body);
	    }

	    //
	    // Now we have the string, write it to the file.
	    //
	    int mode = 0644 & ~change_umask(cp);
	    user_become(up);
	    file_from_string(path, body, mode);
	    user_become_undo();
	    str_free(body);
	}
    }
    str_free(path);
    trace(("}\n"));
}
