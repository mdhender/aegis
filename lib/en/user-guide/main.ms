'\"tp
.\" aegis - project change supervisor
.\" Copyright (C) 1991-1999, 2001, 2002, 2006-2008, 2012 Peter Miller
.\"
.\" This program is free software; you can redistribute it and/or modify
.\" it under the terms of the GNU General Public License as published by
.\" the Free Software Foundation; either version 3 of the License, or
.\" (at your option) any later version.
.\"
.\" This program is distributed in the hope that it will be useful,
.\" but WITHOUT ANY WARRANTY; without even the implied warranty of
.\" MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
.\" General Public License for more details.
.\"
.\" You should have received a copy of the GNU General Public License
.\" along with this program. If not, see <http://www.gnu.org/licenses/>.
.\"
.\"
.\" Format this file with
.\"     roffpp aegis.ms | pic | tbl | troff -t -ms | lpr -t
.\" from the directory this file appears in.
.\" Roffpp is part of "cook" available from the same author,
.\" or you could use soelim instead.
.\"
.\" Note that the GNU groff package uses a command like
.\"     groff -s -p -t -ms aegis.ms | lpr
.\" and gives PostScript output and far better error messages.
.\"
.lg 0 \" turn off ligatures: groff diagrees with everyone else about widths
.if n .ftr CB B
.if n .ftr CI I
.if n .ftr CW R
.if n .ftr C  R
.nr NS 0
.de nh
.NH \\$1
\\$2
.if !\\$1-3 \{\
.XS \\n%
\h'\\$1i/4u-0.25i'\\*[SN] \\$2
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
.so lib/en/user-guide/c1.0.so
.so lib/en/user-guide/c7.0.so
.so lib/en/user-guide/c2.0.so
.so lib/en/user-guide/c3.0.so
.so lib/en/user-guide/c4.0.so
.so lib/en/user-guide/c8.0.so
.so lib/en/user-guide/c5.0.so
.so lib/en/user-guide/c11.0.so
.so lib/en/user-guide/c9.0.so
.so lib/en/user-guide/c6.0.so
.so lib/en/user-guide/c10.0.so
.so lib/en/user-guide/c12.0.so
.so lib/en/user-guide/cA.0.so
.so lib/en/user-guide/cB.0.so
.so lib/en/user-guide/cD.0.so
.so lib/en/user-guide/cI.0.so
.br
.if t .1C
.bp
.\" At this point we are at the top of the page.  The .TC macro will not
.\" throw a new page, and so it will have the wrong page number for the
.\" first page.  We need some garbage on this page to force .TC to throw
.\" a new page.
\&.
.\" I can't use .TC because I need to mess with the page numbers so
.\" that I can use psselect to move the ToC to the front, afterwards.
.\" .TC
.P1
.pg@begin 1001 i
.PX
.\" vim: set ts=8 sw=4 et :
