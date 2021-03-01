.\"
.\"	aegis - project change supervisor
.\"	Copyright (C) 1999-2008 Peter Miller
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
.nr NS 0
.de nh
.NH \\$1
\\$2
.if !\\$1-2 \{\
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
.ta 4n 8n 12n 16n 20n 24n 28n 32n 36n 40n
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
.\" -----------------------------------------------------------------------
.\" (Message inbox:1430)
.\" Return-Path: bud@sistema.it
.\" Return-Path: <bud@sistema.it>
.\" Received: from cartman.jna.com.au (cartman.jna.com.au [203.4.240.162])
.\" 	by rose.jna.com.au (8.8.7/8.8.7) with ESMTP id EAA16447
.\" 	for <peterm@rose.jna.com.au>; Tue, 7 Sep 1999 04:01:40 +1000 (EST)
.\" Received: from kali.jna.com.au (kali.jna.com.au [203.4.240.221]) by
.\"	cartman.jna.com.au with ESMTP id EAA22641
.\"	(8.8.5/IDA-1.6 for <peterm@jna.com.au>);
.\"	Tue, 7 Sep 1999 04:01:39 +1000 (EST)
.\" Received: (from smtp@localhost) by kali.jna.com.au id EAA24263
.\"	(8.8.5/IDA-1.6 for <peterm@jna.com.au>);
.\"	Tue, 7 Sep 1999 04:01:09 +1000 (EST)
.\" Received: from supreme.pcug.org.au(203.10.76.34),
.\"	claiming to be "pcug.org.au"
.\"   via SMTP by kookoo.jna.com.au, id smtpdAAAa005v5; Tue Sep  7 04:00:58 1999
.\" Received: (from smtpd@localhost) by pcug.org.au
.\"	(8.8.7/8.8.7/TIP-1.13) id EAA28324; Tue, 7 Sep 1999 04:00:57 +1000 (EST)
.\" Received: from cleopatra.telnetwork.it(194.20.107.100)
.\"  via SMTP by supreme, id smtpdAAAa006u0; Tue Sep  7 04:00:47 1999
.\" Received: from com1.sistema.it
.\"	(postfix@sistema.telnetwork.it [194.20.107.53])
.\" 	by cleopatra.telnetwork.it (8.9.3/8.8.7) with ESMTP id UAA22044
.\" 	for <millerp@canb.auug.org.au>; Mon, 6 Sep 1999 20:03:32 +0200
.\" Received: from NEWBUD (newbud.sistema.it [10.1.1.9])
.\" 	by com1.sistema.it (Postfix) with SMTP id F2DF32003A
.\" 	for <millerp@canb.auug.org.au>; Mon,  6 Sep 1999 19:24:36 +0200 (CEST)
.\" Message-Id: <1.5.4.32.19990906172434.006f9e18@mail.sistema.it>
.\" X-Sender: bud@mail.sistema.it (Unverified)
.\" X-Mailer: Windows Eudora Light Version 1.5.4 (32)
.\" Mime-Version: 1.0
.\" Content-Type: text/plain; charset="us-ascii"
.\" Date: Mon, 06 Sep 1999 19:24:34 +0200
.\" To: millerp@canb.auug.org.au
.\" From: "Bud P. Bruegger" <bud@sistema.it>
.\" Subject: Aegis vs CVS
.\"
.\" Dear Peter:
.\"
.\" This message is unsolicited feedback on the presentation of Aegis.  I send
.\" it in the hope that it will help you get Aegis used much more.  As a side
.\" effect, it could also help me decide whether to use CVS or Aegis.  I hope
.\" I'm not wasting your bandwidth.
.\"
.\" As an external consultant, I am the lead developer for TADinfo, a veterinary
.\" epidemiology information system developed by FAO/Empres
.\" (http://www.fao.org/empres).  The software has been prototyped in MS Access
.\" (an inherited decision, ugh) and will be ported to a Java environment soon.
.\" It will be open source.  For the further development of TADinfo, as well as
.\" to manage its documentation and related data, I am looking for a repository.
.\" Currently, the two main candidates are CVS and Aegis.
.\"
.\" I am aware that this is somewhat comparing apples and oranges since Aegis
.\" does much more than CVS--but I suppose that the minimal functionality I need
.\" is provided by CVS and I have the feeling that a lot of people will compare
.\" Aegis to CVS.
.\"
.\" The first round I looked at the above choice I went for CVS since it's so
.\" much more accessible:  quick overview, brief documentation that brings me up
.\" to speed, all kinds of clients for all kinds of platforms, lots of projects
.\" using it.
.\"
.\" Then like now, I have the impression that Aegis does things much more
.\" elegantly and in the right way; or that it's much more modular and cleaner
.\" than CVS.  Also, obviously, it offers more (the testing, and reviewing) than
.\" CVS that is very attractive.  When I tried to get more concrete about
.\" things, my problem was that Aegis seems very inaccessible:
.\" * big manuals as opposed to short fact sheets and tutorials of CVS,
.\" * much stress on the features that go beyond CVS, that I would probably only
.\" explore in a second step (for didactic reasons).
.\" * lack of a comparison to CVS  (that, for example, seems to have some of the
.\" same advantages over RCS that Aegis has).
.\"
.\" I heard that your proposal to use Aegis for Linux kernel development some
.\" month ago received very little echo--and I was wondering whether describing
.\" my problems to you could not help present Aegis better to receive the
.\" attention it deserves.  This is some kind of unsolicited advice, I suppose.
.\" I hope you are not offended by this.  I suppose that in the process of
.\" checking out Aegis, I can try to contribute to a better description  (I'm
.\" trying not to promise too much that I won't be able to actually do due to
.\" too pressing deadlines).
.\"
.\" In the following, I write down some notes of how I approach the exploration
.\" of Aegis, what kinds of questions I seek an answer for, and what IMHO may
.\" make Aegis much more accessible.  I hope this is helpful.
.\"
.\" * Mindset:  My base-need (and I assume that I'm not alone here) is a version
.\" control system that supports distributed access and parallel development.
.\" All the rest is a nice add-on that I originally thought of solving in
.\" different ways (even that may not work nicely).  My approach to getting
.\" familiar with Aegis is to turn all the building, testing, review forced by
.\" other person, etc. off to keep the first step simpler.  Once more familiar
.\" with this step, I'd add the cool things that Aegis offers one by one.  This
.\" seems to be the opposite approach to what the manual takes.  Didactically, I
.\" think it will make things much easier.
.\"
.\" * In this context, a comparison to CVS would be very valuable.  It could
.\" give a brief and quick answer to the following question:  Can I do
.\" everything that I can do with CVS with Aegis; what is easier, what is
.\" simpler, and what can I add on top once the simple things are running?
.\"
.\" Some concrete questions in this context include the following.  I'm
.\" currently studying large sections of the manual to figure out questions for
.\" these:
.\"
.\" - While the Aegis manual writes about 2 weeks of work per change and
.\" submitting only complete, consistent change sets, can a developer get
.\" version control services for much smaller and but possibly inconsistent
.\" change sets (if that term still applies).  Is that done by branching and
.\" disabling all the various consistency enforcing features on that branch?
.\"
.\" - If I do distributed development, and in the situation where a CVS
.\" developer would locally install one of the many clients for her specific
.\" platform, do I need to install Aegis (at least in a single user version).
.\" >From what I have read so far, this seems to be the case (haven't gotten to
.\" the Web section yet that may prove me wrong).  If my believe is correct, a
.\" binary distribution for win32, even if it is single user, would be a
.\" prerequisite for me to use.  Since I don't have the Cygnus development
.\" environment installed, the entry hurdle for Aegis becomes very high.
.\"
.\" - Some of the documentation is confusing: Examples are the following:
.\"   * while I know you proposed Aegis for Linux kernel development, the manual
.\" states that
.\"     kernel development is the least suitable application of Aegis.
.\"   * While there is a web interface for Aegis, you write about it as intranet
.\" solution, rather than
.\"     an Internet solution.  Does this imply that the web interface is not
.\" suitable for
.\"     Internet-scale distributed development?
.\"
.\" - There seems to be a large gap between the feature list on home page
.\" getting more detail on these issues in the documentation.  The feature list
.\" is surely wetting my appetite, but to get more detail is stretching my
.\" concentration span and my available time to read through manuals.  Would it
.\" be possible to fill this gap with a one page per feature more detailed
.\" description?  If other people go through the documentation as I do, it takes
.\" too long to find out whether Aegis is the right solution once interested by
.\" its features.  An intermediate level of detail may make Aegis accessible to
.\" many more people.
.\"
.\" - It would be interesting to see a list of companies/projects that use
.\" Aegis.  CVS has this and it surely helps to attract people.
.\"
.\" - I would love an Aegis for dummies tutorial.  Something for people who
.\" don't want to read the background, comparisons, etc. that is there in the
.\" documentation.  I assume that chapter 3 in the user's manual goes in this
.\" direction, but it is too detailed and leaves out distributed development and
.\" possibly branching (see a point on finer grain/possibly inconsistent
.\" versioning services).  Interesting examples are available for CVS.  If you
.\" are interested I can find the precise URL.
.\"
.\" * Aegis seems to be an interesting testing framework.  For people who are
.\" mostly interested in this aspect, it may be a good idea to advertise Aegis
.\" also under this hat.  Freshmeat may be a good place for this.  See:
.\" http://freshmeat.net/appindex/development/test-suites.html
.\" Obviously, comparisons/relations to TET and similar would help a lot to
.\" decide whether to invest in Aegis (install, study doc, try out...).  Are
.\" those tools complementary?  Are they comparable?
.\"
.\" * Another point of accessibility:  Would it be possible to put up the
.\" manuals in HTML on top of the PS versions?
.\"
.\" You see I'm still in the process of finding out whether Aegis is the right
.\" system for my problems and I haven't understood too much yet.  I hope that
.\" my illustration of my struggles can contribute to getting Aegis used much
.\" more.
.\"
.\" Many thanks for writing (seemingly) excellent open source software and again
.\" apologies for unsolicited feedback.
.\"
.\" Kind regards
.\"
.\" Bud
.\" ----------------------------------
.\" Ing. Bud P. Bruegger, Ph.D.  	(bud@sistema.it)
.\" Sistema
.\" Information Systems	(voice general) +39-0564-418667
.\" Via U. Bassi, 54	(voice direct)  +39-0564-411682 + 41 after 3-tone prompt
.\" 58100 Grosseto          (fax)           +39-0564-426104
.\" Italy
.\" ============================================================================
.\" (Message inbox:1431)
.\" To: "Bud P. Bruegger" <bud@sistema.it>
.\" Subject: Re: Aegis vs CVS
.\" In-reply-to: Your message of "Mon, 06 Sep 1999 19:24:34 +0200."
.\"              <1.5.4.32.19990906172434.006f9e18@mail.sistema.it>
.\" Reply-To: Peter Miller <peterm@jna.com.au>
.\" Date: Wed, 15 Sep 1999 14:35:28 +0000
.\" From: Peter Miller <peterm@jna.com.au>
.\"
.\"
.\" "Bud P. Bruegger" writes:
.\" > This message is unsolicited feedback on the presentation of Aegis.  I send
.\" > it in the hope that it will help you get Aegis used much more.
.\" > I hope I'm not wasting your bandwidth.
.\"
.\" This kind of feedback is most welcome.
.\"
.\" > * lack of a comparison to CVS  (that, for example, seems to have
.\" >   some of the
.\" >   same advantages over RCS that Aegis has).
.\"
.\" This is in progress.
.\"
.\" > I heard that your proposal to use Aegis for Linux kernel
.\" > development some
.\" > month ago received very little echo--and I was wondering whether
.\" > describing
.\" > my problems to you could not help present Aegis better to receive the
.\" > attention it deserves.
.\"
.\" Mysteriously little response.
.\" You may be correct about the apparent complexity scaring users away.
.\"
.\" > I suppose that in the process of
.\" > checking out Aegis, I can try to contribute to a better description  (I'm
.\" > trying not to promise too much that I won't be able to actually do due to
.\" > too pressing deadlines).
.\"
.\" Even a few paragraphs will be welcome.
.\"
.\" But I have bigger plans... after talking to other folks about your
.\" suggestions, I plan to have a series for \[lq]mini howto\[rq] pages, on
.\" small subjects.  You suggestion of using the bullet points from
.\" the web page is an excellent starting point.
.\"
.\" > - If I do distributed development, and in the situation where a CVS
.\" > developer would locally install one of the many clients for her specific
.\" > platform, do I need to install Aegis (at least in a single user version).
.\"
.\" Yes.  I'll need to find a way to pre-package the PC version,
.\" or find someone who does this routinely (I use Linux almost always).
.\"
.\" > * while I know you proposed Aegis for Linux kernel development, the manual
.\" >   states that kernel development is the least suitable application
.\" >   of Aegis.
.\"
.\" This remark is about testing.  I have to move all of the scattered
.\" testing remarks into a single chapter.
.\"
.\" Aegis works for anything you want to version and manage: source
.\" code, documentation, webs, configuration settings, yada yada yada.
.\"
.\" > * While there is a web interface for Aegis, you write about it
.\" >   as intranet solution, rather than an Internet solution.  Does
.\" >   this imply that the web interface is not suitable for Internet-scale
.\" >   distributed development?
.\"
.\" I was trying to emphasize that Aegis will not routinely expose your
.\" proprietary secrets to the world.  It *does* work for the Internet
.\" (it's just a simple CGI script, after all) but arrangements are
.\" going to be needed to negotiate firewalls, and those arrangements
.\" tend to be beyond Aegis' scope.
.\"
.\" > - It would be interesting to see a list of companies/projects that use
.\" > Aegis.  CVS has this and it surely helps to attract people.
.\"
.\" This is the most difficult.  I don't actually know!  From time to
.\" time I get email from folks who have been using Aegis for N years,
.\" and this is the first time they have a bug.  They haven't been on
.\" the mailing list and they haven't ever talked to me before.
.\" I'll call for volunteers (soon-ish) and see what happens.
.\"
.\" > - I would love an Aegis for dummies tutorial.
.\"
.\" So would I!  Unfortunately, I'm probably exactly the *wrong* person
.\" to write it.  I hope the mini howto stuff will cover a lot of this.
.\"
.\" > Obviously, comparisons/relations to TET and similar would help a lot to
.\" > decide whether to invest in Aegis (install, study doc, try out...).  Are
.\" > those tools complementary?  Are they comparable?
.\"
.\" Probably complementary.  Aegis delegates all testing, so the
.\" comparison would be between Bourne shell and TET, not Aegis and
.\" TET, I imagine.  I'll have a look, soon-ish.
.\"
.\" > * Another point of accessibility:  Would it be possible to put up the
.\" > manuals in HTML on top of the PS versions?
.\"
.\" Sort of.  Translating groff input into HTML isn't trivial.
.\" Uploading becomes a pain.
.\"
.\"
.\" Thanks for the excellent feedback.
.\" I hope to hear another installment of your evaluation saga soon.
.\" -----------------------------------------------------------------------
.de BP
.bp
.if o \{\
\&.
.bp
.\}
..
.so introductio.so
.BP
.so cheat.so
.BP
.so first_time.so
.bp
.so recreate.so
.bp
.so new_project.so
.BP
.so move_projec.so
.BP
.so team_work.so
.BP
.so python.so
.BP
.so branch.so
.BP
.so developer.so
.\" -----------------------------------------------------------------------
.bp
.if e \{\
\&.
.bp
.\}
.\" do not warn "numeric expression expected"
.warn (\n[.warn]-(((\n[.warn]/2)%2)*2))
.nr % 1 i
.if t .1C
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
