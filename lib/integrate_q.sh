#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1995 Peter Miller;
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
# MANIFEST: shell script to automatically process the integration queue
#
# This shell script assumes that the aegis command is somewhere in the
# command search PATH.
#
# Note: this shell script should be run daily from the per-user crontab of
# one of the integrators, it cannot be run as root.
#
# This script assumes there will not be any collisions,
# so it does not check to see if itself is already running
#

#
# location of the aegis command
#
aegis=aegis

#
# location of the mail command
#
mail=mail

#
# figure out who we are
#
USER=${USER:-${LOGNAME:-`whoami`}}

case $# in
1)
	project=$1
	;;
*)
	echo "usage: $0 project-name" 1>&2
	;;
esac

tmp=/tmp/intq.$$
cat > $tmp.dev << 'fubar'
	columns(80);
	auto cs;
	cs = project[project_name()].state.change[change_number()];
	auto developer;
	developer = "nobody";
	auto h;
	for (h in cs.history)
		if (h.what == develop_end)
			developer = h.who;
	print(developer);
fubar
if test $? -ne 0 ; then exit 1; fi

tmp=/tmp/intq.$$
cat > $tmp.test << 'fubar'
	columns(80);
	auto cs;
	cs = project[project_name()].state.change[change_number()];
	print("test=" ## !cs.test_exempt ## ";");
	print("test_baseline=" ## !cs.test_baseline_exempt ## ";");
	print("regression_test=" ## !cs.regression_test_exempt ## ";");
fubar
if test $? -ne 0 ; then exit 1; fi

changes=`$aegis -ib -l -p $project -ter`
if test $? -ne 0 ; then exit 1; fi

for change in $changes
do
	#
	# run the commands in a subshell, so that we can
	# continue the loop by using the exit command
	# (See below, the subshell output is redirected.)
	#
	(

	#
	# begin integration of the change
	#
	echo "$ aegis -ib -p $project -c $change -v"
	$aegis -ib -p $project -c $change -v
	if test $? -ne 0 ; then
		$mail $USER < $tmp
		exit 1
	fi

	# 
	# find out who the developer is,
	# in case we need to send email
	#
	developer=`$aegis -report -p $project -c $change -file $tmp.dev -ter`
	if test $? -ne 0 ; then exit 1; fi

	#
	# build the change
	#
	echo "$ aegis -b -p $project -c $change -v"
	$aegis -b -p $project -c $change -v -nolog
	if test $? -ne 0 ; then
		mail $developer < $tmp
		tail -10 $tmp > $tmp.2
		$aegis -ifail -f $tmp.2
		rm $tmp.2
		exit 1
	fi

	#
	# find out which tests we need to run
	#
	tests=`$aegis -report -p $project -c $change -file $tmp.test -ter`
	if test $? -ne 0 ; then exit 1; fi
	eval $tests

	#
	# test the change
	#
	if test $test = "true" ; then
		echo "$ aegis -t -p $project -c $change -v"
		$aegis -t -p $project -c $change -v -nolog
		if test $? -ne 0 ; then
			mail $developer < $tmp
			tail -10 $tmp > $tmp.2
			$aegis -ifail -f $tmp.2
			rm $tmp.2
			exit 1
		fi
	fi

	#
	# test -bl the change
	#
	if test $test_baseline = "true" ; then
		echo "$ aegis -t -bl -p $project -c $change -v"
		$aegis -t -bl -p $project -c $change -v -nolog
		if test $? -ne 0 ; then
			mail $developer < $tmp
			tail -10 $tmp > $tmp.2
			$aegis -ifail -f $tmp.2
			rm $tmp.2
			exit 1
		fi
	fi

	#
	# test -reg the change
	#
	if test $regression_test = "true" ; then
		echo "$ aegis -t -reg -p $project -c $change -v"
		$aegis -t -reg -p $project -c $change -v -nolog
		if test $? -ne 0 ; then
			mail $developer < $tmp
			tail -10 $tmp > $tmp.2
			$aegis -ifail -f $tmp.2
			rm $tmp.2
			exit 1
		fi
	fi

	#
	# pass the integration
	#
	echo "$ aegis -ipass -p $project -c $change -v" > $tmp
	$aegis -ipass -p $project -c $change -v -nolog >> $tmp 2>&1
	if test $? -ne 0 ; then
		mail $developer < $tmp
		tail -10 $tmp > $tmp.2
		$aegis -ifail -f $tmp.2
		rm $tmp.2
		exit 1
	fi

	#
	# output from the shell script is entirely
	# redirected into the log file
	# remove the log file after each (attempted) integration
	#
	) < /dev/null > $tmp 2>&1
	rm $tmp
done

#
# clean up and files we map have left behind
#
rm ${tmp}*
exit 0
