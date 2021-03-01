/*
 *	aegis - project change supervisor
 *	Copyright (C) 1999, 2002 Peter Miller;
 *	All rights reserved.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 * MANIFEST: functions to manipulate versions
 */

#include <output/pager.h>
#include <output/wrap.h>
#include <progname.h>
#include <version_stmp.h>
#include <version.h>


void
version_copyright()
{
    static char *text[] =
    {
	"All rights reserved.\n",
	"\n",
	"The %s program comes with ABSOLUTELY NO WARRANTY; ",
	"for details use the '%s -VERSion License' command.  ",
	"The %s program is free software, and you are welcome ",
	"to redistribute it under certain conditions; for ",
	"details use the '%s -VERSion License' command.\n",
    };

    output_ty	    *fp;
    char	    **cpp;
    char	    *progname;

    progname = progname_get();
    fp = output_pager_open();
    fp = output_wrap_open(fp, 1, -1);
    output_fprintf(fp, "%s version %s\n", progname, version_stamp());
    output_fprintf(fp, "Copyright (C) %s Peter Miller;\n", copyright_years());
    for (cpp = text; cpp < ENDOF(text); ++cpp)
	output_fprintf(fp, *cpp, progname);
    output_delete(fp);
}
