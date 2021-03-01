#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2005 Matthew Lee;
#       All rights reserved.
#       Copyright (C) 2007 Peter Miller
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 2 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program; if not, write to the Free Software
#       Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
#
# MANIFEST: Test the rss_item functionality
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

if test "$1" != "" ; then bin="$here/$1/bin"; else bin="$here/bin"; fi

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
        echo "FAILED test of the rss_item functionality ($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 1
}
no_result()
{
        set +x
        echo "NO RESULT when testing the rss_item functionality \
($activity)" 1>&2
        cd $here
        find $work -type d -user $USER -exec chmod u+w {} \;
        rm -rf $work
        exit 2
}
trap "no_result" 1 2 3 15

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

AEGIS_PROJECT=test
export AEGIS_PROJECT
AEGIS_PATH=$work/lib
export AEGIS_PATH


strip_served()
{
    sed -e "s|Content-Length: [0-9][0-9]*|Content-Length: NNN|" \
        -e "s|<lastBuildDate>[A-Za-z0-9,: ]*|<lastBuildDate>XXX|" \
        -e "s|<pubDate>[A-Za-z0-9,: ]*|<pubDate>XXX|" \
        -e "s|<guid isPermaLink=\"false\">[a-z0-9-]*|\
<guid isPermaLink=\"false\">XXX|" < $1 > $2;
}

check_one()
{
    strip_served $1 aeget.served.stripped

    USERNAME=`$bin/aesub '${user name}' -c 10 -p test`
    EMAIL_ADDRESS=`$bin/aesub '${user email}' -c 10 -p test | \
	sed -e 's/</\&lt;/' -e 's/>/\&gt;/'`
    VERSION=`$bin/aegis -version | head -1 | awk '{print $3}'`

    cat > aeget.served.expected << EOF
Content-Type: application/rss+xml
Content-Length: NNN

<?xml version="1.0"?>
<rss version="2.0">
<channel>
<title>Project test, Title of test channel</title>
<description>Description of test channel</description>
<language>en-AU</language>
<link>http://localhost/cgi-bin/aeget/test/?menu</link>
<lastBuildDate>XXX</lastBuildDate>
<generator>aegis $VERSION</generator>
<docs>http://blogs.law.harvard.edu/tech/rss</docs>
<item>
<title>D001 - &amp;lt;one&amp;gt; - completed</title>
<description>Description of changeset &amp;lt;one&amp;gt;</description>
<pubDate>XXX</pubDate>
<link>http://localhost/cgi-bin/aeget/test.C10/?menu</link>
<author>$EMAIL_ADDRESS</author>
<guid isPermaLink="false">XXX</guid>
</item>
</channel>
</rss>
EOF

    activity="check_one"
    diff aeget.served.expected aeget.served.stripped > log 2>&1;
    if test $? -ne 0 ; then cat log; fail ; fi
}

check_two()
{
    strip_served $1 aeget.served.stripped

    USERNAME=`$bin/aesub '${user name}' -c 11 -p test`
    EMAIL_ADDRESS=`$bin/aesub '${user email}' -c 11 -p test | \
	sed -e 's/</\&lt;/' -e 's/>/\&gt;/' `
    VERSION=`$bin/aegis -version | head -1 | awk '{print $3}'`

    cat > aeget.served.expected << EOF
Content-Type: application/rss+xml
Content-Length: NNN

<?xml version="1.0"?>
<rss version="2.0">
<channel>
<title>Project test, Title of test channel</title>
<description>Description of test channel</description>
<language>en-AU</language>
<link>http://localhost/cgi-bin/aeget/test/?menu</link>
<lastBuildDate>XXX</lastBuildDate>
<generator>aegis $VERSION</generator>
<docs>http://blogs.law.harvard.edu/tech/rss</docs>
<item>
<title>C011 - two - awaiting_development</title>
<description>Description of change&amp;amp;set &amp;quot;two&amp;quot;</description>
<pubDate>XXX</pubDate>
<link>http://localhost/cgi-bin/aeget/test.C11/?menu</link>
<author>$EMAIL_ADDRESS</author>
<guid isPermaLink="false">XXX</guid>
</item>
<item>
<title>D001 - &amp;lt;one&amp;gt; - completed</title>
<description>Description of changeset &amp;lt;one&amp;gt;</description>
<pubDate>XXX</pubDate>
<link>http://localhost/cgi-bin/aeget/test.C10/?menu</link>
<author>$EMAIL_ADDRESS</author>
<guid isPermaLink="false">XXX</guid>
</item>
</channel>
</rss>
EOF

    activity="check_two"
    diff aeget.served.expected aeget.served.stripped > log 2>&1;
    if test $? -ne 0 ; then cat log; fail; fi
}


check_three()
{
    strip_served $1 aeget.served.stripped
    USERNAME=`$bin/aesub '${user name}' -c 11 -p test`
    EMAIL_ADDRESS=`$bin/aesub '${user email}' -c 11 -p test | \
	sed -e 's/</\&lt;/' -e 's/>/\&gt;/' `
    VERSION=`$bin/aegis -version | head -1 | awk '{print $3}'`

    cat > aeget.served.expected<<EOF
Content-Type: application/rss+xml
Content-Length: NNN

<?xml version="1.0"?>
<rss version="2.0">
<channel>
<title>Project test, Title of test channel</title>
<description>Description of test channel</description>
<language>en-AU</language>
<link>http://localhost/cgi-bin/aeget/test/?menu</link>
<lastBuildDate>XXX</lastBuildDate>
<generator>aegis $VERSION</generator>
<docs>http://blogs.law.harvard.edu/tech/rss</docs>
<item>
<title>C011 - two - being_reviewed</title>
<description>Description of change&amp;amp;set &amp;quot;two&amp;quot;</description>
<pubDate>XXX</pubDate>
<link>http://localhost/cgi-bin/aeget/test.C11/?menu</link>
<author>$EMAIL_ADDRESS</author>
<guid isPermaLink="false">XXX</guid>
</item>
<item>
<title>C011 - two - being_developed</title>
<description>Description of change&amp;amp;set &amp;quot;two&amp;quot;</description>
<pubDate>XXX</pubDate>
<link>http://localhost/cgi-bin/aeget/test.C11/?menu</link>
<author>$EMAIL_ADDRESS</author>
<guid isPermaLink="false">XXX</guid>
</item>
<item>
<title>C011 - two - awaiting_development</title>
<description>Description of change&amp;amp;set &amp;quot;two&amp;quot;</description>
<pubDate>XXX</pubDate>
<link>http://localhost/cgi-bin/aeget/test.C11/?menu</link>
<author>$EMAIL_ADDRESS</author>
<guid isPermaLink="false">XXX</guid>
</item>
<item>
<title>D001 - &amp;lt;one&amp;gt; - completed</title>
<description>Description of changeset &amp;lt;one&amp;gt;</description>
<pubDate>XXX</pubDate>
<link>http://localhost/cgi-bin/aeget/test.C10/?menu</link>
<author>$EMAIL_ADDRESS</author>
<guid isPermaLink="false">XXX</guid>
</item>
</channel>
</rss>
EOF

    activity="check_three"
    diff aeget.served.expected aeget.served.stripped > log 2>&1;
    if test $? -ne 0 ; then cat log; fail; fi
}


#
# test RSS items are added to an RSS feed.
#
activity="setup project"
$bin/aegis -npr test -v -dir $work/test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="project attributes"
cat > paf << 'EOF'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
EOF

if test $? -ne 0 ; then no_result; fi
$bin/aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff"
$bin/aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
$bin/aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change"
cat > caf << 'fubar'
brief_description = "<one>";
description = "Description of changeset <one>";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin"
$bin/aegis -db 10 -dir $work/test.C010 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file"
$bin/aegis -nf $work/test.C010/aegis.conf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
cat > $work/test.C010/aegis.conf << 'fubar'
build_command = "";

history_get_command = "aesvt -check-out -edit ${quote $edit} "
    "-history ${quote $history} -f ${quote $output}";
history_put_command = "aesvt -check-in -history ${quote $history} "
    "-f ${quote $input}";
history_query_command = "aesvt -query -history ${quote $history}";
history_content_limitation = binary_capable;

diff_command = "set +e; diff $orig $i > $out; test $$? -le 1";
diff3_command = "(diff3 -e $mr $orig $i | sed -e '/^w$$/d' -e '/^q$$/d'; \
        echo '1,$$p' ) | ed - $mr > $out";
link_integration_directory = true;
development_directory_style =
{
    source_file_copy = true;
    derived_file_copy = true;
    derived_at_start_only = true;
};
project_specific =
[
    {
      name = "rss:feedfilename-all.xml";
      value = "awaiting_development being_developed awaiting_review \
               being_reviewed awaiting_integration being_integrated \
               completed";
    },
    {
      name = "rss:feeddescription-all.xml";
      value = "Description of test channel";
    },
    {
      name = "rss:feedtitle-all.xml";
      value = "Title of test channel";
    },
    {
      name = "rss:feedlanguage-all.xml";
      value = "en-AU";
    },
];
fubar
if test $? -ne 0 ; then no_result; fi

activity="build"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass"
$bin/aegis -rpass 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin"
$bin/aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass"
$bin/aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check completed"
REQUEST_METHOD=get SCRIPT_NAME=/cgi-bin/aeget \
    PATH_INFO=test QUERY_STRING=rss+all.xml $bin/aeget > aeget.served 2>&1
if test $? -ne 0 ; then cat aeget.served ; fail; fi

check_one aeget.served

activity="new change two"
cat > caf << 'fubar'
brief_description = "two";
description = "Description of change&set \"two\"";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
$bin/aegis -nc -f caf -v -p test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check awaiting_development"
REQUEST_METHOD=get SCRIPT_NAME=/cgi-bin/aeget \
    PATH_INFO=test QUERY_STRING=rss+all.xml $bin/aeget > aeget.served 2>&1
if test $? -ne 0 ; then fail; fi
check_two aeget.served

activity="develop begin two"
$bin/aegis -db 11 -dir $work/test.C011 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file two"
$bin/aegis -nf $work/test.C011/foo -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="populate foo"
echo foobar > $work/test.C011/foo
if test $? -ne 0; then no_result; fi

activity="build two"
$bin/aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff two"
$bin/aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end two"
$bin/aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check being_reviewed"
REQUEST_METHOD=get SCRIPT_NAME=/cgi-bin/aeget \
    PATH_INFO=test QUERY_STRING=rss+all.xml $bin/aeget > aeget.served 2>&1
if test $? -ne 0 ; then fail; fi
check_three aeget.served

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
