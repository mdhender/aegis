/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999-2001, 2003 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate put_commands
 */

#include <change.h>
#include <change/env_set.h>
#include <change/file.h>
#include <change/history/encode.h>
#include <error.h> /* for assert */
#include <os.h>
#include <project.h>
#include <sub.h>
#include <trace.h>


void
change_run_history_put_command(change_ty *cp, fstate_src_ty *src)
{
    sub_context_ty  *scp;
    string_ty       *hp;
    pconf_ty        *pconf_data;
    string_ty       *the_command;
    time_t          mtime;
    string_ty       *name_of_encoded_file;

    /*
     * Update and existing history.  Only ever happens in the
     * ``being integrated'' state.  Current directory will be set to
     * the base of the history tree.  All of the substitutions
     * described in aesub(5) are avaliable.  In addition:
     *
     * ${Input}
     *	absolute path of source file
     *
     * ${History}
     *	absolute path of history file
     */
    trace(("change_run_history_put_command(cp = %8.8lX, "
	"filename = \"%s\")\n{\n", (long)cp, src->file_name->str_text));
    assert(cp->reference_count >= 1);

    /*
     * See if the file is binary, and if we need to encode it in any way.
     * If the file is encoded, it will be the absolute path of
     * the encoded file.  If the file is not encoded it will be the
     * absolute path of the source file.
     */
    name_of_encoded_file = change_history_encode(cp, src);

    /*
     * Get the path of the source file.
     */
    scp = sub_context_new();
    sub_var_set_string(scp, "Input", name_of_encoded_file);

    /*
     * Get the path of the history file.
     */
    hp = project_history_path_get(cp->pp);
    sub_var_set_format(scp, "History", "%S/%S", hp, src->file_name);

    /*
     * Ancient compatibility.
     * I sure hope no-one is still using this.
     */
    sub_var_set_charstar(scp, "1", "${input}");
    sub_var_set_charstar(scp, "2", "${history}");

    /*
     * Make sure the command has been set.
     */
    pconf_data = change_pconf_get(cp, 1);
    the_command = pconf_data->history_put_command;
    if (!the_command)
    {
	sub_context_ty  *scp2;

	scp2 = sub_context_new();
	sub_var_set_charstar(scp2, "File_Name", THE_CONFIG_FILE);
	sub_var_set_charstar(scp2, "FieLD_Name", "history_get_command");
	change_fatal
	(
	    cp,
	    scp2,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	/* NOTREACHED */
	sub_context_delete(scp2);
    }

    /*
     * Perform the substitutions and run the command.
     */
    the_command = substitute(scp, cp, the_command);
    sub_context_delete(scp);
    change_env_set(cp, 0);
    project_become(cp->pp);
    mtime = os_mtime_actual(name_of_encoded_file);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT, hp);

    /*
     * Reset the file modification time: many history commands
     * gratuitously touch the file.
     */
    os_mtime_set(name_of_encoded_file, mtime);
    str_free(the_command);

    /*
     * If there was an encoded file, get rid of it.
     */
    assert(src->edit);
    if (src->edit->encoding != history_version_encoding_none)
	os_unlink(name_of_encoded_file);
    project_become_undo();
    str_free(name_of_encoded_file);

    /*
     * Read the head revision from the file.
     */
    assert(src->edit);
    src->edit->revision = change_run_history_query_command(cp, src->file_name);
    trace(("}\n"));
}
