//
// aegis - project change supervisor
// Copyright (C) 2006, 2008, 2012 Peter Miller
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

#include <common/ac/string.h>

#include <common/sizeof.h>
#include <libaegis/compres_algo.h>
#include <libaegis/sub.h>


struct table_t
{
    const char *name;
    compression_algorithm_t value;
};

static table_t table[] =
{
    { "none", compression_algorithm_none },
    { "gzip", compression_algorithm_gzip },
    { "gunzip", compression_algorithm_gzip },
    { "bzip2", compression_algorithm_bzip2 },
    { "bunzip2", compression_algorithm_bzip2 },
    // Tridge's rzip soon?
};


compression_algorithm_t
compression_algorithm_by_name(const char *s)
{
    size_t len = strlen(s);
    if (len)
    {
        for (table_t *tp = table; tp < ENDOF(table); ++tp)
        {
            //
            // No need to worry about length of tp->name; if is longer,
            // only compare leading prefix (unique, so far); if it is
            // shorter, the NUL character will ensure inequality.
            //
            if (0 == strncasecmp(s, tp->name, len))
                return tp->value;
        }
    }

    sub_context_ty sc;
    sc.var_set_charstar("Name", s);
    sc.fatal_intl("compression algorithm \"$name\" unknown");
    // NOTREACHED
}


const char *
compression_algorithm_name(compression_algorithm_t x)
{
    switch (x)
    {
    case compression_algorithm_not_set:
        return "not set";

    case compression_algorithm_none:
        return "none";

    case compression_algorithm_gzip:
        return "gzip";

    case compression_algorithm_bzip2:
        return "bzip2";

    case compression_algorithm_unspecified:
        return "set but not specified";
    }
    return "unknown";
}


const char *
compression_algorithm_extension(compression_algorithm_t x)
{
    switch (x)
    {
    case compression_algorithm_not_set:
    case compression_algorithm_none:
    case compression_algorithm_unspecified:
        return "";

    case compression_algorithm_gzip:
        return ".gz";

    case compression_algorithm_bzip2:
        return ".bz2";
    }
    return "";
}


// vim: set ts=8 sw=4 et :
