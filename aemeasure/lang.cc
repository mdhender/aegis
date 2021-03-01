//
// aegis - project change supervisor
// Copyright (C) 2003-2006, 2008, 2012 Peter Miller
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

#include <common/ac/assert.h>
#include <common/ac/math.h>
#include <common/ac/string.h>

#include <common/error.h>
#include <common/sizeof.h>

#include <aemeasure/lang.h>
#include <aemeasure/lang/c.h>
#include <aemeasure/lang/cxx.h>
#include <aemeasure/lang/generic.h>
#include <aemeasure/lang/roff.h>


static lang_t *table[] =
{
    &lang_c,
    &lang_cxx,
    &lang_roff,
    &lang_generic,
};


lang_t *
lang_by_name(const char *name)
{
    lang_t          **tp;
    lang_t          *lp;

    for (tp = table; tp < ENDOF(table); ++tp)
    {
        lp = *tp;
        if (!strcasecmp(name, lp->name))
            return lp;
    }

    fatal_raw("language \"%s\" unknown", name);
    // NOTREACHED
    return 0;
}


lang_t *
lang_by_file(const char *filename)
{
    lang_t          **tp;
    lang_t          *lp;

    if (!filename)
        return &lang_generic;
    for (tp = table; tp < ENDOF(table); ++tp)
    {
        lp = *tp;
        if (lp->test(filename))
        {
            return lp;
        }
    }
    // should never happen
    assert(!"should never happen");
    return &lang_generic;
}


void
lang_data_print(FILE *fp, lang_data_t *ldp)
{
    fprintf(fp, "metrics = [\n");
    if (ldp->file_is_binary)
    {
        fprintf(fp, "{ name = \"length\"; value = %ld; },\n", ldp->length);
    }
    else
    {
        fprintf
        (
            fp,
            "{ name = \"lines\"; value = %ld; },\n",
            ldp->code_lines + ldp->comment_lines + ldp->blank_lines
        );
        fprintf
        (
            fp,
            "{ name = \"code_lines\"; value = %ld; },\n",
            ldp->code_lines
        );
        fprintf
        (
            fp,
            "{ name = \"comment_lines\"; value = %ld; },\n",
            ldp->comment_lines
        );
        fprintf
        (
            fp,
            "{ name = \"blank_lines\"; value = %ld; },\n",
            ldp->blank_lines
        );
        if (ldp->halstead_n2 != 0)
        {
            long            N;
            long            n;
            double          v;
            double          d;

            N = ldp->halstead_N1 + ldp->halstead_N2;
            fprintf
            (
                fp,
                "{ name = \"halstead_length\"; value = %ld; },\n",
                N
            );
            n = ldp->halstead_n1 + ldp->halstead_n2;
            fprintf
            (
                fp,
                "{ name = \"halstead_vocabulary\"; value = %ld; },\n",
                n
            );
            v = N * log((double)n) / M_LN2;
            fprintf
            (
                fp,
                "{ name = \"halstead_volume\"; value = %g; },\n",
                v
            );
            d =
                (
                    ((double)ldp->halstead_n1 / 2.)
                *
                    ((double)ldp->halstead_N2 / (double)ldp->halstead_n2)
                );
            fprintf
            (
                fp,
                "{ name = \"halstead_difficulty\"; value = %g; },\n",
                d
            );
            fprintf
            (
                fp,
                "{ name = \"halstead_effort\"; value = %g; },\n",
                v * d
            );
        }
    }
    fprintf(fp, "];\n");
}


// vim: set ts=8 sw=4 et :
