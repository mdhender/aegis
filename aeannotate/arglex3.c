/*
 *	aegis - project change supervisor
 *	Copyright (C) 2002 Peter Miller;
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
 * MANIFEST: functions to manipulate arglex3s
 */

#include <arglex3.h>


static arglex_table_ty argtab[] =
{
    { "-COLumn", arglex_token_column, },
    { "-File_Statistics", arglex_token_filestat, },
    { "-Not_File_Statistics", arglex_token_filestat_not, },
    { "-OPTion", arglex_token_diff_option, },
    { "-Diff_Option", arglex_token_diff_option, },
    { 0, }
};


void
arglex3_init(argc, argv)
    int		argc;
    char		**argv;
{
    arglex2_init3(argc, argv, argtab);
}
