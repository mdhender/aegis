//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/arglex.h>
#include <common/env.h>
#include <common/language.h>
#include <common/progname.h>

#include <libaegis/help.h>
#include <libaegis/os.h>


static void
usage(void)
{
    const char *prog = progname_get();
    fprintf(stderr, "Usage: %s <string>...\n", prog);
    exit(EXIT_FAILURE);
}


int
main(int argc, char **argv)
{
    os_become_init_mortal();
    arglex_init(argc, argv, 0);
    env_initialize();
    language_init();
    arglex();

    bool ok = false;
    for (;;)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            continue;

        case arglex_token_string:
            puts(nstring(arglex_value.alv_string).quote_c().c_str());
            arglex();
            ok = true;
            continue;

        case arglex_token_eoln:
            break;
        }
        break;
    }
    if (!ok)
        usage();
    return 0;
}


// vim: set ts=8 sw=4 et :
