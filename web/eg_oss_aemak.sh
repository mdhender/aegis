#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2007, 2008 Peter Miller
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 3 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program. If not, see
#       <http://www.gnu.org/licenses/>.
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
brief_description = "build using GNU Auto Tools";
description =
    "This change set adds the necessary file to be able to build the "
    "project using GNU Automake, Autoconf and Libtool.  This method "
    "creates the necessary project source files so that you can start a "
    "new project from scratch.\n\n"
    "You need to read aemakegen(1) for the expected project directory "
    "layout."
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

#  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

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

project_file_command = "rm -f Makefile*";
change_file_command = "rm -f Makefile*";
integrate_begin_exceptions =
[
    "bin/*",
    "Makefile*",
];
fubar
test $? -eq 0 || fail

#  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

#
# now create the boostrapping makefile
#
$bin/aegis -nf $tmp/chan.dir.2/aegis.supplementary/makefile -v -nolog \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
        > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

TAB=`echo x|awk '{print "\t"}'`
sedcmd="s|        |${TAB}|g"
sed "$sedcmd" > $tmp/chan.dir.2/aegis.supplementary/makefile << 'fubar'
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

aclocal.m4: configure.ac
        aclocal

# if you don't use config.h you will need to delete this rule
lib/config.h.in: configure.ac aclocal.m4
        mkdir -p lib
        autoheader

./configure: configure.ac aclocal.m4 ltmain.sh
        autoconf

Makefile.in: Makefile.am configure.ac lib/config.h.in \
                AUTHORS ChangeLog NEWS README ltmain.sh
        automake -a

Makefile lib/config.h: Makefile.in lib/config.h.in configure
        ./configure --prefix=/usr --sysconfdir=/etc

AUTHORS ChangeLog NEWS README:
        touch $@

ltmain.sh:
        libtoolize --copy --force

endif

#
# Build the Makefile.am file from the Aegis file manifest, if it does
# not exist.
#
Makefile.am:
        aemakegen --project=$(AEGIS_PROJECT) --change=$(AEGIS_CHANGE) \
                --target=automake -output=$@
endif
fubar
test $? -eq 0 || fail

#  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

$bin/aegis -nf $tmp/chan.dir.2/configure.ac -v -nolog \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
        > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > $tmp/chan.dir.2/configure.ac << 'fubar'
dnl
dnl    You will need to manually replace the project name and project
dnl    version in the following lines, so that they match your project.
dnl    When you are more familiar with Aegis, you can automate this.
dnl
AC_INIT([project-name], [project-version])
AC_CONFIG_HEADER(lib/config.h)
AC_CONFIG_SRCDIR(configure.ac)
AM_INIT_AUTOMAKE([project-name], [project-version])

dnl put the ACLOCAL flags in the Makefile
ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL

if $CC --version | grep '(GCC) [4-9]' > /dev/null 2>&1
then
    CFLAGS="$CFLAGS -Wall -Wextra -Werror"
    CXXFLAGS="$CXXFLAGS -Wall -Wextra -Werror"
fi

AC_ENABLE_STATIC
AC_DISABLE_SHARED
AM_PROG_LIBTOOL

AH_TOP(
[#ifndef LIB_CONFIG_H
#define LIB_CONFIG_H
])
AH_BOTTOM(
[#endif // LIB_CONFIG_H
])

AC_OUTPUT(Makefile lib/libdir.cc)
fubar
test $? -eq 0 || fail

#  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

$bin/aegis -nf $tmp/chan.dir.2/lib/libdir.cc.in -v -nolog \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
        > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > $tmp/chan.dir.2/lib/libdir.cc.in << 'fubar'
//
// @configure_input@
//

#include <cstdlib>

#include <lib/libdir.h>


const char *
configured_prefix(void)
{
    return "@prefix@";
}


//
// On a network, may ONLY be shared betwen machines of identical
// cpu-vendor-os flavour.  Probably mounted read-only.
//
// This is where the executable files live.
//

const char *
configured_bindir(void)
{
    return "@bindir@";
}


//
// On a network, may ONLY be shared betwen machines of identical
// cpu-vendor-os flavour.  Probably mounted read-only.
//

const char *
configured_libdir(void)
{
    return "@libdir@";
}


//
// On a network, may be shared between all machines.
// Probably mounted read-only.
//
// This is where the various translations of the user documentation are
// parked, all documents, papers and manual pages, for all languages.
//
// This is where the various shell scripts are parked.
//
// This is where the report scripts are parked.
//

const char *
configured_datadir(void)
{
    return "@datadir@";
}


//
// On a network, may be shared betwen all machines.
// Probably mounted read-only.
//
// This is where the default-language manual entries are parked.
//

const char *
configured_mandir(void)
{
    return "@mandir@";
}


//
// On a network, must be shared betwen all machines.
// MUST BE mounted Read-Write.
//

const char *
configured_comdir(void)
{
    return "@sharedstatedir@";
}


const char *
configured_sysconfdir(void)
{
    return "@sysconfdir@";
}
fubar
test $? -eq 0 || fail

#  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

$bin/aegis -nf $tmp/chan.dir.2/lib/libdir.h -v -nolog \
        --project=$AEGIS_PROJECT --change=$AEGIS_CHANGE \
        > log 2>&1
if test $? -ne 0 ; then cat log; fail; fi

cat > $tmp/chan.dir.2/lib/libdir.h << 'fubar'
#ifndef LIB_LIBDIR_H
#define LIB_LIBDIR_H

/**
  * The configured_prefix function is used to retrieve the prefix
  * determined the ./configure script.
  */
const char *configured_prefix(void);

/**
  * The configured_bindir function is used to retrieve the directory
  * used to hold executable program files determined the ./configure
  * script.
  */
const char *configured_bindir(void);

/**
  * The configured_datadir function is used to retrieve the library
  * directory (architecture neutral) determined the ./configure script.
  */
const char *configured_datadir(void);

/**
  * The configured_libdir function is used to retrieve the library
  * directory (architecture specific) determined the ./configure script.
  */
const char *configured_libdir(void);

/**
  * The configured_mandir function is used to retrieve the manual page
  * directory determined the ./configure script.
  */
const char *configured_mandir(void);

/**
  * The configured_sysconfdir function is used to retrieve the system
  * configuraton directory determined the ./configure script.
  */
const char *configured_sysconfdir(void);

#endif // LIB_LIBDIR_H
fubar
test $? -eq 0 || fail

#  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -

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
