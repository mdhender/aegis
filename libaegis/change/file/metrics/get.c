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

#include <change/file.h>
#include <metrics.h>
#include <os.h>
#include <sub.h>


static void metric_check _((metric, string_ty *, change_ty *));

static void
metric_check(mp, fn, cp)
	metric		mp;
	string_ty	*fn;
	change_ty	*cp;
{
	sub_context_ty	*scp;

	if (!mp->name)
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", fn);
		sub_var_set(scp, "FieLD_Name", "name");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: corrupted \"$field_name\" field")
		);
	}
	if (!(mp->mask & metric_value_mask))
	{
		scp = sub_context_new();
		sub_var_set(scp, "File_Name", "%S", fn);
		sub_var_set(scp, "FieLD_Name", "value");
		change_fatal
		(
			cp,
			scp,
			i18n("$filename: corrupted \"$field_name\" field")
		);
	}
}


static void metric_list_check _((metric_list, string_ty *, change_ty *));

static void
metric_list_check(mlp, fn, cp)
	metric_list	mlp;
	string_ty	*fn;
	change_ty	*cp;
{
	size_t		j;

	for (j = 0; j < mlp->length; ++j)
		metric_check(mlp->list[j], fn, cp);
}


static string_ty * change_file_metrics_filename _((change_ty *, string_ty *));

static string_ty *
change_file_metrics_filename(cp, filename)
	change_ty	*cp;
	string_ty	*filename;
{
	string_ty	*metrics_filename_pattern;
	sub_context_ty	*scp;
	string_ty	*absolute_filename;
	string_ty	*metrics_filename;

	metrics_filename_pattern = change_metrics_filename_pattern_get(cp);
	scp = sub_context_new();
	absolute_filename = change_file_path(cp, filename);
	sub_var_set(scp, "File_Name", "%S", absolute_filename);
	str_free(absolute_filename);
	metrics_filename = subst_intl(scp, metrics_filename_pattern->str_text);
	sub_context_delete(scp);
	return metrics_filename;
}


metric_list
change_file_metrics_get(cp, filename)
	change_ty	*cp;
	string_ty	*filename;
{
	string_ty	*metrics_filename;
	metrics		mp;
	metric_list	mlp;

	/*
	 * Get the name of the file to read.
	 */
	metrics_filename = change_file_metrics_filename(cp, filename);

	/*
	 * Read the metrics file if it is there.
	 */
	change_become(cp);
	if (os_exists(metrics_filename))
	{
		mp = metrics_read_file(metrics_filename->str_text);
	}
	else
		mp = 0;
	change_become_undo();

	/*
	 * Extract the metrics list from the file data,
	 * if present.
	 */
	mlp = 0;
	if (mp && mp->metrics && mp->metrics->length)
	{
		mlp = mp->metrics;
		mp->metrics = 0;
	}
	metrics_type.free(mp);

	/*
	 * If we have a metrics list, check each of the list elements.
	 */
	if (mlp)
		metric_list_check(mlp, metrics_filename, cp);
	str_free(metrics_filename);

	/*
	 * Return the list of metrics.
	 */
	return mlp;
}
