#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2001, 2002, 2004-2008, 2012 Peter Miller
#       Copyright (C) 2008 Walter Franzini
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
# Note: this test assumes we are talking to GNU Diff, because we use
# the -a option to tell it to treat all files as ASCII, even if they
# look binary.
#

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
umask 022

if test -z "`diff -v 2>&1 | grep GNU`"
then
        echo ''
        echo '  This test depends on GNU Diff, which you do not seem to'
        echo '  have installed.  This test is declared to pass by default.'
        echo ''
        exit 0
fi

LINES=24
export LINES
COLS=80
export COLS

USER=${USER:-${LOGNAME:-`whoami`}}

work=${AEGIS_TMP:-/tmp}/$$
PAGER=cat
export PAGER
AEGIS_FLAGS="delete_file_preference = no_keep; \
        lock_wait_preference = always; \
        diff_preference = automatic_merge; \
        pager_preference = never; \
        persevere_preference = all; \
        log_file_preference = never;"
export AEGIS_FLAGS
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

bin=$here/${1-.}/bin

if test "$EXEC_SEARCH_PATH" != ""
then
    tpath=
    hold="$IFS"
    IFS=":$IFS"
    for tpath2 in $EXEC_SEARCH_PATH
    do
        tpath=${tpath}${tpath2}/${1-.}/bin:
    done
    IFS="$hold"
    PATH=${tpath}${PATH}
else
    PATH=${bin}:${PATH}
fi
export PATH

pass()
{
        set +x
        echo PASSED 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 0
}
fail()
{
        set +x
        echo "FAILED test of the history encoding functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo "NO RESULT when testing the history encoding functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 2
}
trap \"no_result\" 1 2 3 15

check_it()
{
        sed     -e "s|$work|...|g" \
                -e 's|= [0-9][0-9]*; /.*|= TIME;|' \
                -e "s/\"$USER\"/\"USER\"/g" \
                -e 's/delta[0-9][0-9]*/delta/' \
                -e 's/19[0-9][0-9]/YYYY/' \
                -e 's/20[0-9][0-9]/YYYY/' \
                -e 's/node = ".*"/node = "NODE"/' \
                -e 's/crypto = ".*"/crypto = "GUNK"/' \
                -e 's/uuid = ".*"/uuid = "UUID"/' \
                < $1 > $work/sed.out
        if test $? -ne 0; then fail; fi
        diff -b -a $2 $work/sed.out
        if test $? -ne 0; then fail; fi
}

#
# some variable to make things earier to read
#
PAGER=cat
export PAGER

AEGIS_FLAGS="delete_file_preference = no_keep; \
        lock_wait_preference = always; \
        diff_preference = automatic_merge; \
        pager_preference = never; \
        persevere_preference = all; \
        log_file_preference = never;"
export AEGIS_FLAGS
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

activity="make directories 130"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
worklib=$work/lib
chmod 777 $worklib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

chandir=$work/chan
workproj=$work/proj

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# test the history encoding functionality
#
activity="new project 153"
$bin/aegis -newpro foo -version - -dir $workproj -lib $worklib

#
# change project attributes
#
activity="project attributes 159"
cat > pa << 'fubar'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
developers_may_create_changes = true;
default_test_exemption = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -proatt -f pa -proj foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

activity="project staff 171"
$bin/aegis -nd $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nrv $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi
$bin/aegis -ni $USER -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

#
# create a new change
#
activity="new change 182"
cat > ca << 'end'
brief_description = "This change is used to test the aegis functionality \
with respect to change descriptions.";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 1 -f ca -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

activity="develop begin 192"
$bin/aegis -db 1 -project foo -lib $worklib  -dir $chandir
if test $? -ne 0 ; then no_result; fi

activity="new file 196"
$bin/aegis -nf $chandir/aegis.conf $chandir/qp $chandir/b64 -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

cat > $chandir/aegis.conf << 'end'
build_command =
        "exit 0";

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";

diff_command =
        "set +e; diff -a $orig $i > $out; test $$? -le 1";
diff3_command =
        "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
        echo '1,$$p' ) | ed - $mr > $out";
end
if test $? -ne 0 ; then no_result; fi

cat > qp.in << 'end'
Transfer-Encoding: quoted-printable

This is a simple =01 binary file.
end
if test $? -ne 0 ; then no_result; fi

$bin/test_base64 -qp -i qp.in qp.out
if test $? -ne 0 ; then no_result; fi

cp qp.out $chandir/qp
if test $? -ne 0 ; then no_result; fi

cat > b64.in << 'end'
Transfer-Encoding: quoted-printable

This is a complex binary file.
=83=B9=42=F7=8E=11=6D=47=49=4F=08=F9=EC=39=5B=E5=97=80=1D=93=57=62=68=B8=AD=7D=
=7C=F7=A6=E6=17=4F=AA=68=A3=CB=BD=EA=61=2A=D1=FF=59=92=FC=B1=CB=3E=81=D9=ED=BE=
=34=4F=EF=51=9D=08=5A=D5=D9=1F=1F=FF=88=E1=D2=67=9E=12=A7=2A=94=8E=83=E7=49=01=
=C8=91=06=D4=DE=E7=91=E7=5F=ED=88=F1=1A=0B=CD=89=00=A3=E4=B1=72=D1=BC=DD=CA=0E=
=06=AC=22=3C=06=8F=C8=0D=AD=50=60=91=E9=6E=31=13=EE=AA=A4=96=BF=36=11=40=14=94=
=C1=B0=05=2D=E7=FA=89=E8=9A=DA=CF=72=C6=E0=DF=67=69=FE=F0=30=0E=97=B8=34=BA=4F=
=CF=66=B7=CD=82=01=FA=D8=34=C2=A5=E9=31=38=65=66=82=C7=8D=FD=F6=40=87=13=2D=25=
=83=5E=0A=5F=DA=AE=58=C1=E0=71=D8=D6=05=1E=29=26=6D=AB=A2=6D=87=AC=80=72=96=F1=
=E5=6D=74=06=C2=EB=54=1A=93=A4=AF=7A=C2=D3=A0=60=F7=5A=DF=A1=F6=FC=11=CB=A7=E6=
=EA=A0=CB=9C=3B=8D=ED=0E=10=A4=73=C4=6A=7D=F2=53=80=67=B7=8C=67=CA=21=2D=03=5D=
=CF=83=23=AF=5C=66=7D=F3=80=B1=82=09=85=12=06=F9=35=62=0F=D5=8B=9C=E2=5F=3B=39=
=A0=61=C9=88=DF=66=8C=F9=91=74=84=C9=8E=D1=33=97=47=8B=14=04=26=92=CB=CD=78=3E=
=65=48=BE=D6=AE=1A=01=90=54=1C=9A=2F=CA=3F=14=0A=11=83=4F=5E=E7=47=48=18=51=B2=
=1A=B5=A1=19=CE=AA=6F=83=19=4E=FF=C6=95=BB=98=FA=36=19=9F=FA=66=76=9F=DE=09=17=
=D3=79=59=E7=AD=13=C2=A7=94=08=01=C9=EE=15=88=F4=A4=F5=62=C5=1F=0B=0A=BD=BD=09=
=79=DC=F2=9B=36=6F=EC=4E=8C=6E=5B=B1=FE=A7=BB=A6=D6=17=92=CD=06=CC=E0=28=34=45=
=6B=8E=F0=5E=14=D8=56=C6=82=84=38=FC=83=97=0A=A7=71=44=F1=27=6C=23=84=ED=6C=F8=
=51=9A=F3=82=E4=05=45=41=BE=4E=3E=78=94=A2=B2=C4=CE=1A=85=C7=C4=71=3F=C8=18=AC=
=77=48=02=E9=4B=4D=39=2D=50=DF=26=28=37=07=64=25=E5=92=2C=A7=C9=F2=C5=05=86=3F=
end
if test $? -ne 0 ; then no_result; fi

$bin/test_base64 -qp -i b64.in b64.out
if test $? -ne 0 ; then no_result; fi

cp b64.out $chandir/b64
if test $? -ne 0 ; then no_result; fi

activity="build 266"
$bin/aegis -build -c 1 -project foo -lib $worklib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="diff 270"
$bin/aegis -diff -c 1 -project foo -lib $worklib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop end 274"
$bin/aegis -de -c 1 -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

activity="review pass 278"
$bin/aegis -rpass -c 1 -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

activity="integrate begin 282"
$bin/aegis -ib -c 1 -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

activity="build 286"
$bin/aegis -build -c 1 -project foo -lib $worklib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate pass 290"
$bin/aegis -ipass -c 1 -project foo -lib $worklib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

# check the encoding of the files
activity="check trunk state 295"
cat > ok << 'end'
src =
[
        {
                file_name = "aegis.conf";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
                };
                usage = config;
                file_fp =
                {
                        youngest = TIME;
                        oldest = TIME;
                        crypto = "GUNK";
                };
        },
        {
                file_name = "b64";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = base64;
                        uuid = "UUID";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = base64;
                        uuid = "UUID";
                };
                usage = source;
                file_fp =
                {
                        youngest = TIME;
                        oldest = TIME;
                        crypto = "GUNK";
                };
        },
        {
                file_name = "qp";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = quoted_printable;
                        uuid = "UUID";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = quoted_printable;
                        uuid = "UUID";
                };
                usage = source;
                file_fp =
                {
                        youngest = TIME;
                        oldest = TIME;
                        crypto = "GUNK";
                };
        },
];
end
if test $? -ne 0 ; then no_result; fi

check_it $workproj/info/trunk.fs ok

activity="check change state 371"
cat > ok << 'end'
src =
[
        {
                file_name = "aegis.conf";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
                };
                usage = config;
        },
        {
                file_name = "b64";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = base64;
                        uuid = "UUID";
                };
                usage = source;
        },
        {
                file_name = "qp";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = quoted_printable;
                        uuid = "UUID";
                };
                usage = source;
        },
];
end
if test $? -ne 0 ; then no_result; fi

check_it $workproj/info/change/0/001.fs ok

#
# create a new change
#
activity="new change 417"
cat > ca << 'end'
brief_description = "This change is used to stomp on file contents.";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 2 -f ca -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

activity="develop begin 426"
$bin/aegis -db 2 -project foo -lib $worklib  -dir $chandir
if test $? -ne 0 ; then no_result; fi

activity="copy file 430"
$bin/aegis -cp $chandir/qp $chandir/b64 -c 2 -p foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

cat > $chandir/b64 << 'end'
Stomp on file.
end
if test $? -ne 0 ; then no_result; fi

cat > $chandir/qp << 'end'
Stomp on file.
end
if test $? -ne 0 ; then no_result; fi

activity="build 444"
$bin/aegis -build -c 2 -project foo -lib $worklib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="diff 448"
$bin/aegis -diff -c 2 -project foo -lib $worklib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="develop end 452"
$bin/aegis -de -c 2 -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

activity="review pass 456"
$bin/aegis -rpass -c 2 -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

activity="integrate begin 460"
$bin/aegis -ib -c 2 -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

activity="build 464"
$bin/aegis -build -c 2 -project foo -lib $worklib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; no_result; fi

activity="integrate pass 468"
$bin/aegis -ipass -c 2 -project foo -lib $worklib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

activity="check trunk state 472"
cat > ok << 'end'
src =
[
        {
                file_name = "aegis.conf";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
                };
                edit_origin =
                {
                        revision = "1";
                        encoding = none;
                        uuid = "UUID";
                };
                usage = config;
                file_fp =
                {
                        youngest = TIME;
                        oldest = TIME;
                        crypto = "GUNK";
                };
        },
        {
                file_name = "b64";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "UUID";
                };
                edit_origin =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "UUID";
                };
                usage = source;
                file_fp =
                {
                        youngest = TIME;
                        oldest = TIME;
                        crypto = "GUNK";
                };
        },
        {
                file_name = "qp";
                uuid = "UUID";
                action = create;
                edit =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "UUID";
                };
                edit_origin =
                {
                        revision = "2";
                        encoding = none;
                        uuid = "UUID";
                };
                usage = source;
                file_fp =
                {
                        youngest = TIME;
                        oldest = TIME;
                        crypto = "GUNK";
                };
        },
];
end
if test $? -ne 0 ; then no_result; fi

check_it $workproj/info/trunk.fs ok

#
# create a new change
#
activity="new change 551"
cat > ca << 'end'
brief_description = "This change is used to decode old history versions.";
cause = internal_bug;
end
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc 3 -f ca -project foo -lib $worklib
if test $? -ne 0 ; then no_result; fi

activity="develop begin 560"
$bin/aegis -db 3 -project foo -lib $worklib  -dir $chandir
if test $? -ne 0 ; then no_result; fi

activity="copy file 564"
$bin/aegis -cp $chandir/qp -c 3 -delta 1 -p foo -lib $worklib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi
$bin/aegis -cp $chandir/b64 -c 3 -delta 1 -p foo -lib $worklib > LOG 2>&1
if test $? -ne 0 ; then cat LOG; fail; fi

activity="check change state 570"
cat > ok << 'end'
src =
[
        {
                file_name = "b64";
                uuid = "UUID";
                action = modify;
                edit_origin =
                {
                        revision = "1";
                        encoding = base64;
                        uuid = "UUID";
                };
                usage = source;
        },
        {
                file_name = "qp";
                uuid = "UUID";
                action = modify;
                edit_origin =
                {
                        revision = "1";
                        encoding = quoted_printable;
                        uuid = "UUID";
                };
                usage = source;
        },
];
end
if test $? -ne 0 ; then no_result; fi

check_it $workproj/info/change/0/003.fs ok

#and check that the files decode OK
activity="check decoding 603"
cmp b64.out $chandir/b64
if test $? -ne 0 ; then fail; fi
diff -b qp.out $chandir/qp
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
