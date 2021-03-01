#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2006-2009, 2012 Peter Miller
#       Copyright (C) 2007 Walter Franzini
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

unset AEGIS_PROJECT
unset AEGIS_CHANGE
unset AEGIS_PATH
unset AEGIS
umask 022

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
        log_file_preference = never; \
        default_development_directory = \"$work\";"
export AEGIS_FLAGS
AEGIS_THROTTLE=-1
export AEGIS_THROTTLE

# This tells aeintegratq that it is being used by a test.
AEGIS_TEST_DIR=$work
export AEGIS_TEST_DIR

here=`pwd`
if test $? -ne 0 ; then exit 2; fi

parch=
test "$1" != "" && parch="$1/"
bin="$here/${parch}bin"

if test "$EXEC_SEARCH_PATH" != ""
then
    tpath=
    hold="$IFS"
    IFS=":$IFS"
    for tpath2 in $EXEC_SEARCH_PATH
    do
        tpath=${tpath}${tpath2}/${parch}bin:
    done
    IFS="$hold"
    PATH=${tpath}${PATH}
else
    PATH=${bin}:${PATH}
fi
export PATH

AEGIS_DATADIR=$here/lib
export AEGIS_DATADIR

#
# set the path, so that the aegis command that aepatch/aedist invokes
# is from the same test set as the aepatch/aedist command itself.
#
PATH=${bin}:$PATH
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
        echo "FAILED test of the html_quote href functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo "NO RESULT when testing the html_quote href functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 2
}
trap \"no_result\" 1 2 3 15

activity="create test directory"
mkdir $work $work/lib
if test $? -ne 0 ; then no_result; fi
chmod 777 $work/lib
if test $? -ne 0 ; then no_result; fi
cd $work
if test $? -ne 0 ; then no_result; fi

#
# use the built-in error messages
#
AEGIS_MESSAGE_LIBRARY=$work/no-such-dir
export AEGIS_MESSAGE_LIBRARY
unset LANG
unset LANGUAGE

#
# If the C++ compiler is called something other than "c++", as
# discovered by the configure script, create a shell script called
# "c++" which invokes the correct C++ compiler.  Make sure the current
# directory is in the path, so that it will be invoked.
#
if test "$CXX" != "c++"
then
        cat >> $work/c++ << fubar
#!/bin/sh
exec ${CXX-g++} \$*
fubar
        if test $? -ne 0 ; then no_result; fi
        chmod a+rx $work/c++
        if test $? -ne 0 ; then no_result; fi
        PATH=${work}:${PATH}
        export PATH
fi

#
# test the html_quote href functionality
#
AEGIS_PATH=$work/lib
export AEGIS_PATH

activity="new project 172"
$bin/aegis -npr test -version - -v -dir $work/proj.dir \
        -lib $AEGIS_PATH > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT

activity="project attributes 180"
cat > paf << fubar
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
develop_end_action = goto_awaiting_integration;
default_development_directory = "$work";
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 193"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 201"
cat > caf << 'fubar'
brief_description = "one";
description = "Make sure external links like "
"http://aegis.sf.net/ "
"work properly.\n"
"\n"
"Second style www.google.com\n"
"Third style mailto:pmiller@opensource.org.au\n"
"Fourth style pmiller@opensource.org.au\n"
;
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p $AEGIS_PROJECT > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 211"
$bin/aegis -db 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 215"
$bin/aegis -nf $work/test.C010/aegis.conf $work/test.C010/fred/wilma \
        $work/test.C010/barney -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.C010/aegis.conf << 'fubar'
build_command = "exit 0";

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "exit 0";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
        echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
fubar
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/fred/wilma
if test $? -ne 0 ; then no_result; fi

echo one > $work/test.C010/barney
if test $? -ne 0 ; then no_result; fi

cat > test.ok << 'fubar'
Content-Type: text/html
Content-Length: NNNN

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN""http://www.w3.org/TR/REC-html40/loose.dtd">
<html><head>
<meta name="ROBOTS" content="NOINDEX, NOFOLLOW">
<meta name="GENERATOR" content="aegis-NNNN">
<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<style type="text/css">
tr.even-group { background-color: #CCCCCC; }
body { background-color: white; }
</style>
<link rel="stylesheet" type="text/css" href="/aedefault.css" media="all">
<link rel="stylesheet" type="text/css" href="/test.css" media="all">
<title>Project
test, Change 10
</title></head><body>
<h1 align=center>
<a href="http://localhost/cgi-bin/aeget">Project</a>
&ldquo;<a href="http://localhost/cgi-bin/aeget/test/?menu">test</a>&rdquo;,<br>
Change 10</h1>
<div class="brief-description">
<h2>Brief Description</h2>
one</div>
<div class="description">
<h2>Description</h2>
Make sure external links like <A HREF="http://aegis.sf.net/">http://aegis.sf.net/</A> work properly.
<p>
Second style <A HREF="http://www.google.com">www.google.com</A><br>
Third style <A HREF="mailto:pmiller@opensource.org.au">mailto:pmiller@opensource.org.au</A><br>
Fourth style <A HREF="mailto:pmiller@opensource.org.au">pmiller@opensource.org.au</A></div>
<div class="information">
<h2>Information Available</h2>
<dl>
<dt>State<dd>This change is in the <dfn>being_developed</dfn> state.
<p>
<dt><a href="http://localhost/cgi-bin/aeget/test.C10/?download">Download</a><dd>
This item will provide you with links to several different
download formats.  Select the one appropriate to your needs.
<p>
<dt><a href="http://localhost/cgi-bin/aeget/test.C10/?files">Files</a><dd>
This item will provide you with a listing of files which
are being created, modified or deleted by this change.
<p>
<dt><a href="http://localhost/cgi-bin/aeget/test.C10/?file-inventory">File Inventory</a><dd>
This item will provide you with a listing of files
in this change along with their UUIDs.
<p>
<dt><a href="http://localhost/cgi-bin/aeget/test.C10/">Development Directory</a><dd>
This item will provide you with access to the files in
the development directory.
This will be unioned with all baselines
and presented as a single directory tree.
<p>
If you wish to download the sources using
&ldquo;wget -r&rdquo; or similar, use the
<i><a href="http://localhost/cgi-bin/aeget/test.C10/?file+contents+noindex+nolinks+noderived">no navigation links</a></i> variant.
<p>
<dt><a href="http://localhost/cgi-bin/aeget/test.C10/?activity">File Activity</a><dd>
This item will provide you with a listing of files which
are being modified in this change <em>and also</em>
in other changes.  The list includes who is working on
the changes, and a brief description of each change.
<p>
<dt><a href="http://localhost/cgi-bin/aeget/test.C10/?conflict">File Conflict</a><dd>
This item will provide you with a list of changes which
are actively modifying files in common with this change
<strong>if</strong> more than <em>one</em> change is
modifying the file at the same time.
<p>
<dt><a href="http://localhost/cgi-bin/aeget/test.C10/?file+history">File History</a><dd>
This item will provide you with a listing of all completed
changes which affected the files in this change.  The
<a href="http://localhost/cgi-bin/aeget/test.C10/?file+history+detailed">detailed version</a> can take a long time to generate.
<p>
<dt><a href="http://localhost/cgi-bin/aeget/test.C10/?change+history">History</a><dd>
This item will provide you with a listing of the state
transitions of this change, when they were performed,
and who performed them.
</dl></div>
<hr>
<p align=center class="navbar">[
<a href="http://localhost/cgi-bin/aeget/">Project List</a> |
<a href="http://localhost/cgi-bin/aeget/test/?menu">Project Menu</a> |
<a href="http://localhost/cgi-bin/aeget/test/?changes">Change List</a>
]</p>
<hr>
This page was generated by <em>aeget</em>
version XXXX
on XXXX.
</body></html>
fubar
if test $? -ne 0 ; then no_result; fi

PATH_INFO=/${AEGIS_PROJECT}.c10 \
QUERY_STRING=menu \
SCRIPT_NAME=/cgi-bin/aeget \
REQUEST_METHOD=GET \
$bin/aeget > test.out.raw
if test $? -ne 0 ; then fail; fi

sed \
    -e 's|^version .*|version XXXX|' \
    -e 's|^on .*[.]$|on XXXX.|' \
    -e 's|^Content-Length: .*$|Content-Length: NNNN|' \
    -e 's|content="aegis-[^"]*">$|content="aegis-NNNN">|' \
    test.out.raw > test.out
if test $? -ne 0 ; then no_result; fi

diff test.ok test.out
if test $? -ne 0 ; then fail; fi

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass


# vim: set ts=8 sw=4 et :
