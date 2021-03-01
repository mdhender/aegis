//
//      aegis - project change supervisor
//      Copyright (C) 1999-2009, 2012 Peter Miller
//      Copyright (C) 2008, 2009 Walter Franzini
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <common/ac/assert.h>

#include <common/str.h>
#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/change/env_set.h>
#include <libaegis/change/file.h>
#include <libaegis/change/history/encode.h>
#include <libaegis/os.h>
#include <libaegis/project.h>
#include <libaegis/sub.h>


void
change_run_history_create_command(change::pointer cp, fstate_src_ty *src)
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
    // Create a new history.  Only ever executed in the "being
    // integrated" state.  Current directory will be set to the
    // base of the history tree.  All of the substitutions described
    // in aesub(5) are avaliable.  In addition:
    //
    // ${Input}
    //     The absolute path of the source file to check-in.
    //
    // ${History}
    //     The absolute path of history file.
    //
    // ${File_Name}   (Optional)
    //     The base relative file name of the file for this check-in.
    //     Note that the file name can vary over the lifetime of the
    //     file as it is renamed, but the history file name never does.
    //     DO NOT use this as the name of the history file.
    //
    // ${UUID}   (Optional)
    //     The universally unique identifier for this file.  This is
    //     invariant for the lifetime of the file.
    //     DO NOT use this as the name of the history file.
    //
    trace(("change_run_history_create_command(cp = %p, "
        "filename = \"%s\")\n{\n", cp, src->file_name->str_text));
    assert(cp->reference_count >= 1);

    //
    // Inform the user of what we are doing, just in case they can't
    // tell from the file names.  For example, with a base-64 encoded
    // input file and a UUID history file the user has no way of knowing
    // which source file this concerns.
    //
    scp = sub_context_new();
    scp->var_set_string("File_Name", src->file_name);
    change_error(cp, scp, i18n("history create $filename"));
    sub_context_delete(scp);
    scp = 0;

    //
    // See if the file is binary, and if we need to encode it in any way.
    // The return value is the absolute path of the encoded temporary
    // file, or the absoulute path of the change file if no encoding
    // is required.
    //
    name_of_encoded_file = change_history_encode(cp, src, &unlink_encoded_file);

    //
    // Get the path of the source file.
    //
    trace(("mark\n"));
    scp = sub_context_new();
    sub_var_set_string(scp, "Input", name_of_encoded_file);

    //
    // Get the path of the history file.
    //
    trace(("mark\n"));
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
    trace(("mark\n"));
    pconf_data = change_pconf_get(cp, 1);
    the_command = pconf_data->history_create_command;
    if (!the_command)
    {
        sub_context_ty  *scp2;

        assert(pconf_data->errpos);
        scp2 = sub_context_new();
        sub_var_set_string(scp2, "File_Name", pconf_data->errpos);
        sub_var_set_charstar(scp2, "FieLD_Name", "history_create_command");
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
    trace(("mark\n"));
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
    // We can have a problem (thus, errok) if it is a "build" file that
    // is a symbolic link to a file we have no permission to modify.
    //
    trace(("mark\n"));
    os_mtime_set_errok(name_of_encoded_file, mtime);
    str_free(the_command);

    //
    // If there was an encoded file, get rid of it.
    //
    trace(("mark\n"));
    assert(src->edit);
    if (unlink_encoded_file)
        os_unlink(name_of_encoded_file);
    project_become_undo(cp->pp);
    str_free(name_of_encoded_file);

    //
    // Read the head revision from the file.
    //
    trace(("mark\n"));
    src->edit->revision = change_run_history_query_command(cp, src);

    //
    // Set the edit.uuid field
    //
    if (!cp->uuid_get().empty())
        src->edit->uuid = cp->uuid_get().get_ref_copy();

    trace(("}\n"));
}


// vim: set ts=8 sw=4 et :
