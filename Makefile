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
# H =				# dcosx
# H =				# ULTRIX
# H =				# hpux
# H =				# SCO

#
# the name of the compiler to use
#
CC = cc
# CC = gcc
# CC = cc			# SunOS
# CC = cc			# ConvexOS
# CC = cc			# dgux
# CC = /usr/ucb/cc		# dcosx
# CC = cc			# ULTRIX
# CC = cc			# hpux
# CC = rcc			# SCO

#
# The compiler flags to use, except for include path.
#
CFLAGS = -O
# CFLAGS = -g
# CFLAGS = -O			# SunOS
# CFLAGS = -O			# ConvexOS
# CFLAGS = -O			# dgux
# CFLAGS = -O -Wall -ansi	# gcc
# CFLAGS = -O -Xt -U__STDC__	# dcosx
# CFLAGS = -O			# ULTRIX
# CFLAGS = -O			# hpux
# CFLAGS = -O			# SCO

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
# LIBRARIES = -lucb		# dcosx
# LIBRARIES =			# ULTRIX
# LIBRARIES =			# hpux
# LIBRARIES = -lsocket		# SCO

#
# shell to use to run the tests
#	make sure there are no spaces after the definition,
#	many flavours of make(1) can't cope with them.
#
SHELL = /bin/sh
# SHELL = /bin/sh		# SunOS
# SHELL = /bin/sh		# ConvexOS
# SHELL = /bin/sh		# dgux
# SHELL = /bin/sh		# dcosx
# SHELL = /bin/sh5		# ULTRIX
# SHELL = /bin/ksh		# apollo
# SHELL = /bin/sh		# hpux
# SHELL = /bin/sh		# SCO

# You should not need to alter anything below this point.
#------------------------------------------------------------

all: bin/aegis


aegis/administ.o: aegis/administ.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/administ.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		common/error.h aegis/help.h aegis/list.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/type.h aegis/parse.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/administ.c
	mv administ.o aegis/administ.o

aegis/build.o: aegis/build.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/build.h aegis/col.h aegis/commit.h \
		common/str.h common/s-v-arg.h common/conf.h \
		aegis/change.h aegis/cstate.h aegis/type.h \
		aegis/parse.h aegis/pconf.h common/error.h aegis/help.h \
		aegis/list.h aegis/lock.h aegis/log.h common/option.h \
		aegis/os.h aegis/project.h aegis/pstate.h aegis/pattr.h \
		aegis/sub.h common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/build.c
	mv build.o aegis/build.o

aegis/cattr.c aegis/cattr.h: aegis/cattr.def bin/fmtgen aegis/common.def
	bin/fmtgen -Iaegis aegis/cattr.def aegis/cattr.c aegis/cattr.h

aegis/cattr.o: aegis/cattr.c aegis/cattr.h common/main.h aegis/type.h \
		common/str.h common/s-v-arg.h common/conf.h \
		aegis/parse.h common/error.h aegis/indent.h aegis/io.h \
		common/mem.h aegis/os.h common/trace.h 
	$(CC) $(CFLAGS) -Iaegis -Icommon $(H) -c aegis/cattr.c
	mv cattr.o aegis/cattr.o

aegis/cattr_ed.o: aegis/cattr_ed.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/cattr.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/cattr_ed.h aegis/commit.h aegis/change.h \
		aegis/cstate.h aegis/pconf.h common/error.h \
		aegis/help.h aegis/lock.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/option.h aegis/os.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/cattr_ed.c
	mv cattr_ed.o aegis/cattr_ed.o

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

aegis/chdir.o: aegis/chdir.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/chdir.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h common/error.h aegis/help.h aegis/list.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/chdir.c
	mv chdir.o aegis/chdir.o

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

aegis/copyfile.o: aegis/copyfile.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/commit.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/copyfile.h \
		aegis/change.h aegis/cstate.h aegis/type.h \
		aegis/parse.h aegis/pconf.h common/error.h aegis/file.h \
		aegis/help.h aegis/list.h aegis/lock.h aegis/log.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h \
		common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/copyfile.c
	mv copyfile.o aegis/copyfile.o

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

aegis/develop1.o: aegis/develop1.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/commit.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/develop1.h \
		common/error.h aegis/help.h aegis/lock.h aegis/list.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/type.h aegis/parse.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/develop1.c
	mv develop1.o aegis/develop1.o

aegis/develop2.o: aegis/develop2.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/change.h aegis/cstate.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h aegis/pconf.h aegis/col.h \
		aegis/commit.h aegis/common.h aegis/develop2.h \
		aegis/dir.h common/error.h aegis/help.h aegis/list.h \
		aegis/lock.h common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/develop2.c
	mv develop2.o aegis/develop2.o

aegis/diff.o: aegis/diff.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/change.h aegis/cstate.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h aegis/pconf.h aegis/col.h \
		aegis/commit.h aegis/diff.h common/error.h aegis/help.h \
		aegis/list.h aegis/lock.h aegis/log.h common/option.h \
		aegis/os.h aegis/project.h aegis/pstate.h aegis/pattr.h \
		aegis/sub.h common/trace.h aegis/undo.h aegis/user.h \
		aegis/ustate.h aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/diff.c
	mv diff.o aegis/diff.o

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

aegis/integra1.o: aegis/integra1.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/commit.h common/str.h \
		common/s-v-arg.h common/conf.h common/error.h \
		aegis/help.h aegis/integra1.h aegis/list.h aegis/lock.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/type.h aegis/parse.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/integra1.c
	mv integra1.o aegis/integra1.o

aegis/integra2.o: aegis/integra2.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/commit.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/change.h \
		aegis/cstate.h aegis/type.h aegis/parse.h aegis/pconf.h \
		aegis/dir.h common/error.h aegis/file.h aegis/help.h \
		aegis/integra2.h aegis/list.h aegis/lock.h aegis/log.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/integra2.c
	mv integra2.o aegis/integra2.o

aegis/io.o: aegis/io.c aegis/indent.h common/main.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/io.h aegis/type.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/io.c
	mv io.o aegis/io.o

aegis/lex.o: aegis/lex.c common/error.h common/main.h aegis/glue.h \
		common/conf.h aegis/lex.h common/s-v-arg.h common/str.h \
		aegis/gram.gen.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/lex.c
	mv lex.o aegis/lex.o

aegis/list.o: aegis/list.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/col.h aegis/change.h aegis/cstate.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h aegis/pconf.h \
		common/error.h aegis/gonzo.h aegis/gstate.h \
		aegis/help.h aegis/list.h common/option.h aegis/os.h \
		aegis/project.h aegis/pstate.h aegis/pattr.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/list.c
	mv list.o aegis/list.o

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

aegis/main.o: aegis/main.c aegis/administ.h common/main.h \
		aegis/arglex2.h common/arglex.h aegis/build.h \
		aegis/cattr_ed.h aegis/chdir.h aegis/copyfile.h \
		aegis/develop1.h aegis/develop2.h aegis/diff.h \
		common/error.h aegis/help.h aegis/integra1.h \
		aegis/integra2.h aegis/list.h aegis/log.h \
		aegis/new_chan.h aegis/new_file.h aegis/new_proj.h \
		aegis/new_rele.h aegis/new_test.h common/option.h \
		aegis/os.h common/str.h common/s-v-arg.h common/conf.h \
		aegis/pattr_ed.h aegis/rem_file.h aegis/rem_proj.h \
		aegis/review.h aegis/reviewer.h aegis/test.h \
		common/trace.h aegis/undo.h aegis/version.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/main.c
	mv main.o aegis/main.o

aegis/new_chan.o: aegis/new_chan.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/cattr.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/change.h aegis/cstate.h aegis/pconf.h aegis/col.h \
		aegis/commit.h aegis/common.h common/error.h \
		aegis/help.h aegis/list.h aegis/lock.h aegis/new_chan.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/new_chan.c
	mv new_chan.o aegis/new_chan.o

aegis/new_file.o: aegis/new_file.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/change.h aegis/cstate.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h aegis/pconf.h aegis/col.h \
		aegis/commit.h common/error.h aegis/glue.h aegis/help.h \
		aegis/list.h aegis/lock.h aegis/log.h aegis/new_file.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/user.h aegis/ustate.h aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/new_file.c
	mv new_file.o aegis/new_file.o

aegis/new_proj.o: aegis/new_proj.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/commit.h common/str.h \
		common/s-v-arg.h common/conf.h common/error.h \
		aegis/gonzo.h aegis/gstate.h aegis/type.h aegis/parse.h \
		aegis/help.h aegis/list.h aegis/lock.h aegis/new_proj.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/new_proj.c
	mv new_proj.o aegis/new_proj.o

aegis/new_rele.o: aegis/new_rele.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/change.h aegis/cstate.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h aegis/pconf.h \
		aegis/commit.h aegis/dir.h common/error.h aegis/file.h \
		aegis/gonzo.h aegis/gstate.h aegis/help.h aegis/list.h \
		aegis/lock.h aegis/log.h common/mem.h aegis/new_rele.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		aegis/undo.h aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/new_rele.c
	mv new_rele.o aegis/new_rele.o

aegis/new_test.o: aegis/new_test.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/change.h aegis/cstate.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h aegis/pconf.h aegis/col.h \
		aegis/commit.h common/error.h aegis/glue.h aegis/help.h \
		aegis/list.h aegis/lock.h aegis/new_test.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h common/trace.h \
		aegis/user.h aegis/ustate.h aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/new_test.c
	mv new_test.o aegis/new_test.o

aegis/os.o: aegis/os.c common/conf.h aegis/dir.h common/main.h \
		common/str.h common/s-v-arg.h common/error.h \
		aegis/file.h aegis/glue.h common/mem.h common/option.h \
		aegis/os.h common/trace.h aegis/undo.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/os.c
	mv os.o aegis/os.o

aegis/pager.o: aegis/pager.c common/error.h common/main.h aegis/os.h \
		common/str.h common/s-v-arg.h common/conf.h \
		aegis/pager.h common/trace.h aegis/undo.h 
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

aegis/pattr_ed.o: aegis/pattr_ed.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/commit.h common/str.h \
		common/s-v-arg.h common/conf.h common/error.h \
		aegis/help.h aegis/lock.h common/option.h aegis/os.h \
		aegis/pattr.h aegis/type.h aegis/parse.h \
		aegis/pattr_ed.h aegis/project.h aegis/pstate.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/pattr_ed.c
	mv pattr_ed.o aegis/pattr_ed.o

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

aegis/rem_file.o: aegis/rem_file.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/change.h aegis/cstate.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h aegis/pconf.h \
		aegis/commit.h common/error.h aegis/help.h aegis/list.h \
		aegis/lock.h aegis/log.h common/option.h aegis/os.h \
		aegis/project.h aegis/pstate.h aegis/pattr.h \
		aegis/rem_file.h common/trace.h aegis/user.h \
		aegis/ustate.h aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/rem_file.c
	mv rem_file.o aegis/rem_file.o

aegis/rem_proj.o: aegis/rem_proj.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/change.h aegis/cstate.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h aegis/pconf.h \
		aegis/commit.h common/error.h aegis/gonzo.h \
		aegis/gstate.h aegis/help.h aegis/list.h aegis/lock.h \
		common/option.h aegis/project.h aegis/pstate.h \
		aegis/pattr.h aegis/rem_proj.h common/trace.h \
		aegis/user.h aegis/ustate.h aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/rem_proj.c
	mv rem_proj.o aegis/rem_proj.o

aegis/review.o: aegis/review.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/change.h aegis/cstate.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h aegis/pconf.h \
		aegis/commit.h aegis/dir.h common/error.h aegis/file.h \
		aegis/help.h aegis/list.h aegis/lock.h common/mem.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/review.h aegis/sub.h \
		common/trace.h aegis/undo.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/review.c
	mv review.o aegis/review.o

aegis/reviewer.o: aegis/reviewer.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/commit.h common/str.h \
		common/s-v-arg.h common/conf.h common/error.h \
		aegis/help.h aegis/list.h aegis/lock.h common/option.h \
		aegis/os.h aegis/project.h aegis/pstate.h aegis/type.h \
		aegis/parse.h aegis/pattr.h aegis/reviewer.h \
		common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/reviewer.c
	mv reviewer.o aegis/reviewer.o

aegis/sub.o: aegis/sub.c common/arglex.h common/main.h aegis/change.h \
		aegis/cstate.h aegis/type.h common/str.h \
		common/s-v-arg.h common/conf.h aegis/parse.h \
		aegis/pconf.h common/error.h common/mem.h \
		common/option.h aegis/os.h aegis/project.h \
		aegis/pstate.h aegis/pattr.h aegis/sub.h common/trace.h \
		common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/sub.c
	mv sub.o aegis/sub.o

aegis/test.o: aegis/test.c aegis/arglex2.h common/arglex.h \
		common/main.h aegis/change.h aegis/cstate.h \
		aegis/type.h common/str.h common/s-v-arg.h \
		common/conf.h aegis/parse.h aegis/pconf.h \
		aegis/commit.h common/error.h aegis/help.h aegis/list.h \
		aegis/lock.h aegis/log.h common/option.h aegis/os.h \
		aegis/project.h aegis/pstate.h aegis/pattr.h \
		aegis/test.h common/trace.h aegis/user.h aegis/ustate.h \
		aegis/uconf.h common/word.h 
	$(CC) $(CFLAGS)  -Iaegis -Icommon $(H) -c aegis/test.c
	mv test.o aegis/test.o

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

clean:
	rm -f core bin/fmtgen aegis/administ.o aegis/build.o \
		aegis/cattr.c aegis/cattr.h aegis/cattr.o \
		aegis/cattr_ed.o aegis/change.o aegis/chdir.o \
		aegis/col.o aegis/commit.o aegis/common.c \
		aegis/common.h aegis/common.o aegis/copyfile.o \
		aegis/cstate.c aegis/cstate.h aegis/cstate.o \
		aegis/develop1.o aegis/develop2.o aegis/diff.o \
		aegis/dir.o aegis/file.o aegis/glue.o aegis/gonzo.o \
		aegis/gram.gen.c aegis/gram.gen.h aegis/gram.gen.o \
		aegis/gstate.c aegis/gstate.h aegis/gstate.o \
		aegis/help.o aegis/indent.o aegis/integra1.o \
		aegis/integra2.o aegis/io.o aegis/lex.o aegis/list.o \
		aegis/lock.o aegis/log.o aegis/main.o aegis/new_chan.o \
		aegis/new_file.o aegis/new_proj.o aegis/new_rele.o \
		aegis/new_test.o aegis/os.o aegis/pager.o aegis/parse.o \
		aegis/pattr.c aegis/pattr.h aegis/pattr.o \
		aegis/pattr_ed.o aegis/pconf.c aegis/pconf.h \
		aegis/pconf.o aegis/project.o aegis/pstate.c \
		aegis/pstate.h aegis/pstate.o aegis/rem_file.o \
		aegis/rem_proj.o aegis/review.o aegis/reviewer.o \
		aegis/sub.o aegis/test.o aegis/type.o aegis/uconf.c \
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

AegisFiles = aegis/administ.o aegis/build.o aegis/cattr.o \
		aegis/cattr_ed.o aegis/change.o aegis/chdir.o \
		aegis/col.o aegis/commit.o aegis/common.o \
		aegis/copyfile.o aegis/cstate.o aegis/develop1.o \
		aegis/develop2.o aegis/diff.o aegis/dir.o aegis/file.o \
		aegis/glue.o aegis/gonzo.o aegis/gram.gen.o \
		aegis/gstate.o aegis/help.o aegis/indent.o \
		aegis/integra1.o aegis/integra2.o aegis/io.o \
		aegis/lex.o aegis/list.o aegis/lock.o aegis/log.o \
		aegis/main.o aegis/new_chan.o aegis/new_file.o \
		aegis/new_proj.o aegis/new_rele.o aegis/new_test.o \
		aegis/os.o aegis/pager.o aegis/parse.o aegis/pattr.o \
		aegis/pattr_ed.o aegis/pconf.o aegis/project.o \
		aegis/pstate.o aegis/rem_file.o aegis/rem_proj.o \
		aegis/review.o aegis/reviewer.o aegis/sub.o \
		aegis/test.o aegis/type.o aegis/uconf.o aegis/undo.o \
		aegis/user.o aegis/ustate.o aegis/version.o \
		common/ansi.o common/arglex.o common/error.o \
		common/mem.o common/option.o common/str.o \
		common/trace.o common/word.o 

bin/aegis: $(AegisFiles)
	if test ! -d bin; then mkdir bin; fi; exit 0
	$(CC) -o bin/aegis $(AegisFiles) $(LIBRARIES)

sure: t0001a t0002a t0003a t0004a t0005a t0006a t0007a t0008a t0009a \
		t0010a t0011a t0012a t0013a t0014a t0015a 
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
