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

#ifndef LIBAEGIS_RFC822_H
#define LIBAEGIS_RFC822_H

#include <common/symtab/template.h>
#include <libaegis/output.h>

class input; // forward

/**
  * The rfc822 class is used to represent a database of name and values
  * in RFC822 format.  It ispossible to populate the database by reading
  * the RFC822 header from a file, and the values can be stored by
  * writing an RFC822 header to a file.
  */
class rfc822
{
public:
    /**
      * The destructor.
      */
    virtual ~rfc822();

    /**
      * The default constructor.
      */
    rfc822();

    /**
      * The copy constructor.
      */
    rfc822(const rfc822 &arg);

    /**
      * The assignment operator.
      */
    rfc822 &operator=(const rfc822 &arg);

    /**
      * The set method is used to insert a record into the database.
      *
      * @param name
      *     The name of the record to set.
      * @param value
      *     The value of the record being set.
      */
    void set(const nstring &name, const nstring &value);

    /**
      * The set_minimalist method is used to insert a record into the
      * database, but only is the value supplied is not empty.  If the
      * value is empty, and existing record will be removed.
      *
      * @param name
      *     The name of the record to set.
      * @param value
      *     The value of the record being set.
      */
    void set_minimalist(const nstring &name, const nstring &value);

    /**
      * The set method is a convenience function which reformats the
      * long value as a string, and then sets that value.
      *
      * @param name
      *     The name of the record to set.
      * @param value
      *     The value of the record being set.
      */
    void set(const nstring &name, long value);

    /**
      * The set method is a convenience function which reformats the
      * unsigned long value as a string, and then sets that value.
      *
      * @param name
      *     The name of the record to set.
      * @param value
      *     The value of the record being set.
      */
    void set(const nstring &name, unsigned long value);

    /**
      * The set method is a convenience function which reformats the
      * bool value as a string, and then sets that value.
      *
      * @param name
      *     The name of the record to set.
      * @param value
      *     The value of the record being set.
      */
    void set(const nstring &name, bool value);

    /**
      * The is_set method is used to determine if a given header record
      * has been set.  (Note that even if the value is the empty string,
      * it is still considered to have been set.)
      *
      * @param name
      *     The name of the header record to test for.
      */
    bool is_set(const nstring &name) const;

    /**
      * The get method is used to get the named header value.
      *
      * @param name
      *     The name of the record to get.
      * @returns
      *     A reference to the record's value.  You are not allowed to
      *     change it.  If the named record does not exist, the empty
      *     string will be returned.
      */
    const nstring &get(const nstring &name);

    /**
      * The get method is used to get the named header value.
      *
      * @param name
      *     The name of the record to get.
      * @returns
      *     The record's value.  If the named record does not exist, the
      *     empty string will be returned.
      */
    nstring get(const nstring &name) const;

    /**
      * The get_long method is used to get the named header value, as an
      * integer.
      *
      * @param name
      *     The name of the record to get.
      * @returns
      *     The record's value.
      * @note
      *     Values which are not numbers will be silently converted to
      *     zero.
      */
    long get_long(const nstring &name);

    /**
      * The get_ulong method is used to get the named header value, as an
      * unsigned long integer.
      *
      * @param name
      *     The name of the record to get.
      * @returns
      *     The record's value.
      * @note
      *     Values which are not numbers will be silently converted to
      *     zero.
      */
    unsigned long get_ulong(const nstring &name);

    /**
      * The load method is used to load the database contents by reading
      * the given input for and RFC 822 formatted header.  It stops
      * after reading a blank line.
      *
      * @param src
      *     The input to read the data from.
      */
    void load(input &src, bool maybe_not = false);

    /**
      * The load_from_file method is used to load the database contents
      * by reading from a file.  Blank lines and # comments are ignored.
      *
      * @param filename
      *     The input to read the data from.
      */
    void load_from_file(const nstring &filename);

    /**
      * The store method is used to dump the database out as an RFC 822
      * compliant header, followed by a single blank line.
      *
      * @param dst
      *     The output stream on which to write the data.
      */
    void store(output::pointer dst);

    /**
      * The store_to_file method is used to dump the database to a file.
      * It calls the strore method to do all the work.
      *
      * @param filename
      *     The filename into which to write the data.
      */
    void store_to_file(const nstring &filename);

    /**
      * The empty method is used to determine whether or not the
      * database currently had no data.
      */
    bool empty() const { return database.empty(); }

    /**
      * The date class method is used to obtain the current date and
      * time, conforming to RFC 822 format.
      */
    static nstring date();

    /**
      * The keys method is used to obtain the keys to the database.
      */
    void keys(nstring_list &result) { database.keys(result); }

    /**
      * The clear method is used to discard all settings from the database.
      */
    void clear();

private:
    /**
      * The database instance variable is used to remeber the name-value
      * pairs for each header item.
      */
    symtab<nstring> database;

    /**
      * The sanitize_name class method is used to normalize the name
      * into lower case, with non-alpha-numberics turned into minus
      * symbols.  This is because RFC 822 header names are case-insensitive.
      *
      * @param name
      *     The name to be sanitized.
      */
    static nstring sanitize_name(const nstring &name);
};

#endif // LIBAEGIS_RFC822_H
