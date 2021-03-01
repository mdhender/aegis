#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1999, 2000 Peter Miller;
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
# MANIFEST: Test the batch_test_command functionality
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
umask 022

LINES=24
export LINES
COLS=80
export COLS

USER=${USER:-${LOGNAME:-`whoami`}}

work=${AEGIS_TMP:-/tmp}/$$
PAGER=cat
export PAGER
AEGIS_FLAGS="delete_file_preference = no_keep; \
	lock_wait_preference = always; \
	diff_preference = automatic_merge; \
	pager_preference = never; \
	persevere_preference = all; \
	log_file_preference = never;"
export AEGIS_FLAGS
AEGIS_THROTTLE=2
export AEGIS_THROTTLE

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

bin=$here/${1-.}/bin

pass()
{
	set +x
	echo PASSED 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 0
}
fail()
{
	set +x
	echo 'FAILED test of the batch_test_command functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo 'NO RESULT when testing the batch_test_command functionality' 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the batch_test_command functionality
#
$bin/aegis -npr test -version "" -lib $work/lib -dir $work/proj -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -nd $USER -p test -lib $work/lib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

cat > caf << 'fubar'
brief_description = "not much";
cause = internal_enhancement;
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nc -f caf -p test -lib $work/lib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -db 10 -p test -lib $work/lib -dir $work/chan -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -nf $work/chan/config -c 10 -p test -lib $work/lib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

cat > $work/chan/config << 'fubar'
build_command = "exit 0";
history_create_command = "exit 0";
history_get_command = "exit 0";
history_put_command = "exit 0";
history_query_command = "exit 0";
diff_command = "exit 0";
merge_command = "exit 0";
batch_test_command = "sh ${source tester.sh} $filenames -o $output";
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nf $work/chan/tester.sh -c 10 -p test -lib $work/lib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

cat > $work/chan/tester.sh << 'fubar'
#!/bin/sh
f=
while test $# -ge 1
do
	case $1 in
	-o)
		shift
		output=$1
		shift
		;;
	*)
		f="$f $1"
		shift
		;;
	esac
done
echo 'test_result=[' > $output
for ff in $f
do
	echo "{file_name=\"$ff\";exit_status=0;}," >> $output
done
echo '];' >> $output
fubar
if test $? -ne 0 ; then no_result; fi

$bin/aegis -nt -c 10 -p test -lib $work/lib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi
echo 'exit 1' > $work/chan/test/00/t0001a.sh
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -nt -c 10 -p test -lib $work/lib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi
echo 'exit 1' > $work/chan/test/00/t0002a.sh
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -nt -c 10 -p test -lib $work/lib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi
echo 'exit 1' > $work/chan/test/00/t0003a.sh
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -nt -c 10 -p test -lib $work/lib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi
echo 'exit 1' > $work/chan/test/00/t0004a.sh
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -nt -c 10 -p test -lib $work/lib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi
echo 'exit 1' > $work/chan/test/00/t0005a.sh
if test $? -ne 0 ; then cat LOG; no_result; fi

$bin/aegis -test -c 10 -p test -lib $work/lib -v > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
