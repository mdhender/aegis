#!/bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991, 1992, 1993, 1994, 1995 Peter Miller;
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
#	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# MANIFEST: shell script to generate the Mafile file
#
lib_files=
man1_files=
man5_files=
fmtgen_files=
txt2c_files=
aegis_files=
test_files=
common_files=
clean_files="core bin/fmtgen bin/txt2c common/lib.a .bin .libdir .libdir.report"
for file in $*
do
	case $file in

	fmtgen/*.y)
		stem=`echo $file | sed 's/\.y$//'`
		clean_files="$clean_files ${stem}.gen.c ${stem}.gen.h"
		fmtgen_files="$fmtgen_files ${stem}.gen.o"
		clean_files="$clean_files ${stem}.gen.o"
		;;

	fmtgen/*.c)
		stem=`echo $file | sed 's/\.c$//'`
		fmtgen_files="$fmtgen_files ${stem}.o"
		clean_files="$clean_files ${stem}.o"
		;;

	txt2c/*.c)
		root=`basename $file .c`
		txt2c_files="$txt2c_files txt2c/${root}.o"
		clean_files="$clean_files txt2c/${root}.o"
		;;

	aegis/*.def)
		root=`basename $file .def`
		clean_files="$clean_files aegis/${root}.c aegis/${root}.h"
		aegis_files="$aegis_files aegis/${root}.o"
		clean_files="$clean_files aegis/${root}.o"
		;;

	aegis/*.y)
		stem=`echo $file | sed 's/\.y$//'`
		clean_files="$clean_files ${stem}.gen.c ${stem}.gen.h"
		aegis_files="$aegis_files ${stem}.gen.o"
		clean_files="$clean_files ${stem}.gen.o"
		;;

	aegis/*.c)
		stem=`echo $file | sed 's/\.c$//'`
		aegis_files="$aegis_files ${stem}.o"
		clean_files="$clean_files ${stem}.o"
		;;

	common/*.c)
		root=`basename $file .c`
		common_files="$common_files common/${root}.o"
		clean_files="$clean_files common/${root}.o"
		;;

	man1/*.1)
		root=`basename $file .1`
		clean_files="$clean_files man1/${root}.h"
		man1_files="$man1_files \$(mandir)/$file"
		;;

	man1/*.so)
		root=`basename $file .so`
		clean_files="$clean_files man1/${root}.h"
		;;

	man5/*.5)
		man5_files="$man5_files \$(mandir)/$file"
		;;

	test/*/*)
		root=`basename $file .sh`
		test_files="$test_files $root"
		;;

	lib/*)
		rest=`echo $file | sed 's/^....//'`
		lib_files="$lib_files \$(libdir)/$rest"
		;;

	*)
		;;
	esac
done

echo
echo "AegisFiles =" $aegis_files
echo
echo "CommonFiles =" $common_files
echo
echo "FmtgenFiles =" $fmtgen_files
echo
echo "LibFiles =" $lib_files
echo
echo "Man1Files =" $man1_files
echo
echo "Man5Files =" $man5_files
echo
echo "TestFiles =" $test_files
echo
echo "Txt2cFiles =" $txt2c_files

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

cat << 'fubar'

clean: clean-obj
	rm -f bin/aegis bin/xaegis

distclean: clean
	rm -f config.status Makefile common/config.h aux/Howto.conf

.bin:
	-mkdir bin
	@touch .bin

common/lib.a: $(CommonFiles)
	rm -f common/lib.a
	$(AR) qc common/lib.a $(CommonFiles)
	$(RANLIB) common/lib.a

bin/fmtgen: $(FmtgenFiles) common/lib.a .bin
	@sleep 1
	$(CC) -o bin/fmtgen $(FmtgenFiles) common/lib.a $(LIBS)
	@sleep 1

bin/txt2c: $(Txt2cFiles) common/lib.a .bin
	@sleep 1
	$(CC) -o bin/txt2c $(Txt2cFiles) common/lib.a $(LIBS)
	@sleep 1

bin/aegis: $(AegisFiles) common/lib.a .bin
	@sleep 1
	$(CC) -o bin/aegis $(AegisFiles) common/lib.a $(LIBS)
	@sleep 1

sure: $(TestFiles)
	@echo Passed All Tests

#
# This command could all be done with install,
# but it has two incompatible forms:
#	$(INSTALL_PROGRAM) -m 4755 -o root bin/aegis $(bindir)/aegis
#	$(INSTALL_PROGRAM) -m 4755 -u root bin/aegis $(bindir)/aegis
# The form which takes -u interprets -o as something else.
# This is not determined automagically by autoconf.
#
$(bindir)/aegis: bin/aegis
	$(INSTALL_PROGRAM) bin/aegis $(bindir)/aegis
	chown root $(bindir)/aegis
	chmod 4755 $(bindir)/aegis

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
.libdir:
	-mkdir $(libdir)
	chown $(AEGIS_UID) $(libdir)
	chgrp $(AEGIS_GID) $(libdir)
	chmod 0755 $(libdir)
	@touch .libdir
	@sleep 1

.libdir.report: .libdir
	-mkdir $(libdir)/report
	chown $(AEGIS_UID) $(libdir)/report
	chgrp $(AEGIS_GID) $(libdir)/report
	chmod 0755 $(libdir)/report
	@touch .libdir.report
	@sleep 1

#
# This target is used when preparing for the second
# pass of testing, when aegis is set-uid-root.
#
install-libdir: .libdir
	chown root bin/aegis
	chmod 4755 bin/aegis

install-bin: $(bindir)/aegis

install-man: $(Man1Files) $(Man5Files)

install-lib: $(LibFiles)

install: install-bin install-man install-lib
fubar
exit 0
