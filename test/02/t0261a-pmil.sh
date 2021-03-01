#!/bin/sh
#
# aegis - project change supervisor.
# Copyright (C) 2011, 2012 Peter Miller
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

TEST_SUBJECT="aemakegen vs soelim"

# load up standard prelude and test functions
. test_funcs

activity="new project 25"
aegis -npr libtest -version - -v -dir $work/proj.dir \
    -lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=libtest
export AEGIS_PROJECT

activity="project attributes 33"
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

activity="staff 46"
aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 54"
cat > caf << 'fubar'
brief_description = "one";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 64"
aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 68"
aegis -nf $work/libtest.C010/aegis.conf \
        $work/libtest.C010/fred/main.c \
        $work/libtest.C010/en/man1/fred.1 \
        $work/libtest.C010/configure.ac \
        > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/libtest.C010/aegis.conf << 'fubar'
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
    { name = "aemakegen:debian:homepage"; value = "homepage-attribute"; },
    { name = "aemakegen:debian:maintainer"; value = "maintainer-attribute"; },
    { name = "aemakegen:debian:priority"; value = "important"; },
    { name = "aemakegen:debian:section"; value = "admin"; },
];
fubar
if test $? -ne 0 ; then no_result; fi

cat > $work/libtest.C010/configure.ac << 'fubar'
AC_CHECK_PROGS(GROFF, groff roff)
fubar
if test $? -ne 0 ; then no_result; fi

cat > $work/libtest.C010/en/man1/fred.1 << 'fubar'
.XX aaa bbb
fubar
if test $? -ne 0 ; then no_result; fi

TAB=`awk 'BEGIN{printf("%c", 9)}' /dev/null`

sed "s|{TAB}|$TAB|g" > makefile.ok << 'fubar'
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
# 2. /config.h
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

#
# Where to put the manuals.
#
# On a network, this would be shared between all machines on the network. It can
# be read-only.
#
# The $(DESTDIR) is for packaging.
#
mandir = $(DESTDIR)@mandir@

# define this to silence ./configure warning
datarootdir = $(DESTDIR)@datarootdir@

#
# extra libraries required for your system
#
LIBS = @LIBS@

#
# shell to use to run tests and commands
#
SH = @SH@

#
# Set GROFF to the name of the roff command on your system, usually "groff" or
# "troff" or "nroff".
#
GROFF = @GROFF@

# ---------------------------------------------------------
# You should not need to change anything below this line.

#
# The default target
#
the-default-target: all

en/cat1/fred.1: en/man1/fred.1
{TAB}@mkdir -p en/cat1
{TAB}GROFF_NO_SGR=-c $(GROFF) -I. -Tascii -t -man en/man1/fred.1 > $@

$(mandir)/man1/fred.1: .mkdir.__mandir__man1 en/man1/fred.1
{TAB}sed '/^[.]XX ./d' < en/man1/fred.1 > en/man1/fred.1.tmp
{TAB}$(INSTALL_DATA) en/man1/fred.1.tmp $@
{TAB}@rm -f en/man1/fred.1.tmp

fred/main.o: fred/main.c
{TAB}$(CC) $(CPPFLAGS) $(CFLAGS) -I. -c fred/main.c
{TAB}mv main.o $@

.mkdir.__bindir_:
{TAB}-$(INSTALL_DIR) $(bindir)
{TAB}@-test -d $(bindir) && touch $@
{TAB}@sleep 1

.mkdir.__mandir_:
{TAB}-$(INSTALL_DIR) $(mandir)
{TAB}@-test -d $(mandir) && touch $@
{TAB}@sleep 1

.mkdir.__mandir__man1: .mkdir.__mandir_
{TAB}-$(INSTALL_DIR) $(mandir)/man1
{TAB}@-test -d $(mandir)/man1 && touch $@
{TAB}@sleep 1

#
# The fred program.
#
fred_obj = fred/main.o

bin/fred: $(fred_obj) .bin
{TAB}$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(fred_obj) $(LDFLAGS) $(LIBS)

$(bindir)/fred: .mkdir.__bindir_ bin/fred
{TAB}$(INSTALL_PROGRAM) bin/fred $@

all: all-bin all-doc

all-bin: bin/fred

all-doc: en/cat1/fred.1

.bin:
{TAB}-mkdir bin
{TAB}-chmod 0755 bin
{TAB}@-test -d bin && touch $@
{TAB}@sleep 1

check: sure

sure:
{TAB}@echo Passed All Tests

clean: clean-bin clean-doc clean-misc clean-obj

clean-bin:
{TAB}rm -f bin/fred

clean-doc:
{TAB}rm -f en/cat1/fred.1

clean-misc:
{TAB}rm -f .bin .mkdir.__bindir_ .mkdir.__mandir_ .mkdir.__mandir__man1 core

clean-obj:
{TAB}rm -f fred/main.o

distclean: clean distclean-directories distclean-files

distclean-files:
{TAB}rm -f Makefile config.cache config.log config.status

distclean-directories:
{TAB}rm -rf bin

install: install-bin install-man

install-bin: $(bindir)/fred

install-man: $(mandir)/man1/fred.1

uninstall:
{TAB}rm -f $(bindir)/fred $(mandir)/man1/fred.1

.PHONY: all all-bin all-doc check clean clean-bin clean-doc distclean \
{TAB}{TAB}distclean-directories distclean-files install install-bin \
{TAB}{TAB}install-mandir sure the-default-target

# vim: set ts=8 sw=8 noet :
fubar
if test $? -ne 0 ; then no_result; fi

activity="aemakegen 325"
aemakegen -c 10 --target=makefile > libtest.C010/Makefile.in
if test $? -ne 0 ; then fail; fi

activity="check control 329"
diff makefile.ok libtest.C010/Makefile.in
if test $? -ne 0 ; then fail; fi

cat > test.install.ok << 'fubar'
usr/bin/fred
usr/share/man/man1/fred.1
fubar
if test $? -ne 0 ; then no_result; fi

activity="aemakegen 339"
aemakegen -c 10 --target=debian
if test $? -ne 0 ; then fail; fi

activity="check test.install 343"
diff test.install.ok libtest.C010/debian/test.install
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
