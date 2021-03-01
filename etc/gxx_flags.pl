#
# aegis - project change supervisor
# Copyright (C) 2007 Jerry Pendergraft
# Copyright (C) 2007, 2008 Peter Miller
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
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Generate some values for g++ flags based on the compiler behavior

  # ask g++ for its version
chop(my $vers = `g++ -dumpversion`);
printf("/* vers = %s; */\n", $vers);

  # split up the value segments
my @v = split('\.', $vers);

  # Now generate a number representing the version.
  # convert such that multiple levels will be represented
  # correctly including minor versions up to 99
my $vstr = sprintf("%d%02d%02d", @v);
printf("/* vstr = %s; */\n", $vstr);

$vers = 0 + $vstr;

printf("g++_modern = %d;\n", ($vers >= 29600));

my $Wextra = "-W";

if( $vers >= 30400 )
{
    $Wextra = "-Wextra";
}
printf("c++_flags += %s;\n", $Wextra);

  # old gcc 2.96 up to new gcc 4.1.1
if( $vers >= 29600 && $vers < 40102 )
{
    printf("c++_flags += -Wno-ctor-dtor-privacy;\n");
}
