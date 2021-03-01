//
// aegis - project change supervisor
// Copyright (C) 1995, 2004-2006, 2008, 2012 Peter Miller
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
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//

#ifndef COMMON_LANGUAGE_H
#define COMMON_LANGUAGE_H

/** \addtogroup Language
  * \brief Locale setting
  * \ingroup Common
  * @{
  */

/**
  * The language_init function must be called at the start of the
  * program, to set the various locale features.
  *
  * This function must be called after the setuid initialization.
  * If you forget to call me, all bets are off.
  */
void language_init(void);

/**
  * The language_human function must be called to change the general
  * mode over to the default locale (usually dictated by the LANG
  * environment variable, et al).
  *
  * The language_human and language_C functions MUST bracket human
  * interactions, otherwise the mostly-english C locale will be
  * used.  The default locale through-out the program is otherwise
  * assumed to be C.
  */
void language_human(void);

/**
  * The language_human function must be called to change the general
  * mode over to the default locale (usually dictated by the LANG
  * environment variable, et al).
  *
  * The language_human and language_C functions MUST bracket human
  * interactions, otherwise the mostly-english C locale will be
  * used.  The default locale through-out the program is otherwise
  * assumed to be C.
  */
void language_C(void);

/**
  * The language_check_translations function is used to provide a warning
  * message if the short form of the error messages is being used.
  * The warning will only appear once.  This function should be called
  * <b>outside</b> any language_human/language_C bracketing.
  */
void language_check_translations(void);

/** @} */
#endif // COMMON_LANGUAGE_H
// vim: set ts=8 sw=4 et :
