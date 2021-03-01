#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1996, 2002, 2006, 2008 Peter Miller
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
awkfile=test/activity.awk
for dir in . bl blbl bl/bl blblbl bl/bl/bl blblblbl bl/bl/bl/bl
do
    if test -r $dir/$awkfile
    then
	awkfile=$dir/$awkfile
	break
    fi
done
for f in test/*/*.sh
do
	echo $f...
	if awk -f $awkfile < $f > $f.new
	then
	    mv $f $f.orig
	    mv $f.new $f
	else
	    rm $f.new
	fi
done
exit 0
