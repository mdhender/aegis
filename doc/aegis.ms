.\"
.\"	aegis - project change supervisor
.\"	Copyright (C) 1991, 1992, 1993, 1994 Peter Miller.
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
.\"	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
.\"
.\" MANIFEST: User Guide
.\"
.\" Format this file with 
.\"	roffpp aegis.ms | pic | tbl | troff -t -ms | lpr -t
.\" from the directory this file appears in.
.\" Roffpp is part of "cook" available from the same author,
.\" or you could use soelim instead.
.\"
.\" Note that the GNU groff package uses a command like
.\"	groff -s -p -t -ms aegis.ms | lpr
.\" and gives PostScript output and far better error messages.
.\"
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
.ta 8n 16n 24n 32n 40n 48n 56n 64n 72n 8n
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
.so c1.0.so
.so c7.0.so
.so c2.0.so
.so c3.0.so
.so c4.0.so
.so c8.0.so
.so c5.0.so
.so c6.0.so
.so cA.0.so
.so cB.0.so
.so cD.0.so
.br
.1C
.TC
