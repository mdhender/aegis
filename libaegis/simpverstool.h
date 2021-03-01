//
//	aegis - project change supervisor
//	Copyright (C) 2005, 2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_SIMPVERSTOOL_H
#define LIBAEGIS_SIMPVERSTOOL_H

#include <common/nstring.h>
#include <libaegis/compres_algo.h>
#include <libaegis/output.h>

class rfc822; // forward
class rfc822_functor; // forward
class output_ty; // forward

/**
  * The simple_version_tool class is used to represent the current state
  * of a simple version recording tool.
  *
  * It stores all versions end-to-end in a compressed file, with RFC
  * 882 headers between, to hold things like the version number and the
  * content length.  The trick is, it's all compressed using GNU gzip.
  *
  * The idea is the same one as behind xdelta (RFC nnnn).  Because
  * all the versions are substantially similar most of the time, the
  * compression state carries over between versions, resulting in very
  * high compression ratios for versions later in the file.
  *
  * We put the head revsion first, simply because it is accessed most
  * often.  It doesn't change the premise, or the results.
  *
  *
  * End-to-end issues:
  *
  * Each version also has an Adler 32 checksum stored in the header.
  * This lets us check that the data is valid when we extract it.  The
  * GNU gzip format also has an Adler 32 checksum in the trailer, and
  * this is used for for confirming that previous version are still
  * valid when we do a new checkin.  The checksum in each version is
  * because we don't always read to the end of the compressed file, ans
  * to the checksum in the trailer is not verified.
  */
class simple_version_tool
{
public:
    /**
      * The destructor.
      */
    virtual ~simple_version_tool();

    /**
      * The constructor.
      *
      * @param &history_file_name
      *     The name of the file being used to store the version history.
      * @param comp_alg
      *     The compression algorithm to use.  Defaults to the best
      *     available (may be slow, may use much memory).
      */
    simple_version_tool(const nstring &history_file_name,
       	compression_algorithm_t comp_alg = compression_algorithm_not_set);

    /**
      * The checkin method is used to add another version to the file
      * history.  It will be stored in the history file supplied to the
      * constructor.
      *
      * @param input_file_name
      *     The name of the file to read for the latest version of the file.
      * @param meta_data
      *     Additional information to include in the file header.
      */
    void checkin(const nstring &input_file_name, const rfc822 &meta_data);

    /**
      * The checkin method is used to add another version to the file
      * history.  It will be stored in the history file supplied to the
      * constructor.
      *
      * @param input_file_name
      *     The name of the file to read for the latest version of the file.
      */
    void checkin(const nstring &input_file_name);

    /**
      * The list method is used to extract and process the meta data of
      * each version, from most recent to least.
      *
      * @param arg
      *     The functor used to handle the data.
      * @returns
      *     boolean; false if any of the functor calls returned false,
      *     true if they all returned true (or the file was empty).
      */
    bool list(rfc822_functor &arg);

    /**
      * The checkout method is used to extract version of the file from
      * the history archive.
      *
      * @param output_file_name
      *     The name of the file in which to write for the selected
      *     version of the file.  (The file name "-" understood to mean
      *     the standard output.)
      * @param version
      *     The version of the file to extract.  Version zero is
      *     understood to mean the head revision (the most recently
      *     checked in version).
      */
    void checkout(const nstring &output_file_name, const nstring &version);

    /**
      * The checkout method is used to extract version of the file from
      * the history archive.
      *
      * @param os
      *     The output stream on which the selected file version is to
      *     be written.
      * @param version
      *     The version of the file to extract.  Version zero is
      *     understood to mean the head revision (the most recently
      *     checked in version).
      */
    void checkout(output::pointer os, const nstring &version);

private:
    /**
      * The &history_file_name instance variable is used to remember the
      * name of the file being used to store the version history.
      */
    nstring history_file_name;

    /**
      * The compression algorithm
      */
    compression_algorithm_t compression_algorithm;

    /**
      * The default constructor.  Do not use.
      */
    simple_version_tool();

    /**
      * The copy constructor.  Do not use.
      */
    simple_version_tool(const simple_version_tool &);

    /**
      * The assignment operator.  Do not use.
      */
    simple_version_tool &operator=(const simple_version_tool &);
};

#endif // LIBAEGIS_SIMPVERSTOOL_H
