#!/bin/sh
#
# aegis - project change supervisor
# Copyright (C) 1991-1998, 2006-2008, 2012 Peter Miller
# Copyright (C) 2007 Walter Franzini
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#

TEST_SUBJECT="commands --help options"

# load up standard prelude and test functions
. test_funcs

aegis -help > test.out 2>&1
AEGIS_THROTTLE=-1
if test $? -ne 0 ; then cat test.out; fail; fi
aegis -b -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -ca -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -cd -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -cp -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -cpu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -db -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -dbu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -de -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -deu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -diff -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -ib -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -ibu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -ipass -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -ifail -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -l -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -mv -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -na -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -nc -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -ncu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -nd -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -nf -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -nfu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -ni -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -npr -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -nrls -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -nrv -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -nt -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -ntu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -pa -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -ra -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -rd -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -rm -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -rmu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -rmpr -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -ri -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -rrv -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -rfail -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -rpass -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -rpu -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -t -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi
aegis -vers -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

#
# Check the aeannotate program
#
aeannotate -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

#
# The following commands does not provide the --help option:
#
#     aecomplete
#     aecvsserver
#     aemeasure
#


aedist -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aede-policy -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aediff -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

#
# Check the aefind program
#
aefind -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aefinish -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aeimport -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aelcf -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aelpf -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aels -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aepatch -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aerect -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

ae-repo-ci -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aereport -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aerevml -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aesub -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aesubunit -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aesvt -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aetar -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

aexml -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

#
# Check the aefp program
#
aefp -help > test.out 2>&1
if test $? -ne 0 ; then cat test.out; fail ; fi

#
# the things tested in this test, worked
#
pass
# vim: set ts=8 sw=4 et :
