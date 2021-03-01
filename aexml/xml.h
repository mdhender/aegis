//
// aegis - project change supervisor
// Copyright (C) 2008 Peter Miller
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

#ifndef AEXML_XML_H
#define AEXML_XML_H

#include <libaegis/output.h>

class change_identifier; // forward

/**
  * The xml abstract base class is used to represent a generic report
  * with emits XML.
  */
class xml
{
public:
    typedef aegis_shared_ptr<xml> pointer;

    /**
      * The destructor.
      */
    virtual ~xml();

protected:
    /**
      * The default constructor.
      * Only derived classes may use this.
      */
    xml();

public:
    /**
      * The factory method is used to create a new XML report instance
      * by name.
      *
      * @param name
      *     The name of the report desired.
      */
    static pointer factory(const nstring &name);

    /**
      * The factory_list method is used to list the available reports.
      *
      * @param op
      *     where to write this list of reports
      */
    static void factory_list(output::pointer op);

    /**
      * The report method is used to run the report and write the XML
      * output.
      *
      * @param cid
      *     the change or project to be reported about
      * @param op
      *     the output file to be written
      */
    virtual void report(change_identifier &cid, output::pointer op) = 0;

private:
    /**
      * The copy constructor.  Do not use.
      */
    xml(const xml &);

    /**
      * The assignment operator.  Do not use.
      */
    xml &operator=(const xml &);
};

#endif // AEXML_XML_H
