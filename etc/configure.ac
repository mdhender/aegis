dnl
dnl     aegis - a project change supervisor
dnl     Copyright (C) 1994, 1995, 1997-1999, 2001-2008 Peter Miller
dnl     Copyright (C) 2006-2008 Walter Franzini
dnl
dnl     This program is free software; you can redistribute it and/or
dnl     modify it under the terms of the GNU General Public License as
dnl     published by the Free Software Foundation; either version 3 of
dnl     the License, or (at your option) any later version.
dnl
dnl     This program is distributed in the hope that it will be useful,
dnl     but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl     GNU General Public License for more details.
dnl
dnl     You should have received a copy of the GNU General Public License
dnl     along with this program. If not, see
dnl     <http://www.gnu.org/licenses/>.
dnl
AC_INIT(install-sh)
AC_CONFIG_HEADER(common/config.h)
AC_PROG_CXX
AC_PROG_CPP
AC_CANONICAL_HOST
AC_LANG(C++)
AC_GNU_SOURCE

dnl
dnl     Look for an existing Aegis installation, and use the "aegis
dnl     --configured" command to replicate its configuration.
dnl     Unless they said --with-no-aegis-configured
dnl
if test "x$prefix" = "xNONE"
then
    if test "x$with_no_aegis_configured" = "x"; then
        cfgval=`aegis --configured 2> /dev/null`
        if test $? -eq 0; then
            eval "$cfgval"
        else
            cfgval=`which aegis 2> /dev/null`
            if test $? -ne 0; then
                case "$cfgval" in
                /bin/aegis )
                    prefix=/
                    ;;
                */bin/aegis )
                    prefix=`echo $cfgval | sed 's|/bin/aegis$||'`
                    ;;
                esac
            fi
        fi
    fi
fi

AC_PROG_INSTALL
AC_PROG_YACC

if test "$YACC" = "yacc" -a ! -f /usr/bin/yacc
then
  AC_MSG_RESULT([
        You must have the bison command installed in order
        to build Aegis.
        Homepage: http://www.gnu.org/software/bison/])
  OK=no
  if apt-get --version > /dev/null 2> /dev/null; then
    AC_MSG_RESULT([
        The following command may be used to install it:
        sudo apt-get install bison
    ])
    OK=yes
  fi
  if yum --version > /dev/null 2> /dev/null; then
    AC_MSG_RESULT([
        The following command may be used to install it:
        sudo yum install bison
    ])
    OK=yes
  fi
  if test "$OK" != "yes"; then
    AC_MSG_RESULT([
        If you are using a package based install, you will need the
        bison package.
    ])
  fi
  exit 1
fi

AC_PROG_RANLIB
AC_MINIX
AC_ISC_POSIX
AC_OBJEXT
AC_EXEEXT

dnl
dnl Large file support.
dnl
AC_FUNC_FSEEKO
AC_SYS_LARGEFILE

dnl
dnl     DO NOT check for -ldmalloc, using
dnl         AC_CHECK_LIB(dmalloc, dmalloc_malloc)
dnl     or similar.
dnl
dnl     The dmalloc library can be instructed to log to a file,
dnl     circumventing the Aegis I/O layer, thus it's possible to create
dnl     file owned by root.  The dmalloc library should only ever be
dnl     used as a debugging tool, and NEVER be used in a production build
dnl     of Aegis.
dnl

dnl Solaris 2.5.1 and below wide-character support
AC_CHECK_FUNC(wschr, ,
        unset ac_cv_func_wschr
        AC_CHECK_LIB(w, wschr))

dnl
dnl we need to check for libiconv before we check for libintl
dnl
AC_CHECK_LIB(iconv, iconv, ,
    unset ac_cv_func_iconv)

dnl
dnl Solaris needs to use the GNU Gettext to have any chance of
dnl having working error message translation.  So we look for -lintl
dnl unconditionally.
dnl
AC_CHECK_LIB(intl, gettext, ,
  [
    unset ac_cv_func_gettext
    AC_CHECK_LIB(gettext, gettext, ,
      unset ac_cv_func_gettext)
  ]
)

AC_CHECK_FUNC(setsockopt, ,
        unset ac_cv_func_setsockopt
        AC_CHECK_LIB(socket, setsockopt))
dnl
dnl Check the presence of libm.  We use additional parentheses around
dnl the name of the function to avoid problems if atan2 is *also*
dnl defined as a macro.
dnl
AC_CHECK_LIB(m, (atan2))
AC_CHECK_LIB(rx, regcomp)
AC_CHECK_LIB(z, gzopen, ,
  [
    AC_MSG_RESULT([
        You must have the zlib compression library installed in order
        to build Aegis.  Please install zlib, and then re-run this
        ./configure script.
        Homepage: http://www.zlib.net/])
    OK=no
    if apt-get --version > /dev/null 2> /dev/null; then
      AC_MSG_RESULT([
        The following command may be used to install it:
        sudo apt-get install zlib1g-dev
      ])
      OK=yes
    fi
    if yum --version > /dev/null 2> /dev/null; then
      AC_MSG_RESULT([
        The following command may be used to install it:
        sudo yum install zlib-devel
      ])
      OK=yes
    fi
    if test "$OK" != "yes"; then
      AC_MSG_RESULT([
        If you are using a package based install, you will need the
        zlib-devel package in addition to the zlib package.
      ])
    fi
    exit 1
  ]
)
AC_CHECK_LIB(bz2, BZ2_bzopen, ,
  [
    AC_MSG_RESULT([
        You must have the BZip2 compression library installed in order
        to build Aegis.  Please install the bzip2 library, and then
        re-run this ./configure script.
        Homepage: http://www.bzip.org/])
    OK=no
    if apt-get --version > /dev/null 2> /dev/null; then
      AC_MSG_RESULT([
        The following command may be used to install it:
        sudo apt-get install libbz2-dev bzip2
      ])
      OK=yes
    fi
    if yum --version > /dev/null 2> /dev/null; then
      AC_MSG_RESULT([
        The following command may be used to install it:
        sudo yum install libbz2-devel bzip2
      ])
      OK=yes
    fi
    if test "$OK" != "yes"; then
      AC_MSG_RESULT([
        If you are using a package based install, you will need the
        bzip2-devel package in addition to the bzip package.
      ])
    fi
    exit 1
  ]
)
AC_CHECK_LIB(crypt, crypt)
AC_CHECK_LIB(magic, magic_file, ,
  [
    AC_MSG_RESULT([
        You make like to have the file magic number library installed in
        order to build Aegis.  This will result is superior Content-Type
        information being inserted at several points in the code.
        Homepage: http://www.darwinsys.com/file/])
    OK=no
    if apt-get --version > /dev/null 2> /dev/null; then
      AC_MSG_RESULT([
        The following command may be used to install it:
        sudo apt-get install libmagic-dev
      ])
      OK=yes
    fi
    if yum --version > /dev/null 2> /dev/null; then
      AC_MSG_RESULT([
        The following command may be used to install it:
        sudo yum install libmagic-devel
      ])
      OK=yes
    fi
    if test "$OK" != "yes"; then
      AC_MSG_RESULT([
        If you are using a package based install, you will need the
        libmagic-devel package in addition to the libmagic package.
      ])
    fi
  ]
)
dnl The -lclu library is for OSF/1 clustering
AC_CHECK_LIB(clu, clu_info)

STATIC_LIBS=

dnl
dnl Look for static libraries necessary with -lcurl, but we don't need
dnl them when we are using shared libraries.
dnl
AC_CHECK_PROGS(CURL_CONFIG, curl-config)

if test -n "$CURL_CONFIG"
then
    CFLAGS="$CFLAGS `$CURL_CONFIG --cflags`"
    CXXFLAGS="$CXXFLAGS `$CURL_CONFIG --cflags`"
    STATIC_LIBS="`$CURL_CONFIG --libs`"
fi

dnl
dnl The curl library is used to read from arbitrary URLs.
dnl
AC_CHECK_LIB(curl, curl_multi_perform, [],
  [
    AC_MSG_RESULT([
        You make like to have the multi-protocol file transfer library
        installed in order to build Aegis.
        Homepage: http://curl.haxx.se/])
    OK=no
    if apt-get --version > /dev/null 2> /dev/null; then
      AC_MSG_RESULT([
        One of the following commands may be used to install it:
        sudo apt-get install libcurl4-gnutls-dev
        sudo apt-get install libcurl3-dev
      ])
      OK=yes
    fi
    if yum --version > /dev/null 2> /dev/null; then
      AC_MSG_RESULT([
        The following command may be used to install it:
        sudo yum install libcurl-devel
      ])
      OK=yes
    fi
    if test "$OK" != "yes"; then
      AC_MSG_RESULT([
        If you are using a package based install, you will need the
        libcurl-devel package in addition to the libcurl package.
      ])
    fi
  ],
  [$STATIC_LIBS])

dnl
dnl Look for static libraries necessary with -lxml2, but we don't need
dnl them when we are using shared libraries.
dnl
AC_CHECK_PROGS(XML2_CONFIG, xml2-config)

if test -n "$XML2_CONFIG"
then
    CFLAGS="$CFLAGS `$XML2_CONFIG --cflags`"
    CXXFLAGS="$CXXFLAGS `$XML2_CONFIG --cflags`"
    STATIC_LIBS="$STATIC_LIBS `$XML2_CONFIG --libs` -lpthread"
fi

dnl
dnl The libxml2 library is used by the aerevml(1) command to parse its
dnl input files.
dnl
AC_CHECK_LIB(xml2, xmlTextReaderRead, [],
  [
    AC_MSG_RESULT([
        You must have the Gnome libxml2 library installed in order to
        build Aegis.  Please install the xml2 library version 1.8.17 or
        later, and then re-run this ./configure script.  You do not have
        to install the rest of Gnome, the library can be used on its own.
        Homepage: http://xmlsoft.org/])
    OK=no
    if apt-get --version > /dev/null 2> /dev/null; then
      AC_MSG_RESULT([
        The following command may be used to install it:
        sudo apt-get install libxml2-dev
      ])
      OK=yes
    fi
    if yum --version > /dev/null 2> /dev/null; then
      AC_MSG_RESULT([
        The following command may be used to install it:
        sudo yum install libxml2-devel
      ])
      OK=yes
    fi
    if test "$OK" != "yes"; then
      AC_MSG_RESULT([
        If you are using a package based install, you will need the
        libxml2-devel package in addition to the libxml2 package.
      ])
    fi
    exit 1
  ],
  [$STATIC_LIBS]
)

dnl
dnl Arrange for a special make variable for static builds.
dnl
AC_SUBST(STATIC_LIBS)

dnl
dnl Try to detect a library with the UUID functions, where available
dnl (FreeBSD) prefer libc.
dnl
dnl The OSSP variant has uuid_load in -luuid
dnl     and which include file?
dnl     *** later we check for uuid_create, uuid_make and uuid_export
dnl
dnl The e2fsprogs variant has uuid_generate in -luuid
dnl     and which include file?
dnl     *** later we check for uuid_generate and uuid_unparse
dnl
dnl The DCE compliant variant has uuid_hash in -luuid
dnl     The include file is <dce/uuid.h>
dnl     *** later we check for uuid_create and uuid_to_string
dnl
dnl The BSD variant has uuid_has in libc (no -l required)
dnl     The include file is <uuid.h>
dnl     *** it is supposed to be the same as the DCE variant.  Is it?
AC_CHECK_LIB(c, uuid_hash,,
AC_CHECK_LIB(uuid, uuid_load)
AC_CHECK_LIB(uuid, uuid_generate)
AC_CHECK_LIB(uuid, uuid_hash)
)

dnl
dnl If there are no UUID libraries we can roll our own, but first we
dnl need to see if the Linux kernel will do it for us.
dnl
AC_MSG_CHECKING([for Linux /proc/sys/kernel/random/uuid])
HAVE_LINUX_UUID=no
if test -f "/proc/sys/kernel/random/uuid"
then
        AC_DEFINE(HAVE_LINUX_UUID, 1,
                  [Define this symbol if your system has
/proc/sys/kernel/random/uuid])
        AC_DEFINE(LINUX_UUID_FILENAME,["/proc/sys/kernel/random/uuid"],
                [Define this symbol if your system has
/proc/sys/kernel/random/uuid])
        AC_DEFINE(LINUX_UUID_SIZE, 36,
                [Define this symbol if your system has
/proc/sys/kernel/random/uuid])
        HAVE_LINUX_UUID=yes
fi
AC_MSG_RESULT([$HAVE_LINUX_UUID])

AC_CHECK_PROGS(MSGFMT, gmsgfmt msgfmt)
AC_CHECK_PROGS(MSGCAT, gmsgcat msgcat)
AC_CHECK_PROGS(GROFF, groff roff)
AC_CHECK_PROGS(SOELIM, roffpp gsoelim soelim)
AC_CHECK_PROGS(REFER, grefer refer)
AC_PROG_AWK

dnl
dnl We need some way to determine the value of the default MANPATH
dnl setting, so that we can append to it.  There are (at least) two
dnl incompatible man implementations out there.  The one used by Debian
dnl Linux has a "manpath -q" command, the one used by RedHat Linux has
dnl a "man -w" command.  If neither works, assume a constant string.
dnl
AC_CHECK_PROGS(MANPATH_PROG, manpath)

if test -n "$MANPATH_PROG"
then
    dnl
    dnl Sometimes manpath prints irritating warnings,
    dnl but not all versions have -q to silence them.
    dnl
    MANPATH_PROG="$MANPATH_PROG 2>/dev/null"
else
    dnl
    dnl Some versions of man have a -w option, but it's short for --whatis
    dnl and so they print nothing on stdout, and an error message about
    dnl the missing argument on stderr.  Some versions of man -w do the
    dnl same thing as manpath.
    dnl
    temp_str=`man -w 2>/dev/null`
    if test -n "$temp_str"
    then
        MANPATH_PROG="man -w 2>/dev/null"
    else
        MANPATH_PROG="echo /usr/man:/usr/share/man"
    fi
fi

AC_CHECK_PROGS(LOCALE_PROG, locale, , [${PATH}:/usr/sbin:/sbin])
if test -n "$LOCALE_PROG"
then
    AC_DEFINE(HAVE_LOCALE_PROG, 1,
    [Define this symbol if your system has the locale(1) program.])
fi

AC_CHECK_PROGS(LOCALE_GEN_PROG, locale-gen, , [${PATH}:/usr/sbin:/sbin])
if test -n "$LOCALE_GEN_PROG"
then
    AC_DEFINE(HAVE_LOCALE_GEN_PROG, 1,
    [Define this symbol if your system has the locale-gen(8) program.])
fi

AC_PATH_PROG(WISH, wish, /usr/local/bin/wish)
AC_PATH_PROG(VI_PROG, vi, /bin/vi)

AC_PATH_PROG(PERL, perl, /usr/local/bin/perl)
AC_DEFINE_UNQUOTED(CONF_PERL, ["$PERL"],
[Set this to be the absolute path of a Perl interpreter.])

dnl
dnl     See if we are using GNU diff
dnl
AC_CHECK_PROGS(DIFFsimple, gdiff diff, diff)
AC_PATH_PROG(DIFF, $DIFFsimple, /usr/local/bin/$DIFFsimple)
AC_DEFINE_UNQUOTED(CONF_DIFF, ["$DIFF"],
[Set this to be the absolute path of the GNU diff command.])
AC_MSG_CHECKING([for GNU diff])
HAVE_GNU_DIFF=no
if $DIFF --version > conftest.out 2>&1
then
    if grep 'GNU diff' conftest.out > conftest.err 2>&1
    then
        AC_DEFINE(HAVE_GNU_DIFF, 1,
            [Define this symbol of your system has GNU diff.])
        HAVE_GNU_DIFF=yes
    fi
fi
AC_MSG_RESULT([$HAVE_GNU_DIFF])

dnl
dnl     Find the patch program, and check whether or not it understands
dnl     the -f and -s options.
dnl
AC_CHECK_PROGS(PATCHsimple, gpatch patch)
AC_PATH_PROG(PATCH, $PATCHsimple, /usr/local/bin/$PATCHsimple)
AC_DEFINE_UNQUOTED(CONF_PATCH, ["$PATCH"],
[Set this to be the absolute path of the GNU diff command.])
AC_MSG_CHECKING([for GNU patch])
HAVE_GNU_PATCH=no
if $PATCH -f -s < /dev/null > conftest.out 2>&1
then
    AC_DEFINE(HAVE_GNU_PATCH, 1,
        [Define this symbol of your system has GNU patch.])
    HAVE_GNU_PATCH=yes
fi
AC_MSG_RESULT([$HAVE_GNU_PATCH])

dnl
dnl     Test if groff takes -ms or -mgs for the macro package.
dnl
AC_MSG_CHECKING([for $GROFF -ms macros])
if test `echo ' ' | groff -mgs 2> /dev/null | wc -l` -gt 0
then
        GROFF_MS_MACROS=gs
else
        GROFF_MS_MACROS=s
fi
AC_SUBST(GROFF_MS_MACROS)
AC_MSG_RESULT([-m$GROFF_MS_MACROS])

dnl
dnl     Test if groff takes -mm or -mgm for the macro package.
dnl
AC_MSG_CHECKING([for $GROFF -mm macros])
if test `echo ' ' | groff -mgm 2> /dev/null | wc -l` -gt 0
then
        GROFF_MM_MACROS=gm
else
        GROFF_MM_MACROS=m
fi
AC_SUBST(GROFF_MM_MACROS)
AC_MSG_RESULT([-m$GROFF_MM_MACROS])

dnl
dnl     Test to see where TIOCGWINSZ is defined.
dnl
AC_MSG_CHECKING([for TIOCGWINSZ])
AC_TRY_RUN([
#include <sys/ioctl.h>
int main(int argc, char **argv){
#ifdef TIOCGWINSZ
struct winsize window_size; window_size.ws_col = 0; return(window_size.ws_col);
#else
return(1);
#endif
}]
, AC_DEFINE(HAVE_winsize_SYS_IOCTL_H,1,
[Define this symbol if your system as <sys/ioctl.h> AND it
defines TIOCGWINSZ and struct winsize.])
AC_MSG_RESULT([sys/ioctl.h]),
AC_MSG_RESULT([not found]),
AC_MSG_RESULT(cross))

dnl
dnl     Test to see where struct winsize is defined.
dnl
AC_MSG_CHECKING([for struct winsize])
AC_TRY_RUN([
#include <termios.h>
int main(int argc, char **argv){
#ifdef TIOCGWINSZ
struct winsize window_size; window_size.ws_col = 0; return(window_size.ws_col);
#else
return(1);
#endif
}]
, AC_DEFINE(HAVE_winsize_TERMIOS_H,1,
[Define this symbol if your system as <termios.h> AND it
defines TIOCGWINSZ and struct winsize.])
AC_MSG_RESULT([termios.h]),
AC_MSG_RESULT([not found]),
AC_MSG_RESULT(cross))

dnl
dnl     Test to see if stdarg.h is available *and* works.
dnl
AC_MSG_CHECKING([for working stdarg.h])
AC_TRY_RUN([
#include <stdarg.h>
char *foo = "test";
#if defined(__GNUC__) || __STDC__
int test(char*,...);
#endif
int test(fmt)char*fmt;{va_list ap;char*a;int x;
va_start(ap,fmt);a=va_arg(ap,char*);x=(a!=foo);va_end(ap);return x;}
int main(int argc, char **argv){
return(test("",foo));}]
, AC_DEFINE(HAVE_STDARG_H,1,
[Define this symbol of your system has <stdarg.h> AND it works.])
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no),
AC_MSG_RESULT(cross))

dnl
dnl     Test to see if mblen is available *and* works.
dnl
AC_MSG_CHECKING([for working mblen])
AC_TRY_RUN([
#include <stdlib.h>
int main(){mblen(0,0);
return(mblen("OK", 2) != 1);}],
AC_DEFINE(HAVE_MBLEN,1,
[Define this symbol of your system has mblen AND it works.])
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no),
AC_MSG_RESULT(cross))

AC_CHECK_HEADERS(            \
        bits/c++locale.h     \
        boost/shared_ptr.hpp \
        bzlib.h              \
        crypt.h              \
        curl/curl.h          \
        curl/multi.h         \
        dce/rpc.h            \
        dce/uuid.h           \
        execinfo.h           \
        fcntl.h              \
        iso646.h             \
        libgettext.h         \
        libintl.h            \
        limits.h             \
        locale.h             \
        magic.h              \
        memory.h             \
        mntent.h             \
        new.h                \
        regex.h              \
        rxposix.h            \
        shadow.h             \
        stddef.h             \
        stdlib.h             \
        string.h             \
        sys/clu.h            \
        sys/ioctl.h          \
        sys/prctl.h          \
        sys/uuid.h           \
        tr1/memory           \
        unistd.h             \
        uuid.h               \
        uuid/uuid.h          \
        wchar.h              \
        wctype.h             \
        widec.h              \
        zlib.h               \
	)

#
# Look for the C++ <new> header.
#
AC_MSG_CHECKING([for <new>])
AC_COMPILE_IFELSE([#include <new>],
AC_DEFINE(HAVE_HEADER_NEW, 1,
    [Define this symbol of your system has the <new> C++ header])
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no))

AC_HEADER_DIRENT
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_C_CONST
AC_SYS_LONG_FILE_NAMES
AC_CHECK_FUNCS(backtrace clu_info crypt curl_easy_strerror             \
        curl_multi_strerror getpgrp igetspnam initgroups gettext       \
        gettimeofday iswctype lutime magic_file mbrtowc memmem         \
        pathconf prctl regcomp rpc_string_free seteuid setlocale       \
        setresuid setreuid sighold sigrelse sigprocmask snprintf       \
        strcasecmp strendcpy strlcat strlcpy strncasecmp strerror      \
        strftime strsignal strtoul strverscmp symlink tcgetpgrp        \
        uuid_create uuid_export uuid_generate uuid_hash uuid_load      \
        uuid_make uuid_to_string uuid_unparse vsnprintf wcrtomb wcslen \
        wcswidth wcwidth xmlReaderForIO                                \
	)
AC_CHECK_DECLS([crypt],,, [
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_CRYPT_H
#include <crypt.h>
#endif
])
AC_CHECK_DECLS([mbrtowc, wcrtomb, wcwidth, wcswidth],,, [
#if HAVE_WCHAR_H
#include <wchar.h>
#endif
])
dnl the strsignal function is usually defined in <string.h>
dnl but is sometimes defined in <unistd.h>
AC_CHECK_DECLS([strsignal],,, [
#if HAVE_STRING_H
#include <string.h>
#include <unistd.h>
#endif
])

dnl
dnl	Check to see if wint_t is defined.
dnl	The ANSI C standard states that this symbol shall be defined
dnl	by <wchar.h> and <wctype.h>.  The GNU people also define it in
dnl	<stddef.h>, but this is incorrect.
dnl
AC_MSG_CHECKING([for wint_t])
AC_TRY_COMPILE([
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
],
[ wint_t x; ],
AC_DEFINE(HAVE_WINT_T,1,
[Define this symbol of your system has the wint_t type defined.
It is usually in <stddef.h> or <wctype.h>.])
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no))dnl

dnl
dnl	Check to see if mbstate_t is defined.
dnl
AC_MSG_CHECKING([for mbstate_t])
AC_TRY_COMPILE([
#include <ctype.h>
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif
/* Solaris bug 1250837: include wchar.h before widec.h */
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#ifdef HAVE_WIDEC_H
#include <widec.h>
#endif
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif
],
[ mbstate_t x; return sizeof x;],
AC_DEFINE(HAVE_MBSTATE_T,1,
[Define this symbol of your system has the mbstate_t type defined.
 It is usually in <stddef.h> or <wchar.h>.])
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no))dnl

dnl
dnl	Test to see if iswprint is available *and* works.
dnl
AC_MSG_CHECKING([for working iswprint])
AC_TRY_RUN([
#include <ctype.h>
/* Solaris bug 1250837: include wchar.h before widec.h */
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#ifdef HAVE_WIDEC_H
#include <widec.h>
#endif
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif
int main(int argc, char **argv){
return(iswprint('a') == 0);}]
, AC_DEFINE(HAVE_ISWPRINT,1,
[Define this symbol of your system has iswprint() AND it works.])
AC_MSG_RESULT(yes),
AC_MSG_RESULT(no),
AC_MSG_RESULT(cross))dnl

dnl
dnl	Test to see if getpgrp() or getpgrp(0) should be used to
dnl	discover the process group of the current process.
dnl
AC_MSG_CHECKING([for appropriate getpgrp argument])
AC_TRY_RUN([
int main(int argc, char **argv) {
#ifdef HAVE_GETPGRP
if (getpgrp(32767) == getpgrp(0)) return(2);
#endif
return(0); }]
, AC_DEFINE(CONF_getpgrp_arg, 0,
[Set this to a suitable argument for the getpgrp function to discover
the process group of the current process.])
AC_MSG_RESULT(zero),
AC_DEFINE(CONF_getpgrp_arg, [])
AC_MSG_RESULT(empty),
AC_MSG_RESULT(cross))

dnl
dnl	Test to see if the pw_comment field exists in
dnl	the passwd struct defined in the <pwd.h> include file.
dnl
AC_MSG_CHECKING([for pw_comment in struct passwd])
AC_TRY_COMPILE([
#include <pwd.h>
#include <stdio.h>
], [
struct passwd *pw;
pw = getpwnam("root");
printf("%s\n", pw->pw_comment);
],
AC_DEFINE(HAVE_pw_comment,1,
[Define this symbol if your system has
the pw_comment field in the passwd struct defined in <pwd.h>.])
AC_MSG_RESULT(yes),AC_MSG_RESULT(no))dnl

dnl
dnl	Test to see if the tm_zone field exists in
dnl	the passwd tm defined in the <time.h> include file.
dnl
AC_MSG_CHECKING([for tm_zone in struct tm])
AC_TRY_COMPILE([
#include <stdio.h>
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif], [struct tm *tm;
printf("%s\n", tm->tm_zone);],
AC_DEFINE(HAVE_tm_zone,1,
[Define this symbol if your system has
the tm_zone field in the tm struct defined in <time.h>.])
AC_MSG_RESULT(yes),AC_MSG_RESULT(no))dnl

dnl
dnl	Test to find a Bourne shell which understands functions
dnl
AC_MSG_CHECKING([for a Bourne shell which understands functions])
if test "z$SH" = "z"; then
    if test -f /bin/sh5; then
	SH=/bin/sh5
    else
	SH=/bin/sh
    fi
fi
AC_SUBST(SH)
AC_DEFINE_UNQUOTED(CONF_SHELL, ["$SH"],
[Set this to be the absolute path of a Bourne shell
which understands functions.])
AC_MSG_RESULT($SH)

dnl
dnl Evaluate some of the variables, to remove ${prefix} references.
dnl This way, they can be used in C programs and Roff input.
dnl Make sure that aegis is mentioned in the libdir and datadir paths;
dnl add it if it is not already there.
dnl
test "x$prefix" = xNONE && prefix="${ac_default_prefix-/usr/local}"
test "x$exec_prefix" = xNONE && exec_prefix="$prefix"
eval "bindir=$bindir"
eval "datarootdir=$datarootdir"
eval "datadir=$datadir"
eval "libdir=$libdir"
eval "mandir=$mandir"
eval "sharedstatedir=$sharedstatedir"
case "$datadir" in
*/aegis/* | */aegis )
	;;
*)
	datadir="${datadir}/aegis"
	;;
esac
case "$datarootdir" in
*/aegis/* | */aegis )
	;;
*)
	datarootdir="${datarootdir}/aegis"
	;;
esac
case "$libdir" in
*/aegis/* | */aegis )
	;;
*)
	libdir="${libdir}/aegis"
	;;
esac
case "$sharedstatedir" in
*/aegis/* | */aegis )
	;;
*)
	sharedstatedir="${sharedstatedir}/aegis"
	;;
esac
eval "sysconfdir=$sysconfdir"

dnl support for NLSDIR option
AC_ARG_WITH(nlsdir,
	[  --with-nlsdir=PATH	specify where the locale stuff should go ])

if test "x$NLSDIR" = "x"; then
	if test "x$with_nlsdir" != "x"; then
		NLSDIR=$with_nlsdir
	else
		NLSDIR=${libdir}
	fi
fi

AC_SUBST(NLSDIR)

dnl
dnl	See if there is already existing library files
dnl	Use uid/gid if so, otherwise use 3/3
dnl
dnl	Cygwin always runs in "single user" mode, so this test isn't
dnl	useful.  Just default the values to the preferred ones.
dnl
if test "x$AEGIS_UID" = "x"; then
    AC_MSG_CHECKING([for owner of aegis library files])
    AEGIS_UID=3
    AEGIS_GID=3
    if test "$OSTYPE" != "cygwin32" -a "$OSTYPE" != "cygwin" ; then
    if test -f ${sharedstatedir}/lockfile -o -f ${libdir}/lockfile ; then
	cat > conftest.$ac_ext << 'EOF'
#line __oline__ "configure"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
main(int ac, char **av){struct stat st;
if (stat(*(av+1),&st)!=0)return(1);
printf("%d\n", ac>2?st.st_gid:st.st_uid);return(0);}
EOF
	if AC_TRY_EVAL(ac_link); then
		if maybe=`./conftest ${sharedstatedir}/lockfile`; then
			AEGIS_UID=$maybe
		else
			if maybe=`./conftest ${libdir}/lockfile`; then
				AEGIS_UID=$maybe
			fi
		fi
		if maybe=`./conftest ${sharedstatedir}/lockfile gid`; then
			AEGIS_GID=$maybe
		else
			if maybe=`./conftest ${libdir}/lockfile gid`; then
				AEGIS_GID=$maybe
			fi
		fi
	fi
	rm -rf conftest*
    fi
    fi
    AC_SUBST(AEGIS_UID)
    AC_SUBST(AEGIS_GID)
    AC_MSG_RESULT([$AEGIS_UID, $AEGIS_GID])
fi

dnl
dnl	If the libz.h is not present, Aegis will fail to compile.
dnl	If the -lz is not present, Aegis will fail to link.
dnl
if test "$ac_cv_lib_z_gzopen" != "yes" -o "$ac_cv_header_zlib_h" != "yes"
then
  AC_MSG_RESULT([
	You need to install the GNU zlib compression
	library, and then re-run this configure script.
	Remember to use \`\`rm config.cache'' before you do.])
fi

dnl
dnl If the curl/curl.h and -lcurl are not present, remote files are
dnl inaccessable.
dnl
if test "$ac_cv_lib_curl_curl_multi_perform" != "yes" -o \
    "$ac_cv_header_curl_curl_h" != "yes"
then
  AC_MSG_RESULT([
        You may wish to install the Curl library, used to access
        remote files.  Don't Panic: this library is optional, nothing
        will break, but you will not be able to use all of aedist's
        functions.  If you choose to install the Curl library, first
        use \`\`rm config.cache'', and then re-run this configure
        script.])
fi

dnl
dnl	Warn them about UUID libraries.
dnl
UUID_OK=0
if test "$ac_cv_func_uuid_generate" = "yes" \
    -a "$ac_cv_func_uuid_unparse" = "yes"
then
    UUID_OK=1
fi
if test "$ac_cv_func_uuid_create" = "yes" \
    -a "$ac_cv_func_uuid_make" = "yes" \
    -a "$ac_cv_func_uuid_export" = "yes"
then
    UUID_OK=1
fi
if test "$ac_cv_func_uuid_create" = "yes" \
    -a "$ac_cv_func_uuid_to_string" = "yes"
then
    UUID_OK=1
fi
AC_DEFINE_UNQUOTED([UUID_OK], [$UUID_OK],
    [Define this symbol if your system has a working UUID implementation.])
if test "$UUID_OK" != "1"
then
  AC_MSG_RESULT([
	You may wish to install one of the following UUID
	library flavors:
	 * e2fsprogs implementation
	 * OSSP implementation
	Don't Panic: this library is optional, nothing will break.])
    OK=no
    if apt-get --version > /dev/null 2> /dev/null; then
      AC_MSG_RESULT([
        ONE of the following commands may be used to install it:
        sudo apt-get install uuid-dev
        sudo apt-get install libossp-uuid-dev
      ])
      OK=yes
    fi
    if yum --version > /dev/null 2> /dev/null; then
      AC_MSG_RESULT([
        The following command may be used to install it:
        sudo yum install uuid-devel
      ])
      OK=yes
    fi
fi

dnl
dnl	If the gettext function exists, assume the msgfmt program does
dnl	also, and arrange for the Makefile to install the .mo files
dnl	from the .po files.
dnl
po_files=no
if test "$ac_cv_func_gettext" = "yes"
then
  if test -n "$MSGFMT"
  then
    po_files=yes
  fi
fi
if test "$po_files" = "no"
then
  AC_MSG_RESULT([
	Aegis will emit much more useful error messages if
	you install GNU Gettext and then re-run this
	./configure script.
	Remember to use \`\`rm config.cache'' before you do.
        Homepage: http://www.gnu.org/software/gettext/])
  OK=no
  if apt-get --version > /dev/null 2> /dev/null; then
    AC_MSG_RESULT([
        The following command may be used to install it:
        sudo apt-get install gettext
    ])
    OK=yes
  fi
  if yum --version > /dev/null 2> /dev/null; then
    AC_MSG_RESULT([
        The following command may be used to install it:
        sudo yum install gettext
    ])
    OK=yes
  fi
  if test "$OK" != "yes"; then
    AC_MSG_RESULT([
        If you are using a package based install, you will need the
        gettext package.
    ])
  fi
fi
AC_SUBST(po_files)

dnl
dnl	If the soelim program exists, and understands the -I option,
dnl	arrange for the Makefile to install the .ps files from the
dnl	documentation source files.
dnl
if test -n "$SOELIM"
then
  if $SOELIM -I. /dev/null > /dev/null 2>&1
  then
    : nothing
  else
    AC_MSG_RESULT([
	The $SOELIM program does not understand the -I option.
	GNU Groff 1.15 or later works correctly.
	See the BUILDING file for details.])
    GROFF=
  fi
else
  GROFF=
fi
if test -n "$REFER"
then
    : nothing
else
    # It turns out that somehow Debian systems get a half-assed groff
    # install from build-essential.  Looking for "refer" is a better
    # test of the completeness of the groff install than either
    # "groff" or "soelim" on their own, plus it's needed for the build.
    GROFF=
fi
if test -n "$GROFF"
then
  HAVE_GROFF=yes
else
  HAVE_GROFF=no
  AC_MSG_RESULT([
	The Aegis documentation set and manual pages will
	be formatted and installed if you first install
	GNU Groff 1.15 or later and then re-run this ./configure script.
	Remember to use \`\`rm config.cache'' before you do.
        Homepage: http://www.gnu.org/software/groff/])
  OK=no
  if apt-get --version > /dev/null 2> /dev/null; then
    AC_MSG_RESULT([
        The following command may be used to install it:
        sudo apt-get install groff
    ])
    OK=yes
  fi
  if yum --version > /dev/null 2> /dev/null; then
    AC_MSG_RESULT([
        The following command may be used to install it:
        sudo yum install groff
    ])
    OK=yes
  fi
  if test "$OK" != "yes"; then
    AC_MSG_RESULT([
        If you are using a package based install, you will need the
        groff package.
    ])
  fi
fi
AC_SUBST(HAVE_GROFF)

if test "$HAVE_GNU_DIFF" != "yes"
then
  AC_MSG_RESULT([
        Some of the tests in the Aegis test suite (133, 136, 167, 168,
        180, 195) assume the presence of GNU diff.  Some functionality
        will be more limited without it.  You may wish to install GNU
        Diff 2.8 or later and then re-run this ./configure script.
	Remember to use \`\`rm config.cache'' before you do.
])
fi

dnl
dnl If the $host is Linux then we add -rdynamic to LD_FLAGS in order
dnl to make make backtrace return function names.
dnl
case $host in
     i?86-*-gnu*)
	LDFLAGS="$LDFLAGS -rdynamic" ;;
esac
AC_SUBST(LDFLAGS)

# program prefix is the bit to add to the start of the name
if test ${PROGRAM_PREFIX-NONE} != NONE -a ${program_prefix-NONE} != NONE; then
PROGRAM_PREFIX=${program_prefix}
fi
AC_SUBST(PROGRAM_PREFIX)

# program suffix is the bit to add to the end of the name (before .exe)
if test ${PROGRAM_SUFFIX-NONE} != NONE -a ${program_suffix-NONE} != NONE; then
PROGRAM_SUFFIX=${program_suffix}
fi
AC_SUBST(PROGRAM_SUFFIX)

AH_TOP(
[/*
 *	aegis - project change supervisor
 *	Copyright (C) 1991-2008 Peter Miller
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program. If not, see
 *	<http://www.gnu.org/licenses/>.
 */

#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H
])

AH_BOTTOM(
[/*
 * This defines the file creation mask.  See umask(2) for more information.
 * Some bits are not available, because aegis is _meant_ to be paranoid.
 * Owner: always has read, write and search/exec.
 * Group: always has read and search/exec, so that developers can get at the
 *	baseline.  There is never group write, because then developers
 *	could trash the baseline, which is counter-productive.
 * Others: There is never others write, for the same reason as group.
 *	Others read and search/execute is configurable.
 *
 * The permissions mask in binary looks like
 *	000 010 X1X
 * where the Xs may be configured.
 *
 * Alternatives for default umask are thus
 *	027	others get nothing
 *	026	others can execute the results if they know where they are going
 *	022	others can see and copy and execute anything
 *	023	don't do this (why is left as an exersize for the reader)
 *
 * Projects have a configurable umask, this is just the default.
 *	See aepattr(1) for more information.
 */
#define DEFAULT_UMASK 026

/*
 * These symbols define where non-system user IDs start,
 * and where non-system group IDs start.
 * This is mostly to ensure that accounts "root" and "bin"
 * and "uucp" are not project owners.
 */
#define AEGIS_MIN_UID 100
#define AEGIS_MIN_GID 10

/*
 * There is more to do, but we need to insulate it from config.status,
 * because it screws up the #undef lines.  They are all implications of
 * the above information, so there is no need for you to edit the file,
 * if you are configuring Aegis manually.
 */
#include <common/config.messy.h>

#endif /* COMMON_CONFIG_H */])

AC_OUTPUT(Makefile lib/cshrc lib/profile etc/libdir.so
	common/libdir.cc etc/compat.2.3)
