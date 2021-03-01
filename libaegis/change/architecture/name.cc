//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2001-2008, 2012 Peter Miller
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

#include <common/ac/assert.h>

#include <common/gmatch.h>
#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/sub.h>
#include <libaegis/uname.h>


string_ty *
change_architecture_name(change::pointer cp, int with_arch)
{
    trace(("change_architecture_name(cp = %p)\n{\n", cp));
    assert(cp->reference_count >= 1);
    if (!cp->architecture_name)
    {
        pconf_ty        *pconf_data;
        size_t          j;
        string_ty       *result;
        string_ty       *un;
        string_ty       *discriminator;

        pconf_data = change_pconf_get(cp, 0);
        assert(pconf_data->architecture);
        un = uname_variant_get();

        //
        // The architecture discriminator gets added to the end of the
        // architecture string formed using the uname system call.
        //
        discriminator = change_run_architecture_discriminator_command(cp);
        if (discriminator)
        {
            string_ty       *dash;

            dash = str_from_c("-");
            un = str_cat_three(un, dash, discriminator);
            str_free(dash);
        }
        else
            un = str_copy(un);

        result = 0;
        for (j = 0; j < pconf_data->architecture->length; ++j)
        {
            pconf_architecture_ty *ap;

            ap = pconf_data->architecture->list[j];
            assert(ap->name);
            assert(ap->pattern);
            if (gmatch(ap->pattern->str_text, un->str_text))
            {
                if (ap->mode == pconf_architecture_mode_forbidden)
                {
                    sub_context_ty      *scp;

                    scp = sub_context_new();
                    sub_var_set_string(scp, "Name", un);
                    change_fatal
                    (
                        cp,
                        scp,
                        i18n("architecture \"$name\" forbidden")
                    );
                    // NOTREACHED
                    sub_context_delete(scp);
                }
                result = ap->name;
                break;
            }
        }
        str_free(un);
        if (!result && with_arch)
        {
            sub_context_ty      *scp;

            scp = sub_context_new();
            sub_var_set_string(scp, "Name", un);
            change_fatal(cp, scp, i18n("architecture \"$name\" unknown"));
            // NOTREACHED
            sub_context_delete(scp);
        }
        cp->architecture_name = result;
    }
    trace_string(cp->architecture_name ? cp->architecture_name->str_text : "");
    trace(("}\n"));
    return cp->architecture_name;
}


// vim: set ts=8 sw=4 et :
