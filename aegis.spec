Summary: project change supervisor
Name: aegis
Version: 3.15
Release: 1
Copyright: GPL
Group: Development/Version Control
Source: http://www.canb.auug.org.au/~millerp/aegis-3.15.tar.gz
URL: http://www.canb.auug.org.au/~millerp/aegis.html
BuildRoot: /tmp/aegis-build-root
Icon: aegis.gif

%description
Aegis is a transaction-based software configuration management system.
It provides a framework within which a team of developers may work
on many changes to a program independently, and Aegis coordinates
integrating these changes back into the master source of the program,
with as little disruption as possible.

%package txtdocs
Summary: Aegis documentation, dumb ascii text
Group: Development/Building

%description txtdocs
Aegis documentation in dumb ascii text format.

%package psdocs
Summary: Aegis documentation, PostScript format
Group: Development/Building

%description psdocs
Aegis documentation in PostScript format.

%package dvidocs
Summary: aegis documentation, DVI format
Group: Development/Building

%description dvidocs
Aegis documentation in DVI format.

%prep
%setup

%build
./configure --prefix=/usr
make

%install
make RPM_BUILD_ROOT=$RPM_BUILD_ROOT install

%files
%attr(0755,root,bin) %dir /usr/com/aegis
%attr(0755,root,bin) %dir /usr/lib/aegis
%attr(0755,root,bin) %dir /usr/share/aegis
%attr(0755,root,bin) /usr/bin/aedist
%attr(0755,root,bin) /usr/bin/aefind
%attr(4755,root,bin) /usr/bin/aegis
%attr(0755,root,bin) /usr/bin/aerect
%attr(0755,root,bin) /usr/bin/aereport
%attr(0755,root,bin) /home/httpd/cgi-bin/aegis.cgi
%attr(0755,root,bin) /usr/share/aegis/db_forced.sh
%attr(0755,root,bin) /usr/share/aegis/de.inews.sh
%attr(0755,root,bin) /usr/share/aegis/de.sh
%attr(0755,root,bin) /usr/share/aegis/deu.inews.sh
%attr(0755,root,bin) /usr/share/aegis/deu.sh
%attr(0755,root,bin) /usr/share/aegis/if.inews.sh
%attr(0755,root,bin) /usr/share/aegis/if.sh
%attr(0755,root,bin) /usr/share/aegis/integrate_q.sh
%attr(0755,root,bin) /usr/share/aegis/ip.inews.sh
%attr(0755,root,bin) /usr/share/aegis/ip.sh
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
%attr(0644,root,bin) /usr/share/aegis/aegis.icon
%attr(0644,root,bin) /usr/share/aegis/aegis.mask
%attr(0644,root,bin) /usr/share/aegis/aegis.pgm
%attr(0644,root,bin) /usr/share/aegis/config.example/architecture
%attr(0644,root,bin) /usr/share/aegis/config.example/cake
%attr(0644,root,bin) /usr/share/aegis/config.example/cook
%attr(0644,root,bin) /usr/share/aegis/config.example/fhist
%attr(0644,root,bin) /usr/share/aegis/config.example/make
%attr(0644,root,bin) /usr/share/aegis/config.example/rcs
%attr(0644,root,bin) /usr/share/aegis/config.example/sccs
%attr(0644,root,bin) /usr/share/aegis/cshrc
%attr(0644,root,bin) /usr/lib/aegis/en/LC_MESSAGES/aedist.mo
%attr(0644,root,bin) /usr/lib/aegis/en/LC_MESSAGES/aefind.mo
%attr(0644,root,bin) /usr/lib/aegis/en/LC_MESSAGES/aegis.mo
%attr(0644,root,bin) /usr/lib/aegis/en/LC_MESSAGES/aerect.mo
%attr(0644,root,bin) /usr/lib/aegis/en/LC_MESSAGES/aereport.mo
%attr(0644,root,bin) /usr/share/aegis/en/html/chan_awdev.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/chan_awint.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/chan_bedev.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/chan_beint.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/chan_berev.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/chan_files.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/chan_hstry.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/chan_list.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/chan_menu.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/file_activ.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/file_cflct.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/file_densi.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/file_hstry.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/file_menu.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_cch1.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_cch2.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_cch3.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_files.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_hstg2.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_hstg3.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_hstgm.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_hstry.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_list.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_menu.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_staff.rpt
%attr(0644,root,bin) /usr/share/aegis/en/html/proj_stats.rpt
%attr(0644,root,bin) /usr/share/aegis/en/man1/ae_c.1
%attr(0644,root,bin) /usr/man/man1/ae_c.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/ae_p.1
%attr(0644,root,bin) /usr/man/man1/ae_p.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeb.1
%attr(0644,root,bin) /usr/man/man1/aeb.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeca.1
%attr(0644,root,bin) /usr/man/man1/aeca.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aecd.1
%attr(0644,root,bin) /usr/man/man1/aecd.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aechown.1
%attr(0644,root,bin) /usr/man/man1/aechown.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeclean.1
%attr(0644,root,bin) /usr/man/man1/aeclean.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeclone.1
%attr(0644,root,bin) /usr/man/man1/aeclone.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aecp.1
%attr(0644,root,bin) /usr/man/man1/aecp.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aecpu.1
%attr(0644,root,bin) /usr/man/man1/aecpu.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aed.1
%attr(0644,root,bin) /usr/man/man1/aed.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aedb.1
%attr(0644,root,bin) /usr/man/man1/aedb.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aedbu.1
%attr(0644,root,bin) /usr/man/man1/aedbu.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aede.1
%attr(0644,root,bin) /usr/man/man1/aede.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aedeu.1
%attr(0644,root,bin) /usr/man/man1/aedeu.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aedist.1
%attr(0644,root,bin) /usr/man/man1/aedist.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aedn.1
%attr(0644,root,bin) /usr/man/man1/aedn.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aefind.1
%attr(0644,root,bin) /usr/man/man1/aefind.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aefp.1
%attr(0644,root,bin) /usr/man/man1/aefp.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aegis.1
%attr(0644,root,bin) /usr/man/man1/aegis.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeib.1
%attr(0644,root,bin) /usr/man/man1/aeib.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeibu.1
%attr(0644,root,bin) /usr/man/man1/aeibu.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeif.1
%attr(0644,root,bin) /usr/man/man1/aeif.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeip.1
%attr(0644,root,bin) /usr/man/man1/aeip.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/ael.1
%attr(0644,root,bin) /usr/man/man1/ael.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aelic.1
%attr(0644,root,bin) /usr/man/man1/aelic.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aemv.1
%attr(0644,root,bin) /usr/man/man1/aemv.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aena.1
%attr(0644,root,bin) /usr/man/man1/aena.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenbr.1
%attr(0644,root,bin) /usr/man/man1/aenbr.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenbru.1
%attr(0644,root,bin) /usr/man/man1/aenbru.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenc.1
%attr(0644,root,bin) /usr/man/man1/aenc.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aencu.1
%attr(0644,root,bin) /usr/man/man1/aencu.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aend.1
%attr(0644,root,bin) /usr/man/man1/aend.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenf.1
%attr(0644,root,bin) /usr/man/man1/aenf.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenfu.1
%attr(0644,root,bin) /usr/man/man1/aenfu.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeni.1
%attr(0644,root,bin) /usr/man/man1/aeni.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenpr.1
%attr(0644,root,bin) /usr/man/man1/aenpr.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenrls.1
%attr(0644,root,bin) /usr/man/man1/aenrls.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aenrv.1
%attr(0644,root,bin) /usr/man/man1/aenrv.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aent.1
%attr(0644,root,bin) /usr/man/man1/aent.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aentu.1
%attr(0644,root,bin) /usr/man/man1/aentu.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aepa.1
%attr(0644,root,bin) /usr/man/man1/aepa.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aer.1
%attr(0644,root,bin) /usr/man/man1/aer.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aera.1
%attr(0644,root,bin) /usr/man/man1/aera.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerd.1
%attr(0644,root,bin) /usr/man/man1/aerd.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerect.1
%attr(0644,root,bin) /usr/man/man1/aerect.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerf.1
%attr(0644,root,bin) /usr/man/man1/aerf.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aeri.1
%attr(0644,root,bin) /usr/man/man1/aeri.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerm.1
%attr(0644,root,bin) /usr/man/man1/aerm.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aermpr.1
%attr(0644,root,bin) /usr/man/man1/aermpr.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aermu.1
%attr(0644,root,bin) /usr/man/man1/aermu.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerp.1
%attr(0644,root,bin) /usr/man/man1/aerp.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerpu.1
%attr(0644,root,bin) /usr/man/man1/aerpu.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aerrv.1
%attr(0644,root,bin) /usr/man/man1/aerrv.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aet.1
%attr(0644,root,bin) /usr/man/man1/aet.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/aev.1
%attr(0644,root,bin) /usr/man/man1/aev.1
%attr(0644,root,bin) /usr/share/aegis/en/man1/tkaegis.1
%attr(0644,root,bin) /usr/man/man1/tkaegis.1
%attr(0644,root,bin) /usr/share/aegis/en/man5/aecattr.5
%attr(0644,root,bin) /usr/man/man5/aecattr.5
%attr(0644,root,bin) /usr/share/aegis/en/man5/aecstate.5
%attr(0644,root,bin) /usr/man/man5/aecstate.5
%attr(0644,root,bin) /usr/share/aegis/en/man5/aedir.5
%attr(0644,root,bin) /usr/man/man5/aedir.5
%attr(0644,root,bin) /usr/share/aegis/en/man5/aefstate.5
%attr(0644,root,bin) /usr/man/man5/aefstate.5
%attr(0644,root,bin) /usr/share/aegis/en/man5/aegis.5
%attr(0644,root,bin) /usr/man/man5/aegis.5
%attr(0644,root,bin) /usr/share/aegis/en/man5/aegstate.5
%attr(0644,root,bin) /usr/man/man5/aegstate.5
%attr(0644,root,bin) /usr/share/aegis/en/man5/aemetrics.5
%attr(0644,root,bin) /usr/man/man5/aemetrics.5
%attr(0644,root,bin) /usr/share/aegis/en/man5/aepattr.5
%attr(0644,root,bin) /usr/man/man5/aepattr.5
%attr(0644,root,bin) /usr/share/aegis/en/man5/aepconf.5
%attr(0644,root,bin) /usr/man/man5/aepconf.5
%attr(0644,root,bin) /usr/share/aegis/en/man5/aepstate.5
%attr(0644,root,bin) /usr/man/man5/aepstate.5
%attr(0644,root,bin) /usr/share/aegis/en/man5/aer.5
%attr(0644,root,bin) /usr/man/man5/aer.5
%attr(0644,root,bin) /usr/share/aegis/en/man5/aerptidx.5
%attr(0644,root,bin) /usr/man/man5/aerptidx.5
%attr(0644,root,bin) /usr/share/aegis/en/man5/aesub.5
%attr(0644,root,bin) /usr/man/man5/aesub.5
%attr(0644,root,bin) /usr/share/aegis/en/man5/aeuconf.5
%attr(0644,root,bin) /usr/man/man5/aeuconf.5
%attr(0644,root,bin) /usr/share/aegis/en/man5/aeustate.5
%attr(0644,root,bin) /usr/man/man5/aeustate.5
%attr(0644,root,bin) /usr/share/aegis/en/notes/locale.man
%attr(0644,root,bin) /usr/share/aegis/man1/en/ae_c.1
%attr(0644,root,bin) /usr/share/aegis/man1/en/ae_p.1
%attr(0644,root,bin) /usr/share/aegis/profile
%attr(0644,root,bin) /usr/share/aegis/report.index
%attr(0644,root,bin) /usr/share/aegis/report/chan_files.rpt
%attr(0644,root,bin) /usr/share/aegis/report/chan_lists.rpt
%attr(0644,root,bin) /usr/share/aegis/report/file_activ.rpt
%attr(0644,root,bin) /usr/share/aegis/report/file_cflct.rpt
%attr(0644,root,bin) /usr/share/aegis/report/file_hstry.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_admin.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_devel.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_files.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_integ.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_lists.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_rvwrs.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_staff.rpt
%attr(0644,root,bin) /usr/share/aegis/report/proj_stats.rpt
%attr(0644,root,bin) /usr/share/aegis/report/recursive.rpt

%files txtdocs
%attr(0644,root,bin) /usr/share/aegis/en/auug93.txt
%attr(0644,root,bin) /usr/share/aegis/en/auug96.txt
%attr(0644,root,bin) /usr/share/aegis/en/auug97.txt
%attr(0644,root,bin) /usr/share/aegis/en/faq.txt
%attr(0644,root,bin) /usr/share/aegis/en/refman.txt
%attr(0644,root,bin) /usr/share/aegis/en/user-guide.txt

%files psdocs
%attr(0644,root,bin) /usr/share/aegis/en/auug93.ps
%attr(0644,root,bin) /usr/share/aegis/en/auug96.ps
%attr(0644,root,bin) /usr/share/aegis/en/auug97.ps
%attr(0644,root,bin) /usr/share/aegis/en/faq.ps
%attr(0644,root,bin) /usr/share/aegis/en/refman.ps
%attr(0644,root,bin) /usr/share/aegis/en/user-guide.ps

%files dvidocs
%attr(0644,root,bin) /usr/share/aegis/en/auug93.dvi
%attr(0644,root,bin) /usr/share/aegis/en/auug96.dvi
%attr(0644,root,bin) /usr/share/aegis/en/auug97.dvi
%attr(0644,root,bin) /usr/share/aegis/en/faq.dvi
%attr(0644,root,bin) /usr/share/aegis/en/refman.dvi
%attr(0644,root,bin) /usr/share/aegis/en/user-guide.dvi

%post
chown -R 3 /usr/com/aegis /usr/lib/aegis /usr/share/aegis
chgrp -R 3 /usr/com/aegis /usr/lib/aegis /usr/share/aegis

%clean
rm -rf $RPM_BUILD_ROOT
