//
//	aegis - project change supervisor
//	Copyright (C) 2003, 2004 Peter Miller;
//	All rights reserved.
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
//
// MANIFEST: functions to manipulate commands
//

#include <ac/errno.h>
#include <ac/unistd.h>

#include <get/command.h>
#include <os.h>
#include <sub.h>


void
get_command(const char *cmd)
{
    const char      *shell;
    sub_context_ty  *scp;
    int             errno_old;

    //
    // invoke the command through sh(1)
    //
    shell = os_shell();
    execl(shell, shell, "-ec", cmd, (char *)0);
    errno_old = errno;
    scp = sub_context_new();
    sub_errno_setx(scp, errno_old);
    sub_var_set_charstar(scp, "File_Name", shell);
    fatal_intl(scp, i18n("exec \"$filename\": $errno"));
    // NOTREACHED
}