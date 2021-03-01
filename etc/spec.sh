#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 1997, 1999-2008, 2012 Peter Miller
#       Copyright (C) 2006-2008 Walter Franzini
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
version=${version-0.0.0}
# Name and date will refer to integration, which may be appropriate.
date_str=`date +"%a %b %d %Y"`
name_str=`aesub '${User email}'`
cat <<fubar
#
# This file is GENERATED.  Please DO NOT send the maintainer
# patches to this file.  If there is a problemn with this file,
# fix etc/spec.sh, and set the etc/spec.sh patch to the maintainer.
#
# Users of this spec file are strongly advised to change the Release
# field to something meaningful in their context.
#
Summary: project change supervisor
Name: aegis
Version: ${version}
Release: 1
License: GPL
Group: Development/Version Control
Source: http://aegis.sourceforge.net/aegis-${version}.tar.gz
URL: http://aegis.sourceforge.net
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

#
# Build-time prerequisites, things you have to have installed so that
# this package will build successfully, correctly and completely.
#
# Note: on RedHat-like systems, libmagic stuff is in the "file" package.
# e2fsprogs is used for UUID.
#
BuildPrereq: bison, curl-devel, diffutils, gawk, gettext >= 0.11.4
BuildPrereq: groff >= 1.15, perl, libxml2-devel >= 1.8.17, tk
BuildPrereq: zlib-devel, bzip2-devel, e2fsprogs-devel, file >= 4

%description
Aegis is a transaction-based software configuration management system.
It provides a framework within which a team of developers may work
on many changes to a program independently, and Aegis coordinates
integrating these changes back into the master source of the program,
with as little disruption as possible.

%package txtdocs
Summary: Aegis documentation, dumb ascii text
Group: Development/Version Control

%description txtdocs
Aegis documentation in dumb ascii text format.

%package psdocs
Summary: Aegis documentation, PostScript format
Group: Development/Version Control

%description psdocs
Aegis documentation in PostScript format.

%prep

%setup

%build
%configure --sysconfdir=/etc --prefix=%{_prefix} --mandir=%{_mandir} \
  --with-nlsdir=%{_datadir}/locale
make

%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} install
# If rpmbuild does not strip your binaries, consider adding
#   INSTALL_PROGRAM='/usr/bin/install -s'
# to the above arguments to "make".

#
# See the comment at the top of this file.  If you don't like
# the file attributes, or there is a file missing, DO NOT send
# the maintainer a patch to this file.  This file is GENERATED.
# If you want different attributes, fix the etc/spec.sh file,
# and send THAT patch to the maintainer.
#

%files
%defattr (-,root,root)
%_prefix/bin/*
fubar
for FILE in $suidbins
do
echo "%attr(4755,root,bin) %_prefix/bin/$FILE"
done
cat <<fubar
%_prefix/share/locale/*/LC_MESSAGES/*
%_prefix/share/aegis/*
%attr(0644,root,root) %_prefix/share/aegis/icon/*
%attr(0755,root,bin) /etc/profile.d/aegis.*
%attr(0755,root,bin) %dir %_prefix/com/aegis
%_mandir/man1/*
%_mandir/man5/*

%files txtdocs
%_datadir/aegis/*/*.txt

%files psdocs
%_datadir/aegis/*/*.ps


#
# This next bit is done because when using Aegis with NFS, these
# files must have EXACTLY the same uid and gid on all systems.
# Unfortunately, RPM won't let you give exact numeric uids and gids,
# and the names for low-numbered uids and gids are essentially
# random across the various Unix implementations.  Sigh.
#
%post
chown -R 3 %_prefix/com/aegis && chgrp -R 3 %_prefix/com/aegis

%clean
rm -rf %{buildroot}

%changelog
* ${date_str} ${name_str} ${version}-1
- Update to ${version}

* Thu Apr 03 2008 Ralph A. Smith <smithra@users.sourceforge.net> 4.24-0
- Rationalized spec file to emulate RedHat practices.
- See the Aegis project website for prior history.
fubar

# vim: set ts=8 sw=4 et :
