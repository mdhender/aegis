//
// aegis - project change supervisor
// Copyright (C) 2011 Peter Miller
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

#ifndef LIBAEGIS_OUTPUT_FILTER_SET_WIDTH_H
#define LIBAEGIS_OUTPUT_FILTER_SET_WIDTH_H

#include <libaegis/output/filter.h>

/**
  * The output_filter_set_width class is used to represent the
  * processing required to nominate a specific file output width, in
  * columns.
  */
class output_filter_set_width:
    public output_filter
{
public:
    typedef aegis_shared_ptr<output_filter_set_width> pointer;

    /**
      * The destructor.
      */
    virtual ~output_filter_set_width();

    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      */
    static pointer create(const output::pointer &deeper, int width);

protected:
    // See base class for documentation.
    int page_width(void) const;

private:
    /**
      * The default constructor.
      * It is private on purpose, use the #create class method instead.
      */
    output_filter_set_width(const output::pointer &deeper, int width);

    /**
      * The default constructor.  Do not use.
      */
    output_filter_set_width();

    int width;

    /**
      * The copy constructor.  Do not use.
      */
    output_filter_set_width(const output_filter_set_width &);

    /**
      * The assignment operator.  Do not use.
      */
    output_filter_set_width &operator=(const output_filter_set_width &);
};

// vim: set ts=8 sw=4 et :
#endif // LIBAEGIS_OUTPUT_FILTER_SET_WIDTH_H
