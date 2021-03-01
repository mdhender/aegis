//
//	aegis - project change supervisor
//	Copyright (C) 2004-2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_URL_H
#define LIBAEGIS_URL_H

#include <common/nstring.h>

/**
  * The url class is used to represent a uniform resource locator (URL).
  */
class url
{
public:
    /**
      * The destructor.
      */
    virtual ~url();

    /**
      * The constructor.
      *
      * \param s
      *     The string to be broken up to fill out the component pieces.
      */
    url(const nstring &s);

    /**
      * The constructor.
      *
      * \param s
      *     The string to be broken up to fill out the component pieces.
      */
    url(const char *s);

    /**
      * The copy constructor.
      */
    url(const url &);

    /**
      * The assignment operator.
      */
    url &operator=(const url &);

    /**
      * The split method is used to replace the URL's contents with the
      * given string, once it has been split into its component parts.
      *
      * \param s
      *     The string to be disassembled.
      */
    void split(const char *s);

    /**
      * The is_a_file method is used to determine if the URL is a simple
      * file URL.
      *
      * \returns
      *      bool; true if it's a simple file URL,
      *      false if it's something else.
      */
    bool is_a_file() const;

    /**
      * The get_path method is used to extract the path part of a file
      * URL.  The behaviour is undefined if is_a_file is false.
      */
    nstring get_path() const;

    /**
      * The set_path_if_empty method is used to set the URL's path to the
      * given default if it is not set already.
      */
    void set_path_if_empty(const nstring &dflt);

    /**
      * The set_query_if_empty method is used to set the URL's query to
      * the given default if it is not set already.
      */
    void set_query_if_empty(const nstring &dflt);

    /**
      * The reassemble method is used to reassemble the URL into a
      * string representation.
      *
      * \param exclude_userpass
      *     Set it to true if you need to exclude userpass from the
      *     result URL.  Default to false.
      */
    nstring reassemble(bool exclude_userpass = false) const;

    /**
      * The set_host_part_from method is used to set the host part (and
      * protocol, and port) from the given argument.
      *
      * \param arg
      *     The URL to take the host part from.
      */
    void set_host_part_from(const url &arg);

    /**
      * The get_protocol method is used to obtain the protocol name from
      * the URL.
      */
    nstring get_protocol() const { return protocol; }

    /**
      * The get_userpass method is used to obtain the userpass from
      * the URL.
      */
    nstring get_userpass() const { return userpass; }

    /**
      * The get_hostname method is used to obtain the host name from the
      * URL.
      */
    nstring get_hostname() const { return hostname; }

    /**
      * The get_port method is used to obtain the port name from the
      * URL.
      */
    int get_port() const { return port; }

private:
    /**
      * The protocol instance variable is used to remember the
      * communications protocol to be used.
      */
    nstring protocol;

    /** The userpass instance variable is used to remember the
      * username and password embedded in the url.
      * It must end with the @ sign.
      */
    nstring userpass;

    /**
      * The hostname instance variable is used to remember the name of
      * the host to talk to.
      */
    nstring hostname;

    /**
      * The port instance variable is used to remember the point number
      * to connect to on the remote host.
      */
    int port;

    /**
      * The path instance variable is used to remember the path to the
      * file being queried on the remote host.
      */
    nstring path;

    /**
      * The query instance variable is used to remember the query string
      * (including question mark) to be sent to the remote host.
      */
    nstring query;

    /**
      * The anchor_name instance variable is used to remember the name
      * of the anchor (including the hash sign) within the page fetched.
      */
    nstring anchor_name;

    /**
      * The default constructor.  Do not use.
      */
    url();
};

#endif // LIBAEGIS_URL_H
