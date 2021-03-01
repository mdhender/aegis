#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991-2003 Peter Miller;
#	All rights reserved.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
#
# MANIFEST: shell script to generate the Makefile.in file
#
aefp_files=
cklinlen_files=
aemeasure_files=
aegis_files=
clean_files="core bin/find_sizes\$(EXEEXT) bin/fmtgen\$(EXEEXT)		\
	libaegis/libaegis.\$(LIBEXT) common/common.\$(LIBEXT) .bin	\
	.bindir .po_files .man1dir .man3dir .man5dir .comdir"
common_files=
libaegis_files=
find_sizes_files="\
	common/ac/libintl.\$(OBJEXT) common/ac/stdlib.\$(OBJEXT)	\
	common/ac/string.\$(OBJEXT) common/ac/time.\$(OBJEXT)		\
	common/ac/unistd.\$(OBJEXT) common/ac/wchar.\$(OBJEXT)		\
	common/ac/wctype.\$(OBJEXT) common/arglex.\$(OBJEXT)		\
	common/error.\$(OBJEXT) common/exeext.\$(OBJEXT)		\
	common/libdir.\$(OBJEXT) common/mem.\$(OBJEXT)			\
	common/mprintf.\$(OBJEXT) common/progname.\$(OBJEXT)		\
	common/str.\$(OBJEXT) common/str/format.\$(OBJEXT) 		\
	common/trace.\$(OBJEXT)"
fmtgen_files="\
	common/ac/libintl.\$(OBJEXT) common/ac/stdlib.\$(OBJEXT)	\
	common/ac/string.\$(OBJEXT) common/ac/time.\$(OBJEXT)		\
	common/ac/unistd.\$(OBJEXT) common/ac/wchar.\$(OBJEXT)		\
	common/ac/wctype.\$(OBJEXT) common/arglex.\$(OBJEXT)		\
	common/error.\$(OBJEXT) common/exeext.\$(OBJEXT)		\
	common/fstrcmp.\$(OBJEXT) common/libdir.\$(OBJEXT)		\
	common/mem.\$(OBJEXT) common/mprintf.\$(OBJEXT)			\
	common/progname.\$(OBJEXT) common/str.\$(OBJEXT)		\
	common/str/format.\$(OBJEXT) common/str_list/append.\$(OBJEXT)	\
	common/str_list/constructor.\$(OBJEXT)				\
	common/str_list/append_uniqu.\$(OBJEXT)				\
	common/symtab.\$(OBJEXT) common/trace.\$(OBJEXT)"

#
# bindir - in a network, these may be shared between machines of the
#	same hw-vendor-os flavour.  May be read-only.
# libdir - in a network, these may be shared between machines of the
#	same hw-vendor-os flavour.  May be read-only.
# datadir - in a network, these may be shared by all flavours.
#	May be read-only.
# mandir - in a network, these may be shared by all flavours.
#	May be read-only.
# comdir - in a network, these may be shared by all flavours.
#	MUST BE shared to prevent database corruptions.
#	MUST BE writable.
#
libdir_files=
datadir_files="\$(RPM_BUILD_ROOT)/etc/profile.d/aegis.sh \
	\$(RPM_BUILD_ROOT)/etc/profile.d/aegis.csh"
comdir_files=
man_files=

po_files=
install_po_files=
doc_files=
install_doc_file=
test_files=
commands=
commands_bin=
commands_install=
scripts=

#
# The mkdir could all be done with install,
# but it has two incompatible forms:
#	$(INSTALL_PROGRAM) -d -m 0755 -o $(AEGIS_UID) -g $(AEGIS_GID) $(libdir)
#	$(INSTALL_PROGRAM) -d -m 0755 -u $(AEGIS_UID) -g $(AEGIS_GID) $(libdir)
# The form which takes -u interprets -o as something else.
# This is not determined automagically by autoconf.
#
# Note: if $(libdir) has the wrong owner or the wrong permissions,
#	aegis will exit with a fatal error.
#
recursive_mkdir()
{
	src_dir="$1"
	dst_dir="$2"
	dir_suffix=$3
	while :
	do
		dirvar=`echo mkdir.$src_dir.$dir_suffix |
			sed 's|[^a-zA-Z0-9]|_|g'`
		dotdot=`dirname $src_dir`
		if eval "test \${${dirvar}-no} != yes" ; then
			echo ""
			if test "$dotdot" != "."; then
				echo "$src_dir/.mkdir.${dir_suffix}: \
$dotdot/.mkdir.${dir_suffix}"
			else
				echo "$src_dir/.mkdir.${dir_suffix}:"
			fi
			echo "	-\$(INSTALL) -m 0755 -d $dst_dir"
			echo "	-chown \$(AEGIS_UID) $dst_dir && chgrp \
\$(AEGIS_GID) $dst_dir"
			echo "	-@touch \$@"
			echo "	@sleep 1"
			eval "${dirvar}=yes"
			clean_files="$clean_files $src_dir/.mkdir.${dir_suffix}"
		fi
		src_dir=$dotdot
		dst_dir=`dirname $dst_dir`
		if test $src_dir = . ; then break; fi
	done
}

for file in $*
do
	case $file in
	script/*.in)
		;;
	*.in)
		file=`echo $file | sed 's/[.]in$//'`
		;;
	*)
		;;
	esac

	case $file in
	script/*.in)
		name=`echo $file | sed -e 's|.*/||' -e 's|\.in$||'`
		commands_bin="$commands_bin bin/$name\$(EXEEXT)"
		scripts="$scripts $name"
		commands_install="$commands_install \$(RPM_BUILD_ROOT)\
\$(bindir)/\$(PROGRAM_PREFIX)$name\$(PROGRAM_SUFFIX)\$(EXEEXT)"
		;;
	*/main.c)
		name=`echo $file | sed 's|/.*||'`
		commands="$commands $name"
		commands_bin="$commands_bin bin/$name\$(EXEEXT)"

		case $name in
		aefp | fmtgen | find_sizes | cklinlen )
			;;
		test_*)
			;;
		*)
			commands_install="$commands_install \$(RPM_BUILD_ROOT)\
\$(bindir)/\$(PROGRAM_PREFIX)$name\$(PROGRAM_SUFFIX)\$(EXEEXT)"
			;;
		esac
		;;
	*)
		;;
	esac

	case $file in

	*/*.c)
		dir=`echo $file | sed 's|/.*||'`
		stem=`echo $file | sed 's/\.c$//'`
		eval "${dir}_files=\"\$${dir}_files ${stem}.\\\$(OBJEXT)\""
		clean_files="$clean_files ${stem}.\$(OBJEXT)"
		;;

	*/*.def)
		dir=`echo $file | sed 's|/.*||'`
		stem=`echo $file | sed 's/\.def$//'`
		eval "${dir}_files=\"\$${dir}_files ${stem}.\\\$(OBJEXT)\""
		clean_files="$clean_files ${stem}.\$(OBJEXT) \
${stem}.c ${stem}.h"
		;;

	*/*.y)
		dir=`echo $file | sed 's|/.*||'`
		stem=`echo $file | sed 's/\.y$//'`
		clean_files="$clean_files ${stem}.gen.c ${stem}.gen.h"
		eval "${dir}_files=\"\$${dir}_files ${stem}.gen.\\\$(OBJEXT)\""
		clean_files="$clean_files ${stem}.gen.\$(OBJEXT)"
		;;

	lib/*/libaegis.po)
		# obsolete
		;;

	lib/*.po)
		stem=`echo $file | sed 's|^lib/\(.*\)\.po$|\1|'`
		src="lib/$stem.mo"
		po_files="$po_files $src"
		dst="\$(RPM_BUILD_ROOT)\$(NLSDIR)/$stem.mo"
		install_po_files="$install_po_files $dst"
		recursive_mkdir `dirname $src` `dirname $dst` libdir
		;;

	lib/*/*/*.so)
		;;

	lib/*/*/*.bib)
		;;

	lib/*.uue)
		;;

	lib/*/man[1-9]/*.[1-9])
		stem=`echo $file | sed 's|^lib/\(.*\)|\1|'`
		install_doc_files="$install_doc_files \
\$(RPM_BUILD_ROOT)\$(datadir)/$stem"
		src=$file
		dst="\$(RPM_BUILD_ROOT)\$(datadir)/$stem"
		recursive_mkdir `dirname $src` `dirname $dst` datadir
		part=`echo $file | sed 's|^lib/en/\(man./.*\)|\1|'`
		man_files="$man_files \$(RPM_BUILD_ROOT)\$(mandir)/$part"
		;;

	lib/*/*/main.*)
		stem=`echo $file | sed 's|^lib/\(.*\)/main.*$|\1|'`
		doc_files="$doc_files lib/$stem.ps lib/$stem.dvi lib/$stem.txt"
		clean_files="$clean_files lib/$stem.ps lib/$stem.dvi \
lib/$stem.txt"
		install_doc_files="$install_doc_files \
\$(RPM_BUILD_ROOT)\$(datadir)/$stem.ps \
\$(RPM_BUILD_ROOT)\$(datadir)/$stem.dvi \$(RPM_BUILD_ROOT)\$(datadir)/$stem.txt"
		src=lib/$stem.ps
		dst="\$(RPM_BUILD_ROOT)\$(datadir)/$stem.ps"
		recursive_mkdir `dirname $src` `dirname $dst` datadir
		;;

	lib/*)
		rest=`echo $file | sed 's|^lib/||'`
		dst="\$(RPM_BUILD_ROOT)\$(datadir)/$rest"
		datadir_files="$datadir_files $dst"
		recursive_mkdir `dirname lib/$rest` `dirname $dst` datadir
		;;

	test/*/*)
		stem=`echo $file | sed 's/\.sh$//'`
		test_files="$test_files $stem.ES"
		clean_files="$clean_files $stem.ES"
		;;

	*)
		;;
	esac
done

echo ''
echo "all-bin:" $commands_bin

for name in $commands
do
	echo ''
	eval "echo \"${name}_files =\" \${${name}_files}"
	echo ''

	case $name in

	find_sizes | fmtgen )
		echo "bin/$name\$(EXEEXT): \$(${name}_files) .bin"
		echo '	@sleep 1'
		echo "	\$(CC) \$(LDFLAGS) -o \$@ \$(${name}_files) \$(LIBS)"
		echo '	@sleep 1'
		;;

	aefp )
		echo "bin/$name\$(EXEEXT): \$(${name}_files) \
common/common.\$(LIBEXT) .bin"
		echo '	@sleep 1'
		echo "	\$(CC) \$(LDFLAGS) -o \$@ \$(${name}_files) \
common/common.\$(LIBEXT) \$(LIBS)"
		echo '	@sleep 1'
		;;

	*)
		echo "bin/$name\$(EXEEXT): \$(${name}_files) \
libaegis/libaegis.\$(LIBEXT) .bin"
		echo '	@sleep 1'
		echo "	\$(CC) \$(LDFLAGS) -o \$@ \$(${name}_files) \
libaegis/libaegis.\$(LIBEXT) \$(LIBS)"
		case $name in
		aegis | aeimport)
			echo '	-chown root $@ && chmod 4755 $@'
			;;
		esac
		echo '	@sleep 1'
		;;
	esac

	echo ''
	echo "\$(RPM_BUILD_ROOT)\$(bindir)/\$(PROGRAM_PREFIX)$name\
\$(PROGRAM_SUFFIX)\$(EXEEXT): bin/$name\$(EXEEXT) .bindir"
	echo "	\$(INSTALL_PROGRAM) bin/$name\$(EXEEXT) \$@"
	case $name in
	aegis | aeimport)
		echo '	-chown root $@ && chmod 4755 $@'
		;;
	esac
done

for name in $scripts
do
	echo ''
	echo "bin/$name\$(EXEEXT): script/${name}.in .bin"
	echo '	@sleep 1'
	echo "	CONFIG_FILES=\$@:script/${name}.in CONFIG_HEADERS= \$(SH) \
./config.status"
	echo '	chmod a+rx $@'
	echo '	@sleep 1'

	echo ''
	echo "\$(RPM_BUILD_ROOT)\$(bindir)/\$(PROGRAM_PREFIX)$name\
\$(PROGRAM_SUFFIX)\$(EXEEXT): bin/$name\$(EXEEXT) .bindir"
	echo "	\$(INSTALL_SCRIPT) bin/$name\$(EXEEXT) \$@"
done

echo ''
echo "CommonFiles =" $common_files
echo ''
echo "LibAegisFiles =" $libaegis_files $common_files
echo ''
echo "LibFiles =" $libdir_files
echo ''
echo "DataFiles =" $datadir_files
echo ''
echo "install-man-yes:" $man_files
echo ''
echo "install-man-no:"
echo ''
echo "po_files_yes:" $po_files
echo ''
echo "po_files_no:"
echo ''
echo "install-po-yes:" $install_po_files
echo ''
echo "install-po-no:"
echo ''
echo "doc_files_yes:" $doc_files
echo ''
echo "doc_files_no:"
echo ''
echo "install-doc-yes:" $install_doc_files
echo ''
echo "install-doc-no:"
echo ''
echo "TestFiles =" $test_files

#
# clean up the area
#	(make sure command lines do not get too long)
#
echo ''
echo 'clean-obj:'
echo $clean_files | tr ' ' '\12' | gawk '{
	if (pos > 0 && pos + length($1) > 71) { printf("\n"); pos = 0; }
	if (pos == 0) { printf "	rm -f"; pos = 13; }
	printf " %s", $1
	pos += 1 + length($1);
}
END { if (pos) printf "\n"; }'

echo ''
echo 'clean: clean-obj'
echo "	rm -f $commands_bin bin/xaegis"

echo ''
echo "install-bin:" $commands_install

cat << 'fubar'

.bindir:
	-$(INSTALL) -m 0755 -d $(RPM_BUILD_ROOT)$(bindir)
	-@touch $@
	@sleep 1

.man1dir:
	-$(INSTALL) -m 0755 -d $(RPM_BUILD_ROOT)$(mandir)/man1
	-@touch $@
	@sleep 1

.man3dir:
	-$(INSTALL) -m 0755 -d $(RPM_BUILD_ROOT)$(mandir)/man3
	-@touch $@
	@sleep 1

.man5dir:
	-$(INSTALL) -m 0755 -d $(RPM_BUILD_ROOT)$(mandir)/man5
	-@touch $@
	@sleep 1

.comdir:
	-$(INSTALL) -m 0755 -d $(RPM_BUILD_ROOT)$(comdir)
	-chown $(AEGIS_UID) $(RPM_BUILD_ROOT)$(comdir) && \
		chgrp $(AEGIS_GID) $(RPM_BUILD_ROOT)$(comdir)
	$(SH) etc/compat.2.3
	-@touch $@
	@sleep 1

.po_files: po_files_$(HAVE_MSGFMT)
	@touch $@

.doc_files: doc_files_$(HAVE_GROFF)
	@touch $@

distclean: clean
	rm -f config.status config.log config.cache
	rm -f Makefile common/config.h etc/Howto.conf
	rm -f lib/cshrc lib/profile etc/libdir.so

.bin:
	-mkdir bin
	-@touch $@

common/common.$(LIBEXT): $(CommonFiles)
	rm -f $@
	$(AR) qc $@ $(CommonFiles)
	$(RANLIB) $@

libaegis/libaegis.$(LIBEXT): $(LibAegisFiles)
	rm -f $@
	$(AR) qc $@ $(LibAegisFiles)
	$(RANLIB) $@

sure: $(TestFiles) etc/test.sh
	@$(SH) etc/test.sh -summary $(TestFiles)

#
# This looks at the various integral type sizes
# and constructs a suitable include file
#
common/find_sizes.h: bin/find_sizes$(EXEEXT)
	bin/find_sizes$(EXEEXT) > common/find_sizes.h
	@sleep 1

#
# This target is used when preparing for the second
# pass of testing, when aegis is set-uid-root.
#
install-libdir: lib/.mkdir.datadir lib/.mkdir.libdir .comdir install-po
	-chown root bin/aegis$(EXEEXT) && chmod 4755 bin/aegis$(EXEEXT)
	-chown root bin/aeimport$(EXEEXT) && chmod 4755 bin/aeimport$(EXEEXT)

install-lib: $(LibFiles) $(DataFiles) .comdir

install-po: install-po-$(HAVE_MSGFMT)

install-man: install-man-$(HAVE_GROFF)

install-doc: install-doc-$(HAVE_GROFF)

install: install-bin install-lib install-po \
	install-man install-doc
fubar
exit 0
