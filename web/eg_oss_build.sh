#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2006-2008 Peter Miller
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

activity="project attributes 147"
cat > paf << fubar
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
default_development_directory = "$tmp";
fubar
test $? -eq 0 || fail

$bin/aegis -pa -f paf --project=$AEGIS_PROJECT
test $? -eq 0 || fail

#
# add project staff
#
$bin/aegis --new-dev --project=$AEGIS_PROJECT $USER
test $? -eq 0 || fail

$bin/aegis --new-rev --project=$AEGIS_PROJECT $USER
test $? -eq 0 || fail

$bin/aegis --new-int --project=$AEGIS_PROJECT $USER
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
$bin/aegis --develop-begin -dir $tmp/chan.dir -nolog \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
	> log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

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

$bin/aegis -diff -v -nolog \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
	> log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -dev-end \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE
test $? -eq 0 || fail

# now integrate it

$bin/aegis -integrate-begin -v -nolog \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
	> log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -diff -v -nolog \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
	> log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -int-pass -v -nolog \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
	> log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

# --------------------------------------------------------------------------

AEGIS_CHANGE=2
export AEGIS_CHANGE

#
# Now the second change, which adds the build.
#
cat > caf << 'fubar'
brief_description = "build via make";
description =
    "This change set adds the necessary file to be able to build "
    "the project using GNU Make.  This method does not change any "
    "project source files, but instead adds a supplementary makefile "
    "to bootstrap creating the real Makefile using the GNU Auto Tools."
    ;
cause = internal_enhancement;
test_exempt = true;
test_baseline_exempt = true;
regression_test_exempt = true;
fubar
test $? -eq 0 || fail

$bin/aegis --new-change -f caf \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE
test $? -eq 0 || fail

#
# Begin development of the change
#
$bin/aegis --dev-begin -dir $tmp/chan.dir.2 -v -nolog \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
	> log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

$bin/aegis -copy-file $tmp/chan.dir.2/aegis.conf.d/build -v -nolog \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
	> log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > $tmp/chan.dir.2/aegis.conf.d/build << 'fubar'
build_command = "make -f aegis.supplementary/makefile";

development_directory_style =
{
    /* Use the first of these that work: */
    source_file_link = true;
    source_file_symlink = true;
    source_file_copy = true;

    /*
     * Derived files are those constructed by the build, all files in
     * the baseline which are not primary source files.
     */
    derived_file_copy = true;
    derived_at_start_only = true;
};
fubar
test $? -eq 0 || fail

#
# now create the boostrapping makefile
#
$bin/aegis -nf $tmp/chan.dir.2/aegis.supplementary/makefile -v -nolog \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
	> log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > $tmp/chan.dir.2/aegis.supplementary/makefile << 'fubar'
#
# Include the real Makefile, if it exists, and build it if it does not.
#
include Makefile

#
# If the "Makefile" file exists it means that the "configure" script
# has been run and ${prefix} will have been set.  The following rules
# will only be used when we need to bootstrap to the point where the
# real "Makefile" file exists.
#
ifeq (${prefix},)

supplementary-all: all

ifeq (${wildcard configure.in},configure.in)

aclocal.m4: configure.in
	aclocal

# if you don't use config.h you will need to delete this rule
config.h.in: configure.in aclocal.m4
	autoheader

./configure: configure.in aclocal.m4
	autoconf

ifeq (${wildcard Makefile.am},Makefile.am)

Makefile.in: Makefile.am configure.in config.h.in
	automake -a

endif

else

aclocal.m4: configure.ac
	aclocal

# if you don't use config.h you will need to delete this rule
config.h.in: configure.ac aclocal.m4
	autoheader

./configure: configure.ac aclocal.m4
	autoconf

ifeq (${wildcard Makefile.am},Makefile.am)

Makefile.in: Makefile.am configure.ac config.h.in
	automake -a

endif

endif

# If you don't use config.h you will need to delete both references from
# this rule.
Makefile config.h: Makefile.in config.h.in configure
	./configure --prefix=/usr --sysconfdir=/etc

endif
fubar
test $? -eq 0 || fail

#
# Package the change set.
#
$bin/aedist -send -ndh -mh -naa -comp-alg=gzip -compat=4.6 \
	--ignore-uuid \
	--output="$here/$output" \
	--project=$AEGIS_PROJECT --change=$AEGIS_CHANGE
test $? -eq 0 || fail

# --------------------------------------------------------------------------

cd $here
rm -rf $tmp

exit 0
