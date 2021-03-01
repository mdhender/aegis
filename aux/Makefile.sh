#! /bin/sh
#
#	aegis - project change supervisor
#	Copyright (C) 1991, 1992, 1993 Peter Miller.
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
fmtgen_files=
aegis_files=
test_files=
clean_files="core bin/fmtgen"
echo  "#"
echo  "# You may need to change this for your system."
echo  "# The \`\`h'' directory supplements your system, not replacing it."
echo  "# The first variation is for gcc when it isn't the native complier,"
echo  "# the second variation is for systems with missing ANSI C include files,"
echo  "# the third variation is for conforming ANSI C implementations."
echo  "#"
echo  "# H = -I/usr/local/lib/gcc-include -I/usr/include -Ih"
echo  "# H = -I/usr/include -Ih"
echo  "H ="
echo  "# H =				# SunOS"
echo  "# H =				# ConvexOS"
echo  "# H =				# dgux"
echo  "# H =				# dcosx (pyramid)"
echo  "# H =				# ULTRIX"
echo  "# H =				# hpux"
echo  "# H =				# SCO"
echo  "# H =				# IRIX"
echo
echo  "#"
echo  "# the name of the compiler to use"
echo  "#"
echo  "CC = cc"
echo  "# CC = gcc"
echo  "# CC = cc			# SunOS"
echo  "# CC = cc			# ConvexOS"
echo  "# CC = cc			# dgux"
echo  "# CC = /usr/ucb/cc		# dcosx (pyramid)"
echo  "# CC = cc			# ULTRIX"
echo  "# CC = cc			# hpux"
echo  "# CC = rcc			# SCO"
echo  "# CC = cc			# IRIX"
echo
echo  "#"
echo  "# The compiler flags to use, except for include path."
echo  "#"
echo  "CFLAGS = -O"
echo  "# CFLAGS = -g"
echo  "# CFLAGS = -O			# SunOS"
echo  "# CFLAGS = -O			# ConvexOS"
echo  "# CFLAGS = -O			# dgux"
echo  "# CFLAGS = -O -Wall -ansi	# gcc"
echo  "# CFLAGS = -O -Xt -U__STDC__	# dcosx (pyramid /usr/ucb/cc is brain-dead)"
echo  "# CFLAGS = -O			# ULTRIX"
echo  "# CFLAGS = -O			# hpux"
echo  "# CFLAGS = -O			# SCO"
echo  "# CFLAGS = -O			# IRIX"
echo
echo  "#"
echo  "# which yacc to use"
echo  "#"
echo  "YACC = yacc"
echo  "# YACC = byacc			# Berkeley"
echo  "# YACC = bison -y		# GNU"
echo  ""
echo  "#"
echo  "# where to put the library directory"
echo  "#	(not used in testing mode)"
echo  "#"
echo  "LIB = /usr/local/lib/aegis"
echo  ""
echo  "#"
echo  "# where to put the executables"
echo  "#"
echo  "BIN = /usr/local/bin"
echo  ""
echo  "#"
echo  "# where to put the manual entries"
echo  "#"
echo  "MAN = /usr/local/man"
echo  ""
echo  "#"
echo  "# extra libraries required for your system"
echo  "#"
echo  "LIBRARIES ="
echo  "# LIBRARIES = -lbsd"
echo  "# LIBRARIES =			# SunOS"
echo  "# LIBRARIES =			# ConvexOS"
echo  "# LIBRARIES =			# dgux"
echo  "# LIBRARIES = -lucb		# dcosx (pyramid)"
echo  "# LIBRARIES =			# ULTRIX"
echo  "# LIBRARIES =			# hpux"
echo  "# LIBRARIES = -lsocket		# SCO"
echo  "# LIBRARIES =			# IRIX"
echo  ""
echo  "#"
echo  "# shell to use to run the tests"
echo  "#	make sure there are no spaces after the definition,"
echo  "#	many flavours of make(1) can't cope with them."
echo  "#"
echo  "SHELL = /bin/sh"
echo  "# SHELL = /bin/sh		# SunOS"
echo  "# SHELL = /bin/sh		# ConvexOS"
echo  "# SHELL = /bin/sh		# dgux"
echo  "# SHELL = /bin/sh		# dcosx (pyramid)"
echo  "# SHELL = /bin/sh5		# ULTRIX"
echo  "# SHELL = /bin/ksh		# apollo"
echo  "# SHELL = /bin/sh		# hpux"
echo  "# SHELL = /bin/sh		# SCO"
echo  "# SHELL = /bin/sh		# IRIX"
echo  ""
echo  "# You should not need to alter anything below this point."
echo  "#------------------------------------------------------------"
echo  ""
echo  "all: bin/aegis"
echo
rm -f common/conf.h
cp /dev/null common/conf.h
for file in $*
do
	case $file in

	fmtgen/*.y)
		root=`basename $file .y`
		dep=`c_incl -Ifmtgen -Icommon -ns -nc $file`
		echo
		echo "fmtgen/${root}.gen.c fmtgen/${root}.gen.h: $file"
		echo "	$(YACC) -d $file"
		echo "	sed \"s/[yY][yY]/${root}_/g\" < y.tab.c > fmtgen/${root}.gen.c"
		echo "	rm y.tab.c"
		echo "	sed \"s/[yY][yY]/${root}_/g\" < y.tab.h > fmtgen/${root}.gen.h"
		echo "	rm y.tab.h"
		clean_files="$clean_files fmtgen/${root}.gen.c fmtgen/${root}.gen.h"
		echo
		echo "fmtgen/${root}.gen.o: fmtgen/${root}.gen.c" $dep
		echo "	$(CC) $(CFLAGS) -Ifmtgen -Icommon $(H) -c fmtgen/${root}.gen.c"
		echo "	mv ${root}.gen.o fmtgen/${root}.gen.o"
		fmtgen_files="$fmtgen_files fmtgen/${root}.gen.o"
		clean_files="$clean_files fmtgen/${root}.gen.o"
		;;

	fmtgen/*.c)
		root=`basename $file .c`
		dep=`c_incl -Ifmtgen -Icommon -ns -nc $file`
		echo
		echo "fmtgen/${root}.o: $file" $dep
		echo "	$(CC) $(CFLAGS) -Ifmtgen -Icommon $(H) -c fmtgen/${root}.c"
		echo "	mv ${root}.o fmtgen/${root}.o"
		fmtgen_files="$fmtgen_files fmtgen/${root}.o"
		clean_files="$clean_files fmtgen/${root}.o"
		;;

	aegis/*.def)
		root=`basename $file .def`
		dep=`c_incl -Iaegis -ns -nc $file`
		echo
		echo "aegis/${root}.c aegis/${root}.h: $file bin/fmtgen" $dep
		echo "	bin/fmtgen -Iaegis $file aegis/${root}.c aegis/${root}.h"
		clean_files="$clean_files aegis/${root}.c aegis/${root}.h"
		echo
		dep=`c_incl -Iaegis -Icommon -ns -nc aegis/${root}.c`
		echo "aegis/${root}.o: aegis/${root}.c" $dep
		echo "	$(CC) $(CFLAGS) -Iaegis -Icommon $(H) -c aegis/${root}.c"
		echo "	mv ${root}.o aegis/${root}.o"
		aegis_files="$aegis_files aegis/${root}.o"
		clean_files="$clean_files aegis/${root}.o"
		;;

	aegis/*.y)
		root=`basename $file .y`
		dep=`c_incl -Iaegis -Icommon -ns -nc $file`
		echo
		echo "aegis/${root}.gen.c aegis/${root}.gen.h: $file"
		echo "	$(YACC) -d $file"
		echo "	sed \"s/[yY][yY]/${root}_/g\" < y.tab.c > aegis/${root}.gen.c"
		echo "	rm y.tab.c"
		echo "	sed \"s/[yY][yY]/${root}_/g\" < y.tab.h > aegis/${root}.gen.h"
		echo "	rm y.tab.h"
		clean_files="$clean_files aegis/${root}.gen.c aegis/${root}.gen.h"
		echo
		echo "aegis/${root}.gen.o: aegis/${root}.gen.c" $dep
		echo "	$(CC) $(CFLAGS) -Iaegis -Icommon $(H) -c aegis/${root}.gen.c"
		echo "	mv ${root}.gen.o aegis/${root}.gen.o"
		aegis_files="$aegis_files aegis/${root}.gen.o"
		clean_files="$clean_files aegis/${root}.gen.o"
		;;

	aegis/*.c)
		root=`basename $file .c`
		dep=`c_incl -Iaegis -Icommon -ns -nc $file`
		echo
		echo "aegis/${root}.o: $file" $dep
		oflg=
		if [ $root = gonzo ]
		then
			oflg="-D'LIB=\"$(LIB)\"'"
		fi
		echo "	$(CC) $(CFLAGS) $oflg -Iaegis -Icommon $(H) -c $file"
		echo "	mv ${root}.o aegis/${root}.o"
		aegis_files="$aegis_files aegis/${root}.o"
		clean_files="$clean_files aegis/${root}.o"
		;;

	common/*.c)
		root=`basename $file .c`
		dep=`c_incl -Icommon -ns -nc $file`
		echo
		echo "common/${root}.o: $file" $dep
		echo "	$(CC) $(CFLAGS) -Icommon $(H) -c $file"
		echo "	mv ${root}.o common/${root}.o"
		aegis_files="$aegis_files common/${root}.o"
		fmtgen_files="$fmtgen_files common/${root}.o"
		clean_files="$clean_files common/${root}.o"
		;;

	test/*/*)
		root=`basename $file .sh`
		echo ""
		echo "${root}: all $file"
		echo "	$(SHELL) $file"
		test_files="$test_files $root"
		;;

	*)
		;;
	esac
done

#
# clean up the area
#
echo ''
echo 'clean:'
echo '	rm -f' $clean_files
echo ''
echo 'realclean: clean'
echo '	rm -f bin/aegis'
echo ''
echo 'clobber: realclean'
echo '	rm -f common/conf.h'

#
# default the conf.h file
# if they have forgotten to set it
#
echo ''
echo 'common/conf.h:'
echo '	echo "#include <../conf/`uname -s`-`uname -r`>" > common/conf.h'

echo
echo "FmtgenFiles = $fmtgen_files"
echo
echo "bin/fmtgen: $(FmtgenFiles)"
echo "	if test ! -d bin; then mkdir bin; fi; exit 0"
echo "	$(CC) -o bin/fmtgen $(FmtgenFiles) $(LIBRARIES)"

echo
echo "AegisFiles = $aegis_files"
echo
echo "bin/aegis: $(AegisFiles)"
echo "	if test ! -d bin; then mkdir bin; fi; exit 0"
echo "	$(CC) -o bin/aegis $(AegisFiles) $(LIBRARIES)"

echo
echo "sure:" $test_files
echo "	@echo Passed All Tests"

echo
echo "install: all"
echo "	cp bin/aegis $(BIN)"
echo "	chown root $(BIN)/aegis"
echo "	chmod a+x,u+s $(BIN)/aegis"
echo "	-mkdir $(LIB)"
echo "	chmod 0755 $(LIB)"
echo "	cp lib/* $(LIB)"
echo "	chmod a+r $(LIB)/*"
echo "	chmod a+x $(LIB)/*.sh"
echo "	chown bin $(LIB)"
echo "	chgrp bin $(LIB)"
echo "	$(SHELL) man1/install.sh $(MAN)/man1"
echo "	$(SHELL) man5/install.sh $(MAN)/man5"

rm common/conf.h
exit 0
