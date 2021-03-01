//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2008 Peter Miller
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

#include <ae-repo-ci/repository.h>


void
repository::rename_file(const nstring &old_name, const nstring &new_name,
    const nstring &content_path)
{
    //
    // For repository types which don't grok file renames, model it as a
    // remove_file followed by an add_file.
    //
    remove_file(old_name);
    add_file(new_name, content_path);
}
