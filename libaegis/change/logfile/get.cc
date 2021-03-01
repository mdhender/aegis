//
//      aegis - project change supervisor
//      Copyright (C) 1999, 2002-2008, 2012 Peter Miller
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

#include <common/trace.h>
#include <libaegis/change.h>
#include <libaegis/os.h>
#include <libaegis/sub.h>


string_ty *
change_logfile_get(change::pointer cp)
{
    cstate_ty       *cstate_data;

    trace(("change_logfile_get(cp = %p)\n{\n", cp));
    assert(cp->reference_count >= 1);
    if (!cp->logfile)
    {
        cstate_data = cp->cstate_get();
        string_ty *s1 = 0;
        switch (cstate_data->state)
        {
        case cstate_state_awaiting_development:
        case cstate_state_awaiting_review:
        case cstate_state_being_reviewed:
        case cstate_state_awaiting_integration:
        case cstate_state_completed:
#ifndef DEBUG
        default:
#endif
            change_fatal(cp, 0, i18n("no log file"));

        case cstate_state_being_integrated:
            s1 = change_integration_directory_get(cp, 0);
            break;

        case cstate_state_being_developed:
            s1 = change_development_directory_get(cp, 0);
            break;
        }
        assert(s1);
        cp->logfile = os_path_cat(s1, change_logfile_basename());
        // do NOT str_free(s1);
    }
    trace(("return \"%s\";\n", cp->logfile->str_text));
    trace(("}\n"));
    return cp->logfile;
}


// vim: set ts=8 sw=4 et :
