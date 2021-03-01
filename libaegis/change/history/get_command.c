/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2001-2003 Peter Miller;
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
 * MANIFEST: functions to manipulate get_commands
 */

#include <change.h>
#include <change/env_set.h>
#include <error.h> /* for assert */
#include <input/base64.h>
#include <input/file_text.h>
#include <input/quoted_print.h>
#include <os.h>
#include <output/file.h>
#include <project.h>
#include <sub.h>
#include <trace.h>
#include <user.h>


void
change_run_history_get_command(change_ty *cp, fstate_src_ty *src,
    string_ty *output_file, user_ty *up)
{
    sub_context_ty  *scp;
    string_ty	    *dir;
    cstate_ty       *cstate_data;
    string_ty	    *the_command;
    pconf_ty        *pconf_data;
    string_ty	    *name_of_encoded_file;
    input_ty	    *ip;
    output_ty	    *op;

    trace(("change_run_history_get_command(cp = %08lX)\n{\n", (long)cp));
    assert(cp->reference_count >= 1);
    assert(src->reference_count >= 1);
    assert(src->file_name);
    assert(src->edit);
    assert(src->edit->revision);
    if (src->edit->encoding == history_version_encoding_none)
	name_of_encoded_file = output_file;
    else
	name_of_encoded_file = os_edit_filename(0);
    trace(("file name = \"%s\";\n", src->file_name->str_text));
    trace(("edit number = \"%s\";\n", src->edit->revision->str_text));

    /*
     * If the edit numbers differ, extract the
     * appropriate edit from the baseline (use the
     * history-get-command) into a file in /tmp
     *
     * Current directory set to the base of the history tree.
     * All of the substitutions described in aesub(5) are available;
     * in addition
     *
     * ${History}
     *	    history file
     *
     * ${Edit}
     *	    edit number
     *
     * ${Output}
     *	    output file
     */
    scp = sub_context_new();
    sub_var_set_format
    (
	scp,
	"History",
	"%S/%S",
	project_history_path_get(cp->pp),
	src->file_name
    );
    sub_var_set_string(scp, "Edit", src->edit->revision);
    sub_var_set_string(scp, "Output", name_of_encoded_file);
    sub_var_set_charstar(scp, "1", "${history}");
    sub_var_set_charstar(scp, "2", "${edit}");
    sub_var_set_charstar(scp, "3", "${output}");

    pconf_data = change_pconf_get(cp, 1);
    the_command = pconf_data->history_get_command;
    if (!the_command)
    {
	sub_context_ty	*scp2;

	scp2 = sub_context_new();
	sub_var_set_charstar(scp2, "File_Name", THE_CONFIG_FILE);
	sub_var_set_charstar(scp2, "FieLD_Name", "history_get_command");
	change_fatal
	(
	    cp,
	    scp2,
	    i18n("$filename: contains no \"$field_name\" field")
	);
	sub_context_delete(scp2);
    }
    the_command = substitute(scp, cp, the_command);
    sub_context_delete(scp);

    /*
     * run the command as the current user
     * (always output is to /tmp)
     */
    cstate_data = change_cstate_get(cp);
    dir = project_history_path_get(cp->pp);
    change_env_set(cp, 0);
    user_become(up);
    os_execute(the_command, OS_EXEC_FLAG_NO_INPUT | OS_EXEC_FLAG_SILENT, dir);
    user_become_undo();
    str_free(the_command);

    if (src->edit->encoding == history_version_encoding_none)
    {
	assert(name_of_encoded_file == output_file);
	trace(("}\n"));
	return;
    }

    /*
     * Unencode the file.
     */
    os_become_orig();
    ip = input_file_text_open(name_of_encoded_file);
    op = output_file_binary_open(output_file);
    switch (src->edit->encoding)
    {
    case history_version_encoding_none:
	assert(0);
	break;

    case history_version_encoding_quoted_printable:
	ip = input_quoted_printable(ip, 1);
	break;

    case history_version_encoding_base64:
	ip = input_base64(ip, 1);
	break;
    }
    input_to_output(ip, op);
    input_delete(ip);
    output_delete(op);
    os_unlink(name_of_encoded_file);
    os_become_undo();
    str_free(name_of_encoded_file);
    trace(("}\n"));
}
