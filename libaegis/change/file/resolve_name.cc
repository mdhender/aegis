//
//	aegis - project change supervisor
//	Copyright (C) 2004-2008 Peter Miller
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

#include <libaegis/change/file.h>
#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <common/str_list.h>
#include <libaegis/sub.h>
#include <common/trace.h>
#include <libaegis/user.h>


void
change_file_resolve_names(change::pointer cp, user_ty::pointer up,
    string_list_ty &names)
{
    trace(("change_file_resolve_names(cp = %08lX, up = %08lX)\n{\n",
	(long)cp, (long)up.get()));

    //
    // Get the change's search path.  We will be looking along this to
    // see if we can resolve each file's relative name.
    //
    string_list_ty search_path;
    change_search_path_get(cp, &search_path, 1);

    //
    // Did the user specify the --base-relative option?
    //
    bool based =
        (
            search_path.nstrings >= 1
        &&
            (
                up->relative_filename_preference
                (
                    uconf_relative_filename_preference_current
                )
            ==
                uconf_relative_filename_preference_base
            )
        );

    //
    // Now we are finally in a position to figure out the directory to
    // use to turn relative file names into absolute file names.
    //
    string_ty *base = search_path.string[0];
    if (!based)
    {
        os_become_orig();
        base = os_curdir();
        os_become_undo();
    }

    //
    // Work over each of the file names in the given list of names.
    //
    for (size_t j = 0; j < names.nstrings; ++j)
    {
	//
	// Make the file name absolute, if it isn't already.
	//
	string_ty *filename = names.string[j];
	string_ty *s2 = 0;
	if (filename->str_text[0] == '/')
	    s2 = str_copy(filename);
	else
	    s2 = os_path_join(base, filename);
	assert(s2);

        //
        // Resolve the absolute file name to get rid of symlinks and "."
        // or ".." components that would otherwise confuse the process.
	//
        up->become_begin();
        string_ty *s1 = os_pathname(s2, 1);
        up->become_end();
	str_free(s2);
	s2 = 0;

        //
        // Hunt down the search path to see of the file is below any of
        // the directories in the search path.  It is an error if the
        // file is not in one of them.
	//
	for (size_t k = 0; k < search_path.nstrings; ++k)
	{
	    s2 = os_below_dir(search_path.string[k], s1);
	    if (s2)
		break;
	}
	str_free(s1);
	if (!s2)
	{
	    sub_context_ty sc(__FILE__, __LINE__);
	    sc.var_set_string("File_Name", filename);
	    change_fatal(cp, &sc, i18n("$filename unrelated"));
	}

	//
        // Replace the filename in the list with the resolved path,
        // relative to the base of the dev dir (or baseline).
	//
	str_free(names.string[j]);
	names.string[j] = s2;
    }
    trace(("}\n"));
}


string_ty *
change_file_resolve_name(change::pointer cp, user_ty::pointer up,
    string_ty *filename)
{
    //
    // The single file is handled as a special case of one file, rather
    // than the other way around, for efficiency.  Some commands have
    // thousands of file names to process.
    //
    string_list_ty name_list;
    name_list.push_back(filename);
    change_file_resolve_names(cp, up, name_list);
    return str_copy(name_list.string[0]);
}
