#
# This file is GENERATED.  Please DO NOT send the maintainer
# patches to this file.  If there is a problemn with this file,
# fix etc/spec.sh, and set the etc/spec.sh patch to the maintainer.
#
Summary: project change supervisor
Name: aegis
Version: 4.17
Release: 1
Copyright: GPL
Group: Development/Version Control
Source: http://www.canb.auug.org.au/~millerp/aegis-4.17.tar.gz
URL: http://www.canb.auug.org.au/~millerp/aegis.html
BuildRoot: /tmp/aegis-build-root
Icon: aegis.xpm
BuildPrereq: bison, curl-devel, diffutils, gawk, gettext >= 0.11.4
BuildPrereq: groff >= 1.15, perl, tk, zlib-devel

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

%package dvidocs
Summary: aegis documentation, DVI format
Group: Development/Version Control

%description dvidocs
Aegis documentation in DVI format.

%prep
%setup

%build
./configure --prefix=/usr
make

%install
make RPM_BUILD_ROOT=$RPM_BUILD_ROOT install

#
# See the comment at the top of this file.  If you don't like
# the file attributes, or there is a file missing, DO NOT send
# the maintainer a patch to this file.  This file is GENERATED.
# If you want different attributes, fix the etc/spec.sh file,
# and send THAT patch to the maintainer.
#
%files
%attr(0755,root,bin) %dir /usr/com/aegis
%attr(0755,root,bin) %dir /usr/share/aegis
%attr(0755,root,bin) %dir /usr/share/aegis/config.example
%attr(0755,root,bin) %dir /usr/lib/aegis
%attr(0755,root,bin) %dir /usr/lib/aegis/de
%attr(0755,root,bin) %dir /usr/lib/aegis/de/LC_MESSAGES
%attr(0755,root,bin) %dir /usr/share/aegis/de
%attr(0755,root,bin) %dir /usr/share/aegis/de/LC_MESSAGES
%attr(0755,root,bin) %dir /usr/lib/aegis/en
%attr(0755,root,bin) %dir /usr/lib/aegis/en/LC_MESSAGES
%attr(0755,root,bin) %dir /usr/share/aegis/en
%attr(0755,root,bin) %dir /usr/share/aegis/en/html
%attr(0755,root,bin) %dir /usr/share/aegis/en/man1
%attr(0755,root,bin) %dir /usr/share/aegis/en/man5
%attr(0755,root,bin) %dir /usr/share/aegis/en/notes
%attr(0755,root,bin) %dir /usr/lib/aegis/es
%attr(0755,root,bin) %dir /usr/lib/aegis/es/LC_MESSAGES
%attr(0755,root,bin) %dir /usr/lib/aegis/fr
%attr(0755,root,bin) %dir /usr/lib/aegis/fr/LC_MESSAGES
%attr(0755,root,bin) %dir /usr/share/aegis/fr
%attr(0755,root,bin) %dir /usr/share/aegis/fr/LC_MESSAGES
%attr(0755,root,bin) %dir /usr/share/aegis/man1
%attr(0755,root,bin) %dir /usr/share/aegis/man1/en
%attr(0755,root,bin) %dir /usr/lib/aegis/nl
%attr(0755,root,bin) %dir /usr/lib/aegis/nl/LC_MESSAGES
%attr(0755,root,bin) %dir /usr/share/aegis/remind
%attr(0755,root,bin) %dir /usr/share/aegis/report
%attr(0755,root,bin) %dir /usr/lib/aegis/ro
%attr(0755,root,bin) %dir /usr/lib/aegis/ro/LC_MESSAGES
%attr(0755,root,bin) %dir /usr/lib/aegis/ru
%attr(0755,root,bin) %dir /usr/lib/aegis/ru/LC_MESSAGES
%attr(0755,root,bin) %dir /usr/share/aegis/wish
%attr(0755,root,bin) /usr/bin/aeannotate
%attr(0755,root,bin) /usr/bin/aecomplete
%attr(0755,root,bin) /usr/bin/aecvsserver
%attr(0755,root,bin) /usr/bin/aedist
%attr(0755,root,bin) /usr/bin/aefind
%attr(0755,root,bin) /usr/bin/aeget
%attr(4755,root,bin) /usr/bin/aegis
%attr(4755,root,bin) /usr/bin/aeimport
%attr(0755,root,bin) /usr/bin/aels
%attr(0755,root,bin) /usr/bin/aemeasure
%attr(0755,root,bin) /usr/bin/aepatch
%attr(0755,root,bin) /usr/bin/aerect
%attr(0755,root,bin) /usr/bin/aereport
%attr(0755,root,bin) /usr/bin/aesub
%attr(0755,root,bin) /usr/bin/aetar
%attr(0755,root,bin) /usr/bin/aexml
%attr(0755,root,bin) /usr/share/aegis/cshrc
%attr(0755,root,bin) /usr/share/aegis/db_forced.sh
%attr(0755,root,bin) /usr/share/aegis/de.inews.sh
%attr(0755,root,bin) /usr/share/aegis/de.sh
%attr(0755,root,bin) /usr/share/aegis/deu.inews.sh
%attr(0755,root,bin) /usr/share/aegis/deu.sh
%attr(0755,root,bin) /usr/share/aegis/if.inews.sh
%attr(0755,root,bin) /usr/share/aegis/if.sh
%attr(0755,root,bin) /usr/share/aegis/ip.inews.sh
%attr(0755,root,bin) /usr/share/aegis/ip.sh
%attr(0755,root,bin) /usr/share/aegis/profile
%attr(0755,root,bin) /usr/share/aegis/remind/awt_dvlp.sh
%attr(0755,root,bin) /usr/share/aegis/remind/awt_intgrtn.sh
%attr(0755,root,bin) /usr/share/aegis/remind/bng_dvlpd.sh
%attr(0755,root,bin) /usr/share/aegis/remind/bng_rvwd.sh
%attr(0755,root,bin) /usr/share/aegis/rf.inews.sh
%attr(0755,root,bin) /usr/share/aegis/rf.sh
%attr(0755,root,bin) /usr/share/aegis/rp.inews.sh
%attr(0755,root,bin) /usr/share/aegis/rp.sh
%attr(0755,root,bin) /usr/share/aegis/rpu.inews.sh
%attr(0755,root,bin) /usr/share/aegis/rpu.sh
%attr(0755,root,bin) /usr/bin/ae-cvs-ci
%attr(0755,root,bin) /usr/bin/ae-sccs-put
%attr(0755,root,bin) /usr/bin/ae_diff2htm
%attr(0755,root,bin) /usr/bin/aebuffy
%attr(0755,root,bin) /usr/bin/aecomp
%attr(0755,root,bin) /usr/bin/aeedit
%attr(0755,root,bin) /usr/bin/aegis.cgi.i
%attr(0755,root,bin) /usr/bin/aegis.cgi
%attr(0755,root,bin) /usr/bin/aeintegratq
%attr(0755,root,bin) /usr/bin/tkaeca
%attr(0755,root,bin) /usr/bin/tkaegis
%attr(0755,root,bin) /usr/bin/tkaenc
%attr(0755,root,bin) /usr/bin/tkaepa
%attr(0755,root,bin) /usr/bin/tkaer
%attr(0755,root,bin) /usr/bin/xaegis
%attr(0644,root,bin) /usr/share/aegis/aedefault.css
%attr(0644,root,bin) /usr/share/aegis/aegis.icon
%attr(0644,root,bin) /usr/share/aegis/aegis.mask
%attr(0644,root,bin) /usr/share/aegis/aegis.pgm
%attr(0644,root,bin) /usr/share/aegis/config.example/architecture
%attr(0644,root,bin) /usr/share/aegis/config.example/cake
%attr(0644,root,bin) /usr/share/aegis/config.example/cook
%attr(0644,root,bin) /usr/share/aegis/config.example/diff
%attr(0644,root,bin) /usr/share/aegis/config.example/diff3
%attr(0644,root,bin) /usr/share/aegis/config.example/fhist
%attr(0644,root,bin) /usr/share/aegis/config.example/make
%attr(0644,root,bin) /usr/share/aegis/config.example/rcs
%attr(0644,root,bin) /usr/share/aegis/config.example/sccs
%attr(0644,root,bin) /usr/lib/aegis/de/LC_MESSAGES/aegis.mo
%attr(0644,root,bin) /usr/share/aegis/de/LC_MESSAGES/glossary
%attr(0644,root,bin) /usr/lib/aegis/en/LC_MESSAGES/aegis.mo
%attr(0644,root,bin) /usr/share/aegis/en/html/chan_hstry.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/chan_menu.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/changes.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/cp_command.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/file_activ.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/file_cflct.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/file_densi.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/file_diff.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/file_hstry.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/file_list.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/file_menu.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_cch1.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_cch2.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_cch3.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_filme.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_hstg2.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_hstg3.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_hstgm.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_hstry.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_list.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_menu.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_prgr1.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_prgr2.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_prgr3.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_staff.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_stats.rpt
%attr(0644,root,bin) /usr/share/aegis/en/man1/ae-cvs-ci.1*
%attr(0644,root,bin) /usr/man/man1/ae-cvs-ci.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/ae-sccs-put.1*
%attr(0644,root,bin) /usr/man/man1/ae-sccs-put.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/ae_c.1*
%attr(0644,root,bin) /usr/man/man1/ae_c.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/ae_p.1*
%attr(0644,root,bin) /usr/man/man1/ae_p.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeannotate.1*
%attr(0644,root,bin) /usr/man/man1/aeannotate.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeb.1*
%attr(0644,root,bin) /usr/man/man1/aeb.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aebuffy.1*
%attr(0644,root,bin) /usr/man/man1/aebuffy.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeca.1*
%attr(0644,root,bin) /usr/man/man1/aeca.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aecd.1*
%attr(0644,root,bin) /usr/man/man1/aecd.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aechown.1*
%attr(0644,root,bin) /usr/man/man1/aechown.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeclean.1*
%attr(0644,root,bin) /usr/man/man1/aeclean.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeclone.1*
%attr(0644,root,bin) /usr/man/man1/aeclone.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aecomp.1*
%attr(0644,root,bin) /usr/man/man1/aecomp.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aecomplete.1*
%attr(0644,root,bin) /usr/man/man1/aecomplete.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aecp.1*
%attr(0644,root,bin) /usr/man/man1/aecp.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aecpu.1*
%attr(0644,root,bin) /usr/man/man1/aecpu.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aecvsserver.1*
%attr(0644,root,bin) /usr/man/man1/aecvsserver.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aed.1*
%attr(0644,root,bin) /usr/man/man1/aed.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aedb.1*
%attr(0644,root,bin) /usr/man/man1/aedb.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aedbu.1*
%attr(0644,root,bin) /usr/man/man1/aedbu.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aede.1*
%attr(0644,root,bin) /usr/man/man1/aede.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aedeu.1*
%attr(0644,root,bin) /usr/man/man1/aedeu.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aedist.1*
%attr(0644,root,bin) /usr/man/man1/aedist.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aedn.1*
%attr(0644,root,bin) /usr/man/man1/aedn.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeedit.1*
%attr(0644,root,bin) /usr/man/man1/aeedit.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aefa.1*
%attr(0644,root,bin) /usr/man/man1/aefa.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aefind.1*
%attr(0644,root,bin) /usr/man/man1/aefind.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aefp.1*
%attr(0644,root,bin) /usr/man/man1/aefp.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeget.1*
%attr(0644,root,bin) /usr/man/man1/aeget.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aegis.1*
%attr(0644,root,bin) /usr/man/man1/aegis.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aegis.cgi.1*
%attr(0644,root,bin) /usr/man/man1/aegis.cgi.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeib.1*
%attr(0644,root,bin) /usr/man/man1/aeib.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeibu.1*
%attr(0644,root,bin) /usr/man/man1/aeibu.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeifail.1*
%attr(0644,root,bin) /usr/man/man1/aeifail.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeimport.1*
%attr(0644,root,bin) /usr/man/man1/aeimport.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeintegratq.1*
%attr(0644,root,bin) /usr/man/man1/aeintegratq.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeipass.1*
%attr(0644,root,bin) /usr/man/man1/aeipass.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/ael.1*
%attr(0644,root,bin) /usr/man/man1/ael.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aelic.1*
%attr(0644,root,bin) /usr/man/man1/aelic.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aels.1*
%attr(0644,root,bin) /usr/man/man1/aels.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aemeasure.1*
%attr(0644,root,bin) /usr/man/man1/aemeasure.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aemt.1*
%attr(0644,root,bin) /usr/man/man1/aemt.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aemtu.1*
%attr(0644,root,bin) /usr/man/man1/aemtu.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aemv.1*
%attr(0644,root,bin) /usr/man/man1/aemv.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aemvu.1*
%attr(0644,root,bin) /usr/man/man1/aemvu.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aena.1*
%attr(0644,root,bin) /usr/man/man1/aena.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenbr.1*
%attr(0644,root,bin) /usr/man/man1/aenbr.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenbru.1*
%attr(0644,root,bin) /usr/man/man1/aenbru.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenc.1*
%attr(0644,root,bin) /usr/man/man1/aenc.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aencu.1*
%attr(0644,root,bin) /usr/man/man1/aencu.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aend.1*
%attr(0644,root,bin) /usr/man/man1/aend.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenf.1*
%attr(0644,root,bin) /usr/man/man1/aenf.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenfu.1*
%attr(0644,root,bin) /usr/man/man1/aenfu.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeni.1*
%attr(0644,root,bin) /usr/man/man1/aeni.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenpa.1*
%attr(0644,root,bin) /usr/man/man1/aenpa.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenpr.1*
%attr(0644,root,bin) /usr/man/man1/aenpr.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenrls.1*
%attr(0644,root,bin) /usr/man/man1/aenrls.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenrv.1*
%attr(0644,root,bin) /usr/man/man1/aenrv.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aent.1*
%attr(0644,root,bin) /usr/man/man1/aent.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aentu.1*
%attr(0644,root,bin) /usr/man/man1/aentu.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aepa.1*
%attr(0644,root,bin) /usr/man/man1/aepa.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aepatch.1*
%attr(0644,root,bin) /usr/man/man1/aepatch.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aer.1*
%attr(0644,root,bin) /usr/man/man1/aer.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aera.1*
%attr(0644,root,bin) /usr/man/man1/aera.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerb.1*
%attr(0644,root,bin) /usr/man/man1/aerb.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerbu.1*
%attr(0644,root,bin) /usr/man/man1/aerbu.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerd.1*
%attr(0644,root,bin) /usr/man/man1/aerd.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerect.1*
%attr(0644,root,bin) /usr/man/man1/aerect.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerfail.1*
%attr(0644,root,bin) /usr/man/man1/aerfail.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeri.1*
%attr(0644,root,bin) /usr/man/man1/aeri.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerm.1*
%attr(0644,root,bin) /usr/man/man1/aerm.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aermpr.1*
%attr(0644,root,bin) /usr/man/man1/aermpr.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aermu.1*
%attr(0644,root,bin) /usr/man/man1/aermu.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerpa.1*
%attr(0644,root,bin) /usr/man/man1/aerpa.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerpass.1*
%attr(0644,root,bin) /usr/man/man1/aerpass.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerpu.1*
%attr(0644,root,bin) /usr/man/man1/aerpu.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerrv.1*
%attr(0644,root,bin) /usr/man/man1/aerrv.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aesub.1*
%attr(0644,root,bin) /usr/man/man1/aesub.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aet.1*
%attr(0644,root,bin) /usr/man/man1/aet.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aetar.1*
%attr(0644,root,bin) /usr/man/man1/aetar.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aev.1*
%attr(0644,root,bin) /usr/man/man1/aev.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/aexml.1*
%attr(0644,root,bin) /usr/man/man1/aexml.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/tkaeca.1*
%attr(0644,root,bin) /usr/man/man1/tkaeca.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/tkaegis.1*
%attr(0644,root,bin) /usr/man/man1/tkaegis.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/tkaenc.1*
%attr(0644,root,bin) /usr/man/man1/tkaenc.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/tkaepa.1*
%attr(0644,root,bin) /usr/man/man1/tkaepa.1*
%attr(0644,root,bin) /usr/share/aegis/en/man1/tkaer.1*
%attr(0644,root,bin) /usr/man/man1/tkaer.1*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aecattr.5*
%attr(0644,root,bin) /usr/man/man5/aecattr.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aecstate.5*
%attr(0644,root,bin) /usr/man/man5/aecstate.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aedir.5*
%attr(0644,root,bin) /usr/man/man5/aedir.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aefattr.5*
%attr(0644,root,bin) /usr/man/man5/aefattr.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aefstate.5*
%attr(0644,root,bin) /usr/man/man5/aefstate.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aegis.5*
%attr(0644,root,bin) /usr/man/man5/aegis.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aegstate.5*
%attr(0644,root,bin) /usr/man/man5/aegstate.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aelock.5*
%attr(0644,root,bin) /usr/man/man5/aelock.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aemetrics.5*
%attr(0644,root,bin) /usr/man/man5/aemetrics.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aepattr.5*
%attr(0644,root,bin) /usr/man/man5/aepattr.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aepconf.5*
%attr(0644,root,bin) /usr/man/man5/aepconf.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aepstate.5*
%attr(0644,root,bin) /usr/man/man5/aepstate.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aer.5*
%attr(0644,root,bin) /usr/man/man5/aer.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aerptidx.5*
%attr(0644,root,bin) /usr/man/man5/aerptidx.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aesub.5*
%attr(0644,root,bin) /usr/man/man5/aesub.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aetest.5*
%attr(0644,root,bin) /usr/man/man5/aetest.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aeuconf.5*
%attr(0644,root,bin) /usr/man/man5/aeuconf.5*
%attr(0644,root,bin) /usr/share/aegis/en/man5/aeustate.5*
%attr(0644,root,bin) /usr/man/man5/aeustate.5*
%attr(0644,root,bin) /usr/share/aegis/en/notes/locale.man
%attr(0644,root,bin) /usr/lib/aegis/es/LC_MESSAGES/aegis.mo
%attr(0644,root,bin) /usr/lib/aegis/fr/LC_MESSAGES/aegis.mo
%attr(0644,root,bin) /usr/share/aegis/fr/LC_MESSAGES/glossaire
%attr(0644,root,bin) /usr/share/aegis/man1/en/ae_c.1
%attr(0644,root,bin) /usr/share/aegis/man1/en/ae_p.1
%attr(0644,root,bin) /usr/lib/aegis/nl/LC_MESSAGES/aegis.mo
%attr(0644,root,bin) /usr/share/aegis/report.index
%attr(0644,root,bin) /usr/share/aegis/report/chan_detai.rpt
%attr(0644,root,bin) /usr/share/aegis/report/chan_files.rpt
%attr(0644,root,bin) /usr/share/aegis/report/chan_histo.rpt
%attr(0644,root,bin) /usr/share/aegis/report/chan_lists.rpt
%attr(0644,root,bin) /usr/share/aegis/report/chan_log.rpt
%attr(0644,root,bin) /usr/share/aegis/report/chan_recur.rpt
%attr(0644,root,bin) /usr/share/aegis/report/chan_so.rpt
%attr(0644,root,bin) /usr/share/aegis/report/debchanlog.rpt
%attr(0644,root,bin) /usr/share/aegis/report/file_activ.rpt
%attr(0644,root,bin) /usr/share/aegis/report/file_cflct.rpt
%attr(0644,root,bin) /usr/share/aegis/report/file_hstry.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_activ.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_admin.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_ances.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_brada.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_bran.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_brana.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_detai.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_devel.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_files.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_gantt.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_integ.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_lists.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_recur.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_rvwrs.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_staff.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_stats.rpt
%attr(0644,root,bin) /usr/lib/aegis/ro/LC_MESSAGES/aegis.mo
%attr(0644,root,bin) /usr/lib/aegis/ru/LC_MESSAGES/aegis.mo
%attr(0644,root,bin) /usr/share/aegis/wish/aebuffy.rpt
%attr(0644,root,bin) /usr/share/aegis/wish/aecomp.rpt
%attr(0644,root,bin) /usr/share/aegis/wish/chan_attr.rpt
%attr(0644,root,bin) /usr/share/aegis/wish/chan_list.rpt
%attr(0644,root,bin) /usr/share/aegis/wish/devs_list.rpt
%attr(0644,root,bin) /usr/share/aegis/wish/proj_attr.rpt
%attr(0644,root,bin) /usr/share/aegis/wish/proj_conf.rpt
%attr(0644,root,bin) /usr/share/aegis/wish/proj_list.rpt
%attr(0644,root,bin) /usr/share/aegis/wish/tkaer_info.rpt
%attr(0755,root,bin) /etc/profile.d/aegis.sh
%attr(0755,root,bin) /etc/profile.d/aegis.csh

#
# See the comment at the top of this file.  If you don't like
# the file attributes, or there is a file missing, DO NOT send
# the maintainer a patch to this file.  This file is GENERATED.
# If you want different attributes, fix the etc/spec.sh file,
# and send THAT patch to the maintainer.
#
%files txtdocs
%attr(0755,root,bin) %dir /usr/share/aegis
%attr(0644,root,bin) /usr/share/aegis/de/auug97.txt
%attr(0644,root,bin) /usr/share/aegis/en/aoss4.txt
%attr(0644,root,bin) /usr/share/aegis/en/auug93.txt
%attr(0644,root,bin) /usr/share/aegis/en/auug96.txt
%attr(0644,root,bin) /usr/share/aegis/en/auug97.txt
%attr(0644,root,bin) /usr/share/aegis/en/building.txt
%attr(0644,root,bin) /usr/share/aegis/en/change_log.txt
%attr(0644,root,bin) /usr/share/aegis/en/cvs-comparison.txt
%attr(0644,root,bin) /usr/share/aegis/en/faq.txt
%attr(0644,root,bin) /usr/share/aegis/en/howto.txt
%attr(0644,root,bin) /usr/share/aegis/en/lsm.txt
%attr(0644,root,bin) /usr/share/aegis/en/readme.txt
%attr(0644,root,bin) /usr/share/aegis/en/refman.txt
%attr(0644,root,bin) /usr/share/aegis/en/release.txt
%attr(0644,root,bin) /usr/share/aegis/en/user-guide.txt

#
# See the comment at the top of this file.  If you don't like
# the file attributes, or there is a file missing, DO NOT send
# the maintainer a patch to this file.  This file is GENERATED.
# If you want different attributes, fix the etc/spec.sh file,
# and send THAT patch to the maintainer.
#
%files psdocs
%attr(0755,root,bin) %dir /usr/share/aegis
%attr(0644,root,bin) /usr/share/aegis/de/auug97.ps
%attr(0644,root,bin) /usr/share/aegis/en/aoss4.ps
%attr(0644,root,bin) /usr/share/aegis/en/auug93.ps
%attr(0644,root,bin) /usr/share/aegis/en/auug96.ps
%attr(0644,root,bin) /usr/share/aegis/en/auug97.ps
%attr(0644,root,bin) /usr/share/aegis/en/building.ps
%attr(0644,root,bin) /usr/share/aegis/en/change_log.ps
%attr(0644,root,bin) /usr/share/aegis/en/cvs-comparison.ps
%attr(0644,root,bin) /usr/share/aegis/en/faq.ps
%attr(0644,root,bin) /usr/share/aegis/en/howto.ps
%attr(0644,root,bin) /usr/share/aegis/en/lsm.ps
%attr(0644,root,bin) /usr/share/aegis/en/readme.ps
%attr(0644,root,bin) /usr/share/aegis/en/refman.ps
%attr(0644,root,bin) /usr/share/aegis/en/release.ps
%attr(0644,root,bin) /usr/share/aegis/en/user-guide.ps

#
# See the comment at the top of this file.  If you don't like
# the file attributes, or there is a file missing, DO NOT send
# the maintainer a patch to this file.  This file is GENERATED.
# If you want different attributes, fix the etc/spec.sh file,
# and send THAT patch to the maintainer.
#
%files dvidocs
%attr(0755,root,bin) %dir /usr/share/aegis
%attr(0644,root,bin) /usr/share/aegis/de/auug97.dvi
%attr(0644,root,bin) /usr/share/aegis/en/aoss4.dvi
%attr(0644,root,bin) /usr/share/aegis/en/auug93.dvi
%attr(0644,root,bin) /usr/share/aegis/en/auug96.dvi
%attr(0644,root,bin) /usr/share/aegis/en/auug97.dvi
%attr(0644,root,bin) /usr/share/aegis/en/building.dvi
%attr(0644,root,bin) /usr/share/aegis/en/change_log.dvi
%attr(0644,root,bin) /usr/share/aegis/en/cvs-comparison.dvi
%attr(0644,root,bin) /usr/share/aegis/en/faq.dvi
%attr(0644,root,bin) /usr/share/aegis/en/howto.dvi
%attr(0644,root,bin) /usr/share/aegis/en/lsm.dvi
%attr(0644,root,bin) /usr/share/aegis/en/readme.dvi
%attr(0644,root,bin) /usr/share/aegis/en/refman.dvi
%attr(0644,root,bin) /usr/share/aegis/en/release.dvi
%attr(0644,root,bin) /usr/share/aegis/en/user-guide.dvi

#
# This next bit is done because when using Aegis with NFS, these
# files must have EXACTLY the same uid and gid on all systems.
# Unfortunately, RPM won't let you give exact numeric uids and gids,
# and the names for low-numbered uids and gids are essentially
# random across the various Unix implementations.  Sigh.
#
%post
chown -R 3 /usr/com/aegis /usr/lib/aegis /usr/share/aegis
chgrp -R 3 /usr/com/aegis /usr/lib/aegis /usr/share/aegis

%clean
rm -rf $RPM_BUILD_ROOT
