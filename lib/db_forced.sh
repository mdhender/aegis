#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1994, 1995, 1999-2004, 2006-2008 Peter Miller
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
# forced_develop_begin_notify_command =
#	"$datadir/db_forced.sh $p $c $developer";
#
aegis=aegis
case $# in
3)
	project="$1"
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
# Get any aliases for the project
#
aliases=`aegis -list Project_Aliases -unf -p $project | awk '{print $1}'`
if [ "$aliases" ]
then
    # format as comma separated list in brackets
   aliases=" ["`echo $aliases | tr ' ' ','`"]"
fi

#
# Note the double quotes: the ${email_address} is acted on by aesub BUT
# the $developer is a shell environment variable set earlier in this
# script.
#
to=`aesub "\\\${email_address -comma $developer}" `

#
# build the notice to be mailed
#
cat > $tmp << TheEnd
Subject: Project ${project}$aliases: Change $change: Develop Begin
To: $to

Development of the following change has been assigned to you.

TheEnd
if [ $? -ne 0 ]; then quit; fi

#
# include full details of the change
#
$aegis -list change_details -project $project -change $change -verbose \
	-pl=66 -pw=68 >> $tmp
if [ $? -ne 0 ]; then quit; fi

#
# mail it to the developer
#
/usr/lib/sendmail -t < $tmp
if [ $? -ne 0 ]; then quit; fi

#
# clean up and go home
#
rm $tmp
exit 0
