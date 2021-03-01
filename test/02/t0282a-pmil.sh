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

TEST_SUBJECT="aemakegen, shared library major version number"

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
        $work/libtest.C010/libtest/foo.c \
        $work/libtest.C010/libtest/libtest.h \
        $work/libtest.C010/barney/main.c -v > log 2>&1
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
    { name = "aemakegen:libtool"; value = "true"; },
    { name = "aemakegen:version-info"; value = "52:0:0"; },
];
fubar
if test $? -ne 0 ; then no_result; fi

activity="aemakegen 103"
aemakegen -c 10 --target=debian
if test $? -ne 0 ; then fail; fi

TAB=`awk 'BEGIN{printf("%c", 9)}' /dev/null`

# ----------  debian/control  ----------------------------------------------

activity="check debian/control 111"
sed "s|{TAB}|${TAB}|g" > control.ok << 'fubar'
Source: libtest
Section: admin
Priority: important
Maintainer: maintainer-attribute
Homepage: homepage-attribute
Build-Depends: debhelper (>= 5), libtool
Standards-Version: 3.9.3

Package: test
Architecture: any
Description: The "libtest" program. - programs
Depends: ${misc:Depends}, ${shlibs:Depends}, libtest52

Package: libtest52
Section: libs
Architecture: any
Description: The "libtest" program.
Depends: ${misc:Depends}, ${shlibs:Depends}

Package: libtest52-dbg
Section: debug
Priority: extra
Architecture: any
Description: The "libtest" program. - debugging symbols
Depends: ${misc:Depends}, ${shlibs:Depends}, libtest52 (= ${binary:Version})

Package: libtest-dev
Section: libdevel
Architecture: any
Description: The "libtest" program. - development files
Depends: ${misc:Depends}, ${shlibs:Depends}, libtest52 (= ${binary:Version})
fubar
if test $? -ne 0 ; then no_result; fi

diff -u control.ok $work/libtest.C010/debian/control
if test $? -ne 0 ; then fail; fi

# ----------  debian/control  ----------------------------------------------

activity="check debian/rules 152"
sed "s|{TAB}|${TAB}|g" > ok << 'fubar'
#!/usr/bin/make -f

# Uncomment this to turn on verbose mode.
#export DH_VERBOSE=1

CFLAGS = -Wall -g
ifneq (,$(findstring noopt,$(DEB_BUILD_OPTIONS)))
CFLAGS += -O0
else
CFLAGS += -O2
endif

config.status: configure
{TAB}dh_testdir
{TAB}sh configure --prefix=/usr --localstatedir=/var --sysconfdir=/etc \
{TAB}{TAB}CFLAGS="$(CFLAGS)" LDFLAGS="-Wl,-z,defs"

build: build-arch build-indep

build-arch: build-stamp

build-indep: build-stamp

# Build and test the tarball.
build-stamp: config.status
{TAB}dh_testdir
{TAB}$(MAKE)
{TAB}touch $@

# dpkg-buildpackage (step 3) invokes 'fakeroot debian/rules clean', and after
# that (step 5) does the actual build.
clean:
{TAB}dh_testdir
{TAB}dh_testroot
{TAB}rm -f build-stamp
{TAB}test ! -f Makefile || $(MAKE) distclean
{TAB}dh_clean

# Install the built tarball into the temporary install tree. It depends on the
# 'build' target, so the tarball is also built by this rule.
install: build
{TAB}dh_testdir
{TAB}dh_testroot
{TAB}dh_clean -k
{TAB}dh_installdirs -A --list-missing
{TAB}mkdir -p $(CURDIR)/debian/tmp/usr/lib
{TAB}mkdir -p $(CURDIR)/debian/tmp/usr/share
{TAB}$(MAKE) DESTDIR=$(CURDIR)/debian/tmp install
{TAB}-rm $(CURDIR)/debian/tmp/usr/lib/*.la

# Build the binary package files here.
binary binary-arch binary-indep: install
{TAB}dh_testdir
{TAB}dh_testroot
{TAB}dh_installchangelogs
{TAB}dh_installdocs -A
{TAB}dh_install --fail-missing --sourcedir=debian/tmp
{TAB}dh_strip --dbg-package=libtest52-dbg
{TAB}dh_compress -A
{TAB}dh_fixperms
{TAB}dh_makeshlibs
{TAB}dh_shlibdeps
{TAB}dh_installdeb
{TAB}dh_gencontrol
{TAB}dh_md5sums
{TAB}dh_builddeb

.PHONY: binary binary-arch binary-indep build build-arch build-indep clean \
{TAB}{TAB}install

# vim: set ts=8 sw=8 noet :
fubar
if test $? -ne 0 ; then no_result; fi

diff -u ok $work/libtest.C010/debian/rules
if test $? -ne 0 ; then fail; fi

# ----------  debian/libtest52.install  ------------------------------------

activity="check debian/libtest52.install 227"
sed "s|{TAB}|${TAB}|g" > ok << 'fubar'
usr/lib/libtest.so.*
fubar
if test $? -ne 0 ; then no_result; fi

diff -u ok $work/libtest.C010/debian/libtest52.install
if test $? -ne 0 ; then fail; fi

# ----------  debian/libtest-dev.install  ----------------------------------

activity="check debian/libtest-dev.install 238"
sed "s|{TAB}|${TAB}|g" > ok << 'fubar'
usr/include/libtest.h
usr/lib/libtest.a
usr/lib/libtest.so
fubar
if test $? -ne 0 ; then no_result; fi

diff -u ok $work/libtest.C010/debian/libtest-dev.install
if test $? -ne 0 ; then fail; fi

# ----------  Makefile.in  -------------------------------------------------
#
# because it has to match debian/control and debian/rules

activity="check makefile 253"
sed "s|{TAB}|${TAB}|g" > ok << 'fubar'
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
# 2. libtest/config.h
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
# Where to put the include files for the library.
#
# On a network, this would be shared between all machines of identical
# architecture. It can be read-only.
#
# The $(DESTDIR) is for packaging.
#
includedir = $(DESTDIR)@includedir@

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

libtest/foo.lo libtest/foo.o: libtest/foo.c
{TAB}$(LIBTOOL) --mode=compile --tag=CC $(CC) $(CPPFLAGS) $(CFLAGS) -I. -c \
{TAB}{TAB}libtest/foo.c -o libtest/foo.lo

$(includedir)/libtest.h: .mkdir.__includedir_ libtest/libtest.h
{TAB}$(INSTALL_DATA) libtest/libtest.h $@

#
# The libtest/libtest.la library.
#
libtest_obj = libtest/foo.lo

libtest/libtest.la: $(libtest_obj)
{TAB}rm -f $@
{TAB}$(LIBTOOL) --mode=link --tag=CC $(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) \
{TAB}{TAB}-o $@ $(libtest_obj) $(LIBS) -rpath $(libdir) -version-info \
{TAB}{TAB}52:0:0

#
# The install of the *.la file automatically causes "$(LIBTOOL) --mode=install"
# to install the *.a and *.so* files as well, which is why you don't see them
# explicitly mentioned here.
#
$(libdir)/libtest.la: .mkdir.__libdir_ libtest/libtest.la
{TAB}$(LIBTOOL) --mode=install $(INSTALL_DATA) libtest/libtest.la $@

.mkdir.__bindir_:
{TAB}-$(INSTALL_DIR) $(bindir)
{TAB}@-test -d $(bindir) && touch $@
{TAB}@sleep 1

.mkdir.__includedir_:
{TAB}-$(INSTALL_DIR) $(includedir)
{TAB}@-test -d $(includedir) && touch $@
{TAB}@sleep 1

.mkdir.__libdir_:
{TAB}-$(INSTALL_DIR) $(libdir)
{TAB}@-test -d $(libdir) && touch $@
{TAB}@sleep 1

#
# The barney program.
#
barney_obj = barney/main.lo

bin/barney: $(barney_obj) .bin libtest/libtest.la
{TAB}$(LIBTOOL) --mode=link --tag=CC $(CC) $(CPPFLAGS) $(CFLAGS) -o $@ \
{TAB}{TAB}$(barney_obj) libtest/libtest.la $(LDFLAGS) $(LIBS)

$(bindir)/barney: .mkdir.__bindir_ bin/barney
{TAB}$(LIBTOOL) --mode=install $(INSTALL_PROGRAM) bin/barney $@

all: all-bin

all-bin: bin/barney

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
{TAB}rm -f bin/barney

clean-misc:
{TAB}rm -f .bin .mkdir.__bindir_ .mkdir.__includedir_ .mkdir.__libdir_ core

clean-obj:
{TAB}rm -f barney/main.lo barney/main.o libtest/foo.lo libtest/foo.o
{TAB}rm -f libtest/libtest.a libtest/libtest.la

distclean: clean distclean-directories distclean-files

distclean-files:
{TAB}rm -f Makefile config.cache config.log config.status libtest/config.h

distclean-directories:
{TAB}rm -rf barney/.libs bin libtest/.libs

install: install-bin install-include install-libdir

install-bin: $(bindir)/barney

install-include: $(includedir)/libtest.h

#
# The install of the *.la file automatically causes "$(LIBTOOL) --mode=install"
# to install the *.a and *.so* files as well, which is why you don't see them
# explicitly mentioned here.
#
install-libdir: $(libdir)/libtest.la

uninstall:
{TAB}rm -f $(bindir)/barney $(includedir)/libtest.h $(libdir)/libtest.a
{TAB}rm -f $(libdir)/libtest.la $(libdir)/libtest.so*

.PHONY: all all-bin check clean clean-bin distclean distclean-directories \
{TAB}{TAB}distclean-files install install-bin install-include \
{TAB}{TAB}install-libdir sure the-default-target

# vim: set ts=8 sw=8 noet :
fubar
if test $? -ne 0 ; then no_result; fi

activity="aemakegen 491"
aemakegen -c 10 > $work/libtest.C010/Makefile.in
if test $? -ne 0 ; then fail; fi

diff -u ok $work/libtest.C010/Makefile.in
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
