#
# You may need to change this for your system.
# The ``h'' directory supplements your system, not replacing it.
# The first variation is for gcc when it isn't the native complier,
# the second variation is for systems with missing ANSI C include files,
# the third variation is for conforming ANSI C implementations.
#
# H = -I/usr/local/lib/gcc-include -I/usr/include -Ih
# H = -I/usr/include -Ih
H =
# H =				# SunOS
# H =				# ConvexOS
# H =				# dgux
# H =				# dcosx (pyramid)
# H =				# ULTRIX
# H =				# hpux
# H =				# SCO
# H =				# IRIX

#
# the name of the compiler to use
#
CC = cc
# CC = gcc
# CC = cc			# SunOS
# CC = cc			# ConvexOS
# CC = cc			# dgux
# CC = /usr/ucb/cc		# dcosx (pyramid)
# CC = cc			# ULTRIX
# CC = cc			# hpux
# CC = rcc			# SCO
# CC = cc			# IRIX

#
# The compiler flags to use, except for include path.
#
CFLAGS = -O
# CFLAGS = -g
# CFLAGS = -O			# SunOS
# CFLAGS = -O			# ConvexOS
# CFLAGS = -O			# dgux
# CFLAGS = -O -Wall -ansi	# gcc
# CFLAGS = -O -Xt -U__STDC__	# dcosx (pyramid /usr/ucb/cc is brain-dead)
# CFLAGS = -O			# ULTRIX
# CFLAGS = -O			# hpux
# CFLAGS = -O			# SCO
# CFLAGS = -O			# IRIX

#
# which yacc to use
#
YACC = yacc
# YACC = byacc			# Berkeley
# YACC = bison -y		# GNU

#
# where to put the library directory
#	(not used in testing mode)
#
LIB = /usr/local/lib/aegis

#
# where to put the executables
#
BIN = /usr/local/bin

#
# where to put the manual entries
#
MAN = /usr/local/man

#
# extra libraries required for your system
#
LIBRARIES =
# LIBRARIES = -lbsd
# LIBRARIES =			# SunOS
# LIBRARIES =			# ConvexOS
# LIBRARIES =			# dgux
# LIBRARIES = -lucb		# dcosx (pyramid)
# LIBRARIES =			# ULTRIX
# LIBRARIES =			# hpux
# LIBRARIES = -lsocket		# SCO
# LIBRARIES =			# IRIX

#
# shell to use to run the tests
#	make sure there are no spaces after the definition,
#	many flavours of make(1) can't cope with them.
#
SHELL = /bin/sh
# SHELL = /bin/sh		# SunOS
# SHELL = /bin/sh		# ConvexOS
# SHELL = /bin/sh		# dgux
# SHELL = /bin/sh		# dcosx (pyramid)
# SHELL = /bin/sh5		# ULTRIX
# SHELL = /bin/ksh		# apollo
# SHELL = /bin/sh		# hpux
# SHELL = /bin/sh		# SCO
# SHELL = /bin/sh		# IRIX

# You should not need to alter anything below this point.
#------------------------------------------------------------

all: bin/aegis


aegis/aeb.o: aegis/aeb.c aegis/aeb.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/col.h \
		aegis/commit.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/change.h aegis/cstate.h \
		aegis/type.h aegis/parse.h aegis/pconf.h common/error.h \
		aegis/help.h aegis/lock.h aegis/log.h common/option.h \
		aegis/os.h aegis/project.h aegis/pstate.h aegis/pattr.h \
		aegis/sub.h common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aeb.c
	mv aeb.o aegis/aeb.o

aegis/aeca.o: aegis/aeca.c aegis/aeca.h aegis/cattr.h common/main.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h aegis/arglex2.h \
		common/arglex.h aegis/commit.h aegis/change.h \
		aegis/cstate.h aegis/pconf.h common/error.h \
		aegis/help.h aegis/lock.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/option.h aegis/os.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aeca.c
	mv aeca.o aegis/aeca.o

aegis/aecd.o: aegis/aecd.c aegis/aecd.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h common/error.h aegis/help.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aecd.c
	mv aecd.o aegis/aecd.o

aegis/aecp.o: aegis/aecp.c aegis/aecp.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		aegis/change.h aegis/cstate.h aegis/type.h \
		aegis/parse.h aegis/pconf.h common/error.h aegis/file.h \
		aegis/help.h aegis/lock.h aegis/log.h common/option.h \
		aegis/os.h aegis/project.h aegis/pstate.h aegis/pattr.h \
		common/trace.h aegis/undo.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aecp.c
	mv aecp.o aegis/aecp.o

aegis/aecpu.o: aegis/aecpu.c aegis/aecpu.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		aegis/change.h aegis/cstate.h aegis/type.h \
		aegis/parse.h aegis/pconf.h common/error.h aegis/file.h \
		aegis/help.h aegis/lock.h aegis/log.h common/option.h \
		aegis/os.h aegis/project.h aegis/pstate.h aegis/pattr.h \
		common/trace.h aegis/undo.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aecpu.c
	mv aecpu.o aegis/aecpu.o

aegis/aed.o: aegis/aed.c aegis/aed.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/col.h aegis/commit.h common/error.h \
		aegis/help.h aegis/lock.h aegis/log.h common/option.h \
		aegis/os.h aegis/project.h aegis/pstate.h aegis/pattr.h \
		aegis/sub.h common/trace.h aegis/undo.h aegis/user.h \
		aegis/ustate.h aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aed.c
	mv aed.o aegis/aed.o

aegis/aedb.o: aegis/aedb.c aegis/aedb.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/col.h aegis/commit.h aegis/common.h \
		aegis/dir.h common/error.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aedb.c
	mv aedb.o aegis/aedb.o

aegis/aedbu.o: aegis/aedbu.c aegis/aedbu.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/col.h aegis/commit.h aegis/common.h \
		aegis/dir.h common/error.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aedbu.c
	mv aedbu.o aegis/aedbu.o

aegis/aede.o: aegis/aede.c aegis/aede.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/col.h aegis/commit.h aegis/common.h \
		aegis/dir.h common/error.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aede.c
	mv aede.o aegis/aede.o

aegis/aedeu.o: aegis/aedeu.c aegis/aedeu.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/col.h aegis/commit.h aegis/common.h \
		aegis/dir.h common/error.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aedeu.c
	mv aedeu.o aegis/aedeu.o

aegis/aeib.o: aegis/aeib.c aegis/aeib.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		aegis/change.h aegis/cstate.h aegis/type.h \
		aegis/parse.h aegis/pconf.h aegis/dir.h common/error.h \
		aegis/file.h aegis/help.h aegis/lock.h aegis/log.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aeib.c
	mv aeib.o aegis/aeib.o

aegis/aeibu.o: aegis/aeibu.c aegis/aeibu.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		aegis/change.h aegis/cstate.h aegis/type.h \
		aegis/parse.h aegis/pconf.h aegis/dir.h common/error.h \
		aegis/file.h aegis/help.h aegis/lock.h aegis/log.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aeibu.c
	mv aeibu.o aegis/aeibu.o

aegis/aeif.o: aegis/aeif.c aegis/aeif.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		aegis/change.h aegis/cstate.h aegis/type.h \
		aegis/parse.h aegis/pconf.h aegis/dir.h common/error.h \
		aegis/file.h aegis/help.h aegis/lock.h aegis/log.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aeif.c
	mv aeif.o aegis/aeif.o

aegis/aeip.o: aegis/aeip.c aegis/aeip.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		aegis/change.h aegis/cstate.h aegis/type.h \
		aegis/parse.h aegis/pconf.h aegis/dir.h common/error.h \
		aegis/file.h aegis/help.h aegis/lock.h aegis/log.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aeip.c
	mv aeip.o aegis/aeip.o

aegis/ael.o: aegis/ael.c aegis/ael.h common/main.h aegis/arglex2.h \
		common/arglex.h aegis/col.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h common/error.h aegis/gonzo.h \
		aegis/gstate.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/user.h aegis/ustate.h aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/ael.c
	mv ael.o aegis/ael.o

aegis/aemv.o: aegis/aemv.c aegis/ael.h common/main.h aegis/aemv.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/commit.h common/error.h \
		aegis/file.h aegis/help.h aegis/lock.h aegis/log.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/user.h aegis/ustate.h aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aemv.c
	mv aemv.o aegis/aemv.o

aegis/aena.o: aegis/aena.c aegis/aena.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		common/error.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/type.h aegis/parse.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aena.c
	mv aena.o aegis/aena.o

aegis/aenc.o: aegis/aenc.c aegis/aeca.h aegis/cattr.h common/main.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h aegis/aenc.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/pconf.h aegis/col.h aegis/commit.h \
		aegis/common.h common/error.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aenc.c
	mv aenc.o aegis/aenc.o

aegis/aencu.o: aegis/aencu.c aegis/aeca.h aegis/cattr.h common/main.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h aegis/ael.h aegis/aencu.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/pconf.h aegis/col.h aegis/commit.h \
		aegis/common.h common/error.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aencu.c
	mv aencu.o aegis/aencu.o

aegis/aend.o: aegis/aend.c aegis/ael.h common/main.h aegis/aend.h \
		aegis/arglex2.h common/arglex.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		common/error.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/type.h aegis/parse.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aend.c
	mv aend.o aegis/aend.o

aegis/aenf.o: aegis/aenf.c aegis/ael.h common/main.h aegis/aenf.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/col.h aegis/commit.h common/error.h \
		aegis/glue.h aegis/help.h aegis/lock.h aegis/log.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/user.h aegis/ustate.h aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aenf.c
	mv aenf.o aegis/aenf.o

aegis/aenfu.o: aegis/aenfu.c aegis/ael.h common/main.h aegis/aenfu.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/col.h aegis/commit.h common/error.h \
		aegis/glue.h aegis/help.h aegis/lock.h aegis/log.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/user.h aegis/ustate.h aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aenfu.c
	mv aenfu.o aegis/aenfu.o

aegis/aeni.o: aegis/aeni.c aegis/aeni.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		common/error.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/type.h aegis/parse.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aeni.c
	mv aeni.o aegis/aeni.o

aegis/aenpr.o: aegis/aenpr.c aegis/ael.h common/main.h aegis/aenpr.h \
		aegis/arglex2.h common/arglex.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		common/error.h aegis/gonzo.h aegis/gstate.h \
		aegis/type.h aegis/parse.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aenpr.c
	mv aenpr.o aegis/aenpr.o

aegis/aenrls.o: aegis/aenrls.c aegis/ael.h common/main.h aegis/aenrls.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/commit.h aegis/dir.h common/error.h \
		aegis/file.h aegis/gonzo.h aegis/gstate.h aegis/help.h \
		aegis/lock.h aegis/log.h common/mem.h common/option.h \
		aegis/os.h aegis/project.h aegis/pstate.h aegis/pattr.h \
		aegis/sub.h common/trace.h aegis/undo.h aegis/user.h \
		aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aenrls.c
	mv aenrls.o aegis/aenrls.o

aegis/aenrv.o: aegis/aenrv.c aegis/ael.h common/main.h aegis/aenrv.h \
		aegis/arglex2.h common/arglex.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		common/error.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/type.h aegis/parse.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aenrv.c
	mv aenrv.o aegis/aenrv.o

aegis/aent.o: aegis/aent.c aegis/ael.h common/main.h aegis/aent.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/col.h aegis/commit.h common/error.h \
		aegis/glue.h aegis/help.h aegis/lock.h common/option.h \
		aegis/os.h aegis/project.h aegis/pstate.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aent.c
	mv aent.o aegis/aent.o

aegis/aentu.o: aegis/aentu.c aegis/ael.h common/main.h aegis/aentu.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/col.h aegis/commit.h common/error.h \
		aegis/glue.h aegis/help.h aegis/lock.h common/option.h \
		aegis/os.h aegis/project.h aegis/pstate.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aentu.c
	mv aentu.o aegis/aentu.o

aegis/aepa.o: aegis/aepa.c aegis/aepa.h common/main.h aegis/arglex2.h \
		common/arglex.h aegis/commit.h common/str.h \
		common/s-v-arg.h common/conf.h common/error.h \
		aegis/help.h aegis/lock.h common/option.h aegis/os.h \
		aegis/pattr.h aegis/type.h aegis/parse.h \
		aegis/project.h aegis/pstate.h common/trace.h \
		aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aepa.c
	mv aepa.o aegis/aepa.o

aegis/aera.o: aegis/aera.c aegis/aera.h common/main.h aegis/ael.h \
		aegis/arglex2.h common/arglex.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		common/error.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/type.h aegis/parse.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aera.c
	mv aera.o aegis/aera.o

aegis/aerd.o: aegis/aerd.c aegis/ael.h common/main.h aegis/aerd.h \
		aegis/arglex2.h common/arglex.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		common/error.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/type.h aegis/parse.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aerd.c
	mv aerd.o aegis/aerd.o

aegis/aerf.o: aegis/aerf.c aegis/ael.h common/main.h aegis/aerf.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/commit.h aegis/dir.h common/error.h \
		aegis/file.h aegis/help.h aegis/lock.h common/mem.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aerf.c
	mv aerf.o aegis/aerf.o

aegis/aeri.o: aegis/aeri.c aegis/ael.h common/main.h aegis/aeri.h \
		aegis/arglex2.h common/arglex.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		common/error.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/type.h aegis/parse.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aeri.c
	mv aeri.o aegis/aeri.o

aegis/aerm.o: aegis/aerm.c aegis/ael.h common/main.h aegis/aerm.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/commit.h common/error.h \
		aegis/help.h aegis/lock.h aegis/log.h common/option.h \
		aegis/os.h aegis/project.h aegis/pstate.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aerm.c
	mv aerm.o aegis/aerm.o

aegis/aermpr.o: aegis/aermpr.c aegis/ael.h common/main.h aegis/aermpr.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/commit.h common/error.h \
		aegis/gonzo.h aegis/gstate.h aegis/help.h aegis/lock.h \
		common/option.h aegis/project.h aegis/pstate.h \
		aegis/pattr.h common/trace.h aegis/user.h \
		aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aermpr.c
	mv aermpr.o aegis/aermpr.o

aegis/aermu.o: aegis/aermu.c aegis/ael.h common/main.h aegis/aermu.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/commit.h common/error.h \
		aegis/help.h aegis/lock.h aegis/log.h common/option.h \
		aegis/os.h aegis/project.h aegis/pstate.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aermu.c
	mv aermu.o aegis/aermu.o

aegis/aerp.o: aegis/aerp.c aegis/ael.h common/main.h aegis/aerp.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/commit.h aegis/dir.h common/error.h \
		aegis/file.h aegis/help.h aegis/lock.h common/mem.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aerp.c
	mv aerp.o aegis/aerp.o

aegis/aerpu.o: aegis/aerpu.c aegis/ael.h common/main.h aegis/aerpu.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/commit.h aegis/dir.h common/error.h \
		aegis/file.h aegis/help.h aegis/lock.h common/mem.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aerpu.c
	mv aerpu.o aegis/aerpu.o

aegis/aerrv.o: aegis/aerrv.c aegis/ael.h common/main.h aegis/aerrv.h \
		aegis/arglex2.h common/arglex.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		common/error.h aegis/help.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/type.h aegis/parse.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aerrv.c
	mv aerrv.o aegis/aerrv.o

aegis/aet.o: aegis/aet.c aegis/ael.h common/main.h aegis/aet.h \
		aegis/arglex2.h common/arglex.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/commit.h common/error.h \
		aegis/help.h aegis/lock.h aegis/log.h common/option.h \
		aegis/os.h aegis/project.h aegis/pstate.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/aet.c
	mv aet.o aegis/aet.o

aegis/cattr.c aegis/cattr.h: aegis/cattr.def bin/fmtgen aegis/common.def
	bin/fmtgen -Iaegis aegis/cattr.def aegis/cattr.c aegis/cattr.h

aegis/cattr.o: aegis/cattr.c aegis/cattr.h common/main.h aegis/type.h \
		common/str.h common/s-v-arg.h common/conf.h \
		aegis/parse.h common/error.h aegis/indent.h aegis/io.h \
		common/mem.h aegis/os.h common/trace.h 
	$(CC) $(CFLAGS) -Iaegis -Icommon $(H) -c aegis/cattr.c
	mv cattr.o aegis/cattr.o

aegis/change.o: aegis/change.c aegis/change.h common/main.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/commit.h common/error.h \
		aegis/lock.h common/mem.h common/option.h aegis/os.h \
		aegis/project.h aegis/pstate.h aegis/pattr.h \
		aegis/sub.h common/trace.h aegis/undo.h aegis/user.h \
		aegis/ustate.h aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/change.c
	mv change.o aegis/change.o

aegis/col.o: aegis/col.c aegis/col.h common/main.h common/error.h \
		aegis/glue.h common/conf.h common/mem.h common/option.h \
		aegis/os.h common/str.h common/s-v-arg.h aegis/pager.h \
		common/trace.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/col.c
	mv col.o aegis/col.o

aegis/commit.o: aegis/commit.c aegis/commit.h common/str.h \
		common/s-v-arg.h common/conf.h common/main.h \
		aegis/dir.h common/mem.h aegis/os.h common/trace.h \
		aegis/undo.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/commit.c
	mv commit.o aegis/commit.o

aegis/common.c aegis/common.h: aegis/common.def bin/fmtgen
	bin/fmtgen -Iaegis aegis/common.def aegis/common.c aegis/common.h

aegis/common.o: aegis/common.c aegis/common.h common/main.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h common/error.h \
		aegis/indent.h aegis/io.h common/mem.h aegis/os.h \
		common/trace.h 
	$(CC) $(CFLAGS) -Iaegis -Icommon $(H) -c aegis/common.c
	mv common.o aegis/common.o

aegis/cstate.c aegis/cstate.h: aegis/cstate.def bin/fmtgen \
		aegis/cattr.def aegis/common.def 
	bin/fmtgen -Iaegis aegis/cstate.def aegis/cstate.c aegis/cstate.h

aegis/cstate.o: aegis/cstate.c aegis/cstate.h common/main.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h common/error.h \
		aegis/indent.h aegis/io.h common/mem.h aegis/os.h \
		common/trace.h 
	$(CC) $(CFLAGS) -Iaegis -Icommon $(H) -c aegis/cstate.c
	mv cstate.o aegis/cstate.o

aegis/dir.o: aegis/dir.c aegis/dir.h common/main.h common/str.h \
		common/s-v-arg.h common/conf.h common/error.h \
		aegis/glue.h common/trace.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/dir.c
	mv dir.o aegis/dir.o

aegis/file.o: aegis/file.c aegis/file.h common/main.h common/str.h \
		common/s-v-arg.h common/conf.h common/error.h \
		aegis/glue.h common/mem.h aegis/os.h common/trace.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/file.c
	mv file.o aegis/file.o

aegis/glue.o: aegis/glue.c common/error.h common/main.h aegis/glue.h \
		common/conf.h common/mem.h aegis/os.h common/str.h \
		common/s-v-arg.h common/trace.h aegis/undo.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/glue.c
	mv glue.o aegis/glue.o

aegis/gonzo.o: aegis/gonzo.c aegis/commit.h common/str.h \
		common/s-v-arg.h common/conf.h common/main.h \
		common/error.h aegis/gonzo.h aegis/gstate.h \
		aegis/type.h aegis/parse.h aegis/lock.h common/mem.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h \
		common/word.h 
	$(CC) $(CFLAGS) -D'LIB="$(LIB)"' -Iaegis -Icommon $(H) -c aegis/gonzo.c
	mv gonzo.o aegis/gonzo.o

aegis/gram.gen.c aegis/gram.gen.h: aegis/gram.y
	$(YACC) -d aegis/gram.y
	sed "s/[yY][yY]/gram_/g" < y.tab.c > aegis/gram.gen.c
	rm y.tab.c
	sed "s/[yY][yY]/gram_/g" < y.tab.h > aegis/gram.gen.h
	rm y.tab.h

aegis/gram.gen.o: aegis/gram.gen.c aegis/gram.h common/main.h \
		aegis/lex.h common/mem.h aegis/parse.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/type.h 
	$(CC) $(CFLAGS) -Iaegis -Icommon $(H) -c aegis/gram.gen.c
	mv gram.gen.o aegis/gram.gen.o

aegis/gstate.c aegis/gstate.h: aegis/gstate.def bin/fmtgen
	bin/fmtgen -Iaegis aegis/gstate.def aegis/gstate.c aegis/gstate.h

aegis/gstate.o: aegis/gstate.c aegis/gstate.h common/main.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h common/error.h \
		aegis/indent.h aegis/io.h common/mem.h aegis/os.h \
		common/trace.h 
	$(CC) $(CFLAGS) -Iaegis -Icommon $(H) -c aegis/gstate.c
	mv gstate.o aegis/gstate.o

aegis/help.o: aegis/help.c aegis/arglex2.h common/arglex.h \
		common/main.h common/error.h aegis/gonzo.h \
		aegis/gstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/help.h common/option.h aegis/pager.h \
		common/patchlevel.h common/trace.h aegis/undo.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/help.c
	mv help.o aegis/help.o

aegis/indent.o: aegis/indent.c common/error.h common/main.h \
		aegis/glue.h common/conf.h aegis/indent.h \
		common/s-v-arg.h common/trace.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/indent.c
	mv indent.o aegis/indent.o

aegis/io.o: aegis/io.c aegis/indent.h common/main.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/io.h aegis/type.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/io.c
	mv io.o aegis/io.o

aegis/lex.o: aegis/lex.c common/error.h common/main.h aegis/glue.h \
		common/conf.h aegis/lex.h common/s-v-arg.h common/str.h \
		aegis/gram.gen.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/lex.c
	mv lex.o aegis/lex.o

aegis/lock.o: aegis/lock.c common/error.h common/main.h aegis/glue.h \
		common/conf.h aegis/gonzo.h aegis/gstate.h aegis/type.h \
		common/str.h common/s-v-arg.h aegis/parse.h \
		aegis/lock.h common/mem.h aegis/os.h common/trace.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/lock.c
	mv lock.o aegis/lock.o

aegis/log.o: aegis/log.c common/error.h common/main.h aegis/log.h \
		aegis/os.h common/str.h common/s-v-arg.h common/conf.h \
		common/trace.h aegis/undo.h aegis/user.h aegis/ustate.h \
		aegis/type.h aegis/parse.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/log.c
	mv log.o aegis/log.o

aegis/main.o: aegis/main.c aegis/aeb.h common/main.h aegis/aeca.h \
		aegis/cattr.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/aecd.h aegis/aecp.h aegis/aecpu.h aegis/aed.h \
		aegis/aedb.h aegis/aedbu.h aegis/aede.h aegis/aedeu.h \
		aegis/aeib.h aegis/aeibu.h aegis/aeif.h aegis/aeip.h \
		aegis/ael.h aegis/aemv.h aegis/aena.h aegis/aenc.h \
		aegis/aencu.h aegis/aend.h aegis/aenf.h aegis/aenfu.h \
		aegis/aeni.h aegis/aenpr.h aegis/aenrls.h aegis/aenrv.h \
		aegis/aent.h aegis/aentu.h aegis/aepa.h aegis/aera.h \
		aegis/aerd.h aegis/aerf.h aegis/aeri.h aegis/aerm.h \
		aegis/aermpr.h aegis/aermu.h aegis/aerp.h aegis/aerpu.h \
		aegis/aerrv.h aegis/aet.h aegis/arglex2.h \
		common/arglex.h common/error.h aegis/help.h aegis/log.h \
		common/option.h aegis/os.h common/trace.h aegis/undo.h \
		aegis/version.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/main.c
	mv main.o aegis/main.o

aegis/os.o: aegis/os.c common/conf.h aegis/dir.h common/main.h \
		common/str.h common/s-v-arg.h common/error.h \
		aegis/file.h aegis/glue.h common/mem.h common/option.h \
		aegis/os.h common/trace.h aegis/undo.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/os.c
	mv os.o aegis/os.o

aegis/pager.o: aegis/pager.c common/error.h common/main.h \
		common/option.h aegis/os.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/pager.h \
		common/trace.h aegis/undo.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/pager.c
	mv pager.o aegis/pager.o

aegis/parse.o: aegis/parse.c common/error.h common/main.h aegis/gram.h \
		aegis/lex.h common/mem.h aegis/parse.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/type.h \
		common/trace.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/parse.c
	mv parse.o aegis/parse.o

aegis/pattr.c aegis/pattr.h: aegis/pattr.def bin/fmtgen aegis/common.def
	bin/fmtgen -Iaegis aegis/pattr.def aegis/pattr.c aegis/pattr.h

aegis/pattr.o: aegis/pattr.c aegis/pattr.h common/main.h aegis/type.h \
		common/str.h common/s-v-arg.h common/conf.h \
		aegis/parse.h common/error.h aegis/indent.h aegis/io.h \
		common/mem.h aegis/os.h common/trace.h 
	$(CC) $(CFLAGS) -Iaegis -Icommon $(H) -c aegis/pattr.c
	mv pattr.o aegis/pattr.o

aegis/pconf.c aegis/pconf.h: aegis/pconf.def bin/fmtgen aegis/common.def
	bin/fmtgen -Iaegis aegis/pconf.def aegis/pconf.c aegis/pconf.h

aegis/pconf.o: aegis/pconf.c aegis/pconf.h common/main.h aegis/type.h \
		common/str.h common/s-v-arg.h common/conf.h \
		aegis/parse.h common/error.h aegis/indent.h aegis/io.h \
		common/mem.h aegis/os.h common/trace.h 
	$(CC) $(CFLAGS) -Iaegis -Icommon $(H) -c aegis/pconf.c
	mv pconf.o aegis/pconf.o

aegis/project.o: aegis/project.c aegis/change.h common/main.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h aegis/commit.h common/error.h \
		aegis/gonzo.h aegis/gstate.h aegis/lock.h common/mem.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/user.h aegis/ustate.h aegis/uconf.h aegis/undo.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/project.c
	mv project.o aegis/project.o

aegis/pstate.c aegis/pstate.h: aegis/pstate.def bin/fmtgen \
		aegis/pattr.def aegis/common.def 
	bin/fmtgen -Iaegis aegis/pstate.def aegis/pstate.c aegis/pstate.h

aegis/pstate.o: aegis/pstate.c aegis/pstate.h common/main.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h common/error.h \
		aegis/indent.h aegis/io.h common/mem.h aegis/os.h \
		common/trace.h 
	$(CC) $(CFLAGS) -Iaegis -Icommon $(H) -c aegis/pstate.c
	mv pstate.o aegis/pstate.o

aegis/sub.o: aegis/sub.c common/arglex.h common/main.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h common/error.h common/mem.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/sub.c
	mv sub.o aegis/sub.o

aegis/type.o: aegis/type.c aegis/type.h common/main.h common/str.h \
		common/s-v-arg.h common/conf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/type.c
	mv type.o aegis/type.o

aegis/uconf.c aegis/uconf.h: aegis/uconf.def bin/fmtgen
	bin/fmtgen -Iaegis aegis/uconf.def aegis/uconf.c aegis/uconf.h

aegis/uconf.o: aegis/uconf.c aegis/uconf.h common/main.h aegis/type.h \
		common/str.h common/s-v-arg.h common/conf.h \
		aegis/parse.h common/error.h aegis/indent.h aegis/io.h \
		common/mem.h aegis/os.h common/trace.h 
	$(CC) $(CFLAGS) -Iaegis -Icommon $(H) -c aegis/uconf.c
	mv uconf.o aegis/uconf.o

aegis/undo.o: aegis/undo.c common/error.h common/main.h common/mem.h \
		common/option.h aegis/os.h common/str.h \
		common/s-v-arg.h common/conf.h common/trace.h \
		aegis/undo.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/undo.c
	mv undo.o aegis/undo.o

aegis/user.o: aegis/user.c aegis/commit.h common/str.h common/s-v-arg.h \
		common/conf.h common/main.h common/error.h \
		aegis/gonzo.h aegis/gstate.h aegis/type.h aegis/parse.h \
		aegis/lock.h common/mem.h common/option.h aegis/os.h \
		aegis/project.h aegis/pstate.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h aegis/undo.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/user.c
	mv user.o aegis/user.o

aegis/ustate.c aegis/ustate.h: aegis/ustate.def bin/fmtgen
	bin/fmtgen -Iaegis aegis/ustate.def aegis/ustate.c aegis/ustate.h

aegis/ustate.o: aegis/ustate.c aegis/ustate.h common/main.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h common/error.h \
		aegis/indent.h aegis/io.h common/mem.h aegis/os.h \
		common/trace.h 
	$(CC) $(CFLAGS) -Iaegis -Icommon $(H) -c aegis/ustate.c
	mv ustate.o aegis/ustate.o

aegis/version.o: aegis/version.c aegis/arglex2.h common/arglex.h \
		common/main.h common/error.h aegis/help.h \
		common/option.h common/str.h common/s-v-arg.h \
		common/conf.h common/trace.h aegis/version.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/version.c
	mv version.o aegis/version.o

common/ansi.o: common/ansi.c common/main.h common/conf.h
	$(CC) $(CFLAGS) -Icommon $(H) -c common/ansi.c
	mv ansi.o common/ansi.o

common/arglex.o: common/arglex.c common/arglex.h common/main.h \
		common/error.h common/option.h common/str.h \
		common/s-v-arg.h common/conf.h common/trace.h 
	$(CC) $(CFLAGS) -Icommon $(H) -c common/arglex.c
	mv arglex.o common/arglex.o

common/error.o: common/error.c common/arglex.h common/main.h \
		common/error.h common/option.h common/s-v-arg.h \
		common/conf.h 
	$(CC) $(CFLAGS) -Icommon $(H) -c common/error.c
	mv error.o common/error.o

common/mem.o: common/mem.c common/main.h common/mem.h common/error.h
	$(CC) $(CFLAGS) -Icommon $(H) -c common/mem.c
	mv mem.o common/mem.o

common/option.o: common/option.c common/error.h common/main.h \
		common/option.h common/trace.h 
	$(CC) $(CFLAGS) -Icommon $(H) -c common/option.c
	mv option.o common/option.o

common/str.o: common/str.c common/error.h common/main.h common/mem.h \
		common/s-v-arg.h common/conf.h common/str.h 
	$(CC) $(CFLAGS) -Icommon $(H) -c common/str.c
	mv str.o common/str.o

common/trace.o: common/trace.c common/error.h common/main.h \
		common/mem.h common/option.h common/s-v-arg.h \
		common/conf.h common/str.h common/trace.h 
	$(CC) $(CFLAGS) -Icommon $(H) -c common/trace.c
	mv trace.o common/trace.o

common/word.o: common/word.c common/main.h common/word.h common/str.h \
		common/s-v-arg.h common/conf.h common/error.h \
		common/mem.h 
	$(CC) $(CFLAGS) -Icommon $(H) -c common/word.c
	mv word.o common/word.o

fmtgen/id.o: fmtgen/id.c common/main.h fmtgen/id.h common/str.h \
		common/s-v-arg.h common/conf.h common/word.h \
		common/mem.h common/error.h 
	$(CC) $(CFLAGS) -Ifmtgen -Icommon $(H) -c fmtgen/id.c
	mv id.o fmtgen/id.o

fmtgen/indent.o: fmtgen/indent.c common/error.h common/main.h \
		fmtgen/indent.h common/s-v-arg.h common/conf.h \
		common/trace.h 
	$(CC) $(CFLAGS) -Ifmtgen -Icommon $(H) -c fmtgen/indent.c
	mv indent.o fmtgen/indent.o

fmtgen/lex.o: fmtgen/lex.c common/error.h common/main.h fmtgen/id.h \
		common/str.h common/s-v-arg.h common/conf.h \
		common/word.h fmtgen/lex.h common/mem.h fmtgen/type.h \
		fmtgen/parse.gen.h 
	$(CC) $(CFLAGS) -Ifmtgen -Icommon $(H) -c fmtgen/lex.c
	mv lex.o fmtgen/lex.o

fmtgen/main.o: fmtgen/main.c common/arglex.h common/main.h \
		common/error.h fmtgen/id.h common/str.h \
		common/s-v-arg.h common/conf.h common/word.h \
		fmtgen/lex.h common/option.h fmtgen/parse.h \
		common/trace.h 
	$(CC) $(CFLAGS) -Ifmtgen -Icommon $(H) -c fmtgen/main.c
	mv main.o fmtgen/main.o

fmtgen/parse.gen.c fmtgen/parse.gen.h: fmtgen/parse.y
	$(YACC) -d fmtgen/parse.y
	sed "s/[yY][yY]/parse_/g" < y.tab.c > fmtgen/parse.gen.c
	rm y.tab.c
	sed "s/[yY][yY]/parse_/g" < y.tab.h > fmtgen/parse.gen.h
	rm y.tab.h

fmtgen/parse.gen.o: fmtgen/parse.gen.c common/error.h common/main.h \
		fmtgen/id.h common/str.h common/s-v-arg.h common/conf.h \
		common/word.h fmtgen/indent.h fmtgen/lex.h common/mem.h \
		fmtgen/parse.h common/trace.h fmtgen/type.h 
	$(CC) $(CFLAGS) -Ifmtgen -Icommon $(H) -c fmtgen/parse.gen.c
	mv parse.gen.o fmtgen/parse.gen.o

fmtgen/type.o: fmtgen/type.c fmtgen/type.h common/main.h fmtgen/lex.h \
		common/str.h common/s-v-arg.h common/conf.h 
	$(CC) $(CFLAGS) -Ifmtgen -Icommon $(H) -c fmtgen/type.c
	mv type.o fmtgen/type.o

fmtgen/type_enum.o: fmtgen/type_enum.c fmtgen/type.h common/main.h \
		fmtgen/lex.h common/str.h common/s-v-arg.h \
		common/conf.h fmtgen/indent.h fmtgen/id.h common/word.h \
		common/mem.h 
	$(CC) $(CFLAGS) -Ifmtgen -Icommon $(H) -c fmtgen/type_enum.c
	mv type_enum.o fmtgen/type_enum.o

fmtgen/type_integ.o: fmtgen/type_integ.c fmtgen/type.h common/main.h \
		fmtgen/lex.h common/str.h common/s-v-arg.h \
		common/conf.h fmtgen/indent.h common/mem.h 
	$(CC) $(CFLAGS) -Ifmtgen -Icommon $(H) -c fmtgen/type_integ.c
	mv type_integ.o fmtgen/type_integ.o

fmtgen/type_list.o: fmtgen/type_list.c fmtgen/type.h common/main.h \
		fmtgen/lex.h common/str.h common/s-v-arg.h \
		common/conf.h fmtgen/indent.h common/mem.h fmtgen/id.h \
		common/word.h 
	$(CC) $(CFLAGS) -Ifmtgen -Icommon $(H) -c fmtgen/type_list.c
	mv type_list.o fmtgen/type_list.o

fmtgen/type_ref.o: fmtgen/type_ref.c fmtgen/type.h common/main.h \
		fmtgen/lex.h common/str.h common/s-v-arg.h \
		common/conf.h fmtgen/indent.h common/mem.h fmtgen/id.h \
		common/word.h common/error.h 
	$(CC) $(CFLAGS) -Ifmtgen -Icommon $(H) -c fmtgen/type_ref.c
	mv type_ref.o fmtgen/type_ref.o

fmtgen/type_strin.o: fmtgen/type_strin.c fmtgen/type.h common/main.h \
		fmtgen/lex.h common/str.h common/s-v-arg.h \
		common/conf.h fmtgen/indent.h common/mem.h 
	$(CC) $(CFLAGS) -Ifmtgen -Icommon $(H) -c fmtgen/type_strin.c
	mv type_strin.o fmtgen/type_strin.o

fmtgen/type_struc.o: fmtgen/type_struc.c fmtgen/type.h common/main.h \
		fmtgen/lex.h common/str.h common/s-v-arg.h \
		common/conf.h fmtgen/indent.h common/mem.h fmtgen/id.h \
		common/word.h common/error.h 
	$(CC) $(CFLAGS) -Ifmtgen -Icommon $(H) -c fmtgen/type_struc.c
	mv type_struc.o fmtgen/type_struc.o

t0001a: all test/00/t0001a.sh
	$(SHELL) test/00/t0001a.sh

t0002a: all test/00/t0002a.sh
	$(SHELL) test/00/t0002a.sh

t0003a: all test/00/t0003a.sh
	$(SHELL) test/00/t0003a.sh

t0004a: all test/00/t0004a.sh
	$(SHELL) test/00/t0004a.sh

t0005a: all test/00/t0005a.sh
	$(SHELL) test/00/t0005a.sh

t0006a: all test/00/t0006a.sh
	$(SHELL) test/00/t0006a.sh

t0007a: all test/00/t0007a.sh
	$(SHELL) test/00/t0007a.sh

t0008a: all test/00/t0008a.sh
	$(SHELL) test/00/t0008a.sh

t0009a: all test/00/t0009a.sh
	$(SHELL) test/00/t0009a.sh

t0010a: all test/00/t0010a.sh
	$(SHELL) test/00/t0010a.sh

t0011a: all test/00/t0011a.sh
	$(SHELL) test/00/t0011a.sh

t0012a: all test/00/t0012a.sh
	$(SHELL) test/00/t0012a.sh

t0013a: all test/00/t0013a.sh
	$(SHELL) test/00/t0013a.sh

t0014a: all test/00/t0014a.sh
	$(SHELL) test/00/t0014a.sh

t0015a: all test/00/t0015a.sh
	$(SHELL) test/00/t0015a.sh

t0016a: all test/00/t0016a.sh
	$(SHELL) test/00/t0016a.sh

t0017a: all test/00/t0017a.sh
	$(SHELL) test/00/t0017a.sh

t0018a: all test/00/t0018a.sh
	$(SHELL) test/00/t0018a.sh

clean:
	rm -f core bin/fmtgen aegis/aeb.o aegis/aeca.o aegis/aecd.o \
		aegis/aecp.o aegis/aecpu.o aegis/aed.o aegis/aedb.o \
		aegis/aedbu.o aegis/aede.o aegis/aedeu.o aegis/aeib.o \
		aegis/aeibu.o aegis/aeif.o aegis/aeip.o aegis/ael.o \
		aegis/aemv.o aegis/aena.o aegis/aenc.o aegis/aencu.o \
		aegis/aend.o aegis/aenf.o aegis/aenfu.o aegis/aeni.o \
		aegis/aenpr.o aegis/aenrls.o aegis/aenrv.o aegis/aent.o \
		aegis/aentu.o aegis/aepa.o aegis/aera.o aegis/aerd.o \
		aegis/aerf.o aegis/aeri.o aegis/aerm.o aegis/aermpr.o \
		aegis/aermu.o aegis/aerp.o aegis/aerpu.o aegis/aerrv.o \
		aegis/aet.o aegis/cattr.c aegis/cattr.h aegis/cattr.o \
		aegis/change.o aegis/col.o aegis/commit.o \
		aegis/common.c aegis/common.h aegis/common.o \
		aegis/cstate.c aegis/cstate.h aegis/cstate.o \
		aegis/dir.o aegis/file.o aegis/glue.o aegis/gonzo.o \
		aegis/gram.gen.c aegis/gram.gen.h aegis/gram.gen.o \
		aegis/gstate.c aegis/gstate.h aegis/gstate.o \
		aegis/help.o aegis/indent.o aegis/io.o aegis/lex.o \
		aegis/lock.o aegis/log.o aegis/main.o aegis/os.o \
		aegis/pager.o aegis/parse.o aegis/pattr.c aegis/pattr.h \
		aegis/pattr.o aegis/pconf.c aegis/pconf.h aegis/pconf.o \
		aegis/project.o aegis/pstate.c aegis/pstate.h \
		aegis/pstate.o aegis/sub.o aegis/type.o aegis/uconf.c \
		aegis/uconf.h aegis/uconf.o aegis/undo.o aegis/user.o \
		aegis/ustate.c aegis/ustate.h aegis/ustate.o \
		aegis/version.o common/ansi.o common/arglex.o \
		common/error.o common/mem.o common/option.o \
		common/str.o common/trace.o common/word.o fmtgen/id.o \
		fmtgen/indent.o fmtgen/lex.o fmtgen/main.o \
		fmtgen/parse.gen.c fmtgen/parse.gen.h \
		fmtgen/parse.gen.o fmtgen/type.o fmtgen/type_enum.o \
		fmtgen/type_integ.o fmtgen/type_list.o \
		fmtgen/type_ref.o fmtgen/type_strin.o \
		fmtgen/type_struc.o 

realclean: clean
	rm -f bin/aegis

clobber: realclean
	rm -f common/conf.h

common/conf.h:
	echo "#include <../conf/`uname -s`-`uname -r`>" > common/conf.h

FmtgenFiles = common/ansi.o common/arglex.o common/error.o common/mem.o \
		common/option.o common/str.o common/trace.o \
		common/word.o fmtgen/id.o fmtgen/indent.o fmtgen/lex.o \
		fmtgen/main.o fmtgen/parse.gen.o fmtgen/type.o \
		fmtgen/type_enum.o fmtgen/type_integ.o \
		fmtgen/type_list.o fmtgen/type_ref.o \
		fmtgen/type_strin.o fmtgen/type_struc.o 

bin/fmtgen: $(FmtgenFiles)
	if test ! -d bin; then mkdir bin; fi; exit 0
	$(CC) -o bin/fmtgen $(FmtgenFiles) $(LIBRARIES)

AegisFiles = aegis/aeb.o aegis/aeca.o aegis/aecd.o aegis/aecp.o \
		aegis/aecpu.o aegis/aed.o aegis/aedb.o aegis/aedbu.o \
		aegis/aede.o aegis/aedeu.o aegis/aeib.o aegis/aeibu.o \
		aegis/aeif.o aegis/aeip.o aegis/ael.o aegis/aemv.o \
		aegis/aena.o aegis/aenc.o aegis/aencu.o aegis/aend.o \
		aegis/aenf.o aegis/aenfu.o aegis/aeni.o aegis/aenpr.o \
		aegis/aenrls.o aegis/aenrv.o aegis/aent.o aegis/aentu.o \
		aegis/aepa.o aegis/aera.o aegis/aerd.o aegis/aerf.o \
		aegis/aeri.o aegis/aerm.o aegis/aermpr.o aegis/aermu.o \
		aegis/aerp.o aegis/aerpu.o aegis/aerrv.o aegis/aet.o \
		aegis/cattr.o aegis/change.o aegis/col.o aegis/commit.o \
		aegis/common.o aegis/cstate.o aegis/dir.o aegis/file.o \
		aegis/glue.o aegis/gonzo.o aegis/gram.gen.o \
		aegis/gstate.o aegis/help.o aegis/indent.o aegis/io.o \
		aegis/lex.o aegis/lock.o aegis/log.o aegis/main.o \
		aegis/os.o aegis/pager.o aegis/parse.o aegis/pattr.o \
		aegis/pconf.o aegis/project.o aegis/pstate.o \
		aegis/sub.o aegis/type.o aegis/uconf.o aegis/undo.o \
		aegis/user.o aegis/ustate.o aegis/version.o \
		common/ansi.o common/arglex.o common/error.o \
		common/mem.o common/option.o common/str.o \
		common/trace.o common/word.o 

bin/aegis: $(AegisFiles)
	if test ! -d bin; then mkdir bin; fi; exit 0
	$(CC) -o bin/aegis $(AegisFiles) $(LIBRARIES)

sure: t0001a t0002a t0003a t0004a t0005a t0006a t0007a t0008a t0009a \
		t0010a t0011a t0012a t0013a t0014a t0015a t0016a t0017a \
		t0018a 
	@echo Passed All Tests

install: all
	cp bin/aegis $(BIN)
	chown root $(BIN)/aegis
	chmod a+x,u+s $(BIN)/aegis
	-mkdir $(LIB)
	chmod 0755 $(LIB)
	cp lib/* $(LIB)
	chmod a+r $(LIB)/*
	chmod a+x $(LIB)/*.sh
	chown bin $(LIB)
	chgrp bin $(LIB)
	$(SHELL) man1/install.sh $(MAN)/man1
	$(SHELL) man5/install.sh $(MAN)/man5
