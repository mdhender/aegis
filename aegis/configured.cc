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
// MANIFEST: functions to manipulate configureds
//

#include <ac/stdio.h>

#include <configured.h>
#include <libdir.h>
#include <str.h>
#include <version_stmp.h>


static void
emit(const char *name, const char *value)
{
    string_ty	*s1;
    string_ty	*s2;

    s1 = str_from_c(value);
    s2 = str_quote_shell(s1);
    str_free(s1);
    printf("%s=%s\n", name, s2->str_text);
    str_free(s2);
}


static void
emiti(const char *name, int value)
{
    printf("%s=%d\n", name, value);
}


void
configured(void)
{
    //
    // The ``aegis --configured'' option is used to the ./configure script
    // to look for an existing Aegis installation, and to replicate its
    // configuration settings.
    //
    // Ignore the rest of the command line options
    // and just print the information.
    //
    printf("# Aegis %s\n", version_stamp());
    printf("# " __DATE__ " " __TIME__ "\n");
    emit("prefix", configured_prefix());
    emit("bindir", configured_bindir());
    emit("datadir", configured_datadir());
    emit("sharedstatedir", configured_comdir());
    emit("libdir", configured_libdir());
    emit("mandir", configured_mandir());
    emit("sysconfdir", configured_sysconfdir());
    emit("NLSDIR", configured_nlsdir());
    emiti("AEGIS_UID", configured_aegis_uid());
    emiti("AEGIS_GID", configured_aegis_gid());
}
