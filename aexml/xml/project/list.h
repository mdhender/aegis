//
// aegis - project change supervisor
// Copyright (C) 2003, 2005, 2006, 2008, 2012 Peter Miller
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

#ifndef AEXML_XML_PROJECT_LIST_H
#define AEXML_XML_PROJECT_LIST_H

#include <aexml/xml.h>

/**
  * The xml_project_list class is used to represent a report which
  * emits XML describes the available projects.
  */
class xml_project_list:
    public xml
{
public:
    /**
      * The destructor.
      */
    ~xml_project_list();

private:
    /**
      * The default constructor.
      * It is private on purpose, use the #create class method instead.
      */
    xml_project_list();

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create();

protected:
    // See base class for documentation.
    void report(change_identifier &cid, output::pointer op);

private:
    /**
      * The copy constructor.  Do not use.
      */
    xml_project_list(const xml_project_list &);

    /**
      * The assignment operator.  Do not use.
      */
    xml_project_list &operator=(const xml_project_list &);
};

#endif // AEXML_XML_PROJECT_LIST_H
// vim: set ts=8 sw=4 et :
