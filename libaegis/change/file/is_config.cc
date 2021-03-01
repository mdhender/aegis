//
// aegis - project change supervisor
// Copyright (C) 2002-2008, 2011, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <libaegis/change/file.h>
#include <libaegis/os.h>
#include <libaegis/os/isa/path_prefix.h>
#include <libaegis/pconf.fmtgen.h>


bool
change::file_is_config(string_ty *file_name)
{
    //
    // By reading the project configuration information, all of the config
    // files' usage will be adjusted to call themselves config files.
    //
    pconf_ty *pcp = change_pconf_get(this, 0);

    //
    // Now hunt for the file and see if it has a usage of "config".
    //
    fstate_src_ty *src = file_find(nstring(file_name), view_path_extreme);
    if (src)
    {
        return (src->usage == file_usage_config);
    }

    //
    // It is a new file.
    // If the file is within the config directory it will be a config file.
    //
    if
    (
        pcp->configuration_directory
    &&
        os_isa_path_prefix(pcp->configuration_directory, file_name)
    )
        return true;

    //
    // If this is a bogus config file, it won't have the build_command set.
    // It means that there is no config file in existence in the project yet.
    //
    if (!pcp->build_command)
    {
        string_ty       *s;
        int             ok;

        s = str_from_c(THE_CONFIG_FILE_NEW);
        ok = str_equal(s, file_name);
        str_free(s);
        if (ok)
            return true;

        s = str_from_c(THE_CONFIG_FILE_OLD);
        ok = str_equal(s, file_name);
        str_free(s);
        if (ok)
            return true;
    }

    //
    // It isn't a config file.
    //
    return false;
}


// vim: set ts=8 sw=4 et :
