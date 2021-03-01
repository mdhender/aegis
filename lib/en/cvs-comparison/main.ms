.\"
.\"	aegis - project change supervisor
.\"	Copyright (C) 1999 Peter Miller;
.\"	All rights reserved.
.\"
.\"	This program is free software; you can redistribute it and/or modify
.\"	it under the terms of the GNU General Public License as published by
.\"	the Free Software Foundation; either version 2 of the License, or
.\"	(at your option) any later version.
.\"
.\"	This program is distributed in the hope that it will be useful,
.\"	but WITHOUT ANY WARRANTY; without even the implied warranty of
.\"	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
.\"	GNU General Public License for more details.
.\"
.\"	You should have received a copy of the GNU General Public License
.\"	along with this program; if not, write to the Free Software
.\"	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
.\"
.\" MANIFEST: document compaing Aegis and CVS
.\"
.\" Format this file with 
.\"	roffpp main.ms | pic | tbl | troff -t -ms | lpr -t
.\" from the directory this file appears in.
.\" Roffpp is part of "cook" available from the same author,
.\" or you could use soelim instead.
.\"
.\" Note that the GNU groff package uses a command like
.\"	groff -s -p -t -ms main.ms | lpr
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
.so c1.so
.so c2.so
.br
.if t .1C
.if o .bp
.\" At this point we are at the top of the page.  The .TC macro will not
.\" throw a new page, and so it will have the wrong page number for the
.\" first page.  We need some garbage on this page to force .TC to throw
.\" a new page.
\&.
.TC
