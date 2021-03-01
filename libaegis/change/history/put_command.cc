//
//	aegis - project change supervisor
//	Copyright (C) 1999-2001, 2003-2008 Peter Miller
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
#include <libaegis/change/env_set.h>
#include <libaegis/change/file.h>
#include <libaegis/change/history/encode.h>
#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>
#include <common/trace.h>


void
change_run_history_put_command(change::pointer cp, fstate_src_ty *src)
{
    sub_context_ty  *scp;
    string_ty       *hp;
    pconf_ty        *pconf_data;
    string_ty       *the_command;
    time_t          mtime;
    string_ty       *name_of_encoded_file;
    int             unlink_encoded_file;
    string_ty       *hfn;
    string_ty       *hfnr;

    //
    // Update and existing history.  Only ever happens in the
    // "being integrated" state.  Current directory will be set to
    // the base of the history tree.  All of the substitutions
    // described in aesub(5) are avaliable.  In addition:
    //
    // ${Input}
    //	absolute path of the source file to check-in
    //
    // ${History}
    //	absolute path of the history file
    //
    // ${File_Name}   (Optional)
    //  the base relative file name of the file for this check-in.
    //  Note that the filename can vary, but the history file never does.
    //  DO NOT use this as the name of the history file.
    //
    // ${UUID}        (Optional)
    //  The universally unique identifier for this file.  This is
    //  invariant for the lifetime of the file.
    //  DO NOT use this as the name of the history file.
    //
    trace(("change_run_history_put_command(cp = %8.8lX, "
	"filename = \"%s\")\n{\n", (long)cp, src->file_name->str_text));
    assert(cp->reference_count >= 1);

    //
    // Inform the user of what we are doing, just in case they can't
    // tell from the file names.  For example, with a base-64 encoded
    // input file and a UUID history file the user has no way of knowing
    // which source file this concerns.
    //
    scp = sub_context_new();
    scp->var_set_string("File_Name", src->file_name);
    change_error(cp, scp, i18n("history put $filename"));
    sub_context_delete(scp);
    scp = 0;

    //
    // See if the file is binary, and if we need to encode it in any way.
    // If the file is encoded, it will be the absolute path of
    // the encoded file.  If the file is not encoded it will be the
    // absolute path of the source file.
    //
    name_of_encoded_file = change_history_encode(cp, src, &unlink_encoded_file);

    //
    // Get the path of the source file.
    //
    scp = sub_context_new();
    sub_var_set_string(scp, "Input", name_of_encoded_file);

    //
    // Get the path of the history file.
    //
    hp = cp->pp->history_path_get();
    hfn = project_history_filename_get(cp->pp, src);
    sub_var_set_string(scp, "History", hfn);
    hfnr = os_below_dir(hp, hfn);
    str_free(hfn);

    //
    // Some optional variables, for history tool meta-data.
    //
    scp->var_set_string("File_Name", src->file_name);
    scp->var_optional("File_Name");
    scp->var_set_string
    (
	"Universally_Unique_IDentifier",
	(src->uuid ? src->uuid : src->file_name)
    );
    scp->var_optional("Universally_Unique_IDentifier");

    //
    // Make sure the command has been set.
    //
    pconf_data = change_pconf_get(cp, 1);
    the_command = pconf_data->history_put_command;
    if (!the_command)
    {
	sub_context_ty  *scp2;

	assert(pconf_data->errpos);
	scp2 = sub_context_new();
	sub_var_set_string(scp2, "File_Name", pconf_data->errpos);
	sub_var_set_charstar(scp2, "FieLD_Name", "history_get_command");
	change_fatal
	(
	    cp,
	    scp2,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	// NOTREACHED
	sub_context_delete(scp2);
    }

    //
    // Perform the substitutions and run the command.
    //
    the_command = substitute(scp, cp, the_command);
    sub_context_delete(scp);
    change_env_set(cp, 0);
    project_become(cp->pp);
    os_mkdir_between(hp, hfnr, 02755);
    str_free(hfnr);
    mtime = os_mtime_actual(name_of_encoded_file);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, hp);

    //
    // Reset the file modification time: many history commands
    // gratuitously touch the file.
    //
    os_mtime_set(name_of_encoded_file, mtime);
    str_free(the_command);

    //
    // If there was an encoded file, get rid of it.
    //
    assert(src->edit);
    if (unlink_encoded_file)
	os_unlink(name_of_encoded_file);
    project_become_undo(cp->pp);
    str_free(name_of_encoded_file);

    //
    // Read the head revision from the file.
    //
    assert(src->edit);
    src->edit->revision = change_run_history_query_command(cp, src);
    trace(("}\n"));
}
