#!/bin/sh
#
#       aegis - project change supervisor
#       Copyright (C) 2006, 2008, 2012 Peter Miller
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

cat << 'fubar'
<html>
<head>
<title>
Icons by Gregory Delattre
</title>
</head>
<body bgcolor="#eeffee">
<h1>
<img src="../aegis.png" align="left">
<img src="boxopen_2.png" align="right">
<center>Icons by Gr&eacute;gory Delattre</center>
</h1>
The beautiful icons you see on this web site are by
<a href="mailto:delattre.greg@free.fr">
<i>Gr&eacute;gory Delattre</i> &lt;delattre.greg@free.fr&gt;</a>, who
generously makes them available for free.  You may like to <a
href="http://zone72.free.fr/">visit his web site</a> and look at some of
his other excellent work.
<p>
The README file accompanying the icons reads
<blockquote>
    <i>This set represents a very very very long work... if you
    appreciate it, thank you to write me a small word on my guest
    book... it'll make me happy!</i> <tt>;)</tt>
</blockquote>
Below you will find each of Gr&eacute;gory Delattre's beautiful icons
used on this web site.
<p align=center>
fubar

for j in $*
do
    jj=`basename $j`
    echo "<a href=\"$j\"><img src=\"$j\" border=0></a>"
done

cat << 'fubar'
</p>
<p>
<img src="../aegis.png" align="left" >
The <a href="http://www.metmuseum.org/toah/ho/06/eust/ho_1984.184.1,2.htm"
>shield boss</a> used for the Aegis icon is in the <a
href="http://www.metmuseum.org/" >Metropolitan Museum of Art</a>.
It is made of iron, decorated with engraved appliqu&eacute;s of gilt bronze.
</body>
</html>
fubar
exit 0
# vim: set ts=8 sw=4 et :
