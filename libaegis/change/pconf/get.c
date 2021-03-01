/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999 Peter Miller;
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
 * MANIFEST: functions to manipulate gets
 */

#include <change.h>
#include <change/lock_sync.h>
#include <error.h> /* for assert */
#include <os.h>
#include <sub.h>
#include <trace.h>


static void pconf_improve _((change_ty *, pconf, string_ty *));

static void
pconf_improve(cp, d, filename)
	change_ty	*cp;
	pconf		d;
	string_ty	*filename;
{
	sub_context_ty	*scp;

	if (!d->build_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", filename);
		sub_var_set(scp, "FieLD_Name", "build_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!d->development_build_command)
		d->development_build_command = str_copy(d->build_command);
	if (!d->history_create_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", filename);
		sub_var_set(scp, "FieLD_Name", "history_create_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!d->history_get_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", filename);
		sub_var_set(scp, "FieLD_Name", "history_get_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!d->history_put_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", filename);
		sub_var_set(scp, "FieLD_Name", "history_put_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!d->history_query_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", filename);
		sub_var_set(scp, "FieLD_Name", "history_query_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!d->diff_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", filename);
		sub_var_set(scp, "FieLD_Name", "diff_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!d->diff3_command && !d->merge_command)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", filename);
		sub_var_set(scp, "FieLD_Name", "merge_command");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: contains no \"$field_name\" field")
		);
		/* NOTREACHED */
		sub_context_delete(scp);
	}
	if (!(d->mask & pconf_shell_safe_filenames_mask))
	{
		d->shell_safe_filenames = 1;
		d->mask |= pconf_shell_safe_filenames_mask;
	}
	if (d->file_template)
	{
		size_t		j;

		for (j = 0; j < d->file_template->length; ++j)
		{
			pconf_file_template tp;

			tp = d->file_template->list[j];
			if (!tp->pattern || !tp->pattern->length)
			{
				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", filename);
				sub_var_set(scp, "FieLD_Name",
					"file_template.pattern");
				change_fatal
				(
					cp,
					scp,
			    i18n("$filename: contains no \"$field_name\" field")
				);
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			if (!tp->body && !tp->body_command)
			{
				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", filename);
				sub_var_set(scp, "FieLD_Name",
					"file_template.body");
				change_fatal
				(
					cp,
					scp,
			    i18n("$filename: contains no \"$field_name\" field")
				);
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			if (tp->body && tp->body_command)
			{
				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", filename);
				sub_var_set(scp, "FieLD_Name",
					"file_template.body");
				change_fatal
				(
					cp,
					scp,
			      i18n("$filename: corrupted \"$field_name\" field")
				);
				/* NOTREACHED */
				sub_context_delete(scp);
			}
		}
	}
}


static void set_pconf_symlink_exceptions_defaults _((pconf));

static void
set_pconf_symlink_exceptions_defaults(pconf_data)
	pconf		pconf_data;
{
	type_ty		*type_p;
	string_ty	**str_p;

	/*
	 * make sure the list is there
	 */
	if (!pconf_data->symlink_exceptions)
		pconf_data->symlink_exceptions =
			pconf_symlink_exceptions_list_type.alloc();

	/*
	 * append the logfile to the list
	 */
	str_p =
		pconf_symlink_exceptions_list_type.list_parse
		(
			pconf_data->symlink_exceptions,
			&type_p
		);
	assert(type_p == &string_type);
	*str_p = str_copy(change_logfile_basename());
}


pconf
change_pconf_get(cp, required)
	change_ty	*cp;
	int		required;
{
	static string_ty *star_comma_d;
	size_t		j;

	trace(("change_pconf_get(cp = %8.8lX, required = %d)\n{\n"/*}*/,
		(long)cp, required));
	assert(cp->reference_count >= 1);
	change_lock_sync(cp);
	if (!cp->pconf_data)
	{
		string_ty	*filename;

		filename = change_pconf_path_get(cp);
		assert(filename);
		change_become(cp);
		if (!os_exists(filename))
		{
			if (required)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%s", THE_CONFIG_FILE);
				change_fatal
				(
					cp,
					scp,
				    i18n("you must create a \"$filename\" file")
				);
				/* NOTREACHED */
				sub_context_delete(scp);
			}
			cp->pconf_data = pconf_type.alloc();
			cp->pconf_data->shell_safe_filenames = 1;
		}
		else
		{
			cp->pconf_data = pconf_read_file(filename->str_text);
			pconf_improve(cp, cp->pconf_data, filename);
		}
		change_become_undo();

		/*
		 * set the architecture default
		 */
		if (!cp->pconf_data->architecture)
			cp->pconf_data->architecture =
				pconf_architecture_list_type.alloc();
		if (!cp->pconf_data->architecture->length)
		{
			type_ty		*type_p;
			pconf_architecture *app;
			pconf_architecture ap;

			app =
				pconf_architecture_list_type.list_parse
				(
					cp->pconf_data->architecture,
					&type_p
				);
			assert(type_p == &pconf_architecture_type);
			ap = pconf_architecture_type.alloc();
			*app = ap;
			ap->name = str_from_c("unspecified");
			ap->pattern = str_from_c("*");
			ap->mask =
				(
					pconf_architecture_name_mask
				|
					pconf_architecture_pattern_mask
				);
		}
		for (j = 0; j < cp->pconf_data->architecture->length; ++j)
		{
			if
			(
				cp->pconf_data->architecture->list[j]->mask
			!=
				(
					pconf_architecture_name_mask
				|
					pconf_architecture_pattern_mask
				)
			)
			{
				sub_context_ty	*scp;

				scp = sub_context_new();
				sub_var_set(scp, "File_Name", "%S", filename);
				change_fatal
				(
					cp,
					scp,
				  i18n("$filename: arch needs name and pattern")
				);
				/* NOTREACHED */
				sub_context_delete(scp);
			}
		}

		/*
		 * set the maximum_filename_length default
		 */
		if (cp->pconf_data->mask & pconf_maximum_filename_length_mask)
		{
			if (cp->pconf_data->maximum_filename_length < 9)
				cp->pconf_data->maximum_filename_length = 9;
			if (cp->pconf_data->maximum_filename_length > 255)
				cp->pconf_data->maximum_filename_length = 255;
		}
		else
			cp->pconf_data->maximum_filename_length = 255;

		/*
		 * set the filename_pattern_accept default
		 */
		if (!cp->pconf_data->filename_pattern_accept)
			cp->pconf_data->filename_pattern_accept =
				pconf_filename_pattern_accept_list_type.alloc();
		if (!cp->pconf_data->filename_pattern_accept->length)
		{
			type_ty		*type_p;
			string_ty	**addr_p;

			addr_p =
				pconf_filename_pattern_accept_list_type.list_parse
				(
					cp->pconf_data->filename_pattern_accept,
					&type_p
				);
			assert(type_p == &string_type);
			*addr_p = str_from_c("*");
		}
		if (!star_comma_d)
			star_comma_d = str_from_c("*,D");
		if (!cp->pconf_data->filename_pattern_reject)
			cp->pconf_data->filename_pattern_reject =
				pconf_filename_pattern_reject_list_type.alloc();
		for (j = 0; j < cp->pconf_data->filename_pattern_reject->length; ++j)
		{
			if
			(
				str_equal
				(
					star_comma_d,
					cp->pconf_data->filename_pattern_reject
						->list[j]
				)
			)
				break;
		}
		if (j >= cp->pconf_data->filename_pattern_reject->length)
		{
			type_ty		*type_p;
			string_ty	**addr_p;

			addr_p =
				pconf_filename_pattern_reject_list_type.list_parse
				(
					cp->pconf_data->filename_pattern_reject,
					&type_p
				);
			assert(type_p == &string_type);
			*addr_p = str_copy(star_comma_d);
		}

		/*
		 * make sure symlink_exceptions is there, even if empty
		 */
		set_pconf_symlink_exceptions_defaults(cp->pconf_data);

		/*
		 * set the test_command default
		 */
		if (!cp->pconf_data->test_command)
			cp->pconf_data->test_command =
				str_from_c("$shell $file_name");
		if (!cp->pconf_data->development_test_command)
			cp->pconf_data->development_test_command =
				str_copy(cp->pconf_data->test_command);

		/*
		 * set the development directory template default
		 */
		if (!cp->pconf_data->development_directory_template)
		{
			cp->pconf_data->development_directory_template =
				str_from_c
				(
"$ddd/${left $project ${expr ${namemax $ddd} - ${length .$magic${zpad $c 3}}}}\
.$magic${zpad $c 3}"
				);
		}

		/*
		 * Set the test filename template
		 */
		if (!cp->pconf_data->new_test_filename)
		{
			cp->pconf_data->new_test_filename =
				str_from_c
				(
		  "test/${zpad $hundred 2}/t${zpad $number 4}${left $type 1}.sh"
				);
		}
	}
	trace(("return %8.8lX;\n", cp->pconf_data));
	trace((/*{*/"}\n"));
	return cp->pconf_data;
}