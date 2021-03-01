//
//      aegis - project change supervisor
//      Copyright (C) 2004-2006, 2008, 2011, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#ifndef AECVSSERVER_MODULE_PROJECT_H
#define AECVSSERVER_MODULE_PROJECT_H

#include <aecvsserver/module.h>

class project; // forward

/**
  * The module_project class is used to represent a CVS module
  * synthesized from an Aegis project baseline.
  */
class module_project:
    public module_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~module_project();

    /**
      * The constructor.
      */
    module_project(project *arg);

    // See base class for documentation.
    void modified(server_ty *sp, string_ty *file_name, file_info_ty *fip,
        input &contents);

    // See base class for documentation.
    bool checkin(server_ty *sp, string_ty *client_side,
        string_ty *server_side);

    // See base class for documentation.
    bool update(server_ty *sp, string_ty *client_side,
        string_ty *server_side, const options &opt);

    // See base class for documentation.
    bool add(server_ty *sp, string_ty *client_side,
        string_ty *server_side, const options &opt);

    // See base class for documentation.
    bool remove(server_ty *sp, string_ty *client_side,
        string_ty *server_side, const options &opt);

    // See base class for documentation.
    string_ty *calculate_canonical_name() const;

private:
    project *pp;

    /**
      * The groan method is used to output an error when an
      * inappropriate request is made of this module.
      */
    void groan(server_ty *sp, const char *request_name);

    /**
      * The default constructor.  Do not use.
      */
    module_project();

    /**
      * The copy constructor.  Do not use.
      */
    module_project(const module_project &);

    /**
      * The assignment operator.  Do not use.
      */
    module_project &operator=(const module_project &);
};

/**
  * The module_project_new fucntion is used to dynamically allocate a
  * module based on the projet name.
  *
  * You should rarely use the "new module_project(pp)" expression,
  * because this function takes care of allocating module_project_bogus
  * for projects which don't exist.
  */
module_ty *module_project_new(string_ty *projname);

#endif // AECVSSERVER_MODULE_PROJECT_H
// vim: set ts=8 sw=4 et :
