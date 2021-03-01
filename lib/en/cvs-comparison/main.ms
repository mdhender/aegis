.\"
.\"     aegis - project change supervisor
.\"     Copyright (C) 1999, 2001, 2002, 2006-2009, 2012 Peter Miller
.\"
.\"     This program is free software; you can redistribute it and/or modify
.\"     it under the terms of the GNU General Public License as published by
.\"     the Free Software Foundation; either version 3 of the License, or
.\"     (at your option) any later version.
.\"
.\"     This program is distributed in the hope that it will be useful,
.\"     but WITHOUT ANY WARRANTY; without even the implied warranty of
.\"     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
.\"     GNU General Public License for more details.
.\"
.\"     You should have received a copy of the GNU General Public License
.\"     along with this program. If not, see
.\"     <http://www.gnu.org/licenses/>.
.\"
.\" Format this file with
.\"     roffpp main.ms | pic | tbl | troff -t -ms | lpr -t
.\" from the directory this file appears in.
.\" Roffpp is part of "cook" available from the same author,
.\" or you could use soelim instead.
.\"
.\" Note that the GNU groff package uses a command like
.\"     groff -s -p -t -ms main.ms | lpr
.\" and gives PostScript output and far better error messages.
.\"
.if n .ftr CB B
.if n .ftr CI I
.if n .ftr CW R
.if n .ftr C  R
.nr NS 0
.de nh
.NH \\$1
\\$2
.if !\\n(NS-2 \{\
.XS \\n%
\h'\\n(NSi/2u-0.5i'\\*(SN \\$2
.XE
.\}
..
.\" Example Start, use Courier if you have it
.de E(
.DS
.ft R
.ft C
.nf
.ta 8n 16n 24n 32n 40n 48n 56n 64n 72n 80n
.ps -2
.vs -2
..
.\" Example End
.de E)
.vs +2
.ps +2
.ft R
.fi
.DE
..
.\" ------------------------------------------------------------------------
\&.
.sp 2i
.ps 24
.vs 26
.ce 2
Aegis
A Project Change Supervisor
.sp 0.5i
.ps 28
.vs 30
.ce 2
CVS Transition Guide
.sp 1i
.ps 18
.vs 20
.ce 1
Peter Miller
.ft I
.ce 1
pmiller@opensource.org.au
.ft P
.OH 'Aegis''CVS Transition Guide'
.EH 'CVS Transition Guide''Aegis'
.bp
.OF 'Peter Miller'\\\\s-3(\\\\n(.F)\\\\s+3'Page \\\\n[PN]'
.EF 'Page \\\\n[PN]'\\\\s-3(\\\\n(.F)\\\\s+3'Peter Miller'
.ps 12
.vs 14
\&.
.sp 4i
.so etc/version.so
.so etc/libdir.so
This document describes Aegis version \*(v)
.br
and was prepared \*(DY.
.br
.sp 1i
.if n .ds C) (C)
.if t .ds C) \(co
This document describing the Aegis program,
and the Aegis program itself,
are
.br
Copyright \*(C) \*(Y) Peter Miller
.sp
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
.sp
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
.sp
You should have received a copy of the GNU General Public License along
with this program. If not, see <http://www.gnu.org/licenses/>.
.ps 10
.vs 12
.bp
.\" ------------------------------------------------------------------------
.so lib/en/cvs-comparison/c1.so
.so lib/en/cvs-comparison/c2.so
.so lib/en/cvs-comparison/c3.so
.br
.if t .1C
.bp
\&.
.if o .bp
.\" At this point we are at the top of the page.  The .TC macro will not
.\" throw a new page, and so it will have the wrong page number for the
.\" first page.  We need some garbage on this page to force .TC to throw
.\" a new page.
\&.
.\"
.\" I can't use .TC because I need to mess with the page numbers so
.\" that I can use psselect to move the ToC to the front, afterwards.
.\" .TC
.P1
.pg@begin 1001 i
.PX
.if o .bp
.\" vim: set ts=8 sw=4 et :
