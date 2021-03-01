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

TEST_SUBJECT="aemakegen rpm-spec"

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
        $work/libtest.C010/libtest/bar.c \
        $work/libtest.C010/libtest/foo.c \
        $work/libtest.C010/libtest/libtest.h \
        $work/libtest.C010/fred/main.c \
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
];
fubar
if test $? -ne 0 ; then no_result; fi

TAB=`awk 'BEGIN{printf("%c", 9)}' /dev/null`

sed "s|{TAB}|${TAB}|g" > test.ok << 'fubar'
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
Summary: libtest
Name: libtest
Version: C010
Release: 1
License: GPL
Group: Development/Tools
Source: homepage-attribute/%{name}-%{version}.tar.gz
URL: homepage-attribute/
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildPrereq: diffutils, libtool, sharutils

%description
This package contains the shared libraries for applications that libtest

%package bin
Summary: libtest programs

%description bin
libtest

%package libs0
Summary: libtest libraries
Group: Development/Tools

%description libs0
This package contains the shared libraries for applications that libtest

%package devel
Summary: libtest development files
Group: Development/Tools
Requires: libtest-libs%{?_isa} = %{?epoch:%{epoch}:}%{version}-%{release}

%description devel
This package contains static libraries and header files for applications that
libtest

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
%{_bindir}/barney
%{_bindir}/fred


%files libs
%defattr (-,root,root,-)
%{_libdir}/libtest.so.*


%files devel
%defattr (-,root,root,-)
%{_includedir}/libtest.h
%{_libdir}/libtest.a
%{_libdir}/libtest.so
fubar
if test $? -ne 0 ; then no_result; fi

activity="aemakegen 209"
aemakegen -c 10 --target=rpm-spec > test.out
if test $? -ne 0 ; then fail; fi

activity="check 213"
diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
