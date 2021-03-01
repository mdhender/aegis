#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2006, 2008 Peter Miller
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
output=
here=`pwd`
test $? -eq 0 || exit 1
tmp=${TMP_DIR:-/tmp}/$$

fail() {
    cd $here
    rm -rf $tmp
    exit 1
}
trap "fail" 1 2 3 15

set -x
mkdir $tmp $tmp/lib
test $? -eq 0 || fail
cd $tmp
test $? -eq 0 || fail

arch=$1
shift
bin=$here/$arch/bin

AEGIS_PATH=$tmp/lib
export AEGIS_PATH

chmod 0777 $tmp/lib
test $? -eq 0 || fail

AEGIS_PROJECT=example
export AEGIS_PROJECT

USER=`whoami`
export USER

AEGIS_FLAGS="delete_file_preference = no_keep; \
	lock_wait_preference = always; \
	diff_preference = automatic_merge; \
	pager_preference = never; \
	persevere_preference = all; \
	log_file_preference = never;"

#
# Create a new project
#
$bin/aegis -new-project $AEGIS_PROJECT --version=- --dir=$tmp/proj.dir
test $? -eq 0 || fail

#
# add minimal staff, we wont actually be completing any change sets.
#
$bin/aegis --new-dev --project=$AEGIS_PROJECT $USER
test $? -eq 0 || fail

AEGIS_CHANGE=1
export AEGIS_CHANGE

#
# Create a change set.
#
cat > caf << 'fubar'
brief_description = "minimum initial configuration";
description =
    "This change set creates the minimum Aegis configuration to get a "
    "small project up and running quickly.  It does not include any "
    "source files, as those are expected to be supplied by release "
    "tarballs imported using the aetar command."
    ;
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
test $? -eq 0 || fail

$bin/aegis --new-change -f caf \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE
test $? -eq 0 || fail

#
# Begin development of the change
#
$bin/aegis --develop-begin -dir $tmp/chan.dir -v -nolog \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE
test $? -eq 0 || fail

#
# Create the top-level aegis.conf file,
# pointing into the aegis.conf.d directory.
#
$bin/aegis -nf $tmp/chan.dir/aegis.conf -v -nolog \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
	> log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

echo 'configuration_directory = "aegis.conf.d";' > $tmp/chan.dir/aegis.conf
test $? -eq 0 || fail

#
# Create the build file.  It says we don't do builds at all.
# We will have to wait until there is some content for that to be useful.
#
$bin/aegis -nf $tmp/chan.dir/aegis.conf.d/build -v -nolog \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
	> log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

echo 'build_command = "exit 0";' > $tmp/chan.dir/aegis.conf.d/build
test $? -eq 0 || fail

#
# Add anything else given on the command line.
# (This includes history, diff, merge, etc.)
#
while [ $# -ge 1 ]
do
    case "$1" in
    output=*)
	output=`echo $1 | sed 's|.*=||'`
	;;

    *=*)
	name=`echo $1 | sed 's|=.*||'`
	value=`echo $1 | sed 's|.*=||'`

	$bin/aegis -nf $tmp/chan.dir/$name -v -nolog \
	    --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
	    > log 2>&1
	if test $? -ne 0 ; then cat log; fail; fi

	cp $here/$value $tmp/chan.dir/$name
	test $? -eq 0 || fail
	;;

    *)
	echo "$0 name=value ..." 1>&2
	fail
	;;
    esac
    shift

done

if test -z "$output"
then
    echo "No output=[target] option given."
    fail
fi

#
# Package the change set.
#
$bin/aedist -send -ndh -mh -naa -comp-alg=gzip \
	--ignore-uuid \
	--output="$here/$output" \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE

cd $here
rm -rf $tmp

exit 0
