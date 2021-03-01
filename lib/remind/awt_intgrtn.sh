#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1997 Peter Miller;
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
# MANIFEST: shell script to remind users of changes which need to be integrated
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
addr=`$aegis -l integrators -terse -p $project`
$aegis -ib -l -p $project > /tmp/$$
if cmp /dev/null /tmp/$$ > /dev/null 2>&1 ; then
	: do nothing
else
	cat > /tmp/$$.intro << fubar
Subject: Outstanding "$project" Integrations

The following changes are ready to be integrated.  Please
integrate them at your earliest possible convenience, so that
changes may pass through the system at the fastest possible
rate.  If you have received this email, you are authorised to
integrate changes for the "$project" project.
fubar
	cat /tmp/$$.intro /tmp/$$ | mail $addr
	rm /tmp/$$.intro
fi
rm /tmp/$$
exit 0
