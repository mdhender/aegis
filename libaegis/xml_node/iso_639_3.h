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

#ifndef LIBAEGIS_XML_NODE_ISO_639_3_H
#define LIBAEGIS_XML_NODE_ISO_639_3_H

#include <common/config.h>
#include <map>

#include <common/nstring.h>
#include <libaegis/xml_node.h>

/**
  * The xml_node_iso_639_3 class is used to represent
  * processing required when reading the iso-639-3 xml data.
  */
class xml_node_iso_639_3:
    public xml_node
{
public:
    /**
      * The destructor.
      */
    virtual ~xml_node_iso_639_3();

    /**
      * The default constructor.
      */
    xml_node_iso_639_3();

    bool need_setup(void);

    bool is_a_valid_code(const nstring &text) const;

protected:
    // See base class for documentation.
    void element_begin(const nstring &name);

    // See base class for documentation.
    void attribute(const nstring &name, const nstring &value);

    // See base class for documentation.
    void element_end(const nstring &name);

private:
    typedef std::map<nstring, nstring> codes_t;

    /**
      * The codes instance variable is used to remember the set of
      * language codes parsed so far.
      */
    codes_t codes;

    /**
      * The id instance variable is used to remember the value of the
      * "id" attribute for the current record.
      */
    nstring id;

    /**
      * The part1_code instance variable is used to remember the value
      * of the "part1_code" (2-letter code) attribute for the current
      * record.
      */
    nstring part1_code;

    /**
      * The part2_code instance variable is used to remember the value
      * of the "part2_code" (3-letter code) attribute for the current
      * record.
      */
    nstring part2_code;

    /**
      * The assign method is used to insert data into the #codes
      * instance variable.
      *
      * @param key
      *     The left hand side of the assignment.
      * @param value
      *     The right hand side of the assignment.
      */
    void assign(const nstring &key, const nstring &value);

    /**
      * The copy constructor.  Do not use.
      */
    xml_node_iso_639_3(const xml_node_iso_639_3 &);

    /**
      * The assignment operator.  Do not use.
      */
    xml_node_iso_639_3 &operator=(const xml_node_iso_639_3 &);
};

// vim: set ts=8 sw=4 et :
#endif // LIBAEGIS_XML_NODE_ISO_639_3_H
