#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2005 Matthew Lee;
#       All rights reserved.
#       Copyright (C) 2007, 2008 Peter Miller
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
#       along with this program; if not, see
#       <http://www.gnu.org/licenses/>.
#

TEST_SUBJECT="rss_item functionality"

# load up standard prelude and test functions
. test_funcs


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

    USERNAME=`aesub '${user name}' -c 10 -p test`
    EMAIL_ADDRESS=`aesub '${user email}' -c 10 -p test | \
	sed -e 's/</\&lt;/' -e 's/>/\&gt;/'`
    VERSION=`aegis -version | head -1 | awk '{print $3}'`

    cat > aeget.served.expected << EOF
Content-Type: application/rss+xml
Content-Length: NNN

<?xml version="1.0"?>
<rss version="2.0">
<channel>
<title>Project test, Titolo del canale di test</title>
<description>Descrizione del canale dei change set completi</description>
<language>it</language>
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

    activity="check_one 74"
    diff -u aeget.served.expected aeget.served.stripped > log 2>&1;
    if test $? -ne 0 ; then cat log; fail ; fi
}

check_two()
{
    strip_served $1 aeget.served.stripped

    USERNAME=`aesub '${user name}' -c 11 -p test`
    EMAIL_ADDRESS=`aesub '${user email}' -c 11 -p test | \
	sed -e 's/</\&lt;/' -e 's/>/\&gt;/' `
    VERSION=`aegis -version | head -1 | awk '{print $3}'`

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
</channel>
</rss>
EOF

    activity="check_two 114"
    diff -u aeget.served.expected aeget.served.stripped > log 2>&1;
    if test $? -ne 0 ; then cat log; fail; fi
}


check_three()
{
    strip_served $1 aeget.served.stripped
    USERNAME=`aesub '${user name}' -c 11 -p test`
    EMAIL_ADDRESS=`aesub '${user email}' -c 11 -p test | \
	sed -e 's/</\&lt;/' -e 's/>/\&gt;/' `
    VERSION=`aegis -version | head -1 | awk '{print $3}'`

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
</channel>
</rss>
EOF

    activity="check_three 170"
    diff -u aeget.served.expected aeget.served.stripped > log 2>&1;
    if test $? -ne 0 ; then cat log; fail; fi
}


#
# test RSS items are added to an RSS feed.
#
activity="setup project 179"
aegis -npr test -v -dir $work/test -lib $work/lib > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

AEGIS_PROJECT=test
export AEGIS_PROJECT

AEGIS_PATH=$work/lib
export AEGIS_PATH

activity="project attributes 189"
cat > paf << 'EOF'
developer_may_review = true;
developer_may_integrate = true;
reviewer_may_integrate = true;
default_test_exemption = true;
EOF
if test $? -ne 0 ; then no_result; fi
aegis -pa -f paf -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="staff 200"
aegis -nd $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -nrv $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi
aegis -ni $USER -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new change 208"
cat > caf << 'fubar'
brief_description = "<one>";
description = "Description of changeset <one>";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
aegis -nc -f caf -v -p test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop begin 219"
aegis -db 10 -dir $work/test.C010 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file 223"
aegis -nf $work/test.C010/aegis.conf -v > log 2>&1
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
               being_reviewed awaiting_integration being_integrated";
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
    {
	name = "rss:feedfilename-completed.xml";
	value = "completed";
    },
    {
      name = "rss:feedtitle-completed.xml";
      value = "Titolo del canale di test";
    },
    {
	name = "rss:feeddescription-completed.xml";
	value = "Descrizione del canale dei change set completi";
    },
    {
	name = "rss:feedlanguage-completed.xml";
	value = "it";
    },
];
fubar
if test $? -ne 0 ; then no_result; fi

activity="build 285"
aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff 289"
aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end 293"
aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="review pass 297"
aegis -rpass 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate begin 301"
aegis -ib 10 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate build 305"
aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="integrate pass 309"
aegis -ipass -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check completed 313"
REQUEST_METHOD=get SCRIPT_NAME=/cgi-bin/aeget \
    PATH_INFO=test QUERY_STRING=rss+completed.xml aeget \
    > aeget.served 2>&1
if test $? -ne 0 ; then cat aeget.served ; fail; fi

check_one aeget.served

activity="new change two 321"
cat > caf << 'fubar'
brief_description = "two";
description = "Description of change&set \"two\"";
cause = internal_enhancement;
test_baseline_exempt = true;
fubar
if test $? -ne 0 ; then no_result; fi
aegis -nc -f caf -v -p test > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check awaiting_development 332"
REQUEST_METHOD=get SCRIPT_NAME=/cgi-bin/aeget \
    PATH_INFO=test QUERY_STRING=rss+all.xml aeget > aeget.served 2>&1
if test $? -ne 0 ; then fail; fi

check_two aeget.served

activity="develop begin two 339"
aegis -db 11 -dir $work/test.C011 -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="new file two 343"
aegis -nf $work/test.C011/foo -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="populate foo 347"
echo foobar > $work/test.C011/foo
if test $? -ne 0; then no_result; fi

activity="build two 351"
aegis -b -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="diff two 355"
aegis -diff -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="develop end two 359"
aegis -de -v > log 2>&1
if test $? -ne 0 ; then cat log; no_result; fi

activity="check being_reviewed 363"
REQUEST_METHOD=get SCRIPT_NAME=/cgi-bin/aeget \
    PATH_INFO=test QUERY_STRING=rss+all.xml aeget > aeget.served 2>&1
if test $? -ne 0 ; then fail; fi

check_three aeget.served

#
# Only definite negatives are possible.
# The functionality exercised by this test appears to work,
# no other guarantees are made.
#
pass
