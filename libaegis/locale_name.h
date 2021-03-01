//
// aegis - project change supervisor
// Copyright (C) 2012 Peter Miller
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

#ifndef LIBAEGIS_LOCALE_NAME_H
#define LIBAEGIS_LOCALE_NAME_H

#include <common/nstring.h>

/**
  * The is_a_locale_name is used to test whether or not a string
  * (usually a filename component directory) looks like a valid
  * localisation locale name.
  *
  * Local names have the format
  *
  *     "{language} [ _ {territory} ][ . {charset} ][ '@' {qualifier} ]"
  *
  * In theory {language} is a 2-letter or 3-letter language code from
  * isoNNNN, but in practice can be uglier, and sometimes even contain
  * "_.@" characters.
  *
  * In theory {territory} is a 2-letter or 3-letter country code from
  * isoNNNN, but in practice can be uglier, and sometimes even contain
  * "_.@" characters.
  *
  * @param text
  *     The text string to be validated.
  * @returns
  *     true if it looks like a valid locale name, false if not.
  */
bool is_a_locale_name(const nstring &text);

/**
  * The is_a_language_name is used to test whether or not a string
  * (usually part of a filename) looks like a valid localisation
  * language name.
  *
  * Lanugae names are defined by iso-639-3 as 2-letter or 3-letter codes,
  * but in practice can be uglier, and sometimes even contain a variety
  * of punctuation characters.
  *
  * @param text
  *     The text string to be validated.
  * @returns
  *     true if it looks like a valid language name, false if not.
  */
bool is_a_language_name(const nstring &text);

/**
  * The is_a_territory_name is used to test whether or not a string
  * (usually part of a filename) looks like a valid localisation
  * territory name.
  *
  * Territory names are defined by iso-3166-2 as 2-letter or 3-letter codes,
  * but in practice can be uglier, and sometimes even contain a variety
  * of punctuation characters.
  *
  * @param text
  *     The text string to be validated.
  * @returns
  *     true if it looks like a valid territory name, false if not.
  */
bool is_a_territory_name(const nstring &text);

// vim: set ts=8 sw=4 et :
#endif // LIBAEGIS_LOCALE_NAME_H
