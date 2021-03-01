#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1997, 2002, 2003, 2006-2008 Peter Miller
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
case $# in
1)
	project=$1
	;;
*)
	echo "Usage: $0 <project>" 1>&2
	exit 1
	;;
esac

aegis=aegis
addr=`$aegis -l developers -terse -p $project`
addr=`echo $addr | sed 's/ /,/g'`

$aegis -db -l -p $project > /tmp/$$
if cmp /dev/null /tmp/$$ > /dev/null 2>&1 ; then
	: do nothing
else
    #
    # Get any aliases for the project
    #
    aliases=`aegis -list Project_Aliases -unf -p $project | awk '{ print $1 }'`
    if [ "$aliases" ]
    then
	# format as comma separated list in brackets
       aliases=" ["`echo $aliases | tr ' ' ','`"]"
    fi

    cat > /tmp/$$.intro << fubar
Subject: Outstanding "${project}$aliases" Changes
To: $addr

The following changes are ready to be developed.  Please
develop them at your earliest possible convenience, so that
changes may pass through the system at the fastest possible
rate.  If you have received this email, you are authorised to
develop changes for the "$project" project.
fubar
	cat /tmp/$$.intro /tmp/$$ | /usr/lib/sendmail -t
	rm /tmp/$$.intro
fi
rm /tmp/$$
exit 0
