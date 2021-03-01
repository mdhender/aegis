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

TEST_SUBJECT="aemakegen, locale vs manpage"

# load up standard prelude and test functions
. test_funcs

AEGIS_PROJECT=noinsttest
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
aegis -nf \
        $work/${AEGIS_PROJECT}.C010/aegis.conf \
        $work/${AEGIS_PROJECT}.C010/barney/main.c \
        $work/${AEGIS_PROJECT}.C010/noinst-two/main.c \
        $work/${AEGIS_PROJECT}.C010/script/test_prelude.sh \
        $work/${AEGIS_PROJECT}.C010/configure.ac \
        -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $work/${AEGIS_PROJECT}.C010/configure.ac << 'fubar'
AC_CHECK_PROGS(GROFF, groff roff)
fubar
if test $? -ne 0 ; then no_result; fi

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

project_specific =
[
    {
        name = "aemakegen:debian:maintainer";
        value = "maintainer@example.com";
    },
];
fubar
if test $? -ne 0 ; then no_result; fi

# ----------  makefile  ----------------------------------------------------

activity="aemakegen 112"
aemakegen -c 10 --target=makefile --flavour=aegis > test.out
if test $? -ne 0 ; then fail; fi

TAB=`awk 'BEGIN{printf("%c", 9)}' /dev/null`

activity="check makefile 118"
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

barney/main.o: barney/main.c
{TAB}$(CC) $(CPPFLAGS) $(CFLAGS) -I. -c barney/main.c
{TAB}mv main.o $@

noinst-two/main.o: noinst-two/main.c
{TAB}$(CC) $(CPPFLAGS) $(CFLAGS) -I. -c noinst-two/main.c
{TAB}mv main.o $@

bin/test_prelude: .bin script/test_prelude.sh
{TAB}cp script/test_prelude.sh $@
{TAB}chmod a+rx $@

.mkdir.__bindir_:
{TAB}-$(INSTALL_DIR) $(bindir)
{TAB}@-test -d $(bindir) && touch $@
{TAB}@sleep 1

#
# The barney program.
#
barney_obj = barney/main.o

bin/barney: $(barney_obj) .bin
{TAB}$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(barney_obj) $(LDFLAGS) $(LIBS)

$(bindir)/barney: .mkdir.__bindir_ bin/barney
{TAB}$(INSTALL_PROGRAM) bin/barney $@

#
# The noinst-two program.
#
noinst_two_obj = noinst-two/main.o

bin/noinst-two: $(noinst_two_obj) .bin
{TAB}$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(noinst_two_obj) $(LDFLAGS) $(LIBS)

all: all-bin

all-bin: bin/barney bin/noinst-two bin/test_prelude

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
{TAB}rm -f bin/barney bin/noinst-two bin/test_prelude

clean-misc:
{TAB}rm -f .bin .mkdir.__bindir_ core

clean-obj:
{TAB}rm -f barney/main.o noinst-two/main.o

distclean: clean distclean-directories distclean-files

distclean-files:
{TAB}rm -f Makefile config.cache config.log config.status

distclean-directories:
{TAB}rm -rf bin

install: install-bin

install-bin: $(bindir)/barney

uninstall:
{TAB}rm -f $(bindir)/barney

.PHONY: all all-bin check clean clean-bin distclean distclean-directories \
{TAB}{TAB}distclean-files install install-bin sure the-default-target

# vim: set ts=8 sw=8 noet :
fubar
if test $? -ne 0 ; then no_result; fi

diff -u ok test.out
if test $? -ne 0 ; then fail; fi

# ----------  automake  ----------------------------------------------------

activity="aemakegen 312"
aemakegen -c 10 --target=automake --flavour=aegis > test.out
if test $? -ne 0 ; then fail; fi

TAB=`awk 'BEGIN{printf("%c", 9)}' /dev/null`

activity="check automake 318"
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
# Tell automake to put the object file for foo/bar.cc in directory foo/
#
AUTOMAKE_OPTIONS = subdir-objects

bin/test_prelude: script/test_prelude.sh
{TAB}@mkdir -p bin
{TAB}cp script/test_prelude.sh $@
{TAB}chmod a+rx $@

# executables to be installed
bin_PROGRAMS = bin/barney

# executables not to be installed
noinst_PROGRAMS = bin/noinst-two

# scripts not to be installed
noinst_SCRIPTS = bin/test_prelude

# scripts needed to run tests
check_SCRIPTS = bin/test_prelude

# The barney program.
bin_barney_SOURCES = barney/main.c

# The noinst-two program.
bin_noinst_two_SOURCES = noinst-two/main.c

# Additional source files to be included in the tarball.
EXTRA_DIST = aegis.conf configure.ac script/test_prelude.sh

# vim: set ts=8 sw=8 noet :
fubar
if test $? -ne 0 ; then no_result; fi

diff -u ok test.out
if test $? -ne 0 ; then fail; fi

# ----------  rpm-spec  ----------------------------------------------------

activity="aemakegen 377"
aemakegen -c 10 --target=rpm-spec --flavour=aegis > test.out
if test $? -ne 0 ; then fail; fi

TAB=`awk 'BEGIN{printf("%c", 9)}' /dev/null`

activity="check rpm-spec 383"
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
Summary: noinsttest
Name: noinsttest
Version: C010
Release: 1
License: GPL
Group: Development/Tools
Source: http://noinsttest.sourceforge.net/%{name}-%{version}.tar.gz
URL: http://noinsttest.sourceforge.net/
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildPrereq: diffutils, groff, sharutils

%description
noinsttest


%prep
%setup -q


%build
%configure --sysconfdir=/etc --prefix=%{_prefix}
make


%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install


%check
true


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr (-,root,root,-)
%doc LICENSE BUILDING README
%{_bindir}/barney
fubar
if test $? -ne 0 ; then no_result; fi

diff -u ok test.out
if test $? -ne 0 ; then fail; fi

# ----------  debian  ------------------------------------------------------

activity="aemakegen 450"
aemakegen -c 10 --target=debian --flavour=aegis
if test $? -ne 0 ; then fail; fi

TAB=`awk 'BEGIN{printf("%c", 9)}' /dev/null`

activity="check debian/control 456"
sed "s|{TAB}|${TAB}|g" > ok << 'fubar'
Source: noinsttest
Section: utils
Priority: optional
Maintainer: maintainer@example.com
Build-Depends: debhelper (>= 5)
Standards-Version: 3.9.3

Package: noinsttest
Architecture: any
Description: The "noinsttest" program.
Depends: ${misc:Depends}, ${shlibs:Depends}
fubar
if test $? -ne 0 ; then no_result; fi

diff -u ok $work/${AEGIS_PROJECT}.C010/debian/control
if test $? -ne 0 ; then fail; fi


activity="check debian/rules 476"
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

# Build the binary package files here.
binary binary-arch binary-indep: install
{TAB}dh_testdir
{TAB}dh_testroot
{TAB}dh_installchangelogs
{TAB}dh_installdocs -A
{TAB}dh_install --fail-missing --sourcedir=debian/tmp
{TAB}dh_strip
{TAB}dh_compress -A
{TAB}dh_fixperms
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

diff -u ok $work/${AEGIS_PROJECT}.C010/debian/rules
if test $? -ne 0 ; then fail; fi

activity="check debian/noinsttest.install 547"
sed "s|{TAB}|${TAB}|g" > ok << 'fubar'
usr/bin/barney
fubar
if test $? -ne 0 ; then no_result; fi

diff -u ok $work/${AEGIS_PROJECT}.C010/debian/noinsttest.install
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
