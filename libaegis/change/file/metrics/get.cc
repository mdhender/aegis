//
// aegis - project change supervisor
// Copyright (C) 1999, 2001, 2003-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see
// <http://www.gnu.org/licenses/>.
//

#include <libaegis/change/file.h>
#include <libaegis/metric_file.fmtgen.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>


static void
metric_check(metric_ty *mp, string_ty *fn, change::pointer cp)
{
    if (!mp->name)
    {
        sub_context_ty *scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", fn);
        sub_var_set_charstar(scp, "FieLD_Name", "name");
        change_fatal
        (
            cp,
            scp,
            i18n("$filename: corrupted \"$field_name\" field")
        );
    }
    if (!(mp->mask & metric_value_mask))
    {
        sub_context_ty *scp = sub_context_new();
        sub_var_set_string(scp, "File_Name", fn);
        sub_var_set_charstar(scp, "FieLD_Name", "value");
        change_fatal
        (
            cp,
            scp,
            i18n("$filename: corrupted \"$field_name\" field")
        );
    }
}


static void
metric_list_check(metric_list_ty *mlp, string_ty *fn, change::pointer cp)
{
    for (size_t j = 0; j < mlp->length; ++j)
        metric_check(mlp->list[j], fn, cp);
}


static string_ty *
change_file_metrics_filename(change::pointer cp, string_ty *filename)
{
    string_ty *metrics_filename_pattern =
        change_metrics_filename_pattern_get(cp);
    sub_context_ty *scp = sub_context_new();
    string_ty *absolute_filename = cp->file_path(filename);
    sub_var_set_string(scp, "File_Name", absolute_filename);
    str_free(absolute_filename);
    string_ty *metrics_filename =
        subst_intl(scp, metrics_filename_pattern->str_text);
    sub_context_delete(scp);
    return metrics_filename;
}


metric_list_ty *
change_file_metrics_get(change::pointer cp, string_ty *filename)
{
    //
    // Get the name of the file to read.
    //
    string_ty *metrics_filename = change_file_metrics_filename(cp, filename);

    //
    // Read the metrics file if it is there.
    //
    change_become(cp);
    metric_file_ty *mp = 0;
    if (os_exists(metrics_filename))
    {
        mp = metric_file_read_file(metrics_filename);
    }
    change_become_undo(cp);

    //
    // Extract the metrics list from the file data,
    // if present.
    //
    metric_list_ty *mlp = 0;
    if (mp && mp->metrics && mp->metrics->length)
    {
        mlp = mp->metrics;
        mp->metrics = 0;
    }
    metric_file_type.free(mp);
    mp = 0;

    //
    // If we have a metrics list, check each of the list elements.
    //
    if (mlp)
        metric_list_check(mlp, metrics_filename, cp);
    str_free(metrics_filename);

    //
    // Return the list of metrics.
    //
    return mlp;
}


// vim: set ts=8 sw=4 et :
