//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002-2008 Peter Miller
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

#include <common/error.h> // for assert
#include <libaegis/change/file.h>
#include <libaegis/os/isa/path_prefix.h>
#include <common/str_list.h>
#include <common/trace.h>


void
change_file_directory_query(change::pointer cp, string_ty *file_name,
    string_list_ty *result_in, string_list_ty *result_out)
{
    fstate_ty       *fstate_data;
    size_t		    j;
    fstate_src_ty   *src_data;

    trace(("change_file_dir(cp = %08lX, file_name = \"%s\")\n{\n",
	(long)cp, file_name->str_text));
    assert(result_in);
    result_in->clear();
    if (result_out)
	result_out->clear();
    fstate_data = change_fstate_get(cp);
    assert(fstate_data->src);
    for (j = 0; j < fstate_data->src->length; ++j)
    {
	src_data = fstate_data->src->list[j];
	if (src_data->about_to_be_created_by && !src_data->deleted_by)
	    continue;
	switch (src_data->usage)
	{
	case file_usage_build:
	    continue;

	case file_usage_source:
	case file_usage_config:
	case file_usage_test:
	case file_usage_manual_test:
	    break;
	}
	if (os_isa_path_prefix(file_name, src_data->file_name))
	{
	    if (!src_data->deleted_by)
		result_in->push_back(src_data->file_name);
	    else if (result_out)
		result_out->push_back(src_data->file_name);
	}
    }
    trace(("}\n"));
}
