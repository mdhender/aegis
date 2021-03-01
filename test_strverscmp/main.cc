//
// aegis - project change supervisor
// Copyright (C) 2007, 2008 Peter Miller
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
#include <common/ac/string.h>

#include <common/arglex.h>
#include <common/language.h>
#include <common/nstring.h>
#include <common/progname.h>


enum
{
    arglex_token_lt,
    arglex_token_gt,
    arglex_token_le,
    arglex_token_ge,
    arglex_token_eq,
    arglex_token_ne
};


static arglex_table_ty argtab[] =
{
    { "!=", arglex_token_ne },
    { "-EQual_to", arglex_token_eq },
    { "-Greater_Than", arglex_token_gt },
    { "-Greater_than_or_Equal_to", arglex_token_ge },
    { "-Less_Than", arglex_token_lt },
    { "-Less_than_or_Equal_to", arglex_token_le },
    { "-Not_Equal_to", arglex_token_ne },
    { "<", arglex_token_lt },
    { "<=", arglex_token_le },
    { "<>", arglex_token_ne },
    { "=", arglex_token_eq },
    { "==", arglex_token_eq },
    { ">", arglex_token_lt },
    { ">=", arglex_token_ge },
    ARGLEX_END_MARKER
};


static void
usage()
{
    const char *prog = progname_get();
    fprintf(stderr, "Usage: %s {string} {op} {string}\n", prog);
    fprintf(stderr, "\n");
    fprintf(stderr, "where {op} is one of the usual six comparisons\n");
    exit(1);
}


typedef bool (*func_t)(const nstring &lhs, const nstring &rhs);


static nstring
get_string()
{
    switch (arglex_token)
    {
    case arglex_token_number:
    case arglex_token_string:
        {
            nstring result = arglex_value.alv_string;
            arglex();
            return result;
        }

    default:
        usage();
    }
    return "";
}


static bool
lt(const nstring &lhs, const nstring &rhs)
{
    return (strverscmp(lhs.c_str(), rhs.c_str()) < 0);
}


static bool
le(const nstring &lhs, const nstring &rhs)
{
    return (strverscmp(lhs.c_str(), rhs.c_str()) <= 0);
}


static bool
gt(const nstring &lhs, const nstring &rhs)
{
    return (strverscmp(lhs.c_str(), rhs.c_str()) > 0);
}


static bool
ge(const nstring &lhs, const nstring &rhs)
{
    return (strverscmp(lhs.c_str(), rhs.c_str()) >= 0);
}


static bool
eq(const nstring &lhs, const nstring &rhs)
{
    return (strverscmp(lhs.c_str(), rhs.c_str()) == 0);
}


static bool
ne(const nstring &lhs, const nstring &rhs)
{
    return (strverscmp(lhs.c_str(), rhs.c_str()) != 0);
}


static func_t
get_op()
{
    switch (arglex_token)
    {
    case arglex_token_le:
        arglex();
        return le;

    case arglex_token_lt:
        arglex();
        return lt;

    case arglex_token_ge:
        arglex();
        return ge;

    case arglex_token_gt:
        arglex();
        return gt;

    case arglex_token_eq:
        arglex();
        return eq;

    case arglex_token_ne:
        arglex();
        return ne;

    default:
        usage();
        return 0;
    }
}


static void
get_eoln()
{
    if (arglex_token == arglex_token_eoln)
        return;
    usage();
}


int
main(int argc, char **argv)
{
    arglex_init(argc, argv, argtab);
    language_init();
    arglex();

    //
    // we expect to see
    //     string-or-number
    //     operator
    //     string-or-number
    //
    nstring lhs = get_string();
    func_t op = get_op();
    nstring rhs = get_string();
    get_eoln();

    return (op(lhs, rhs) ? EXIT_SUCCESS : EXIT_FAILURE);
}
