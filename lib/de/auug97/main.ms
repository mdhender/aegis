.\"
.\"	aegis - project change supervisor
.\"	Copyright (C) 1997, 1999, 2001, 2002, 2006-2008 Peter Miller
.\"
.\"	This program is free software; you can redistribute it and/or modify
.\"	it under the terms of the GNU General Public License as published by
.\"	the Free Software Foundation; either version 3 of the License, or
.\"	(at your option) any later version.
.\"
.\"	This program is distributed in the hope that it will be useful,
.\"	but WITHOUT ANY WARRANTY; without even the implied warranty of
.\"	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
.\"	GNU General Public License for more details.
.\"
.\"	You should have received a copy of the GNU General Public License
.\"	along with this program. If not, see
.\"	<http://www.gnu.org/licenses/>.
.\"
.if n .ftr CB B
.if n .ftr CI I
.if n .ftr CW R
.if n .ftr C  R
.po +0.4i
.nr NS 0
.de nh
.br
.ne 1i
.NH \\$1
\\$2
.if !\\n(NS-2 \{\
.XS \\n%
\h'\\n(NSi/2u-0.5i'\\*(SN \\$2
.XE
.\}
.LP
..
.LP
.EH "'\s9CM Magazin, Dec-2002''Probleme durch rekursives Make\s0'"
.OH "'\s9CM Magazin, Dec-2002''Probleme durch rekursives Make\s0'"
.EF "'\s9Ulrike Amoore'\*(DY'Page %\s0'"
.OF "'\s9Peter Miller'\*(DY'Page %\s0'"
.LP
.ps 24
.vs 25
.ce 1
Probleme durch rekursives Make
.LP
.TS
center;
cw(2i) cw(2i).
T{
.ce 2
\fIPeter Miller\fP
\f(CWmillerp@canb.auug.org.au\fP
T}	T{
.ce 4
Aus dem Amerikanischen
ins Deutsche \*:ubersetzt von
\fIUlrike Amoore\fP
\f[CW]uamoore@cmmagazin.de\fP
T}
.TE
.LP
.in +0.5i
.ll -0.5i
.ce 1
\fBVorwort\fP
.sp 0.5
Zur Produktion gro\(sser UNIX-Projekte verwendet man
traditionellerweise rekursives Make. Bei manchen
Projekten f\*:uhrt das zu sehr langen Produktionszeiten,
was insbesondere, wenn man nur eine Datei \*:andern m\*:ochte,
unvertretbar ist. Als wir das Ph\*:anomen n\*:aher
untersuchten, stellte sich heraus, dass eine Reihe von
Problemen, die voneinander unabh\*:angig zu sein schienen,
gemeinsam f\*:ur die Verz\*:ogerung verantwortlich waren. Eine
genaue Analyse zeigte jedoch eine gemeinsame Ursache.
.sp 0.5
Dieser Artikel besch\*:aftigt sich mit einigen der
Probleme, die im Zusammenhang mit der Anwendung von
rekursivem Make regelm\*:a\(ssig auftreten, und legt dar, dass
sie alle Symptome desselben Problems sind. Diese
Symptome haben die UNIX-Anwender lange als unumg\*:angliche
Tatsachen hingenommen, aber sie m\*:ussen nicht l\*:anger
geduldet werden. Dazu geh\*:ort, dass ein rekursives Make
oft sehr lange braucht, um herauszufinden, dass es
nichts zu tun braucht, dass es zu viel oder zu wenig tut
oder dass es \*:uberm\*:a\(ssig empfindlich auf Ver\*:anderungen von
Quellkode reagiert und f\*:ur eine ungest\*:orte Funktion den
st\*:andigen Eingriff in das \f[CW]Makefile\fP notwendig macht.
.sp 0.5
Man kann die L\*:osung f\*:ur diese Probleme finden, indem man
sein Augenmerk erst einmal darauf richtet, wie Make
grunds\*:atzlich arbeitet, und dann die Auswirkungen
analysiert, die durch das Hinzuf\*:ugen von rekursivem
Make, hervorgerufen werden. Die Analyse zeigt, dass das
Problem von der k\*:unstlichen Einteilung des Builds in
gesonderte Teilmengen herr\*:uhrt. Das wiederum f\*:uhrt zu
den beschriebenen Symptomen. Um die Symptome zu
vermeiden, ist es lediglich notwendig diese Einteilung
zu verhindern, d. h. einen einzigen Make-Durchgang zu
veranlassen, was nicht bedeutet, dass es nur ein
einziges \f[CW]Makefile\fP gibt.
.sp 0.5
Diese Schlussfolgerung widerspricht vielen bez\*:uglich der
Produktion gro\(sser Projekte angesammelten
Volksweisheiten. Einige der von Vertretern dieser
Volksweisheiten vorgebrachten Einw\*:ande werden hier
untersucht und erweisen sich als unbegr\*:undet. Die
praktische Umsetzung dieser Schlussfolgerung f\*:uhrt zu
wesentlich ermutigenderen Ergebnissen. Wird diese
Methode laufend weiterentwickelt, werden Verbesserungen
der Effizienz erheblich schneller als erwartet sichtbar,
ohne dass die Modularit\*:at aufgegeben werden muss. Die
Durchf\*:uhrung eines Ganzprojekt-Makes ist nicht so
schwierig umzusetzen, wie es zun\*:achst erscheint.
.sp
.ll +0.5i
.in -0.5i
.sp 2
.if t .2C
.\" ---------------------------------------------------------------------------
.so 01-intro.so
.so 02-problem.so
.so 03-analysis.so
.so 04-cure.so
.so 05-makefile.so
.so 06-vpath.so
.so 07-synthesi.so
.so 08-books.so
.so 09-summary.so
.so 10-ref.so
.LP
.TS
center,allbox;
lw(2.5i).
T{
Miller, P.A. (1998), \fIRecursive Make Considered Harmful\fP,
AUUGN Journal of AUUG Inc., 19(1), pp. 14-25.
T}
.TE
See \f[CW]http://www.cmmagazin.de\fP December 2002 issue for the HTML
version of the transaltion.
.br
.ne 3i
.nh 1 "\*:Uber den Autor"
Peter Miller hat viele Jahre lang in der Software R&D
Industrie gearbeitet, vor allem auf UNIX-Systemen. In
dieser Zeit schrieb er Werkzeuge wie Aegis (ein System
des Software Configuration Management) und Cook (eine
weitere Make-Version), beide kann man frei \*:uber das
Internet beziehen. Die Betreuung bei der Verwendung
dieser Werkzeuge auf vielen Internet Sites erm\*:oglichte
den Einblick, der zu diesem Artikel f\*:uhrte.
.LP
Wenn Sie Interesse am Bezug der freien Software des Autors haben, besuchen
.\" do not warn "cannot adjust line"
.warn (\n[.warn]-(((\n[.warn]/4)%2)*4))
Sie bitte: \f(CWhttp://www\%.canb\%.auug\%.org\%.au\%/~millerp/\fP
