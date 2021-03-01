#!/bin/sh
#
# aegis - project change supervisor.
# Copyright (C) 2008, 2010-2012 Peter Miller
# Copyright (C) 2008 Walter Franzini
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>.
#

TEST_SUBJECT="aemakegen vs libtool"

# load up standard prelude and test functions
. test_funcs

activity="new project 26"
aegis -npr test -version - -v -dir $work/proj.dir \
    -lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="project attributes 34"
cat > paf << fubar
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
default_development_directory = "$work";
fubar
if test $? -ne 0 ; then no_result; fi
aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 47"
aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 55"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 65"
aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 69"
aegis -nf $work/test.C010/aegis.conf \
        $work/test.C010/lib/bar.c \
        $work/test.C010/lib/foo.c \
        $work/test.C010/fred/main.c \
        $work/test.C010/barney/main.c -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.C010/aegis.conf << 'fubar'
build_command = "exit 0";

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
    echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;

test_command = "$sh $filename $arch";

project_specific =
[
    { name = "aemakegen:libtool"; value = "true"; },
];
fubar
if test $? -ne 0 ; then no_result; fi

TAB=`awk 'BEGIN{printf("%c", 9)}' /dev/null`

sed "s|{TAB}|$TAB|g" > test.ok << 'fubar'
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#
#    W   W    A    RRRR   N   N   III  N   N  III  N   N   GGG
#    W   W   A A   R   R  NN  N    I   NN  N   I   NN  N  G   G
#    W   W  A   A  RRRR   N N N    I   N N N   I   N N N  G
#    W W W  AAAAA  R R    N  NN    I   N  NN   I   N  NN  G  GG
#    W W W  A   A  R  R   N   N    I   N   N   I   N   N  G   G
#     W W   A   A  R   R  N   N   III  N   N  III  N   N   GGG
#
# Warning: DO NOT send patches which fix this file. IT IS NOT the original
# source file. This file is GENERATED from the Aegis repository file manifest.
# If you find a bug in this file, it could well be an Aegis bug.
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#

#
# @configure_input@
#
# The configure script generates 2 files:
# 1. This Makefile
# 2. lib/config.h
# If you change this Makefile, you may also need to change these files. To see
# what is configured by the configure script, search for @ in the Makefile.in
# file.
#
# If you wish to reconfigure the installations directories it is RECOMMENDED
# that you re-run the configure script.
#
# Use "./configure --help" for a list of options.
#

#
# directory containing the source
#
srcdir = @srcdir@
VPATH = @srcdir@

#
# the name of the install program to use
#
INSTALL = @INSTALL@
INSTALL_PROGRAM = @INSTALL_PROGRAM@
INSTALL_DATA = @INSTALL_DATA@
INSTALL_DIR = @INSTALL@ -m 0755 -d

#
# The name of the C compiler to use.
#
CC = @CC@

#
# The C compiler flags to use.
#
CFLAGS = @CFLAGS@

#
# The C preprocessor flags to use.
#
CPPFLAGS = @CPPFLAGS@

#
# The linker flags to use
#
LDFLAGS = @LDFLAGS@

#
# prefix for installation path
#
prefix = @prefix@
exec_prefix = @exec_prefix@

#
# Where to put the executables.
#
# On a network, this would only be shared between machines of identical
# cpu-hw-os flavour. It can be read-only.
#
# The $(DESTDIR) is for packaging.
#
bindir = $(DESTDIR)@bindir@

# define this to silence ./configure warning
datarootdir = $(DESTDIR)@datarootdir@

#
# Where to put the supplementary package executables.
#
# On a network, this would be shared between all machines of identical
# architecture. It can be read-only.
#
# The $(DESTDIR) is for packaging.
#
libdir = $(DESTDIR)@libdir@

#
# The name of the GNU Libtool command.
#
LIBTOOL = @LIBTOOL@

#
# extra libraries required for your system
#
LIBS = @LIBS@

#
# shell to use to run tests and commands
#
SH = @SH@

# ---------------------------------------------------------
# You should not need to change anything below this line.

#
# The default target
#
the-default-target: all

barney/main.lo barney/main.o: barney/main.c
{TAB}$(LIBTOOL) --mode=compile --tag=CC $(CC) $(CPPFLAGS) $(CFLAGS) -I. -c \
{TAB}{TAB}barney/main.c -o barney/main.lo

fred/main.lo fred/main.o: fred/main.c
{TAB}$(LIBTOOL) --mode=compile --tag=CC $(CC) $(CPPFLAGS) $(CFLAGS) -I. -c \
{TAB}{TAB}fred/main.c -o fred/main.lo

lib/bar.lo lib/bar.o: lib/bar.c
{TAB}$(LIBTOOL) --mode=compile --tag=CC $(CC) $(CPPFLAGS) $(CFLAGS) -I. -c \
{TAB}{TAB}lib/bar.c -o lib/bar.lo

lib/foo.lo lib/foo.o: lib/foo.c
{TAB}$(LIBTOOL) --mode=compile --tag=CC $(CC) $(CPPFLAGS) $(CFLAGS) -I. -c \
{TAB}{TAB}lib/foo.c -o lib/foo.lo

$(libdir)/pkgconfig/libtest.pc: .mkdir.__libdir__pkgconfig \
{TAB}{TAB}libdir/pkgconfig/libtest.pc
{TAB}$(INSTALL_DATA) libdir/pkgconfig/libtest.pc $@

#
# The lib/libtest.la library.
#
lib_obj = lib/bar.lo lib/foo.lo

lib/libtest.la: $(lib_obj)
{TAB}rm -f $@
{TAB}$(LIBTOOL) --mode=link --tag=CC $(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) \
{TAB}{TAB}-o $@ $(lib_obj) $(LIBS) -rpath $(libdir) -version-info 0:0:0

#
# The install of the *.la file automatically causes "$(LIBTOOL) --mode=install"
# to install the *.a and *.so* files as well, which is why you don't see them
# explicitly mentioned here.
#
$(libdir)/libtest.la: .mkdir.__libdir_ lib/libtest.la
{TAB}$(LIBTOOL) --mode=install $(INSTALL_DATA) lib/libtest.la $@

.mkdir.__bindir_:
{TAB}-$(INSTALL_DIR) $(bindir)
{TAB}@-test -d $(bindir) && touch $@
{TAB}@sleep 1

.mkdir.__libdir_:
{TAB}-$(INSTALL_DIR) $(libdir)
{TAB}@-test -d $(libdir) && touch $@
{TAB}@sleep 1

.mkdir.__libdir__pkgconfig: .mkdir.__libdir_
{TAB}-$(INSTALL_DIR) $(libdir)/pkgconfig
{TAB}@-test -d $(libdir)/pkgconfig && touch $@
{TAB}@sleep 1

#
# The barney program.
#
barney_obj = barney/main.lo

bin/barney: $(barney_obj) .bin lib/libtest.la
{TAB}$(LIBTOOL) --mode=link --tag=CC $(CC) $(CPPFLAGS) $(CFLAGS) -o $@ \
{TAB}{TAB}$(barney_obj) lib/libtest.la $(LDFLAGS) $(LIBS)

$(bindir)/barney: .mkdir.__bindir_ bin/barney
{TAB}$(LIBTOOL) --mode=install $(INSTALL_PROGRAM) bin/barney $@

#
# The fred program.
#
fred_obj = fred/main.lo

bin/fred: $(fred_obj) .bin lib/libtest.la
{TAB}$(LIBTOOL) --mode=link --tag=CC $(CC) $(CPPFLAGS) $(CFLAGS) -o $@ \
{TAB}{TAB}$(fred_obj) lib/libtest.la $(LDFLAGS) $(LIBS)

$(bindir)/fred: .mkdir.__bindir_ bin/fred
{TAB}$(LIBTOOL) --mode=install $(INSTALL_PROGRAM) bin/fred $@

all: all-bin

all-bin: bin/barney bin/fred

.bin:
{TAB}-mkdir bin
{TAB}-chmod 0755 bin
{TAB}@-test -d bin && touch $@
{TAB}@sleep 1

check: sure

sure:
{TAB}@echo Passed All Tests

clean: clean-bin clean-misc clean-obj

clean-bin:
{TAB}rm -f bin/barney bin/fred

clean-misc:
{TAB}rm -f .bin .mkdir.__bindir_ .mkdir.__libdir_ .mkdir.__libdir__pkgconfig
{TAB}rm -f core

clean-obj:
{TAB}rm -f barney/main.lo barney/main.o fred/main.lo fred/main.o lib/bar.lo
{TAB}rm -f lib/bar.o lib/foo.lo lib/foo.o lib/libtest.a lib/libtest.la

distclean: clean distclean-directories distclean-files

distclean-files:
{TAB}rm -f Makefile config.cache config.log config.status lib/config.h

distclean-directories:
{TAB}rm -rf barney/.libs bin fred/.libs lib/.libs

install: install-bin install-libdir

install-bin: $(bindir)/barney $(bindir)/fred

#
# The install of the *.la file automatically causes "$(LIBTOOL) --mode=install"
# to install the *.a and *.so* files as well, which is why you don't see them
# explicitly mentioned here.
#
install-libdir: $(libdir)/libtest.la $(libdir)/pkgconfig/libtest.pc

uninstall:
{TAB}rm -f $(bindir)/barney $(bindir)/fred $(libdir)/libtest.a
{TAB}rm -f $(libdir)/libtest.la $(libdir)/libtest.so*
{TAB}rm -f $(libdir)/pkgconfig/libtest.pc

.PHONY: all all-bin check clean clean-bin distclean distclean-directories \
{TAB}{TAB}distclean-files install install-bin install-libdir sure \
{TAB}{TAB}the-default-target

# vim: set ts=8 sw=8 noet :
fubar
if test $? -ne 0 ; then no_result; fi

activity="aemakegen 359"
aemakegen -c 10 libdir/pkgconfig/libtest.pc > test.out
if test $? -ne 0 ; then fail; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
