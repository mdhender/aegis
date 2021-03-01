//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2002-2008 Peter Miller
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

#include <libaegis/change.h>
#include <common/error.h> // for assert
#include <libaegis/os.h>
#include <libaegis/project/history.h>
#include <libaegis/sub.h>
#include <libaegis/user.h>


string_ty *
change_development_directory_template(change::pointer cp, user_ty::pointer up)
{
    pconf_ty    *pconf_data;

    //
    // Get the project config file.  Don't insist that it exists,
    // because it will not for the first change.
    //
    assert(cp->reference_count >= 1);
    pconf_data = change_pconf_get(cp, 0);
    assert(pconf_data->new_test_filename);

    //
    // Ask the project where to put the development directory.
    // (By putting them all in the one place, it's easier to backup
    // a whole project.)
    //
    nstring ddd(project_default_development_directory_get(cp->pp));

    //
    // If the user did not give the directory to use,
    // we must construct one.
    // The length is limited by the available filename
    // length limit, trim the project name if necessary.
    //
    if (!ddd)
    {
        ddd = up->default_development_directory(cp->pp);
        assert(ddd);
    }

    nstring devdir;
    for (unsigned long k = 0;; ++k)
    {
        char suffix[30];
        char *tp = suffix;

        //
        // Construct the magic string.  This is used to
        // construct unique file names, should there be
        // a conflict for some reason (usually to do
        // with truncating file names, or re-using file
        // names).
        //
        sub_context_ty sc;
        unsigned long n = k;
        for (;;)
        {
            *tp++ = (n & 15) + 'C';
            n >>= 4;
            if (!n)
                break;
        }
        *tp = 0;
        sc.var_set_charstar("Magic", suffix);

        //
        // The default development directory is
        // directory within which the new development
        // directory is to be placed.  It is needed by
        // the substitution if it is to crop file names
        // by the maximum filename length (see the
        // ${namemax} substitution).
        //
        sc.var_set_string("Default_Development_Directory", ddd);

        //
        // Perform the substitution to construct the
        // development directory name.
        //
        devdir =
            nstring
            (
                sc.substitute
                (
                    cp,
                    pconf_data->development_directory_template
                )
            );

        //
        // See if this path is unique.
        //
        user_ty::become scoped(up);
        bool exists = os_exists(devdir);
        if (!exists)
            break;
    }
    assert(devdir);
    return str_copy(devdir.get_ref());
}
