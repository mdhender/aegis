//
//	aegis - project change supervisor
//	Copyright (C) 2006, 2008 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program. If not, see
//	<http://www.gnu.org/licenses/>.
//

#ifndef LIBAEGIS_COMPRES_ALGO_H
#define LIBAEGIS_COMPRES_ALGO_H


enum compression_algorithm_t
{
    compression_algorithm_not_set,
    compression_algorithm_none,
    compression_algorithm_gzip,
    compression_algorithm_bzip2,
    compression_algorithm_unspecified,
};

/**
  * The compression_algorithm_by_name function is used to turn an
  * algorithm name into an algorithm enum value.  Typically this is used
  * to translate command line options.
  *
  * @param name
  *     The string to translate.
  * @returns
  *     algorithm enum value
  */
compression_algorithm_t compression_algorithm_by_name(const char *name);

/**
  * The compression_algorithm_name function is used to translate a
  * compression algorithm enum value into the equivalent string.
  * This is typically used in error messages and trace output.
  *
  * @param x
  *     The value to be translated.
  * @returns
  *     pointer to C string.  DO NOT free or delete.
  */
const char *compression_algorithm_name(compression_algorithm_t x);

/**
  * The compression_algorithm_extension function is used to obtain
  * the file name extension corresponding to the given compression
  * algorithm.
  *
  * @param x
  *     The value to be translated.
  * @returns
  *     pointer to C string.  DO NOT free or delete.
  */
const char *compression_algorithm_extension(compression_algorithm_t x);

#endif // LIBAEGIS_COMPRES_ALGO_H
