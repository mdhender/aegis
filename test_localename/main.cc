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

#include <common/arglex.h>
#include <common/boolean.h>
#include <common/nstring/list.h>
#include <common/progname.h>
#include <common/quit.h>

#include <libaegis/arglex2.h>
#include <libaegis/help.h>
#include <libaegis/locale_name.h>
#include <libaegis/os.h>


static void
usage(void)
{
    const char *prog = progname_get();
    fprintf(stderr, "Usage: %s [ <option>... ] <string>...\n", prog);
    quit(1);
}


int
main(int argc, char **argv)
{
    arglex2_init(argc, argv);
    arglex();
    os_become_init_mortal();
    nstring_list words;
    while (arglex_token != arglex_token_eoln)
    {
        switch (arglex_token)
        {
        default:
            generic_argument(usage);
            continue;

        case arglex_token_string:
            words.push_back(arglex_value.alv_string);
            break;
        }
        arglex();
    }

    for (size_t j = 0; j < words.size(); ++j)
    {
        nstring text = words[j];
        bool ok = is_a_locale_name(text);
        printf
        (
            "%s => %s\n",
            text.quote_c().c_str(),
            bool_to_string(ok).c_str()
        );
    }
    quit(0);
    return 0;
}


// vim: set ts=8 sw=4 et :
