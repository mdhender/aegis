'\"p
.\" aegis - project change supervisor
.\" Copyright (C) 2002, 2005-2008, 2012 Peter Miller
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
.if n .ftr CB B
.if n .ftr CI I
.if n .ftr CW R
.if n .ftr C  R
.so etc/version.so
.S 10
.TL


Testing?
What testing?

.AF "Platypus Technology"
.AU "Peter Miller"
.AS
.ll -0.5i
.in +0.5i
This paper presents a simplistic yet powerful model of what a test is.
When you intend to test your software, you have to design your software
to be test\fIable\fP.  This paper will examine attributes of software implied
by this model.  Some examples of automated testing will be given.
.in -0.5i
.ll +0.5i
.AE
.MT 4 1
.PF "'Testing?  What testing?'Peter Miller'Page %'"
.ds HF 3 3 2 2 2 2 2
.ds HP 12 11 11 11 11 11 11
.nr Hb 1
.nr Hs 1
.de E(
.ll -0.1i
.in +0.1i
.ft CW
.nf
..
.de E)
.fi
.ft R
.in -0.1i
.ll +0.1i
..
.if t .2C
.H 1 "What is a test?"
.P
The core thesis of this paper is the idea\*F
.FS
There is a growing body of knowledge called "Transaction Based testing"
or sometimes "Transaction Based Verification".
.FE
that a test
consists of three things: a system in a defined state, a defined
transaction, and a confirmation that the system arrives in a defined
state.
.PS
right
ellipse "initial" "state"
arrow 0.7 "" "transaction"
ellipse "destination" "state"
.PE
.P
This is an overly simplistic statement, but remains remarkable useful.
The "system" under test could be a simple object, a collection of
interrelated objects, a whole application, or a distributed multi-layer
client-server system.
Equally, the transaction could be a single byte of input, a single edge
of a state transition diagram, or a series of transactions lumped together
as a single event being considered.
.P
Confirming that the system under test has arrived in a particular state
can be done in may ways.  Some states are clearly visable,
sometimes they are available but not useful, and some internal states are not
for user consumption and are much harder to access and therefore harder
to confirm.
.PS
right
E1: ellipse "initial" "state"
E2: ellipse "destination" "state" with .w at E1.e+(0.7,0)
E3: ellipse "\fIoops\fP" with .n at E2.s-(0,0.25)
move to E1.e
spline -> right 0.6 then down 0.5 then left 0.25 \
    then up 0.25 then to E3.nw
.PE
.P
Please note that this is a \fIsimplistic\fP definition of a test.
It does not cover all forms of testing (such as tests of usability,
maintainability, portability, robustness and so on which make up the
other zillion software sub-characteristics listed in ISO 9126) and it
is no substitute for a well thought out test plan.  It does, however,
provide some language for talking about functional testing.
.P
.H 1 "Manual testing is no testing"
.P
Humans are really bad at boring, repetitive tasks.
If your test plan is based on the idea that your staff will faithfully
execute a long list of printed instructions, at least once per release,
then your testing is probably not effective.
.P
For example, many manual test plans contain long sequences of things the
operator is required to do, often with information on the screen to be
confirmed as correct.
This is all very well for successful tests, but what happens when one fails?
Usually, these test scripts cover large numbers of behaviors.
There is thus a motivation to complete the rest of the script,
rather than stop, and have to do the start of the script again when the
software has been fixed.
.PS
ellipseht = 0.25
ellipsewid = 0.25 * 1.6
E1: ellipse
E2: ellipse with .c at E1.c+(0.4,0.6)
E3: ellipse with .c at E2.c+(0.4,-0.6)
E4: ellipse with .c at E3.c+(0.4,0.6)
E5: ellipse with .c at E4.c+(0.4,-0.6) invis
arrow from E1.ne to E2.sw
arrow from E2.se to E3.nw
arrow from E3.ne to E4.sw
line  from E4.se to E5.nw dashed
.PE
.P
There are two themes here: (a) testers have to look "productive" or
they might not get paid, and (b) redoing the first bit again and again
is boring.
.P
Let's look at that definition again, rephrasing what our manual test
scripts are doing.  "Usually, these test scripts start from a defined
state, and define a transaction and a confirmations of the destination
state, then the next transaction and confirmation, \fIad nauseum\fP."
Now, what happens when one of those confirmations fails?  Well, we know
it's in \fIthe wrong state\fP, so going on to execute the rest of the
script, we are no longer fulfilling the initial portion of our three-part
definition: we aren't in the defined state that the transaction is to
be applied to.  After the first failure, the rest of the results are
\fIno information\fP.
.PS
ellipseht = 0.25
ellipsewid = 0.25 * 1.6
E1: ellipse
E2: ellipse with .c at E1.c+(0.4,0.6)
E3: ellipse with .c at E2.c+(0.4,-0.6)
E4: ellipse with .c at E3.c+(0.4,0.6)
E7: ellipse with .c at E2.c+(0.4,0.6) "\fIoops\fP"
arrow from E1.ne to E2.sw
move to E2.se
spline -> to E3.nw \
    then to E7.s-(0,0.2) \
    then left 0.2 down 0.1 \
    then right 0.2 down 0.1 \
    then to E7.s
.PE
.P
For effective testing, then, you need something that is very good at
accurately repeating the same script over and over again,
and reporting very promptly when something goes wrong.
Computers are very good at boring, repetitious tasks.
They don't complain when you ask them to run the same stupid scripts
tens or even thousands of times.
And if the script breaks, they stop.
For effective testing, then, you need automated testing.
Let the humans \fIwrite\fP the tests,
and let the computers \fIrun\fP the tests.
.P
.H 1 "Software Attributes"
.P
Automated testing requires the ability to automatically get
the system under test into a defined state, the ability to automatically
apply one or more transaction, and the
ability to automatically confirm the current state (either
read-and-compare, or write-and-diff, usually).
.P
Some things are easy to test, e.g.
.E(
cat > test.in
cat > test.sed
cat > expected-output
sed-clone -f test.sed test.in \e
    > test.out
diff expected-output test.out
.E)
.P
But some things require some specific changes to get the three properties.
\fIE.g.\fP a virtual machine simulator needs the ability to set registers and
stack, \fIetc\fP, and later to dump them do they can be confirmed.  This may
be observable \fIe.g.\fP as some interesting opcodes only present in the
simulator, and not the real machine, maybe to get the simulator to exit
with a success/fail indicator.
.P
.H 2 "Initial State"
.P
The system under test needs a way to be placed in a well defined initial
state.  This is something that most programs are reasonably good at.
Word processors can load a file, image processing systems can load an
image, databases can be created and populated with test sets, \fIetc\fP.
.P
It was mentioned above that transactions can actually be a series of
transactions.  Sometimes, getting the system under test into a defined
state requires starting from the default state and applying a series of
known-to-work transactions.
Provided that  you can \fIget\fP the system under test into a defined
state automatically, it can be tested automatically.
.P
.H 2 "Transactions"
.P
Automating transactions can often be the hardest part of automated testing.
Usually, this means automating the simulation of input.
This could be user input, or a network connection, or a hardware
simulation for an embedded application.
.P
.H 3 "Command Line"
.P
The design of
UNIX
makes the testing of command line programs relatively simple,
because you can redirect input from a file.
This means that you don't actually need to change your software (or not
much, anyway).
.P
.H 3 "Full Screen"
.P
Full-screen programs are often similar, with input again directed from a file,
although you may need to make it tolerant of non-tty input possibly
under the control of a command line option.
The trickier cases can be handled with \fIexpect\fP.
.P
.H 3 "GUI"
.P
On the other hand GUI interfaces are harder.  There are some utilities,
such as \fITkReplay\fP which help.  But they lead us to looking at the
problem differently: where can we inject the input?
.br
We can inject it into the X server (or have a fake X server which exists
solely to provide test input).
.br
We can proxy the X server, and inject the input via the proxy.
.br
We can inject it into the event loop of our application.
This, of course, requires changing the system under test.
.br
We can have alternate input classes, a "real" one and an "automated" one.
This, of course, means that the "real" input class doesn't get tested,
but the rest of the system does, and that may be enough.
.P
.H 3 "Client Server"
.P
Most of the techniques useful for X programs work for client server
systems as well.  Fake clients, fake servers, proxies, alternative input
classes, \fIetc\fP.
.P
.H 3 "Observation"
.P
In order to test the system, some aspect of it was changed.
Auxiliary test support, more tolerant input, multiple input sources.
.P
.H 2 "Verify State"
.P
Some programs, such as the \fIsed\fP example given above,
are relatively easy to test.
Many programs store a significant amount of state when you save to a file,
and this may be compared with \fIdiff\fP(1) or \fPcmp\fP(1).
Other systems, however, are more challenging.
.P
.H 3 "Full Screen"
.P
Many \fIcurses\fP(3) programs need a special command to dump the screen
into a text file for comparison using \fIdiff\fP(1).
It is also possible to use \fIexpect\fP in many cases.
.P
.H 3 "GUI"
.P
Many of the input solutions also work for output, but you
will probably need special commands or options to get screen dumps at
strategic moments, for comparison.
.P
Wholesale capture and comparison of the output stream is problematic,
usually because of gratuitous differences not relevant to the test.
.P
.H 3 "Client Server"
.P
You can use bogus clients, bogus servers, or clever proxies.
.P
.H 3 "Observation"
.P
In order to test the system, some aspect of it was changed.
Auxiliary test support, captured output, multiple output destinations.
.P
.H 1 "Discussion"
.P
There are some things which arise from consideration of these ideas.
.P
.H 2 "No Result"
.P
In coming up with a testing regime, it is necessary to remember that
tests do not simply \fIpass\fP or \fIfail\fP.
.P
This is further complicated by the inverted sense of some tests.
For example, your development process may require that a bug fix be
accompanied by a test which \fIfails\fP on the unfixed system, and
\fPpasses\fP on the fixed system.
.P
Consider the issues in achieving a necessary initial state by
applying transactions to an initial state.  What happens when one of these
transactions, which are not the transaction under test, \fIfail\fP?
In such a case it can't \fIfail\fP, because the bug fix case will give
a false \fIpositive\fP,
but equally it can't succeed because this renders the test meaningless.
.P
The solution is to have a third result, often called \fIno result\fP,
which when negated still means \fIno result\fP.
.P
Similar problems can occur with the transaction and verification stages
of the test.
.P
.H 2 "Negative Testing"
.P
Some other examples of negative testing will be given (i.e. \fIdidn't\fP
arrive in the right state, or invalid transactions resulting in an
invalid state change).
.P
.H 2 "Watch Me"
.P
A useful facility for creating tests is a "watch me" mode.  This is a
mode or tool or whatnot that allows the system to record inputs and
output for replay and confirmation (respectively) at a later time.
While this is \fInot\fP one of the necessary attributes, it is often
a useful side effect.
.P
.H 2 "Assert"
.P
This simple model of testing gives a different spin on the humble
\f[CW]assert\fP statement.  The use of \f[CW]assert\fP can be thought
of as verifying that the system is in a particular state, or that the
transaction (input) is valid.
This is not the kinf of artifact you \fIwant\fP to see in production code;
it is usually compiled out of production code.
.P
.H 2 "Trace on Request"
.P
Another thing which is often compiled out of production code is a variety
of tracing macros, which allow you to see the state of various portions
of the system as they are executed.
You sometimes see this in production systems, whene there is little
performance impact; it is extremely useful feature for tech support,
as well as testing.
.P
.H 1 "Testing?  What testing?"
.P
I once worked on an image processing system for which the company had
partial source, and the inner workings where supplied as a library from
the vendor.  One of the transforms had some trouble, and I fixed it,
but then I wondered how I should test it.  How many of us can confirm
visually that a 2D Walsh-Hadamard transform has worked correctly?
While the destination
state was visible on the screen, giving humans 2 side-by-side pictures (a
"does it look like this" manual test) you will almost certainly get a false
positive.  \fIE.g.\fP those "find 10 differences" cartoon pictures on
the funnies section of the newspaper.
If humans are so bad at spotting
\fIgross\fP differences, how can we expect them to find one pixel
different in a million?  So, I looked for the tool to compare two images
and tell me how many pixels were different.  \fIThere wasn't one.\fP
How did the vendor test their product?
.P
If you have testability as a requirement of your software, you will
write different software than if testability was not a requirement.
.P
Do all the tools we use every day
have these three properties:
Can their initial state be loaded automatically?
Can their transactions be applied automatically?
Can their destination state be confirmed automatically?
If any one of these is missing (but usually the last one),
what gives us any confidence that they were tested at all?
.\" vim: set ts=8 sw=4 et :
