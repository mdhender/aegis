//
// aegis - project change supervisor
// Copyright (C) 2007, 2008 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef COMMON_SAFE_STRSIG_H
#define COMMON_SAFE_STRSIG_H

/**
  * The safe_strsignal() function returns a string describing the signal
  * number passed in the argument @a sig.  The string can only be used
  * until the next call to safe_strsignal().
  *
  * @param sig
  *     The number of the signal
  *
  * @returns
  *     The safe_strsignal() function returns the appropriate
  *     description string, or an unknown signal message if the signal
  *     number is invalid.
  *
  * @note
  *     On some systems, a NULL pointer may be returned by strsignal for
  *     an invalid signal number.  This wrapper function cataches those
  *     cases.
  */
const char *safe_strsignal(int sig);

#endif // COMMON_SAFE_STRSIG_H
