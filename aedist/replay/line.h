//
//	aegis - project change supervisor
//	Copyright (C) 2004 Walter Franzini
//	Copyright (C) 2008 Peter Miller
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

#ifndef AEDIST_REPLAY_LINE_H
#define AEDIST_REPLAY_LINE_H

#include <common/nstring.h>

/**
  * The replay line is used to represent a line read from the output of
  * /cgi-bin/aeget/projname?inventory
  */
class replay_line
{
public:
    /**
      * The destructor.
      */
    virtual ~replay_line();

    /**
      * The default constructor.
      */
    replay_line();

    /**
      * The copy constructor.
      */
    replay_line(const replay_line &arg);

    /**
      * The assignment operator.
      */
    replay_line &operator=(const replay_line &arg);

    /**
      * The extract method is used to dismantle a line of text into its
      * component parts.
      *
      * \param arg
      *     The string representing the line to be dismantled.
      * \returns
      *     bool; false if the line is not in the correct format, true
      *     if the line was in the correct format and was successfully
      *     dismantled.
      */
    bool extract(const nstring &arg);

    /**
      * The get_version method is used to get the version part of the
      * dismantled line.
      */
    nstring get_version() const { return version; }

    /**
      * The get_uuid method is used to get the UUID part of the
      * dismantled line.
      */
    nstring get_uuid() const { return uuid; }

    /**
      * The get_description method is used to get the brief description
      * part of the dismantled line.
      */
    nstring get_description() const { return description; }

    /**
      * The get_url2 method is used to get the URL wrapped around the
      * UUID, which is the one for downloading from.
      */
    nstring get_url2() const { return url2; }

private:
    /**
      * The url1 instance variable is used to remember the change menu URL
      * extracted (it is wrapped around the version number in each row).
      */
    nstring url1;

    /**
      * The version instance variable is used to remember the version
      * extracted.
      */
    nstring version;

    /**
      * The url2 instance variable is used to remember the download URL
      * extracted (it is wrapped around the UUID in each row).
      */
    nstring url2;

    /**
      * The uuid instance variable is used to remember the UUID
      * extracted.
      */
    nstring uuid;

    /**
      * The version instance variable is used to remember the breif
      * description extracted.
      */
    nstring description;
};

#endif // AEDIST_REPLAY_LINE_H
