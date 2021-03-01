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

#ifndef AEXML_XML_PROJECT_FILESBYDELTA_H
#define AEXML_XML_PROJECT_FILESBYDELTA_H

#include <aexml/xml.h>

/**
  * The xml_project_files_by_delta class is used to represent a report
  * which details the set of project files immediately after an
  * historical commit, identified by delta.
  */
class xml_project_files_by_delta:
    public xml
{
public:
    /**
      * The destructor.
      */
    virtual ~xml_project_files_by_delta();

private:
    /**
      * The default constructor.
      * It is private on purpose, use the #creat class method instead.
      */
    xml_project_files_by_delta();

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
    xml_project_files_by_delta(const xml_project_files_by_delta &);

    /**
      * The assignment operator.  Do not use.
      */
    xml_project_files_by_delta &operator=(const xml_project_files_by_delta &);
};

#endif // AEXML_XML_PROJECT_FILESBYDELTA_H
