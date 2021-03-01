'\"p
.\"     aegis - project change supervisor
.\"     Copyright (C) 1996, 1997, 2002, 2005-2009, 2012 Peter Miller
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
.EH "'\s9AUUG\'96''Aegis is Only For Software, Isn\'t It?\s0'"
.OH "'\s9AUUG\'96''Aegis is Only For Software, Isn\'t It?\s0'"
.EF "'\s9Peter Miller''Page %\s0'"
.OF "'\s9Peter Miller''Page %\s0'"
.LP
.ps 24
.vs 25
.ce 2
Aegis is Only for Software,
Isn't It?
.LP
.ce 2
Peter Miller
pmiller@opensource.org.au
.LP
.in +0.5i
.ll -0.5i
.sp 2
Aegis is a Software Configuration Management system,
which provides a
method for managing concurrent development and peer review with strong
auditability.
These features are useful to more environments than the
development of software.
This paper presents two systems being
managed with Aegis at AGSO:
DNS and the Web.
.sp
Using Aegis to manage DNS provides a reliable way to add entries to the
DNS tables, check the tables, and generate the reverse maps.
It is even generating some NIS+ tables.
The system is peer reviewed,
so no \[lq]broken\[rq] changes are able to get into the system tables.
The net result is a more reliable service to the users.
.sp
Using Aegis to manage the AGSO Web (http:\%//www.agso.gov.au/)
models the production of scientific papers.
In the normal publication process an author writes a paper and
it is then peer reviewed,
the reviewers may return it with comments or approve it to
the publisher.
The publisher in turn may accept it for publication or return it.
A similar model is available using Aegis when publishing Web pages;
the publication analogy is deliberate since the work is
indeed available to the public.
The \[lq]build\[rq] step which is so central to producing software is
used in the Web
case to resolve server-side includes,
to check the HTML for obvious errors,
and to generate the various indices.
Some Aegis reports are also used,
such as the one from which
the \[lq]What's New\[rq] page (http:\%//www.agso.gov.au/\%whatsnew/)
is generated.
The provision by Aegis of individual \[lq]sand pits\[rq]
greatly facilitates concurrent development of Web pages and improves
productivity.
In general,
staff have been happy with the Web development model in use at AGSO.
.sp 2
.ll +0.5i
.in -0.5i
.if t .2C
.nh 1 "Aegis at a Glance"
.FS
Copyright
.if t \(co
.if n (C)
1996 Peter Miller
.FE
The term
.I "Configuration Management"
was coined many decades ago in the engineering disciplines.
It refers to the process of managing all of
the components of an engineering design.
In designing a car, for example,
it is important to know which version of
the shock absorbers goes with which versions of the chassis and wheel
arches.
.LP
A similar problems exists for the construction of software,
and so the
term was borrowed by software engineers to become
.I "Software Configuration Management."
This refers to keeping track of the various
source components of a software package, and how to assemble them into
a working program.
.LP
There is a huge similarity between assembling a software program,
and assembling a web.
Source files are processed and stitched together to
form the final product,
much the same way program source files are
compiled and linked together to form the final software product.
.LP
Aegis is a software configuration management system,
and it provides
controlled accesses to the source files of a project.
It can support concurrent development,
and enforces mandatory reviews.
As Aegis is described more completely elsewhere [1, 2],
this paper will not dwell too heavily on how to use Aegis,
but rather how Aegis is useful in the two case studies below.
.nh 1 "DNS Management"
The first case study presented here is that of using Aegis to manage
Domain Name Service (DNS) information.
This is a relatively small system, it is reasonably well known,
and if you get it wrong your network may cease to function until
you get it right again.
DNS itself has been described elsewhere [3, 4]
and will not be described in depth here.
.LP
The design of the Unix \fInamed\fP(8),
the program which serves the DNS protocol,
is intriguing, mostly because of the
almost-but-not-quite redundancy in most of the configurations files.
This revolves around the fact that DNS has both
.I forward
maps which associate a domain name with an IP address,
and
.I reverse
maps which associate an IP address with a domain name.
Naturally,
if these maps are not carefully synchronized,
a variety of problems may result.
This is traditionally solved using
\fImake\fP(1)
and some baroque \fIawk\fP(1) scripts.
The maintainer logs in as
\fIroot\fP,
edits the forward map, runs make,
and informs named that it needs to re-read its input files,
usually by running a shell script which sends a signal to the server process.
.LP
But what do you do when there is more than one authorized maintainer?
How do they coordinate their activities when they are in 3 different buildings?
How do they make sure the configuration files are OK
.I before
they bring the whole network to its knees?
.LP
None of these problems are particularly new or interesting.
They are well known to software development teams the world over.
At AGSO, we are using Aegis to resolve them.
.nh 2 "Concurrent Development"
Aegis has the concept of a
.I baseline
which is the
known-good currently-working
master source for a project.
Aegis partitions alterations to this baseline into
sets of files which must be altered simultaneously
to preserve the currently-working-ness of the baseline.
These sets of files may be as small as a single file,
or as large as every source file in the baseline.
.LP
These alterations to the baseline are known, unoriginally, as
\fIchanges\fP,
and each change is given a separate development directory.
By developing each change in a separate directory,
there is no possibility than one maintainer can accidentally blow away
another's work.
At worst, if they have a file in common,
one will need to merge the other's work with his own.
This merge is fully supported by Aegis.
.LP
The other important feature of using separate development
directories is that \[lq]half finished\[rq] changes
are not mistaken for valid input,
should the DNS server be rebooted during the development of the change.
Until a change is completed
.I and
has been reviewed, the baseline remains unaltered.
.PP
This entire process is performed by users logged in as themselves.
Access is controlled by access control lists based on user names,
no special system user accounts are required.
need for users to login to a special account to work on web pages.
.nh 2 "Validation"
When constructing software, the program is built in some way.
One traditional Unix approach is to delegate remembering how to do this
to a program such as
\fImake\fP(1).
While
\fImake\fP(1)
can be used with Aegis,
the author prefers
\fIcook\fP(1)
as a more capable and descriptive build tool.
.LP
The build step is used to translate the forward maps into reverse maps.
This is not done with
\fIawk\fP(1)
for a few reasons
.IP \(bu
The SRRF input format used by \fInamed\fP(8) makes for very hard to read
awk scripts if
you are determined to use the full input language.
.IP \(bu
We use include files to describe our various buildings and other
internal network structure.
This also reduces the number of opportunities for conflicts between changes.
.LP
As a result, we have a suite of C programs
(also developed using Aegis, naturally)
which perform the various translations and filters we require.
.LP
A very important side-effect of these translations is that they
validate their input.
In this we detect duplicate host names,
invalid host names,
duplicate IP addresses,
invalid IP addresses, etc.
When we converted our DNS procedures to use DNS,
a number of problems were discovered which had been in the system for years.
.LP
Yes, these problems could have been discovered with the old root make system,
but they were not discovered until we decided to us a system specifically
designed to be maintained by many people,
and to double check everything before it went \[lq]live\[rq] and
inconvenienced staff.
.nh 2 "Derived Data"
We also use the DNS data to generate some of our NIS+ tables.
Obviously, the NIS+ hosts table needs to be synchronized with the DNS tables,
and this translation is relatively easy.
.LP
We also invented an \[lq]ether\[rq] type,
which we filter out and don't actually give to \fInamed\fP(8),
to record Ethernet addresses.
This information is used to derive the NIS+ ethers table.
This could have been managed differently, but as it is intimately related
to the IP address management, we decided to do it here,
for a sort of \[lq]one stop\[rq] network shop.
.LP
The \f(CWhinfo\fP records are
\fIgrep\fP(1)ed
to generate the NIS+ netgroup table.
This is used to minimize changes to
\fI/etc/hosts.equiv\fP,
\fI/etc/dfs/dfstab\fP,
etc.
As a new machine is added, it receives a formula \f(CWhinfo\fP description,
which results in it being included in the appropriate net group,
and thus the appropriate network permissions.
.LP
The \f(CWhinfo\fP records are also filtered
to generate the NIS+ bootparamd map.
When combined with the hosts table and the ethers table,
this is all that is required to configure a workstation for
the Solaris Install Server.
.nh 2 "Review"
Aegis will not allow a change to finish development until it builds cleanly.
Thus, if there are any errors found during the various filters
performed by the build step,
a change may not end development.
.LP
Once a change ends development it is not immediately installed
into the baseline.
It enters a \[lq]being reviewed\[rq] state.
Developers are prevented from reviewing their own work,
as an obvious conflict of interests.
Some other authorized review must do so.
The access control lists for developers and reviewers are separate,
and it is up to your individual preferences whether they overlap or not.
Once a reviewer OKs the change, it is then integrated.
.nh 2 "Integration"
The integration involves making a copy of the baseline
(usually with hard links, it is faster)
and applying the change to this copy.
The build is performed again on this copy - mostly as a quick double check,
though it does not in the Web case, below.
.LP
The integrator can also serve as an editor, or a second reviewer if necessary.
It answers the perennial \[lq]who will watch the watchers\[rq] question.
.LP
Aegis' notification facility, used at most transition to send email or
news articles,
is also used at the end of integration to notify DNS and NIS+ to re-read the
relevant files.
.nh 2 "Observable Results"
It was long suspected that there was a fairly high rate of change
in our DNS data, but we didn't track it too closely,
we already had more than enough to do.
Aegis tracks this activity in a very non-intrusive manner.
At present, we are averaging 3.5 DNS changes per week,
a figure not previously available.
.LP
The other thing which has changed is that there are no unexplained changes.
You no longer hear they cry \[lq]Who the smeg did that?\[rq] echoing down
the corridor.
There is never the silent anonymous untrackable
duel between two administrators undoing and redoing
each others changes.
.LP
By adding Ethernet numbers, we can detect when machines move between staff
and between floors and change names,
meaning that we can detect now-vacant IP addresses
by the Ethernet address duplicate.
.LP
The most important result is stability.
More is checked than ever before,
and it is discovered earlier than ever before.
Only changes which pass all our automated checks,
and also pass the scrutiny of a human DNS boffin,
are inflicted on users.
.nh 1 "Web Management"
The second case study presented is that of AGSO's Web.
This is also managed using Aegis,
though this project has many, many more sources files.
The environment presented by Aegis is the same - though
the terminology used is more aimed at authors than
system administrators.
.LP
The method of Web development implemented using Aegis is presented as
an analogue of the publications of scientific papers.
.PS
boxwid=1.5
down
box "Author" "\fIwrites web pages\fP"
line
box "Reviewer" "\fIperforms peer review\fP"
line
box "Editor" "\fIperforms editorial review\fP"
.PE
In particular, it is expected that much of the science content of our web pages
will be authored by out scientists.
The peer review is expected to be, and should be, performed by
the scientist's science peers.
Everything else - layout, formatting, copyright issues, structure, etc,
are the editor's problem.
Only the author may alter a change - the peer reviewer and the
editor only have the power of veto.
The file system permissions are exploited to ensure that
there is no \[lq]back door'
to this process.
Even the editor (integrator) does not have write permissions to the baseline.
All changes to the baseline \fImust\fP traverse the entire change process.
.nh 2 "Build and Validation"
The build step in the Web project is very little different from the build
step in the DNS project.
We use the build step to:
.IP \(bu
Resolve server-side include files, so that the files
fetched by external clients are complete,
and the HTTP server can be as dumb as possible.
We use common headers and footers in common include files
to implement much of the look and feel of our web pages.
.IP \(bu
Perform SGML checking against the HTML 2.0 DTD.
The finds all of the overt HTML errors,
but it cannot check spelling or style.
We use \fIsgmls\fP(1) obtained from the network,
and the standard \f(CWhtml.dtd\fP also obtained from the network.
.LP
It takes longer to perform this build than in the DNS case,
because there are more source files and more include dependencies to check.
.LP
The build works out what to build from the source files of the project.
The \fIcook\fP(1) programs asks Aegis for a list of files,
and then generates a list of output files from that.
In this way, there is no need to add a new file to a list in a \fIMakefile\fP
of some sort.
.nh 2 "File Templates"
To add a new page to the Web, a user initiates a Web change and then
creates a new file via Aegis.
The new file will be created according to a pre-configured file template.
This will give them the common headers and footers automatically,
all that is required is to fill in the text in the middle.
.LP
So that the page may be accessed, it will also be necessary to copy
some other file, again via Aegis, and embed an anchor to the new page
in the copied page.
.LP
Files are copied and created via Aegis so that Aegis knows
which files to transfer from your
working directory into the baseline.
If you don't tell Aegis, they will not be built, and later
they will not reach the baseline.
.nh 2 "Preview and Review"
Once your Web pages build successfully, it is possible to
see what the public will see - to preview them.
We do this by running a specialized web server that \[lq]overlays\[rq] the files
in your development directory over those in the baseline.
Essentially, this is a simple search path.
The specialized web server tell you a URL to open to preview
your changes, and it will exist until you interrupt it,
usually with Control-C.
.LP
Reviewers may use the same technique to look at an author's change,
however reviewers are also encouraged to look at the HTML itself.
.LP
Using this specialized web server, it can be established exactly what the
public will see when the pages finally arrive in the Web baseline.
All of the Web pages are available in this way, as the change's pages
are seamlessly integrated with the baseline pages.
In this way, it is only necessary to copy the pages you actually need to edit.
All other pages are obtained from the baseline.
This often represents a large saving in working disk storage.
.LP
We have a set of layout guidelines which are available internally for
our Web developers to read.
Reviewers (theoretically) review against these guidelines,
and developers are encouraged to have a quick look at the guidelines
before completing the development of their changes.
.nh 2 "Integration"
The integration stage is not particularly different from the integration
stage for DNS.
However, some of the more time consuming tasks,
which are not of great interest to developers are done at this time:
mostly generating or updating the various indexes for our Web
as required for the specific change.
.nh 2 "Concurrent Development"
Performing file copies via Aegis allows Aegis to note the version of the file
at the time it was added to the change.
This version information is used later to detect file conflicts,
and to merge file contents.
.LP
The chances of having Web pages in common are fairly low,
however it happens fairly regularly.
The use of separate development directories means that this can be ignore
for as long
as the author wants to.
When it is time to complete development of a Web change,
the versions of the files are checked by Aegis.  If any are out-of-date,
a 3-way merge may be performed to merge the Web baseline updates with
the edits in the change.
.nh 2 "Results"
The AGSO Web is intended to be a \[lq]publication on demand\[rq] medium.
As a result, any data the public can see needs to be of publication
quality already.
It is then only required for the client to print interesting material
one the client's printer.
While the HTML formatting may leave a little to be desired at present,
we believe that the process we employ is more than capable of delivering
the quality of content that we desire.
.nh 1 "Summary"
Many of the processes we perform on computers
involve manipulating and assembling files in some way,
many can be managed using SCM techniques.
Even where significant manipulation (e.g. compiling software) is not required,
automated input validation helps reduce the number of avoidable errors
(much as compilers validate against input language).
.LP
The other advantage conferred by Aegis is that privileged operations are
recorded, and are available for auditing purposes.
The privilege is controlled by access control lists,
and requires active conspiracy by more than one person to subvert.
Even then, the audit record is inaccessible.
By using Aegis' notification facilities, it is even possible
for highly privileged system tables to be updated,
and yet
.I root
passwords need not be disclosed.
.LP
The use of Aegis to assist in the management and availability
of critical system services
has been very successful, and will be used again.
.LP
The use of Aegis to ensure that our Web is of high quality
has been equally successful, and is a model we would happily recommend.
.nh 1 "References"
.IP \(bu
Miller, P. A.,
.ft I
\[lq]Aegis Is Only For Software, Isn't It?,\[rq]
.ft P
AUUG '96 ACT Summer Conference Papers,
1996.
.IP [1]
Miller, P. A.,
.ft I
\[lq]Aegis - A Project Change Supervisor,\[rq]
.ft P
AUUG '93 Conference Papers,
1993,
p. 169-178.
.IP [2]
Miller, P. A.,
.ft I
Aegis User Guide
.ft R
.\" do not warn "cannot adjust line"
.warn (\n[.warn]-(((\n[.warn]/4)%2)*4))
\f(CWhttp://aegis..sourceforge.net/\fP,
.IP [3]
Mockapetris, P.,
.ft I
\[lq]Domain Names - Concepts and Facilities\[rq],
.ft P
STD 13,
RFC 1034,
USC/Information Sciences Institute,
November 1987.
.IP [4]
Mockapetris, P.,
.ft I
\[lq]Domain Names - Implementation and Specification\[rq],
STD 13,
RFC 1035,
USC/Information Sciences Institute,
November 1987.
.ft P
.so lib/en/auug96/foil.01.so
.so lib/en/auug96/foil.02.so
.so lib/en/auug96/foil.03.so
.so lib/en/auug96/foil.04.so
.so lib/en/auug96/foil.05.so
.so lib/en/auug96/foil.06.so
.so lib/en/auug96/foil.07.so
.so lib/en/auug96/foil.08.so
.so lib/en/auug96/foil.09.so
.so lib/en/auug96/foil.10.so
.so lib/en/auug96/foil.11.so
.so lib/en/auug96/foil.12.so


// vim: set ts=8 sw=4 et :
