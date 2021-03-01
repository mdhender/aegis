#
#	aegis - project change supervisor
#	Copyright (C) 1995 Peter Miller;
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
#	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# MANIFEST: awk script to extract the table of contents from the faq
#
/^\.H /{
	level = $2
	split($0, a, "\"")
	heading = a[2]
	count[level]++
	for (j = level + 1; j < 8; ++j)
		count[j] = 0;
	for (j = 1; j <= level; ++j)
		printf("%d.", count[j]);
	printf("   %s\n", heading);
}
