#!/bin/awk
#
#	aegis - project change supervisor
#	Copyright (C) 2001 Peter Miller;
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
# MANIFEST: chan_files.awk
#
BEGIN {
    change_files_removed = 0;
}
$2 == "remove" {
	change_files_removed++
	next
}
$1 == "build" {
	bf[$NF] = 1
	next
}
{
	cf[$NF] = 1
}
END {
	print "change_files_removed = " change_files_removed ";"
	print "change_files ="
	for (f in cf)
		print "\"" f "\""
	print ";"
	print "change_files_build ="
	for (f in bf)
		print "\"" f "\""
	print ";"
}
