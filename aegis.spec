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
Summary: aegis
Name: aegis
Version: 4.25.D510
Release: 1
License: GPL
Group: Development/Tools
Source: http://aegis.sourceforge.net/%{name}-%{version}.tar.gz
URL: http://aegis.sourceforge.net/
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildPrereq: bison, bzip2, cssc, cvs, debhelper (>= 7), diffutils, ed, fhist
BuildPrereq: flex, gawk, gettext, ghostscript, groff, iso-codes, libbz2-dev
BuildPrereq: libcurl4-gnutls-dev, libmagic-dev, libxml2-dev, linux-libc-dev
BuildPrereq: locales, patchutils (>= 0.2.25), perl, po-debconf, rcs, sharutils
BuildPrereq: subversion [!sh4 !avr32], uuid-dev, zlib1g-dev

%description
transaction-based software configuration management (DVCS)


%prep
%setup -q


%build
%configure --sysconfdir=/etc --prefix=%{_prefix} --mandir=%{_mandir} \
		--with-nlsdir=%{_datadir}/locale
make


%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install


%check
make sure


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr (-,root,root,-)
%doc LICENSE BUILDING README
%doc %{_datarootdir}/doc/aegis/aoss4.pdf
%doc %{_datarootdir}/doc/aegis/auug93.pdf
%doc %{_datarootdir}/doc/aegis/auug96.pdf
%doc %{_datarootdir}/doc/aegis/auug97.pdf
%doc %{_datarootdir}/doc/aegis/building.pdf
%doc %{_datarootdir}/doc/aegis/change_log.pdf
%doc %{_datarootdir}/doc/aegis/cvs-comparison.pdf
%doc %{_datarootdir}/doc/aegis/de/auug97.pdf
%doc %{_datarootdir}/doc/aegis/faq.pdf
%doc %{_datarootdir}/doc/aegis/howto.pdf
%doc %{_datarootdir}/doc/aegis/lsm.pdf
%doc %{_datarootdir}/doc/aegis/readme.pdf
%doc %{_datarootdir}/doc/aegis/refman.pdf
%doc %{_datarootdir}/doc/aegis/release.pdf
%doc %{_datarootdir}/doc/aegis/user-guide.pdf
%{_localedir}/da/LC_MESSAGES/aegis.mo
%{_localedir}/de/LC_MESSAGES/aegis.mo
%{_localedir}/en/LC_MESSAGES/aegis.mo
%{_localedir}/es/LC_MESSAGES/aegis.mo
%{_localedir}/fr/LC_MESSAGES/aegis.mo
%{_localedir}/it/LC_MESSAGES/aegis.mo
%{_localedir}/nl/LC_MESSAGES/aegis.mo
%{_localedir}/pt_BR/LC_MESSAGES/aegis.mo
%{_localedir}/ro/LC_MESSAGES/aegis.mo
%{_localedir}/ru/LC_MESSAGES/aegis.mo
%{_localedir}/vi/LC_MESSAGES/aegis.mo
%{_bindir}/ae-cvs-ci
%{_bindir}/ae-repo-ci
%{_bindir}/ae-sccs-put
%{_bindir}/ae_diff2htm
%{_bindir}/aeannotate
%{_bindir}/aebisect
%{_bindir}/aebuffy
%{_bindir}/aecomp
%{_bindir}/aecomplete
%{_bindir}/aecvsserver
%{_bindir}/aede-policy
%{_bindir}/aediff
%{_bindir}/aedist
%{_bindir}/aeedit
%{_bindir}/aefind
%{_bindir}/aefinish
%{_bindir}/aefp
%{_bindir}/aeget
%{_bindir}/aeget.instal
%{_bindir}/aegis
%{_bindir}/aegrep
%{_bindir}/aeimport
%{_bindir}/aeintegratq
%{_bindir}/aelcf
%{_bindir}/aelock
%{_bindir}/aelpf
%{_bindir}/aels
%{_bindir}/aelsf
%{_bindir}/aemakegen
%{_bindir}/aemeasure
%{_bindir}/aepatch
%{_bindir}/aepromptcmd
%{_bindir}/aerect
%{_bindir}/aereport
%{_bindir}/aerevml
%{_bindir}/aesub
%{_bindir}/aesubunit
%{_bindir}/aesvt
%{_bindir}/aetar
%{_bindir}/aexml
%{_bindir}/aexver
%{_bindir}/tkaeca
%{_bindir}/tkaegis
%{_bindir}/tkaenc
%{_bindir}/tkaepa
%{_bindir}/tkaer
%{_bindir}/xaegis
%{_datadir}/aegis/aedefault.css
%{_datadir}/aegis/aegis.icon
%{_datadir}/aegis/aegis.mask
%{_datadir}/aegis/aegis.pgm
%{_datadir}/aegis/config.example/aesvt
%{_datadir}/aegis/config.example/architecture
%{_datadir}/aegis/config.example/cake
%{_datadir}/aegis/config.example/cook
%{_datadir}/aegis/config.example/diff
%{_datadir}/aegis/config.example/diff3
%{_datadir}/aegis/config.example/diff_not
%{_datadir}/aegis/config.example/fhist
%{_datadir}/aegis/config.example/gnu_diff
%{_datadir}/aegis/config.example/make
%{_datadir}/aegis/config.example/makefile
%{_datadir}/aegis/config.example/merge
%{_datadir}/aegis/config.example/rcs
%{_datadir}/aegis/config.example/sccs
%{_datadir}/aegis/cshrc
%{_datadir}/aegis/db_forced.sh
%{_datadir}/aegis/de.inews.sh
%{_datadir}/aegis/de.sh
%{_datadir}/aegis/de/LC_MESSAGES/glossary
%{_datadir}/aegis/deu.inews.sh
%{_datadir}/aegis/deu.sh
%{_datadir}/aegis/en/html/chan_hstry.rpt
%{_datadir}/aegis/en/html/chan_menu.rpt
%{_datadir}/aegis/en/html/changes.rpt
%{_datadir}/aegis/en/html/cp_command.rpt
%{_datadir}/aegis/en/html/file_activ.rpt
%{_datadir}/aegis/en/html/file_cflct.rpt
%{_datadir}/aegis/en/html/file_densi.rpt
%{_datadir}/aegis/en/html/file_diff.rpt
%{_datadir}/aegis/en/html/file_hstry.rpt
%{_datadir}/aegis/en/html/file_list.rpt
%{_datadir}/aegis/en/html/file_menu.rpt
%{_datadir}/aegis/en/html/proj_cch1.rpt
%{_datadir}/aegis/en/html/proj_cch2.rpt
%{_datadir}/aegis/en/html/proj_cch3.rpt
%{_datadir}/aegis/en/html/proj_filme.rpt
%{_datadir}/aegis/en/html/proj_hstg2.rpt
%{_datadir}/aegis/en/html/proj_hstg3.rpt
%{_datadir}/aegis/en/html/proj_hstgm.rpt
%{_datadir}/aegis/en/html/proj_hstry.rpt
%{_datadir}/aegis/en/html/proj_list.rpt
%{_datadir}/aegis/en/html/proj_menu.rpt
%{_datadir}/aegis/en/html/proj_prgr1.rpt
%{_datadir}/aegis/en/html/proj_prgr2.rpt
%{_datadir}/aegis/en/html/proj_prgr3.rpt
%{_datadir}/aegis/en/html/proj_staff.rpt
%{_datadir}/aegis/en/html/proj_stats.rpt
%{_datadir}/aegis/en/notes/locale.man
%{_datadir}/aegis/fr/LC_MESSAGES/glossaire
%{_datadir}/aegis/icon/aegis.gif
%{_datadir}/aegis/icon/rss.gif
%{_datadir}/aegis/icon2/64x64.png
%{_datadir}/aegis/icon2/bigger.png
%{_datadir}/aegis/icon3/aegis.png
%{_datadir}/aegis/if.inews.sh
%{_datadir}/aegis/if.sh
%{_datadir}/aegis/ip.inews.sh
%{_datadir}/aegis/ip.sh
%{_datadir}/aegis/profile
%{_datadir}/aegis/profile.bash
%{_datadir}/aegis/profile.zsh
%{_datadir}/aegis/remind/awt_dvlp.sh
%{_datadir}/aegis/remind/awt_intgrtn.sh
%{_datadir}/aegis/remind/bng_dvlpd.sh
%{_datadir}/aegis/remind/bng_rvwd.sh
%{_datadir}/aegis/report.index
%{_datadir}/aegis/report/chan_detai.rpt
%{_datadir}/aegis/report/chan_files.rpt
%{_datadir}/aegis/report/chan_histo.rpt
%{_datadir}/aegis/report/chan_lists.rpt
%{_datadir}/aegis/report/chan_log.rpt
%{_datadir}/aegis/report/chan_recur.rpt
%{_datadir}/aegis/report/chan_so.rpt
%{_datadir}/aegis/report/debchanlog.rpt
%{_datadir}/aegis/report/file_activ.rpt
%{_datadir}/aegis/report/file_cflct.rpt
%{_datadir}/aegis/report/file_hstry.rpt
%{_datadir}/aegis/report/file_vhist.rpt
%{_datadir}/aegis/report/proj_activ.rpt
%{_datadir}/aegis/report/proj_admin.rpt
%{_datadir}/aegis/report/proj_ances.rpt
%{_datadir}/aegis/report/proj_brada.rpt
%{_datadir}/aegis/report/proj_bran.rpt
%{_datadir}/aegis/report/proj_brana.rpt
%{_datadir}/aegis/report/proj_detai.rpt
%{_datadir}/aegis/report/proj_devel.rpt
%{_datadir}/aegis/report/proj_files.rpt
%{_datadir}/aegis/report/proj_gantt.rpt
%{_datadir}/aegis/report/proj_integ.rpt
%{_datadir}/aegis/report/proj_lists.rpt
%{_datadir}/aegis/report/proj_parent.rpt
%{_datadir}/aegis/report/proj_recur.rpt
%{_datadir}/aegis/report/proj_rvwrs.rpt
%{_datadir}/aegis/report/proj_staff.rpt
%{_datadir}/aegis/report/proj_stats.rpt
%{_datadir}/aegis/report/user_list.rpt
%{_datadir}/aegis/report/user_recurs.rpt
%{_datadir}/aegis/rf.inews.sh
%{_datadir}/aegis/rf.sh
%{_datadir}/aegis/rp.inews.sh
%{_datadir}/aegis/rp.sh
%{_datadir}/aegis/rpu.inews.sh
%{_datadir}/aegis/rpu.sh
%{_datadir}/aegis/wish/aebuffy.rpt
%{_datadir}/aegis/wish/aecomp.rpt
%{_datadir}/aegis/wish/chan_attr.rpt
%{_datadir}/aegis/wish/chan_list.rpt
%{_datadir}/aegis/wish/devs_list.rpt
%{_datadir}/aegis/wish/proj_attr.rpt
%{_datadir}/aegis/wish/proj_conf.rpt
%{_datadir}/aegis/wish/proj_list.rpt
%{_datadir}/aegis/wish/tkaer_info.rpt
%{_mandir}/man1/ae-cvs-ci.1
%{_mandir}/man1/ae-repo-ci.1
%{_mandir}/man1/ae-sccs-put.1
%{_mandir}/man1/ae_c.1
%{_mandir}/man1/ae_diff2htm.1
%{_mandir}/man1/ae_p.1
%{_mandir}/man1/aeannotate.1
%{_mandir}/man1/aeb.1
%{_mandir}/man1/aebisect.1
%{_mandir}/man1/aebuffy.1
%{_mandir}/man1/aeca.1
%{_mandir}/man1/aecd.1
%{_mandir}/man1/aechown.1
%{_mandir}/man1/aeclean.1
%{_mandir}/man1/aeclone.1
%{_mandir}/man1/aecomp.1
%{_mandir}/man1/aecomplete.1
%{_mandir}/man1/aecp.1
%{_mandir}/man1/aecpu.1
%{_mandir}/man1/aecvsserver.1
%{_mandir}/man1/aed.1
%{_mandir}/man1/aedb.1
%{_mandir}/man1/aedbu.1
%{_mandir}/man1/aede-policy.1
%{_mandir}/man1/aede.1
%{_mandir}/man1/aedeu.1
%{_mandir}/man1/aediff.1
%{_mandir}/man1/aedist.1
%{_mandir}/man1/aedn.1
%{_mandir}/man1/aeedit.1
%{_mandir}/man1/aefa.1
%{_mandir}/man1/aefind.1
%{_mandir}/man1/aefinish.1
%{_mandir}/man1/aefp.1
%{_mandir}/man1/aeget.1
%{_mandir}/man1/aegis.1
%{_mandir}/man1/aegis.cgi.1
%{_mandir}/man1/aegrep.1
%{_mandir}/man1/aeib.1
%{_mandir}/man1/aeibu.1
%{_mandir}/man1/aeifail.1
%{_mandir}/man1/aeimport.1
%{_mandir}/man1/aeintegratq.1
%{_mandir}/man1/aeipass.1
%{_mandir}/man1/ael.1
%{_mandir}/man1/aelcf.1
%{_mandir}/man1/aelic.1
%{_mandir}/man1/aelock.1
%{_mandir}/man1/aelpf.1
%{_mandir}/man1/aels.1
%{_mandir}/man1/aelsf.1
%{_mandir}/man1/aemakegen.1
%{_mandir}/man1/aemeasure.1
%{_mandir}/man1/aemt.1
%{_mandir}/man1/aemtu.1
%{_mandir}/man1/aemv.1
%{_mandir}/man1/aemvu.1
%{_mandir}/man1/aena.1
%{_mandir}/man1/aenbr.1
%{_mandir}/man1/aenbru.1
%{_mandir}/man1/aenc.1
%{_mandir}/man1/aencu.1
%{_mandir}/man1/aend.1
%{_mandir}/man1/aenf.1
%{_mandir}/man1/aenfu.1
%{_mandir}/man1/aeni.1
%{_mandir}/man1/aenpa.1
%{_mandir}/man1/aenpr.1
%{_mandir}/man1/aenrls.1
%{_mandir}/man1/aenrv.1
%{_mandir}/man1/aent.1
%{_mandir}/man1/aentu.1
%{_mandir}/man1/aepa.1
%{_mandir}/man1/aepatch.1
%{_mandir}/man1/aepromptcmd.1
%{_mandir}/man1/aer.1
%{_mandir}/man1/aera.1
%{_mandir}/man1/aerb.1
%{_mandir}/man1/aerbu.1
%{_mandir}/man1/aerd.1
%{_mandir}/man1/aerect.1
%{_mandir}/man1/aerevml.1
%{_mandir}/man1/aerfail.1
%{_mandir}/man1/aeri.1
%{_mandir}/man1/aerm.1
%{_mandir}/man1/aermpr.1
%{_mandir}/man1/aermu.1
%{_mandir}/man1/aerpa.1
%{_mandir}/man1/aerpass.1
%{_mandir}/man1/aerpu.1
%{_mandir}/man1/aerrv.1
%{_mandir}/man1/aesub.1
%{_mandir}/man1/aesubunit.1
%{_mandir}/man1/aesvt.1
%{_mandir}/man1/aet.1
%{_mandir}/man1/aetar.1
%{_mandir}/man1/aev.1
%{_mandir}/man1/aexml.1
%{_mandir}/man1/aexver.1
%{_mandir}/man1/tkaeca.1
%{_mandir}/man1/tkaegis.1
%{_mandir}/man1/tkaenc.1
%{_mandir}/man1/tkaepa.1
%{_mandir}/man1/tkaer.1
%{_mandir}/man5/aecattr.5
%{_mandir}/man5/aecstate.5
%{_mandir}/man5/aedir.5
%{_mandir}/man5/aefattr.5
%{_mandir}/man5/aefstate.5
%{_mandir}/man5/aegis.5
%{_mandir}/man5/aegstate.5
%{_mandir}/man5/aelock.5
%{_mandir}/man5/aemetrics.5
%{_mandir}/man5/aepattr.5
%{_mandir}/man5/aepconf.5
%{_mandir}/man5/aepstate.5
%{_mandir}/man5/aer.5
%{_mandir}/man5/aerptidx.5
%{_mandir}/man5/aesub.5
%{_mandir}/man5/aetest.5
%{_mandir}/man5/aeuconf.5
%{_mandir}/man5/aeustate.5
%{_sysconfdir}/profile.d/aegis.bash
%{_sysconfdir}/profile.d/aegis.csh
%{_sysconfdir}/profile.d/aegis.sh
%{_sysconfdir}/profile.d/aegis.zsh
