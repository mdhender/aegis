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
# MANIFEST: command used to notify by email that a change review passed is rescinded
#
aegis=aegis
case $# in
3)
	project=$1
	change=$2
	developer=$3
	;;

*)
	echo "Usage: $0 <project> <change> <developer>" 1>&2
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
# find out who the reviewers are
#
reviewers=`$aegis -list reviewers -project $project -terse`
if [ $? -ne 0 ]; then quit; fi
if [ "$reviewers" = "" ]
then
	reviewers=`$aegis -list administrators -project $project -terse`
	if [ $? -ne 0 ]; then quit; fi
fi

#
# build the notice to be mailed
#
cat > $tmp << 'TheEnd'
The change described below has had its review
pass rescinded, pending further review.

TheEnd
if [ $? -ne 0 ]; then quit; fi

#
# include full details of the change
#
$aegis -list change_details -project $project -change $change -verbose >> $tmp
if [ $? -ne 0 ]; then quit; fi

#
# mail it to the developer and all reviewers
#
mail $developer $reviewers < $tmp
if [ $? -ne 0 ]; then quit; fi

#
# clean up and go home
#
rm $tmp
exit 0;
