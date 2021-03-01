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

#ifndef LIBAEGIS_XML_NODE_ISO_3166_H
#define LIBAEGIS_XML_NODE_ISO_3166_H

#include <common/config.h>
#include <map>

#include <common/nstring.h>
#include <libaegis/xml_node.h>

/**
  * The xml_node_iso_3166 class is used to represent the data in the
  * iso-3166 data in /usr/share/xml/iso-codes/iso_3166.cml file from the
  * "iso-codes" Debian package.
  */
class xml_node_iso_3166:
    public xml_node
{
public:
    /**
      * The destructor.
      */
    virtual ~xml_node_iso_3166();

    /**
      * The default constructor.
      */
    xml_node_iso_3166();

    /**
      * The need_setup method is used to determine whether or not this
      * data needs to be read in from the XML file.  It will only ever
      * return "true" once.
      */
    bool need_setup(void);

    /**
      * The is_a_valid_code method is used to determien whether the
      * given text is a value country code.
      *
      * @param code_name
      *     The country code candidate
      * @returns
      *     true if the text is a valid country code, false if not.
      */
    bool is_a_valid_code(const nstring &code_name) const;

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
      * The codes instance variable is used to remember the cumulative
      * code mappings read from the XML file.
      */
    codes_t codes;

    /**
      * The alpha_2_code instance variable is used to remember the
      * 2-letter country code, a temporary used when parsing a
      * &lt;iso_3166_entry/&gt; element.
      */
    nstring alpha_2_code;

    /**
      * The alpha_3_code instance variable is used to remember
      * 3-letter country code, a temporary used when parsing a
      * &lt;iso_3166_entry/&gt; element.
      */
    nstring alpha_3_code;

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
    xml_node_iso_3166(const xml_node_iso_3166 &);

    /**
      * The assignment operator.  Do not use.
      */
    xml_node_iso_3166 &operator=(const xml_node_iso_3166 &);
};

// vim: set ts=8 sw=4 et :
#endif // LIBAEGIS_XML_NODE_ISO_3166_H
