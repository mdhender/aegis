.\"
.\" The is the source of the paper
.\"	Aegis - A Project Change Supervisor
.\"	Copyright (C) 1993, 2006 Peter Miller
.\"
.\" This paper appeared in the proceedings of the AUUG'93 conference.
.\"
.\" It is a troff -mm source file and
.\" it should be preprocessed by pic and tbl, e.g.:
.\"	pic aegis.mm | tbl | troff -t -mm | lpr -t
.\" or you could use groff
.\"	groff -p -t -mgm aegis.mm | lpr
.\"
.\" Any queries concerened with copyright should be addressed to the
.\" Principal Information Officer, AGSO.
.\"
.\" Any matter regarding the content of the paper should be referred to
.\" Peter Miller <millerp@canb.auug.org.au>
.\"
.if n .ftr CB B
.if n .ftr CI I
.if n .ftr CW R
.if n .ftr C  R
.so version.so
.nr UX 0
.de UX
.ie \\n(UX \s-1UNIX\s0\\$1
.el \{\
\s-1UNIX\s0\\$1\\*F
.FS
\s-1UNIX\s0 is a trademark of Bell Laboratories.
.FE
.nr UX 1
.\}
..
.S 10
.TL


Aegis

A Project Change Supervisor


.AF "Australian Geological Survey Organisation"
.AU "Peter Miller"
.AS
.ll -0.5i
.in +0.5i
Many CASE systems attempt to provide everything,
including bubble charts, source control and compilers;
even if you don't like one of the tools, you are stuck with it.
In contrast,
.UX
utilities provide many components of a CASE system \- compilers,
editors, dependency maintenance tools (such as \fImake\fP),
and source control tools (such as RCS).
You may substitute the tool of your choice if you don't like
the ones supplied with the system.
Aegis adds
software configuration management
to these tools.
True to
.UX
philosophy,
Aegis does not dictate the choice
of any of the other tools (although it may stretch them
to their limits).
.in -0.5i
.ll +0.5i
.AE
.MT 4 1
.PF "'Aegis'Peter Miller'Page %'"
.ds HF 3 3 3 3 3 3 3
.ds HP 12 12 12 12 12 12 12
.nr Hb 1
.nr Hs 1
.if t .2C
.H 1 Introduction
Aegis performs some of the tasks increasingly referred to
as software configuration management (SCM).
It supervises the development of changes to a project and
the integration of those changes back into the master source of the project.
.P
There are existing programs,
such as RCS or CVS, which could do some of this task.
The difference is that Aegis does not allow changes to
be unconditionally added to the master source.
It enforces a number of requirements,
each designed to ensure that the project
does not "go backwards" because of a change.
.P
The word aegis was chosen as the name because of its meaning:
.DS
.ce 1
\fBaegis\fP (ee.j.iz) \fIn.\fP, a protection, a defence.
.DE
In Greek mythology,
the god Zeus had a shield called Aegis,
which provided a supernatural defence.
While
Aegis does not claim supernatural powers,
it does provide a way of managing changes to a software project
and a solution to many of the problems
encountered when a team of developers write software.
Some common examples of these problems include:
.BL
.LI
bugs which refuse to die;
.LI
lost changes,
from developers "tripping over" each other;
.LI
not knowing who changed the source,
or why;
.LI
using the wrong versions of the sources to build the project;
.LI
not having a working copy to demonstrate to anxious management,
or anxious customers.
.LE
.P
While Aegis can help solve these problems,
and many others,
it cannot solve every problem,
it is not a silver bullet.
.H 1 "What is SCM?"
Software configuration management (SCM)
is a large and increasingly complex discipline.
It can be briefly described as
consisting of a number of parts,
which include:
.VL 0.25i
.LI "Manifest Control"
.br
It is necessary to know what all the source files of a project are,
and where they can be found.
It is also necessary to know when they were added or removed.
.LI "Version Control"
.br
It is necessary to know which version of each source file
is used.
It is necessary to be able to recreate earlier versions
of the project from this information.
.LI "Build Control"
.br
It is necessary to know how to construct the object of
the project from the source files.
.LI "Change Control"
.br
It is necessary to know who performed each change and when,
who initiated each change and why.
.LI "Quality Control"
.br
It is necessary to know that the changes made
to your project meet your quality criteria.
It is essential that changes do not "break" an otherwise working project.
.LE
.P
The last item on this list is frequently absent from SCM systems,
and is a major focus of Aegis' design.
.H 2 "Development Model"
The master source of a project,
and all the implications flowing from it,
such as object files and executables,
and all the tests,
is called a
.IR baseline ,
to use common SCM terminology.
.P
Aegis is designed to try to ensure that
the baseline always
.IR works ,
where
"works"
is defined as passing all the tests in the baseline.
.P
All file history tools include two functions:
you can "check out" a file for editing,
and you "check in" the file when you are finished.
The concept may be generalized for sets of files.
The problem with using
such a simple process
is that the "check in" is unconditional.
Aegis breaks the "check in" into several steps,
so that inadequate or defective alterations to the baseline
are far less likely.
.P
In Aegis,
the unit of change to the baseline is, un-originally, called a
.IR change .
Each change must be atomic,
it must leave the baseline in a working state,
and must not depend on any other change being performed simultaneously.
For example,
when the interface to a function is altered,
the change must also include alterations to every call of that function.
.P
Aegis tracks all the source files included in a change,
and sufficient history information for each file
so that when a change is finished,
an algorithm similar to that used in RCS-Merge
or CVS-Update may be employed to resolve any problems
caused by the ability to
simultaneously
include the same source file in several different changes.
.P
Developers may not directly edit the baseline.
The baseline is updated by a user called the
.IR integrator ,
who integrates the baseline with the change,
and then validates the result,
before accepting it as the new baseline.
.H 1 "Change Control"
Change control in Aegis is implemented as six states which
a change must pass through.
Various criteria must be met to leave one state and advance to the next.
See Figure 1 for an overview of the change states.
.DF
.PS
boxwid = 1
down
S0: arrow " new" ljust " change" ljust
S1: box "awaiting" "development"
arrow " develop" ljust " begin" ljust
S2: box ht 1 "being" "developed"
arrow " develop" ljust " end" ljust
S3: box "being" "reviewed"
arrow " review" ljust " pass" ljust
S4: box "awaiting" "integration"
arrow " integrate" ljust " begin" ljust
S5: box "being" "integrated"
arrow " integrate" ljust " pass" ljust
S6: box "completed"
spline -> from 1/3<S1.nw,S1.sw> then left 0.75 then up 2/3 "new" "change" "undo"
T1: spline -> from 1/2<S2.nw,S2.w> then left 0.75 then up 11/12 \
	then to 1/3<S1.sw,S1.nw>
" develop" ljust " begin" ljust " undo" ljust at T1.c - (0.75,0)
T2: spline -> from S3.w then left 0.5 then up 1 \
	then to 1/2<S2.sw,S2.w>
" develop" ljust " end" ljust " undo" ljust at T2.c - (0.5,0)
T3: spline -> from 1/3<S4.nw,S4.sw> then left 1 then up 2.25-1/12 \
	then to S2.w
"" "" " develop" ljust " end" ljust " undo" ljust at T3.c - (1,0)
T4: spline -> from S5.w then left 0.75 then up 11/12 then to 1/3<S4.sw,S4.nw>
" integrate" ljust " begin" ljust " undo" ljust at T4.c - (0.75,0)
T5: spline -> from 1/3<S3.ne,S3.se> then right 0.5 \
	then up 1 then to 1/3<S2.se,S2.ne>
"review " rjust "fail " rjust at T5.c + (0.5,0)
T6: spline -> from S4.e then right 0.5 then up 11/12 then to 1/3<S3.se,S3.ne>
"review " rjust "pass " rjust "undo " rjust at T6.c + (0.5,0)
T7: spline -> from S5.e then right 1 then up 3.5-1/12 then to 1/3<S2.ne,S2.se>
"integrate " rjust "fail " rjust "" "" at T7.c + (1,0)
.PE
.ce 1
\fBFigure 1:\fP Change States and Transitions
.sp 0.5
.DE
.H 2 "Awaiting Development"
Not all members of the team may create changes.
This is controlled by an access list,
and thus may be as restricted or open as your project requires.
Once a change has been created,
it is in the
.I "awaiting development"
state.
A change consists of little more than a description in this state.
.H 2 "Being Developed"
Not all members of the team may develop changes.
This is controlled by an access list,
and thus may be as open or restricted as your project requires.
.P
A variety of methods may be used to assign changes to developers,
but at some point,
either from receiving instructions to do so,
or browsing and finding one,
a developer assigns a change to herself.
Once a change is assigned to a developer,
a development directory is created for it,
and it advances to the
.I "being developed"
state.
.P
This state is the coal face.
Project source files can be edited in this state only.
Source files which are to be edited are copied from the baseline
into the development directory.\*F
.FS
Only those files which are to be edited need to be copied.
The baseline acts as a cache for object files
not present in the development directory.
.FE
Aegis is used to copy the files,
so that it knows which files are being modified by this change.
Files can also be created or deleted by a change.
Again,
Aegis is used to do this,
so that it knows what is happening.
You don't have to issue two commands this way,
one to tell
.UX
to do it,
and another to tell Aegis that you have done it.
.P
.B "\s+2Build\s0 "
Once you have edited the source files,
the change must be built.
.I Building
is the process of manipulating or translating the source files
in some way to produce the object of the project.
For programs
this usually means compiling the source files and linking them into
an executable program.
The build is performed via Aegis,
so that it can look at the exit status
and know if the build succeeded or not.
A successful build is a requirement for leaving the
.I "being developed"
state and advancing to the next.
.P
There are no facilities in Aegis for describing how to build the project.
Instead,
Aegis delegates this task to a
.I "dependency maintenance tool"
(DMT).
This delegation is stored as a
.UX
command
to be performed when a build is requested.
There is no provision for Aegis to understand
.I any
dependencies,
as these are expected to be described in the DMT's configuration file,
itself a source file of the project.
.P
Typically a DMT is a program like
.I make (1),
however this old-faithful is not able to cope with
the demands placed upon it by Aegis.\*F
.FS
Even GNU Make 3.65 is not up to the task;
the VPATH semantics are too limited.
.FE
The major problem is the two-directory structure used:
when the DMT is looking for files,
it must first search in the development directory and then in the baseline.
It is best if the DMT can do this transparently,
because it makes the rules much easier to write.
The
.I cook
program,
also written by the author,
is a DMT known to work with Aegis.
.P
.B "\s+2Test\s0 "
Each change must be accompanied by at least one test.
Except for the way tests are created,
so that Aegis knows they are tests rather than ordinary source files,
they are treated identically to source files:
they may be modified and even deleted by later changes,
by the same commands as used for non-test source files,
and they are subject to the same process.
.P
Tests are Bourne shell scripts.
They are executed via Aegis
so that it can examine the exit status,
and know if each test has passed or failed.
Having tests,
and passing them,
is a requirement for leaving the
.I "being developed"
state and advancing to the next.
.P
As changes are integrated into the baseline,
the tests which accompany each change accumulate into a regression test suite.
A developer may optionally run all or part of this regression test suite
to make sure that her change has not broken any existing functionality.
Because this regression test suite grows steadily,
it is not practical to run all of it for every change;
so Aegis is designed to make it relatively easy
to run a (hopefully representative) subset.
.P
.B "\s+2Difference\s0 "
Once a change builds and tests successfully,
it is
.IR differenced .
This is the process of creating files
showing the difference between the baseline and the
development directory, for each file in the change.
This allows reviewers to examine all the edits made by the developer,
not just the ones they can find.
The difference command is configurable,
as appropriate for each project.
.P
.B "\s+2Conflicts\s0 "
This difference stage is also where problems with
out-of-date files are resolved.
If two developers copy the same revision of the same file into two different
changes, one of them will be integrated into the baseline before the other,
hence the possibility of one or more files being out-of-date.
A three-way merge
between the common ancestor,
the file in the development directory,
and the current file in the baseline,
is performed.\*F
.FS
This is the same algorithm as used by CVS-Update and RCS-Merge.
.FE
This produces a merge of the two competing edits,
which the developer should then examine to make sure the automatic merge
has produced sensible results.
This merge tool is also configurable,
as appropriate for each project.
The out-of-date file is then marked as up-to-date,
and the change will require another build and test,
to ensure that the merge has not broken anything.
.H 2 "Being Reviewed"
Once the files are up-to-date, built and tested,
the developer may advance the change to the
.I "being reviewed"
state.
At this point,
all the source files in the change are locked,
preventing any other changes with files in common
from advancing to the
.I "being reviewed"
state.
If any change file is already locked,
the developer will be told to try again later.
.P
The style of review is not dictated by Aegis.
The only requirement is that an authorised user
tell Aegis that the review passed or failed.
Not all members of the team may review changes.
This is controlled by an access list,
and thus may be as restricted or open as your project requires.
.P
A number of review schemes have been observed.
Two extremes are presented here:
.BL
.LI
A single team member is responsible for coordinating all reviews.
Each review is performed by a panel of four team members in addition
to the developer of the change.
Only the review coordinator,
after receiving the paperwork from the review panel,
may pass or fail reviews.
.LI
Any developer may review any change;
this is done informally.
Aegis prevents a developer from reviewing her own change,
to avoid an obvious conflict of interests.
.LE
.P
Many other review styles are possible,
but the one best for your project will probably fall
between these extremes.
.P
Reviewers know several things about
a change in the
.I "being reviewed"
state,
because of the requirements for getting there.
.BL
.LI
The change is known to build successfully.
.LI
The change is known to have tests and to have passed them.
.LI
The source files in the change are known to
be up-to-date with respect to the baseline.
.LE
.P
This allows reviewers to concentrate on completeness of the code,
completeness of the tests,
and standards issues, etc.
.P
If a change fails review,
it is returned to the
.I "being developed"
state for further work by the original developer.
The reviewer is not responsible for fixing problems found by the review.
.H 2 "Awaiting Integration"
Once a change passes review it is advanced to the
.I "awaiting integration"
state.
This state is a queue.
Only one change at a time is integrated into the baseline,
even though all changes in this state have no files in common.
This allows clear indications of which change is at fault,
should the integrator discover that there are problems.
See Figure 2 for a diagram of how files flow through this model.
.DF
.PS
boxwid = 0.75
down
B1: box "baseline"
move left down
B2: box "development" "directory"
move right down
E1: ellipse "integrator"
arrow "integrate " rjust "begin " rjust
B3: box "integration" "directory"
S1: spline -> from B3.s then down then right 1.25 then up \
	then to B1.n right 1.25 then to B1.n up right 1.25 \
	then to B1.n up then to B1.n
"integrate " rjust "pass " rjust at S1.c+(1.25,-0.75)
arrow from B2.s to E1.nw
arrow from 1/3<B1.sw,B1.se> to B2.n

move to B1.s
move right down
B4: box "development" "directory"
arrow from 1/3<B1.se,B1.sw> to B4.n
arrow from B4.s to E1.ne
.PE
.sp 0.5
.ce 1
\fBFigure 2:\fP Flow of Files through the Model
.sp 0.5
.DE
.H 2 "Being Integrated"
Not all members of the team may integrate changes.
This is controlled by an access list,
and thus may be as open or restricted as your project requires.
.P
To integrate
a change with the baseline,
an integration directory is created
by copying the baseline,
or more usually creating a logical copy using links.
The change is then applied to this integration directory.
.P
The integration copy is then built and tested.
This is to ensure that it was not
just
some quirk of the developer's environment
that allowed the change to get this far,\*F
.FS
For example,
a weird environment variable setting,
or a bogus
.I cc
command in the PATH which always exits success.
.FE
and also to have all files in the new baseline
consistent with each other.
.P
The integrator may choose to run a representative
subset of the regression test suite,
in addition to the tests which accompanied the change.
.P
In addition to rejecting a change because it fails to build or test,
an integrator may also act as a reviewer;
this is a good place to watch the watchers,
and monitor the quality of reviews.
Integrators may thus veto a change even though it builds and tests successfully.
.P
If a change fails integration,
the integration directory is removed,
and the change is returned to the
.I "being developed"
state for further work by the original developer.
The integrator is not responsible for fixing any problems found.
.H 2 "Completed"
Once a change builds and tests successfully,
it may be advanced to the
.I "completed"
state.
The file histories are updated at this point,
and the file locks released.
The old baseline is removed,
and the integration directory is renamed to be the baseline.
The development directory is also removed.
.P
Unlike earlier states,
a change in this state cannot be reversed.
If you subsequently wish to remove the change,
you will need to create another change and repeat the
whole process with all edits in reverse.
.P
A change consists of a description and a list of
files and versions in this state.
A full history of state transitions has been kept,
including who performed them and when.
.H 1 "Quality Control"
Quality is addressed in a number of ways.
As you can see from the above description of how
Aegis manages change control,
the mandatory testing and reviewing are steps in this direction.
Exactly
.I what
is tested or reviewed is up to you,
but the places exist in the process for them to be done.
They can be as elaborate or simple as your project requires.
Note that there is more to software quality than these two items,
and they are not the only places where tests and reviews can take place.
.H 2 "Does it work?"
A major advantage of Aegis is the ability to answer
the question "Does it work?".
This question is asked from a number of perspectives:
.BL
.LI
The developer wants to know "does this change work?".
Aegis provides the answer with tests for each change.
Developers have always tested their code,
but Aegis provides ergonomic advantages,
never forgets to test something,
and the tests are preserved for future use.
.LI
The integrator wants to know "does this change break anything else?".
Aegis provides the answer with a constantly growing regression test suite,
and also makes the developer's own tests available to the integrator.
.LI
The project leader, and management further up the tree,
want to know "does the project work?".
They want to be able to touch and feel progress towards the target,
and they want some confidence that
the project will not cease to work
at random (but usually disastrous) times.
Aegis provides the answer here in the form of a baseline which always works,
and is always available for demonstrations.
.LE
.P
The various mandatory tests and validations are configurable
for each project (and in some cases, for each change).
You may use as many or as few of the safeguards provided by Aegis
as you need for each project.
.H 2 "No Back Door"
Another issue is whether there is a "back door" into the process,
so that a developer who finds the process tedious
can avoid it and just "fix" the baseline directly.
With Aegis, there is no way to circumvent the process.
.P
Access to the baseline is read-only for the development team,
including the integrator.
Access is protected by the
.UX
group and
.I umask
mechanisms.
A
.UX
group and umask is associated with each project,
and any commands Aegis executes will arrange to have the
appropriate group and umask,
to ensure that all users in that group have access (even if
the user has a different default group).
.P
All developer commands are run as the developer,
and hence have the developer's permissions.
All integration commands are run as the
.I "project owner"
who is usually not
the integrator (so the integrator can't edit, even if she wants to).
It is only ever necessary to login as the project owner
to perform actions beyond Aegis' scope,
such as recovering after a disk crash.
.H 1 "Manifest Control"
Aegis remembers where all the source files are.
They are initially created in changes,
and only exist in the baseline after a successful integration.
The location of the baseline and all development directories are known,
and the names of all the source files in them are known.
.H 1 "Version Control"
As described above,
Aegis delegates the file
history maintenance to the package of your choice.
All Aegis requires is the ability to determine the latest version number
for each file's history at integrate pass,
so that the random string (it need not look anything like a number)
may be used later to extract an earlier version,
should it be needed.
.P
The version string for each source file in the baseline
and each development directory is known,
so that the difference and merge facility described above can work.
.P
In addition,
a project version may be specified when copying files from the baseline
into a change.
Thus
an earlier version of the project may be recreated,
in order to reproduce a bug, for example.
.H 1 "Build Control"
The change control description
mentioned that build control is delegated
to the dependency maintenance tool (DMT) of your choice.
.H 2 Capabilities
The DMT needs to be able to cope with the
fundamental concept of two directories.
This is a "search path" for every file,
no matter what the file is used for.
.P
The baseline contains all
the implications flowing from the source files,
this typically means the object files from compilations
and the linked executable.
It could also include
documentation and manual entries formatted
from appropriate source files.
.P
The development build may thus compile a minimum of code,
and link the rest from the baseline,
minimizing disk usage and compile time across all developers.
.P
There is a catch:
the dependency maintenance tool
must be able to detect when
an include file in the development directory
logically invalidates an object file in the baseline,
necessitating re-compilation of a baseline source file,
and leaving the object file in the development directory
for linking.
.P
Experience has shown that the various
.I makedepend
programs do not work very well.
What is most needed is the ability to determine
the include files "on the fly".
This implies the ability to give
DMT
rules like
.DS 1
.ft C
%.o: %.c `\fIincludes-of\fP %.c`
	$(CC) -c %.c
.ft R
.DE
where
.I includes-of
is a program to be invoked when the rule is matched,
rather than when it is read (note the back quotes).
.H 2 Dependencies
The DMT is expected to know
.I all
project dependencies.
This functionality is completely delegated,
and so Aegis knows
.I nothing
about any dependencies.
.P
The configuration file for the DMT is a project
source file,
and
therefore is altered by the same process.
.br
.ne 2i
.H 1 "Directory Structure"
Aegis attempts to dictate as little
as possible about the directory structure of the projects it supervises.
There is one mandatory file,
and one mandatory directory.
The mandatory file contains Aegis' configuration
information for the project,
the mandatory directory contains all tests for the project.
The configuration file,
and all tests,
are treated as source files,
and are subject to the same change process.
.P
The source directory tree of each project may be as
deep or as shallow as required.
.P
The placement of project directories is completely configurable.
Each project may be owned by a unique user if desired,
and Aegis can manage many projects simultaneously.
Security is through the
.UX
groups mechanism,
so it can be as open or restricted as required.
.P
When a change is being developed,
it has its own development directory.
This development directory is a subset of the baseline.
Only those source files which need to be edited
are present in the development directory.
.H 1 Applicability
There are some projects which are well suited to
supervision by Aegis,
and there are others which are not.
.P
Ideally suited projects are programs which take a set of input files,
process them, and generate a set of output files.
Test cases may be easily generated,
and actual output compared with expected output.
.P
Another class of programs have full-screen text interfaces
or GUIs
and thus are not so well suited to supervision by Aegis.
Because tests are Bourne shell scripts,
only the functionality accessible from the command line
can be automatically tested.
In these cases there are three options:
.BL
.LI
Change the program to optionally accept fake input and to
write screen dumps to files,
thus providing a testable case.
.LI
Change the program to allow access to the functionality from the command line,
thus providing a testable case.
Note that this cannot test the user interface.
.LI
Do not do any automated testing.
This may be configured for a single change or for a whole project.
You still get the supervision aspects of Aegis,
but no regression test suite.
.LE
.P
The least suitable class of programs
for supervision by Aegis
are stand-alone programs.
Operating systems and embedded systems are members of this class.
The program in a hand-held calculator,
for example,
would be extremely difficult to test from a shell script.
It is possible to test this class of programs with the right hardware,
but it is usually impractical.
.H 1 Summary
This paper has given a very short overview of Aegis,
and described a few of its strengths.
Things to remember about Aegis include:
.P
Aegis is designed to be a small piece in a larger system,
like many other
.UX
utilities.
.P
Aegis is a project change supervisor,
it performs part of what is becoming known as
software configuration management (SCM).
This provides control for manifest, versions, building, changes and quality.
.P
Aegis is not a history tool, such as RCS.
It is layered above such a tool.
.P
Aegis is not a dependency maintenance tool, such as
.IR make (1).
It is layered above such a tool.
Any dependencies which cannot be expressed in the rules file of the DMT,
cannot be expressed by Aegis.
.P
Aegis is not a bug tracking system,
it has no mechanisms for tracking bugs
and telling you which are fixed and which are not.
However,
there are notification hooks to liaise with such a system.
.P
Aegis does not draw
Gantt charts, bubble charts, flow charts, or any other pretty pictures.
It does not itself generate any code.
It is not a CASE system,
it is a component of a CASE system.
.P
Aegis attempts to dictate as little as possible about each project.
It dictates very little directory structure,
and it does not dictate test content or the review method.
Reviews and tests
in addition to those required by Aegis
may be performed.
.P
Aegis is free.
This means that it has an excellent cost/benefit ratio,
compared to commercial products,
even if it doesn't have all their features.
.br
.ne 2i
.H 1 Availability
You can get Aegis by WWW from
.TS
center,tab(;);
l lf(CW) s
l lf(CW) lf(I).
URL:;http://www.canb.auug.org.au/~millerp/
File:;aegis.\*(v).tar.gz;the full source
File:;aegis.\*(v).ps.gz;the User Guide
.TE
.P
Aegis is distributed under the terms and
conditions of the GNU General Public License.
Aegis is Copyright \(co \*(Y) Peter Miller
.P
This paper is
Copyright \(co 1993 Australian Geological Survey Organisation.
Apart from any fair dealings for the purposes of study,
research, criticism or review, as permitted under the Australian
.IR "Copyright ACT 1968" ,
no part may be reproduced
by any process without prior written permission.
Copyright is the responsibility of the
Executive Director.
Inquiries should be directed to the
Principal Information Officer.
.H 1 "References"
The following free software,
and their documentation,
are referred to in this paper:
.VL 0.25i
.LI \(bu
Miller, P. A.,
"Aegis - A Project Change Supervisor,"
.I "AUUG '93 Conference Papers,"
1993, p. 169-178.
.LI [1]
RCS 5.6
.br
Copyright \(co 1982, 1988, 1989 by Walter F. Tichy.
.br
Copyright \(co 1990, 1991 by Paul Eggert.
.LI [2]
CVS 1.3
.br
Copyright \(co 1986, 1988-1992 Free Software Foundation, Inc.
.br
Numerous authors,
principally Brian Berliner.
.LI [3]
GNU Make 3.65
.br
Copyright \(co 1988-1993 Free Software Foundation, Inc.
.br
Numerous authors,
principally Roland McGrath.
.LE
.P
All of these programs may be fetched by FTP from your
closest GNU archive site.
Within Australia,
you can find them at
.I archie.au
in the
.I /gnu
directory.
