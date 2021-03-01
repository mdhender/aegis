#! /bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1992, 1993 Peter Miller.
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
# MANIFEST: command used to notify by usenet that a change failed integration
#
aegis=aegis
case $# in
3)
	project=$1
	change=$2
	newsgroups=$3
	;;

*)
	echo "Usage: $0 <project> <change> <newsgroups>" 1>&2
	exit 1
	;;
esac

tmp=/tmp/de.$$
trap "rm -f $tmp" 1 2 3 15
quit()
{
	rm -f $tmp
	exit 1
}

#
# build the notice
#
cat > $tmp << TheEnd
Subject: project $project, change $change, integrate fail
Newsgroups: $newsgroups

The change described below has failed integration.
It has been returned to the developer for further work.

TheEnd
if [ $? -ne 0 ]; then quit; fi

#
# include full details of the change
#
$aegis -list change_details -project $project -change $change -verbose >> $tmp
if [ $? -ne 0 ]; then quit; fi

#
# post on usenet
#
inews -h < $tmp
if [ $? -ne 0 ]; then quit; fi

#
# clean up and go home
#
rm $tmp
exit 0
