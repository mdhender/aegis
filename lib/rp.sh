#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1992, 1993, 1995, 1999-2004, 2006-2008 Peter Miller
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
# review_pass_notify_command = "$datadir/rp.sh $p $c $developer $reviewer";
#
aegis=aegis
aesub=aesub
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
# find out who the integrators are
#
integrators=`$aegis -list integrators -project $project -terse`
if [ $? -ne 0 ]; then quit; fi
if [ "$integrators" = "" ]
then
	integrators=`$aegis -list administrators -project $project -terse`
	if [ $? -ne 0 ]; then quit; fi
fi

#
# Get any aliases for the project
#
aliases=`aegis -list Project_Aliases -unf -p $project | awk '{ print $1 }'`
if [ "$aliases" ]
then
    # format as comma separated list in brackets
    aliases=" ["`echo $aliases | tr ' ' ','`"]"
fi

state=`$aesub '$state' -c $change -p $project `
case $state in
awaiting_integration)
    coda="It is now awaiting integration."
    #
    # Note the double quotes: the ${email_address} is acted on by aesub
    # BUT the $developer and $integrators are a shell environment
    # variables set earlier in this script.
    #
    to=`$aesub -p $project "\\\${email_address -comma $developer $integrators}"`
    ;;
awaiting_review | being_reviewed)
    coda="It is still awaiting further review."
    to=`$aesub -p $project '\${email_address -comma $reviewer_list}'`
    ;;
*)
    coda="It is now $state"
    to=`$aesub -p $project '${email_addr -comma $administrator_list}'`
    ;;
esac

#
# build the notice to be mailed
#
cat > $tmp << TheEnd
Subject: Project ${project}$aliases: Change $change: passed review
To: $to

The change described below has passed review.
$coda

TheEnd
if [ $? -ne 0 ]; then quit; fi

#
# include full details of the change
#
$aegis -list change_details -project $project -change $change -verbose \
	-pl=66 -pw=80 >> $tmp
if [ $? -ne 0 ]; then quit; fi

#
# mail it to the developer and all integrators
#
/usr/lib/sendmail -t < $tmp
if [ $? -ne 0 ]; then quit; fi

#
# clean up and go home
#
rm $tmp
exit 0;
