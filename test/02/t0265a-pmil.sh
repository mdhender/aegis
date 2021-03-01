#!/bin/sh
#
# aegis - project change supervisor.
# Copyright (C) 2012 Peter Miller
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>.
#

TEST_SUBJECT="aecp ind"

# load up standard prelude and test functions
. test_funcs

#
# test the functionality
#
P=bug-four
AEGIS_PROJECT=$P
export AEGIS_PROJECT

AEGIS_PATH=$work/lib
export AEGIS_PATH


SNAPSHOTDIR=snapshots
mkdir $SNAPSHOTDIR || no_result

# take a snapshot of the project baseline
# $1 => branch number
# $2 => change number
snapshot () {
    to=$SNAPSHOTDIR/b$1.c$2
    from=`aegis -cd -p ${P}.$1 -bl`
    cp -rL $from/. $to || no_result
}

#
# set up the project
#
activity="new project 51"
aegis -npr $AEGIS_PROJECT -dir $work/proj -vers - -lib $AEGIS_PATH > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="project staff 55"
aegis -nd $USER > log 2>&1
if test $? -ne 0; then cat log; no_result; fi
aegis -ni $USER > log 2>&1
if test $? -ne 0; then cat log; no_result; fi
aegis -nrv $USER > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="project attributes 63"
cat > pa << 'EOF'
description = "project to test aecp -ind";
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
developers_may_create_changes = true;
umask = 022;
default_test_exemption = true;
default_test_regression_exemption = true;
minimum_change_number = 10;
reuse_change_numbers = false;
minimum_branch_number = 1;
skip_unlucky = false;
compress_database = false;
develop_end_action = goto_awaiting_integration;
protect_development_directory = false;
EOF
if test $? -ne 0; then no_result; fi

aegis -pa -file pa > log 2>&1
if test $? -ne 0; then cat log; no_result; fi
rm -f pa

#
# change set the first, establish project
#
activity="(10) new change 90"
aegis -nc 10 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; no_result; fi
aegis -db 10 -p $AEGIS_PROJECT -dir $work/c10 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

activity="(10) new file 96"
aegis -nf $work/c10/aegis.conf \
    -uuid 01010101-0101-0101-0101-010101010101 \
    > log 2>&1
if test $? -ne 0; then cat log; no_result; fi

cat > $work/c10/aegis.conf << 'EOF'
build_command = "true";
development_directory_style = {
    source_file_symlink = true;
};
merge_command =
    "set +e; "
    "merge -p -L baseline -L C$c ${quote $mostrecent} ${quote $original} "
    "${quote $input} > ${quote $output}; "
    "test $? -le 1";
diff_command =
    "diff -pU10 ${quote $original} ${quote $input} >${quote $output} "
    " || [ $? -eq 1 ] || grep \"^Binary files\" ${quote $output}";
architecture =
[
    {
        name = "unspecified";
        pattern = "*";
    }
];
history_create_command = "fhist ${b $h} -create -cu -i $i -p ${d $h} -r";
history_get_command = "fhist ${b $h} -e '$e' -o $o -p ${d $h}";
history_put_command = "fhist ${b $h} -create -cu -i $i -p ${d $h} -r";
history_query_command = "fhist ${b $h} -l 0 -p ${d $h} -q";
history_content_limitation = international_text;
EOF
if test $? -ne 0; then no_result; fi

activity="(10) finish development 130"
aefinish 10 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi
activity="(10) integrate 133"
aefinish 10 > log 2>&1
if test $? -ne 0; then cat log; no_result; fi


#
# create a branch
#
activity="(1) new branch 141"
aegis -nbr 1 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
AEGIS_PROJECT=$AEGIS_PROJECT.1
export AEGIS_PROJECT

activity="(11) new change 147"
aegis -nc 11 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aegis -db 11 -p $AEGIS_PROJECT -dir $work/1.c11 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(11) new file 153"
aegis -nf $work/1.c11/dir1/file1 \
    -uuid 02020202-0202-0202-0202-020202020202 \
    > log 2>&1
if test $? -ne 0; then cat log; fail; fi
echo create dir1/file1 $AEGIS_PROJECT 11 > $work/1.c11/dir1/file1
if test $? -ne 0; then fail; fi

activity="(11) finish development 161"
aefinish 11 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(11) integrate 165"
aefinish 11 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

# take a snapshot for later
snapshot 1 11

activity="(1) end branch 172"
AEGIS_PROJECT=$P
export AEGIS_PROJECT
aegis -de -c 1 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(1) integrate branch 178"
aefinish 1 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

#
# create another branch, to mess with the files even more
#
activity="(2) new branch 185"
aegis -nbr 2 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
AEGIS_PROJECT=$AEGIS_PROJECT.2
export AEGIS_PROJECT

activity="(12) new change 191"
aegis -nc 12 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aegis -db 12 -p $AEGIS_PROJECT -dir $work/2.c12 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(12) copy file 197"
aegis -cp $work/2.c12/dir1/file1 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

echo "modify dir1/file1 $AEGIS_PROJECT 12" >> $work/2.c12/dir1/file1
if test $? -ne 0; then fail; fi

activity="(12) new file 204"
aegis -nf $work/2.c12/dir2/file2  \
    -uuid 03030303-0303-0303-0303-030303030303 \
    > log 2>&1
if test $? -ne 0; then cat log; fail; fi
echo "create dir2/file2 $AEGIS_PROJECT 12" >> $work/2.c12/dir2/file2
if test $? -ne 0; then fail; fi

activity="(12) finish development 212"
aefinish 12 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(12) integrate 216"
aefinish 12 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

# take a snapshot for later
snapshot 2 12

activity="(2) end branch 223"
AEGIS_PROJECT=$P
aegis -de 2 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aefinish 2 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="verify project state 230"
cat > ok << 'fubar'
src =
[
    {
        file_name = "aegis.conf";
        uuid = "01010101-0101-0101-0101-010101010101";
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
        diff_file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
    },
    {
        file_name = "dir1/file1";
        uuid = "0YYYY202-0202-0202-0202-020202020202";
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
        diff_file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
    },
    {
        file_name = "dir2/file2";
        uuid = "03030303-0303-0303-0303-030303030303";
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
        usage = source;
        file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
        diff_file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
    },
];
fubar
if test $? -ne 0; then fail; fi

check_it()
{
    sed -e "s|$work|...|g" \
        -e 's|= [0-9][0-9]*; /.*|= TIME;|' \
        -e "s/\"$USER\"/\"USER\"/g" \
        -e 's/19[0-9][0-9]/YYYY/' \
        -e 's/20[0-9][0-9]/YYYY/' \
        -e 's/node = ".*"/node = "NODE"/' \
        -e 's/crypto = ".*"/crypto = "GUNK"/' \
        -e 's/uuid = ".*[4-9a-f].*"/uuid = "UUID"/' \
        < $2 > $work/sed.out
    if test $? -ne 0; then no_result; fi
    diff -b $1 $work/sed.out
    if test $? -ne 0; then fail; fi
}

check_it ok $work/proj/info/trunk.fs

#
# create another branch, keep messing with with the files
#
activity="(3) new branch 349"
aegis -nbr 3 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
AEGIS_PROJECT=$AEGIS_PROJECT.3
export AEGIS_PROJECT

activity="(13) new change 355"
aegis -nc 13 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aegis -db 13 -p $AEGIS_PROJECT -dir $work/3.c13 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(13) move file 361"
aegis -mv $work/3.c13/dir1/file1 $work/3.c13/dir2/file1 -c 13 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
echo move dir1/file1 dir2/file1 $AEGIS_PROJECT 13 >> $work/3.c13/dir2/file1
if test $? -ne 0; then fail; fi

activity="(13) finish development 367"
aefinish 13 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
activity="(13) integrate 370"
aefinish 13 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

# take a snapshot for later
snapshot 3 13

activity="(14) new change 377"
aegis -nc 14 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aegis -db 14 -p $AEGIS_PROJECT -dir $work/3.c14 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(14) move file 383"
aegis -cp $work/3.c14/dir2/file1 -c 14 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
echo modify dir2/file1 $AEGIS_PROJECT 14 >> $work/3.c14/dir2/file1
if test $? -ne 0; then fail; fi

activity="(14) finish development 389"
aefinish 14 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
activity="(14) integrate 392"
aefinish 14 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

# take a snapshot for later
snapshot 3 14

activity="(3) end branch 399"
AEGIS_PROJECT=$P
aegis -de 3 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aefinish 3 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="verify project file state 406"
cat > ok << 'fubar'
src =
[
    {
        file_name = "aegis.conf";
        uuid = "01010101-0101-0101-0101-010101010101";
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
        diff_file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
    },
    {
        file_name = "dir1/file1";
        uuid = "0YYYY202-0202-0202-0202-020202020202";
        action = remove;
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
        move = "dir2/file1";
        deleted_by = 3;
    },
    {
        file_name = "dir2/file1";
        uuid = "0YYYY202-0202-0202-0202-020202020202";
        action = create;
        edit =
        {
            revision = "4";
            encoding = none;
            uuid = "UUID";
        };
        edit_origin =
        {
            revision = "4";
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
        diff_file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
        move = "dir1/file1";
    },
    {
        file_name = "dir2/file2";
        uuid = "03030303-0303-0303-0303-030303030303";
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
        usage = source;
        file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
        diff_file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
    },
];
fubar
if test $? -ne 0; then fail; fi

check_it ok $work/proj/info/trunk.fs

#
# make sure aecp -ind will work
#
aegis -cp -indep . -odir=$work/indep1 -p $P > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="verify aecp -ind 533"
( cd $work/indep1 && find . -print | sort ) > test.out
cat > ok << 'fubar'
.
./aegis.conf
./dir2
./dir2/file1
./dir2/file2
fubar
if test $? -ne 0; then fail; fi

diff ok test.out
if test $? -ne 0; then fail; fi

rm -rf $work/indep1

#
# another branch, more files
#
activity="(4) new branch 552"
aegis -nbr 4 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
AEGIS_PROJECT=$AEGIS_PROJECT.4
export AEGIS_PROJECT

activity="(15) new change 558"
aegis -nc 15 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aegis -db 15 -p $AEGIS_PROJECT -dir $work/4.c15 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(15) new file 564"
aegis -nf $work/4.c15/dir3/file3  \
    -uuid 01230123-0123-0123-0123-012301230123 \
    > log 2>&1
if test $? -ne 0; then cat log; fail; fi
echo "create dir3/file3 $AEGIS_PROJECT 15" >> $work/4.c15/dir3/file3
if test $? -ne 0; then fail; fi

activity="(15) finish development 572"
aefinish 15 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(15) integrate 576"
aefinish 15 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

# take a snapshot for later
snapshot 4 15

activity="(4) end branch 583"
AEGIS_PROJECT=$P
aegis -de 4 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aefinish 4 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="verify project state 590"
cat > ok << 'fubar'
src =
[
    {
        file_name = "aegis.conf";
        uuid = "01010101-0101-0101-0101-010101010101";
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
        diff_file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
    },
    {
        file_name = "dir1/file1";
        uuid = "0YYYY202-0202-0202-0202-020202020202";
        action = remove;
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
        move = "dir2/file1";
        deleted_by = 3;
    },
    {
        file_name = "dir2/file1";
        uuid = "0YYYY202-0202-0202-0202-020202020202";
        action = create;
        edit =
        {
            revision = "4";
            encoding = none;
            uuid = "UUID";
        };
        edit_origin =
        {
            revision = "4";
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
        diff_file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
        move = "dir1/file1";
    },
    {
        file_name = "dir2/file2";
        uuid = "03030303-0303-0303-0303-030303030303";
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
        usage = source;
        file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
        diff_file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
    },
    {
        file_name = "dir3/file3";
        uuid = "01230123-0123-0123-0123-012301230123";
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
        usage = source;
        file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
        diff_file_fp =
        {
            youngest = TIME;
            oldest = TIME;
            crypto = "GUNK";
        };
    },
];
fubar
if test $? -ne 0; then fail; fi

check_it ok $work/proj/info/trunk.fs

#
# Now check the contents of the files.
#
activity="verify aecp -ind 744"
aegis -cp -indep . -odir $work/indep2 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

( cd $work/indep2 && find . -print | sort ) > test.out
cat > ok << 'fubar'
.
./aegis.conf
./dir2
./dir2/file1
./dir2/file2
./dir3
./dir3/file3
fubar
if test $? -ne 0; then fail; fi

diff ok test.out
if test $? -ne 0; then fail; fi

cat > ok << fubar
create dir1/file1 ${P}.1 11
modify dir1/file1 ${P}.2 12
move dir1/file1 dir2/file1 ${P}.3 13
modify dir2/file1 ${P}.3 14
fubar
if test $? -ne 0; then fail; fi

diff ok indep2/dir2/file1
if test $? -ne 0; then fail; fi

cat > ok << fubar
create dir2/file2 ${P}.2 12
fubar
if test $? -ne 0; then fail; fi

diff ok indep2/dir2/file2
if test $? -ne 0; then fail; fi

cat > ok << fubar
create dir3/file3 ${P}.4 15
fubar
if test $? -ne 0; then fail; fi

diff ok indep2/dir3/file3
if test $? -ne 0; then fail; fi

rm -rf $work/indep2

activity="(5) new branch 792"
aegis -nbr 5 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
AEGIS_PROJECT=$AEGIS_PROJECT.5
export AEGIS_PROJECT

activity="(16) new change 798"
aegis -nc 16 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aegis -db 16 -p $AEGIS_PROJECT -dir $work/5.c16 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(16) copy and modify file 804"
aegis -cp $work/5.c16/dir3/file3 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
echo modify dir3/file3 $AEGIS_PROJECT 16 >> $work/5.c16/dir3/file3
if test $? -ne 0; then fail; fi

activity="(16) develop end 810"
aefinish 16 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
activity="(16) integrate 813"
aefinish 16 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

# take a snapshot for later
snapshot 5 16

activity="(17) new change 820"
aegis -nc 17 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aegis -db 17 -p $AEGIS_PROJECT -dir $work/5.c17 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(17) move file 826"
aegis -mv $work/5.c17/dir2/file1 $work/5.c17/dir3/file1 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
echo move dir2/file1 dir3/file1 $AEGIS_PROJECT 17 >> $work/5.c17/dir3/file1
if test $? -ne 0; then fail; fi

activity="(17) develop end 832"
aefinish 17 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
activity="(17) integrate 835"
aefinish 17 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

# take a snapshot for later
snapshot 5 17

activity="(5) develop end branch 842"
AEGIS_PROJECT=$P
aegis -de -p $AEGIS_PROJECT -c 5 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
activity="(5) integrate branch 846"
aefinish 5 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="confirm source files 850"
aelsf -bl > test.out
if test $? -ne 0; then fail; fi

cat > ok << 'fubar'
aegis.conf
dir2/file2
dir3/file1
dir3/file3
fubar
diff ok test.out
if test $? -ne 0; then fail; fi

activity="(6) new branch 863"
aegis -nbr 6 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
AEGIS_PROJECT=$AEGIS_PROJECT.6
export AEGIS_PROJECT

activity="(18) new change 869"
aegis -nc 18 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aegis -db 18 -p $AEGIS_PROJECT -dir $work/6.c18 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(18) modify file 875"
aegis -cp 18 -p $AEGIS_PROJECT $work/6.c18/dir3/file3 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
echo modify dir3/file3 $AEGIS_PROJECT 18 >> $work/6.c18/dir3/file3
if test $? -ne 0; then fail; fi

activity="(18) end development 881"
aefinish 18 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
activity="(18) integrate 884"
aefinish 18 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

# take a snapshot for later
snapshot 6 18


activity="(19) new change 892"
aegis -nc 19 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aegis -db 19 -p $AEGIS_PROJECT -dir $work/6.c19 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(19) move file 898"
aegis -mv 19 -p $AEGIS_PROJECT \
    $work/6.c19/dir2/file2 $work/6.c19/dir3/file2 \
    > log 2>&1
if test $? -ne 0; then cat log; fail; fi
echo move dir2/file2 dir3/file2 $AEGIS_PROJECT 19 >> $work/6.c19/dir3/file2
if test $? -ne 0; then fail; fi

activity="(19) end development 906"
aefinish 19 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
activity="(19) integrate 909"
aefinish 19 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

# take a snapshot for later
snapshot 6 19


activity="(6) end branch dev 917"
AEGIS_PROJECT=$P
aegis -de 6 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(6) integrate branch 922"
aefinish 6 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="confirm source files 926"
aelsf -bl > test.out
if test $? -ne 0; then fail; fi

cat > ok << 'fubar'
aegis.conf
dir3/file1
dir3/file2
dir3/file3
fubar
diff ok test.out
if test $? -ne 0; then fail; fi


activity="(7) new branch 940"
aegis -nbr 7 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
AEGIS_PROJECT=$AEGIS_PROJECT.7
export AEGIS_PROJECT


activity="(20) new change 947"
aegis -nc 20 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aegis -db 20 -p $AEGIS_PROJECT -dir $work/7.c20 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(20) modify file 953"
aegis -cp -baserel dir3/file2 -c 20 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
echo modify dir3/file2 $AEGIS_PROJECT 20 >> $work/7.c20/dir3/file2
if test $? -ne 0; then fail; fi

activity="(20) finish development 959"
aefinish 20 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
activity="(20) integrate 962"
aefinish 20 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

# take a snapshot for later
snapshot 7 20


activity="(21) new change 970"
aegis -nc 21 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aegis -db 21 -p $AEGIS_PROJECT -dir $work/7.c21 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(21) modify file 976"
aegis -cp -baserel dir3/file2 -c 21 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
echo modify dir3/file2 $AEGIS_PROJECT 21 >> $work/7.c21/dir3/file2
if test $? -ne 0; then fail; fi

activity="(21) finish development 982"
aefinish 21 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
activity="(21) integrate 985"
aefinish 21 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

# take a snapshot for later
snapshot 7 21


activity="(7) end branch dev 993"
AEGIS_PROJECT=$P
aegis -de 7 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(7) integrate branch 998"
aefinish 7 > log 2>&1
if test $? -ne 0; then cat log; fail; fi


activity="(8) new branch 1003"
aegis -nbr 8 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
AEGIS_PROJECT=$AEGIS_PROJECT.8
export AEGIS_PROJECT


activity="(22) new change 1010"
aegis -nc 22 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aegis -db 22 -p $AEGIS_PROJECT -dir $work/8.c22 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(22) modify file 1016"
aegis -cp -baserel dir3/file3 -c 22 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
echo modify dir3/file3 $AEGIS_PROJECT 22 >> $work/8.c22/dir3/file3
if test $? -ne 0; then fail; fi

activity="(22) finish development 1022"
aefinish 22 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
activity="(22) integrate 1025"
aefinish 22 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

# take a snapshot for later
snapshot 8 22


activity="(23) new change 1033"
aegis -nc 23 -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0; then cat log; fail; fi
aegis -db 23 -p $AEGIS_PROJECT -dir $work/8.c23 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(23) modify file 1039"
aegis -cp -baserel dir3/file1 -c 23 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
echo modify dir3/file1 $AEGIS_PROJECT 23 >> $work/8.c23/dir3/file1
if test $? -ne 0; then fail; fi

activity="(23) finish development 1045"
aefinish 23 > log 2>&1
if test $? -ne 0; then cat log; fail; fi
activity="(23) integrate 1048"
aefinish 23 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

# take a snapshot for later
snapshot 8 23


activity="(8) end branch dev 1056"
AEGIS_PROJECT=$P
aegis -de 8 > log 2>&1
if test $? -ne 0; then cat log; fail; fi

activity="(8) integrate branch 1061"
aefinish 8 > log 2>&1
if test $? -ne 0; then cat log; fail; fi


# make sure snapshot and aecp -ind agree
# $1 => branch number
# $2 => change number
verify() {
    activity="verify, branch $1, change $2 1070"
    aegis -cp -p ${P}.$1 --delta-from-change $2 \
        -indep . -odir verify.$1.$2 > log 2>&1
    if test $? -ne 0; then cat log; fail; fi
    # this assumes gnu diff
    diff -urN -x \*,D -x aegis.log $SNAPSHOTDIR/b$1.c$2 verify.$1.$2
    test $? -eq 0 || fail
}

verify 1 11
verify 2 12
verify 3 13
verify 3 14
verify 4 15
verify 5 16
verify 5 17
verify 6 18
verify 6 19
verify 7 20
verify 7 21
verify 8 22
verify 8 23

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
# vim: set ts=8 sw=4 et :
