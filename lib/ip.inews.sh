#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1992-2008 Peter Miller
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program. If not, see
#	<http://www.gnu.org/licenses/>.
#
# Suggested project attribute:
# integrate_pass_notify_command =
#	"$datadir/ip.inews.sh $p $c a.local.newsgroup";
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
Subject: project $project, change $change, integrate pass
Newsgroups: $newsgroups

The change described below has passed integration
and is now part of the baseline.

TheEnd
if [ $? -ne 0 ]; then quit; fi

#
# include full details of the change
#
$aegis -list change_details -project $project -change $change -verbose \
	-pl=66 -pw=80 >> $tmp
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
