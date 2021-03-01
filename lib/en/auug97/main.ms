.\"
.\"	aegis - project change supervisor
.\"	Copyright (C) 1997, 1999, 2001, 2002, 2005-2008 Peter Miller
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
.EH "'\s9AUUGN\'97''Recursive Make Considered Harmful\s0'"
.OH "'\s9AUUGN\'97''Recursive Make Considered Harmful\s0'"
.EF "'\s9Peter Miller'\*(DY'Page %\s0'"
.OF "'\s9Peter Miller'\*(DY'Page %\s0'"
.LP
.ps 24
.vs 25
.ce 1
Recursive Make Considered Harmful
.LP
.ce 2
\fIPeter Miller\fP
\f(CWmillerp@canb.auug.org.au\fP
.LP
.sp 2
.in +0.5i
.ll -0.5i
.ce 1
\fBABSTRACT\fP
.sp 0.5
For large UNIX projects, the traditional method of building the
project is to use recursive
.I make.
On some projects, this results in
build times which are unacceptably large, when all you want to
do is change one file.	In examining the source of the overly
long build times, it became evident that a number of
apparently unrelated problems combine to produce the delay,
but on analysis all have the same root cause.
.sp 0.5
This paper explores a number of problems regarding the use of
recursive
.I make,
and shows that they are all symptoms of the same
problem.  Symptoms that the UNIX community have long accepted
as a fact of life, but which need not be endured any longer.
These problems include recursive
\fImake\fPs which take \[lq]forever\[rq] to
work out that they need to do nothing, recursive \fImake\fPs which do
too much, or too little, recursive \fImake\fPs which are
overly sensitive to changes in the source code
and require constant \f(CWMakefile\fP intervention to keep them working.
.sp 0.5
The resolution of these problems can be found by looking at
what
.I make
does, from first principles, and then analyzing
the effects of introducing recursive
.I make
to this activity.
The analysis shows that the problem stems from the artificial partitioning
of the build into separate subsets.  This, in turn, leads to the
symptoms described.  To avoid the symptoms, it is only necessary to
avoid the separation; to use a single \fImake\fP session to build the
whole project, which is not quite the same as a single \f(CWMakefile\fP.
.sp 0.5
This conclusion runs counter to much accumulated folk wisdom in building
large projects on UNIX.  Some of the main objections raised by this folk
wisdom are examined and shown to be unfounded.  The results of actual
use are far more encouraging, with routine development performance
improvements significantly faster than intuition may indicate, and
without the intuitvely expected compromise of modularity.  The use of
a whole project \fImake\fP is not as difficult to put into practice as
it may at first appear.
.sp
.ll +0.5i
.in -0.5i
.TS
center,allbox;
lw(3.5i).
T{
Miller, P.A. (1998), \fIRecursive Make Considered Harmful,\fP
.br
AUUGN Journal of AUUG Inc., 19(1), pp. 14-25.
T}
.TE
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
.br
.ne 3i
.nh 1 "About the Author"
Peter Miller has worked for many years in the software R&D
industry, principally on UNIX systems.	In that time he has
written tools such as Aegis (a software configuration management
system) and Cook (yet another \fImake\fP-oid), both of which are freely
available on the Internet.  Supporting the use of these tools at
many Internet sites provided the insights which led to this paper.
.LP
Please visit \f(CWhttp://www\%.canb\%.auug\%.org\%.au\%/~millerp/\fP if
you would like to look at some of the author's free software.
