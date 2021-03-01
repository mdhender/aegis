#
#	aegis - project change supervisor
#	Copyright (C) 2004 Peter Miller;
#	All rights reserved.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
#
# MANIFEST: include file check
#
# This script is used to make sure there are no conflicts between
# libaegis/*.h and include files in other programs.
#
/.*\.h$/ {
    filename = $0
    dir = filename
    sub(/\/.*/, "", dir)

    if (dir == "libaegis" || dir == "common")
    {
	fn2 = filename
	sub(/^[^/]*\//, "", fn2)
	lib[fn2] = filename;
    }
    else
	other[filename] = filename;
}
END {
    oops = 0;
    for (f in other)
    {
	filename = other[f];

	dir = filename
	sub(/\/.*/, "", dir)
	fn2 = filename
	sub(/^[^/]*\//, "", fn2)

	if (lib[fn2] != "")
	{
	    printf("include duplicate (%s vs %s)\n", lib[fn2], filename);
	    oops = 1;
	}
    }
    if (oops)
	exit(1);
}
