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

TEST_SUBJECT="aemakegen, vs pkg-config"

# load up standard prelude and test functions
. test_funcs

AEGIS_PROJECT=projname
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
        $work/${AEGIS_PROJECT}.C010/configure.ac \
        $work/${AEGIS_PROJECT}.C010/aegis/main.cc \
        $work/${AEGIS_PROJECT}.C010/libprojname/fred.cc \
        $work/${AEGIS_PROJECT}.C010/libprojname/libprojname.h \
        -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

cat > $work/${AEGIS_PROJECT}.C010/configure.ac << 'fubar'
AC_PROG_LIBTOOL
AC_CHECK_PROGS(AR, ar)
AC_PROG_RANLIB
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
    {
        name = "aemakegen:libtool";
        value = "true";
    },
];
fubar
if test $? -ne 0 ; then no_result; fi

TAB=`awk 'BEGIN{printf("%c", 9)}' /dev/null`

# ----------  makefile  ----------------------------------------------------

activity="aemakegen 118"
aemakegen -c 10 --target=makefile projname.pc > test.out
if test $? -ne 0 ; then fail; fi

activity="check makefile 122"
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
# 2. libprojname/config.h
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
# The name of the C++ compiler to use.
#
CXX = @CXX@

#
# The C++ compiler flags to use.
#
CXXFLAGS = @CXXFLAGS@

#
# The C++ preprocessor flags to use.
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

aegis/main.lo aegis/main.o: aegis/main.cc
{TAB}$(LIBTOOL) --mode=compile --tag=CXX $(CXX) $(CPPFLAGS) $(CXXFLAGS) -I. \
{TAB}{TAB}-c aegis/main.cc -o aegis/main.lo

libprojname/fred.lo libprojname/fred.o: libprojname/fred.cc
{TAB}$(LIBTOOL) --mode=compile --tag=CXX $(CXX) $(CPPFLAGS) $(CXXFLAGS) -I. \
{TAB}{TAB}-c libprojname/fred.cc -o libprojname/fred.lo

$(includedir)/libprojname.h: .mkdir.__includedir_ libprojname/libprojname.h
{TAB}$(INSTALL_DATA) libprojname/libprojname.h $@

$(libdir)/pkgconfig/projname.pc: .mkdir.__libdir__pkgconfig projname.pc
{TAB}$(INSTALL_DATA) projname.pc $@

#
# The libprojname/libprojname.la library.
#
libprojname_obj = libprojname/fred.lo

libprojname/libprojname.la: $(libprojname_obj)
{TAB}rm -f $@
{TAB}$(LIBTOOL) --mode=link --tag=CXX $(CXX) $(CPPFLAGS) $(CXXFLAGS) \
{TAB}{TAB}$(LDFLAGS) -o $@ $(libprojname_obj) $(LIBS) -rpath $(libdir) \
{TAB}{TAB}-version-info 0:0:0

#
# The install of the *.la file automatically causes "$(LIBTOOL) --mode=install"
# to install the *.a and *.so* files as well, which is why you don't see them
# explicitly mentioned here.
#
$(libdir)/libprojname.la: .mkdir.__libdir_ libprojname/libprojname.la
{TAB}$(LIBTOOL) --mode=install $(INSTALL_DATA) libprojname/libprojname.la $@

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

.mkdir.__libdir__pkgconfig: .mkdir.__libdir_
{TAB}-$(INSTALL_DIR) $(libdir)/pkgconfig
{TAB}@-test -d $(libdir)/pkgconfig && touch $@
{TAB}@sleep 1

#
# The aegis program.
#
aegis_obj = aegis/main.lo

bin/aegis: $(aegis_obj) .bin libprojname/libprojname.la
{TAB}$(LIBTOOL) --mode=link --tag=CXX $(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ \
{TAB}{TAB}$(aegis_obj) libprojname/libprojname.la $(LDFLAGS) $(LIBS)

$(bindir)/aegis: .mkdir.__bindir_ bin/aegis
{TAB}$(LIBTOOL) --mode=install $(INSTALL_PROGRAM) bin/aegis $@

all: all-bin

all-bin: bin/aegis

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
{TAB}rm -f bin/aegis

clean-misc:
{TAB}rm -f .bin .mkdir.__bindir_ .mkdir.__includedir_ .mkdir.__libdir_
{TAB}rm -f .mkdir.__libdir__pkgconfig core

clean-obj:
{TAB}rm -f aegis/main.lo aegis/main.o libprojname/fred.lo libprojname/fred.o
{TAB}rm -f libprojname/libprojname.a libprojname/libprojname.la

distclean: clean distclean-directories distclean-files

distclean-files:
{TAB}rm -f Makefile config.cache config.log config.status
{TAB}rm -f libprojname/config.h

distclean-directories:
{TAB}rm -rf aegis/.libs bin libprojname/.libs

install: install-bin install-include install-libdir

install-bin: $(bindir)/aegis

install-include: $(includedir)/libprojname.h

#
# The install of the *.la file automatically causes "$(LIBTOOL) --mode=install"
# to install the *.a and *.so* files as well, which is why you don't see them
# explicitly mentioned here.
#
install-libdir: $(libdir)/libprojname.la $(libdir)/pkgconfig/projname.pc

uninstall:
{TAB}rm -f $(bindir)/aegis $(includedir)/libprojname.h
{TAB}rm -f $(libdir)/libprojname.a $(libdir)/libprojname.la
{TAB}rm -f $(libdir)/libprojname.so* $(libdir)/pkgconfig/projname.pc

.PHONY: all all-bin check clean clean-bin distclean distclean-directories \
{TAB}{TAB}distclean-files install install-bin install-include \
{TAB}{TAB}install-libdir sure the-default-target

# vim: set ts=8 sw=8 noet :
fubar
if test $? -ne 0 ; then no_result; fi

diff -u ok test.out
if test $? -ne 0 ; then fail; fi

# ----------  automake  ----------------------------------------------------

activity="aemakegen 380"
aemakegen -c 10 --target=automake projname.pc > test.out
if test $? -ne 0 ; then fail; fi

activity="check automake 384"
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

# executables to be installed
bin_PROGRAMS = bin/aegis

pkgconfigdir = $(libdir)/pkgconfig

# Data files to be installed in $(pkgconfigdir)
pkgconfig_DATA = projname.pc

# The libprojname/libprojname.la library.
libprojname_libprojname_la_includes = libprojname/libprojname.h
libprojname_libprojname_la_SOURCES = libprojname/fred.cc
libprojname_libprojname_la_LDFLAGS = -version-info 0:0:0

# Shared libraries, to be installed.
lib_LTLIBRARIES = libprojname/libprojname.la

# header files to be installed
nobase_include_HEADERS = libprojname/libprojname.h

# Files to be removed by the "distclean" make target.
DISTCLEANFILES = libprojname/config.h

# The aegis program.
bin_aegis_SOURCES = aegis/main.cc
bin_aegis_LDADD = libprojname/libprojname.la

# Additional source files to be included in the tarball.
EXTRA_DIST = aegis.conf configure.ac

all-am: goose-libtool-writable-crap
goose-libtool-writable-crap: $(PROGRAMS)
{TAB}-bin/aegis -V

# vim: set ts=8 sw=8 noet :
fubar
if test $? -ne 0 ; then no_result; fi

diff -u ok test.out
if test $? -ne 0 ; then fail; fi

# ----------  rpm-spec  ----------------------------------------------------

activity="aemakegen 435"
aemakegen -c 10 --target=rpm-spec projname.pc > test.out
if test $? -ne 0 ; then fail; fi

activity="check rpm-spec 439"
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
Summary: projname
Name: projname
Version: C010
Release: 1
License: GPL
Group: Development/Tools
Source: http://projname.sourceforge.net/%{name}-%{version}.tar.gz
URL: http://projname.sourceforge.net/
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildPrereq: diffutils, libtool, sharutils

%description
This package contains the shared libraries for applications that projname

%package bin
Summary: projname programs

%description bin
projname

%package libs0
Summary: projname libraries
Group: Development/Tools

%description libs0
This package contains the shared libraries for applications that projname

%package devel
Summary: projname development files
Group: Development/Tools
Requires: projname-libs%{?_isa} = %{?epoch:%{epoch}:}%{version}-%{release}

%description devel
This package contains static libraries and header files for applications that
projname

%post
/sbin/ldconfig

%postun
/sbin/ldconfig


%prep
%setup -q


%build
%configure --sysconfdir=/etc --prefix=%{_prefix}
make


%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install
rm -f $RPM_BUILD_ROOT/usr/lib/*.la


%check
true


%clean
rm -rf $RPM_BUILD_ROOT


%files bin
%defattr (-,root,root,-)
%doc LICENSE BUILDING README
%{_bindir}/aegis


%files libs
%defattr (-,root,root,-)
%{_libdir}/libprojname.so.*


%files devel
%defattr (-,root,root,-)
%{_includedir}/libprojname.h
%{_libdir}/pkgconfig/projname.pc
%{_libdir}/libprojname.a
%{_libdir}/libprojname.so
fubar
if test $? -ne 0 ; then no_result; fi

diff -u ok test.out
if test $? -ne 0 ; then fail; fi

# ----------  debian  ------------------------------------------------------

activity="aemakegen 509"
aemakegen -c 10 --target=debian projname.pc
if test $? -ne 0 ; then fail; fi

activity="check debian/projname.install 513"
sed "s|{TAB}|${TAB}|g" > ok << 'fubar'
usr/bin/aegis
fubar
if test $? -ne 0 ; then no_result; fi

diff -u ok $work/${AEGIS_PROJECT}.C010/debian/projname.install
if test $? -ne 0 ; then fail; fi

activity="check debian/projname-dev.install 513"
sed "s|{TAB}|${TAB}|g" > ok << 'fubar'
usr/include/libprojname.h
usr/lib/libprojname.a
usr/lib/libprojname.so
usr/lib/pkgconfig/projname.pc
fubar
if test $? -ne 0 ; then no_result; fi

diff -u ok $work/${AEGIS_PROJECT}.C010/debian/libprojname-dev.install
if test $? -ne 0 ; then fail; fi

activity="check debian/libprojname0.install 513"
sed "s|{TAB}|${TAB}|g" > ok << 'fubar'
usr/lib/libprojname.so.*
fubar
if test $? -ne 0 ; then no_result; fi

diff -u ok $work/${AEGIS_PROJECT}.C010/debian/libprojname0.install
if test $? -ne 0 ; then fail; fi

# ----------  pkg-config  --------------------------------------------------

activity="aemakegen 509"
aemakegen -c 10 --target=pkg-config projname.pc \
        > $work/${AEGIS_PROJECT}.C010/projname.pc
if test $? -ne 0 ; then fail; fi

activity="check projname.pc 513"
sed "s|{TAB}|${TAB}|g" > ok << 'fubar'
prefix=@prefix@
exec_prefix=@exec_prefix@
bindir=@bindir@
libdir=@libdir@
datarootdir=@datarootdir@
includedir=@includedir@

Name: projname
Description: The "projname" program.
Version: 0~.D001~C010
Libs: -L${libdir} -lprojname
Libs.private: @LIBS@
Cflags: -I${includedir}
fubar
if test $? -ne 0 ; then no_result; fi

diff -u ok $work/${AEGIS_PROJECT}.C010/projname.pc
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
