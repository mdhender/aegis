//
// aegis - project change supervisor
// Copyright (C) 2008, 2009, 2011 Peter Miller
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

#include <common/ac/ctype.h>

#include <libaegis/change.h>
#include <libaegis/change/branch.h>
#include <libaegis/project.h>
#include <libaegis/project/history.h>


nstring
change::version_debian_get(void)
{
    if (is_completed() || is_being_integrated())
    {
        nstring result = version_get();
        if (result.empty() || !isdigit((unsigned char)result[0]))
        {
            // Debian version strings can never be empty, and must
            // always start with a digit.  Cope with things like "D013".
            result = "0~" + result;
        }
        if (was_a_branch())
        {
            // make sure the branch sorts larger than its sub-changes
            // so lintian is happy.
            result += ".branch";
        }
        return result;
    }

    // In dpkg, ~ sorts less than end-of-string.  Typically it's used
    // for release candidates (1.2~rc3).  Treat a being-developed change
    // as a release candidate for the next delta, so lintian is happy.
    nstring s1(project_version_short_get(pp));
    if (s1.empty() || !isdigit((unsigned char)s1[0]))
    {
        // Debian version strings can never be empty, and must always
        // start with a digit.  Cope with things like "D013".
        s1 = "0~" + s1;
    }
    long dn = project_next_delta_number(pp);
    return nstring::format("%s.D%3.3ld~C%3.3ld", s1.c_str(), dn, number);
}


nstring
change::version_rpm_get(void)
{
    if (is_completed() || is_being_integrated())
    {
        nstring result = version_get();
        if (result.empty() || !isdigit((unsigned char)result[0]))
        {
            result = "0." + result;
        }
        if (was_a_branch())
        {
            // make sure the branch sorts larger than its sub-changes
            result += ".branch";
        }
        return result;
    }

    nstring s1(project_version_short_get(pp));
    if (s1.empty() || !isdigit((unsigned char)s1[0]))
    {
        // Debian version strings can never be empty, and must always
        // start with a digit.
        s1 = "0." + s1;
    }
    long dn = project_history_delta_latest(pp);
    return nstring::format("%s.D%3.3ld.C%3.3ld", s1.c_str(), dn, number);
}
