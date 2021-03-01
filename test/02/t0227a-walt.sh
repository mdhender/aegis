#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 2005 Walter Franzini;
#	All rights reserved.
#       Copyright (C) 2007, 2008 Peter Miller
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
	log_file_preference = never; \
	default_development_directory = \"$work\";"
export AEGIS_FLAGS
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

# This tells aeintegratq that it is being used by a test.
AEGIS_TEST_DIR=$work
export AEGIS_TEST_DIR

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

#
# set the path, so that the aegis command that aepatch/aedist invokes
# is from the same test set as the aepatch/aedist command itself.
#
PATH=${bin}:$PATH
export PATH

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
	echo "FAILED test of the cpio functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 1
}
no_result()
{
	set +x
	echo "NO RESULT when testing the cpio functionality ($activity)" 1>&2
	cd $here
	find $work -type d -user $USER -exec chmod u+w {} \;
	rm -rf $work
	exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory"
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
# If the C++ compiler is called something other than "c++", as
# discovered by the configure script, create a shell script called
# "c++" which invokes the correct C++ compiler.  Make sure the current
# directory is in the path, so that it will be invoked.
#
if test "$CXX" != "c++"
then
	cat >> $work/c++ << fubar
#!/bin/sh
exec ${CXX-g++} \$*
fubar
	if test $? -ne 0 ; then no_result; fi
	chmod a+rx $work/c++
	if test $? -ne 0 ; then no_result; fi
	PATH=${work}:${PATH}
	export PATH
fi

#
# test the test_cpio functionality
#
activity="test_cpio -create"

mkdir $work/in
mkdir $work/in/dante

cat > $work/in/dante/c1.01 <<'EOF'
Nel mezzo del cammin di nostra vita
mi ritrovai per una selva oscura
che` la diritta via era smarrita.
EOF

cat > $work/in/dante/c1.02 <<'EOF'
Ahi quanto a dir qual era  cosa dura
esta selva selvaggia e aspra e forte
che nel pensier rinova la paura!
EOF

cat > $work/in/dante/c1.03 <<'EOF'
Tant'e` amara che poco e` piu` morte;
ma per trattar del ben ch'i' vi trovai,
diro` de l'altre cose ch'i' v'ho scorte.
EOF

cat > $work/list.ok <<EOF
dante/c1.01
dante/c1.02
dante/c1.03
EOF
if test $? -ne 0; then no_result; fi

cd $work

$bin/test_cpio -create -f dante.cpio -cd $work/in \
    dante/c1.01 dante/c1.02 dante/c1.03 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

$bin/test_cpio -list -f dante.cpio > $work/list
if test $? -ne 0; then fail; fi

cmp $work/list.ok $work/list > log 2>&1
if test $? -ne 0; then cat log; fail; fi

mkdir out
$bin/test_cpio -extract -cd $work/out -f $work/dante.cpio > log 2>&1
if test $? -ne 0; then cat log; fail; fi

diff -b $work/in/dante/c1.01 $work/out/dante/c1.01 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

cmp $work/in/dante/c1.02 $work/out/dante/c1.02 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

cmp $work/in/dante/c1.03 $work/out/dante/c1.03 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
