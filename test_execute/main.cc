//
//	aegis - project change supervisor
//	Copyright (C) 2007, 2008 Peter Miller
//
//      This program is free software; you can redistribute it and/or
//      modify it under the terms of the GNU General Public License as
//      published by the Free Software Foundation; either version 3 of
//      the License, or (at your option) any later version.
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

#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>

#include <common/arglex.h>
#include <common/env.h>
#include <common/error.h>
#include <common/language.h>
#include <common/nstring/accumulator.h>
#include <common/progname.h>
#include <libaegis/help.h>
#include <libaegis/os.h>


static void
usage(void)
{
    const char *progname = progname_get();
    fprintf(stderr, "Usage: %s <command>...\n", progname);
    exit(1);
}


int
main(int argc, char **argv)
{
    os_become_init_mortal();
    arglex_init(argc, argv, 0);
    env_initialize();
    language_init();
        arglex();

    nstring_accumulator ac;
    for (;;)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            continue;

        case arglex_token_string:
        case arglex_token_number:
            if (!ac.empty())
                ac.push_back(' ');
            ac.push_back(arglex_value.alv_string);
            arglex();
            continue;

        case arglex_token_eoln:
            break;
        }
        break;
    }
    if (ac.empty())
        usage();

    os_become_orig();
    nstring dir(os_curdir());
    nstring cmd = ac.mkstr();
    os_execute(cmd, 0, dir);
    os_become_undo();
    return 0;
}
