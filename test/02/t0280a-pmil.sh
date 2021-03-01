#!/bin/sh
#
# aegis - project change supervisor.
# Copyright (C) 2012 Peter Miller
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

TEST_SUBJECT="aemakegen, documents need ghostscript"

# load up standard prelude and test functions
. test_funcs

AEGIS_PROJECT=test-gs-proj
export AEGIS_PROJECT

activity="new project 28"
aegis -npr ${AEGIS_PROJECT} -version - -v -dir $work/proj.dir \
    -lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

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
aegis -nf $work/${AEGIS_PROJECT}.C010/aegis.conf \
    $work/${AEGIS_PROJECT}.C010/etc/example.man \
    $work/${AEGIS_PROJECT}.C010/configure.ac \
    > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $work/${AEGIS_PROJECT}.C010/aegis.conf << 'fubar'
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
fubar
if test $? -ne 0 ; then no_result; fi

TAB=`awk 'BEGIN{printf("%c", 9)}' /dev/null`

sed "s|{TAB}|$TAB|g" > ok << 'fubar'
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
# prefix for installation path
#
prefix = @prefix@
exec_prefix = @exec_prefix@

# define this to silence ./configure warning
datarootdir = $(DESTDIR)@datarootdir@

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

etc/example.pdf: etc/example.man
{TAB}$(GROFF) -Tps -s -I. -t -man etc/example.man > etc/example.ps
{TAB}ps2pdf etc/example.ps $@
{TAB}rm etc/example.ps

$(datarootdir)/doc/test-gs-proj/example.pdf: \
{TAB}{TAB}.mkdir.__datarootdir__doc_test_gs_proj etc/example.pdf
{TAB}$(INSTALL_DATA) etc/example.pdf $@

.mkdir.__datarootdir_:
{TAB}-$(INSTALL_DIR) $(datarootdir)
{TAB}@-test -d $(datarootdir) && touch $@
{TAB}@sleep 1

.mkdir.__datarootdir__doc: .mkdir.__datarootdir_
{TAB}-$(INSTALL_DIR) $(datarootdir)/doc
{TAB}@-test -d $(datarootdir)/doc && touch $@
{TAB}@sleep 1

.mkdir.__datarootdir__doc_test_gs_proj: .mkdir.__datarootdir__doc
{TAB}-$(INSTALL_DIR) $(datarootdir)/doc/test-gs-proj
{TAB}@-test -d $(datarootdir)/doc/test-gs-proj && touch $@
{TAB}@sleep 1

all: all-doc

all-doc: etc/example.pdf

check: sure

sure:
{TAB}@echo Passed All Tests

clean: clean-doc clean-misc

clean-doc:
{TAB}rm -f etc/example.pdf

clean-misc:
{TAB}rm -f .mkdir.__datarootdir_ .mkdir.__datarootdir__doc
{TAB}rm -f .mkdir.__datarootdir__doc_test_gs_proj etc/example.pdf

distclean: clean distclean-directories distclean-files

distclean-files:
{TAB}rm -f Makefile config.cache config.log config.status

distclean-directories:
{TAB}rm -rf bin

install: install-doc

install-doc: $(datarootdir)/doc/test-gs-proj/example.pdf

uninstall:
{TAB}rm -f $(datarootdir)/doc/test-gs-proj/example.pdf

.PHONY: all all-doc check clean clean-doc distclean distclean-directories \
{TAB}{TAB}distclean-files install sure the-default-target

# vim: set ts=8 sw=8 noet :
fubar
if test $? -ne 0 ; then no_result; fi

cat > $work/${AEGIS_PROJECT}.C010/configure.ac << 'fubar'
AC_CHECK_PROGS(GROFF, groff roff)
fubar
if test $? -ne 0 ; then no_result; fi

activity="aemakegen target=makefile 241"
aemakegen -c 10 --flavour=aegis --target=makefile > test.out
if test $? -ne 0 ; then fail; fi

activity="check rpm-spec 245"
diff -u ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
