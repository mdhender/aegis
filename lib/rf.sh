#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1992, 1993, 1995, 1999 Peter Miller;
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
# MANIFEST: command used to notify by email that a change failed review
#
# Suggested project attribute:
# review_fail_notify_command = "$datadir/rf.sh $p $c $developer $reviewer";
#
aegis=aegis
case $# in
4)
	project=$1
	change=$2
	developer=$3
	reviewer=$4
	;;

*)
	echo "Usage: $0 <project> <change> <developer> <reviewer>" 1>&2
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
# build the notice to be mailed
#
cat > $tmp << 'TheEnd'
The change described below has failed review.
It has been returned to the developer for further work.

TheEnd
if [ $? -ne 0 ]; then quit; fi

#
# include full details of the change
#
$aegis -list change_details -project $project -change $change -verbose >> $tmp
if [ $? -ne 0 ]; then quit; fi

#
# mail it to the developer and the reviewer
#
mail $developer $reviewer < $tmp
if [ $? -ne 0 ]; then quit; fi

#
# clean up and go home
#
rm $tmp
exit 0
