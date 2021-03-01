#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1992, 1993, 1994 Peter Miller.
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
# MANIFEST: shell script to generate the MANIFEST file
#
echo
echo
for f in $*
do
	case $f in
	MANIFEST)
		info="This file"
		;;
	LICENSE)
		info="GNU General Public License"
		;;
	README)
		info="Blurb about aegis"
		;;
	BUILDING)
		info="Instructions how to build, test and install aegis"
		;;
	aux/CHANGES.*)
		info="Change history of aegis"
		;;
	Makefile*)
		info="Instructions to make(1) how to build and test aegis"
		;;
	common/patchlevel.h)
		info="The patch level of this distribution."
		;;
	doc/version.so)
		info="The patch level of this distribution."
		;;
	aux/new.so)
		info="Include redirection for release notes."
		;;
	aux/config.h*)
		info="Template information for common/config.h.in"
		;;
	aux/template/*)
		info="New file template"
		;;
	lib/aegis.icon)
		info="X Bitmap of the aegis icon"
		;;
	lib/aegis.mask)
		info="X Bitmap of the mask for the aegis icon"
		;;

	configure)
		info="Shell script to automagically configure aegis"
		;;

	common/config.h.in)
		info="Template for configuration definitions.";
		;;

	*)
		info=`gawk '
/[ 	]MANIFEST:[ 	]/ {
	for (j = 1; j < NF; ++j)
		if ($j == "MANIFEST:")
			break;
	for (k = j + 1; k <= NF; ++k)
		printf(" %s", $k);
	printf("\n");
	exit;
}' $f`
		;;
	esac
	echo "$f	$info"
done
exit 0