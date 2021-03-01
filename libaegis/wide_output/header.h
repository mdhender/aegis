//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2001, 2002, 2005, 2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_WIDE_OUTPUT_HEADER_H
#define LIBAEGIS_WIDE_OUTPUT_HEADER_H

#include <common/wstring.h>
#include <libaegis/wide_output.h>

/**
  * The wide_output_header class is used to add page headers to the output.
  */
class wide_output_header:
    public wide_output
{
public:
    typedef aegis_shared_ptr<wide_output_header> hpointer;

    /**
      * The destructor.
      */
    ~wide_output_header();

private:
    /**
      * The constructor.
      *
      * @param deeper
      *     where to write the filtered output
      */
    wide_output_header(const wide_output::pointer &deeper);

public:
    /**
      * The open class method is used to create new dynamically
      * allocated instances of this class.
      */
    static hpointer open(const wide_output::pointer &deeper);

    void title(const char *, const char *);

    static void
    title(wide_output::pointer fp, const char *l1, const char *l2)
    {
        wide_output_header *hp = dynamic_cast<wide_output_header *>(fp.get());
        if (hp)
            hp->title(l1, l2);
    }

    void need(int);

    void need1(int);

    static void
    need1(wide_output::pointer fp, int x)
    {
        wide_output_header *hp = dynamic_cast<wide_output_header *>(fp.get());
        if (hp)
            hp->need1(x);
    }

    void eject();

    static void
    eject(wide_output::pointer fp)
    {
        wide_output_header *hp = dynamic_cast<wide_output_header *>(fp.get());
        if (hp)
            hp->eject();
    }

    bool is_at_top_of_page();

    static bool
    is_at_top_of_page(wide_output::pointer fp)
    {
        wide_output_header *hp = dynamic_cast<wide_output_header *>(fp.get());
        return (hp && hp->is_at_top_of_page());
    }

protected:
    // See base class for documentation.
    nstring filename();

    // See base class for documentation.
    int page_width();

    // See base class for documentation.
    int page_length();

    // See base class for documentation.
    const char *type_name() const;

    // See base class for documentation.
    void write_inner(const wchar_t *data, size_t len);

    // See base class for documentation.
    void flush_inner();

    // See base class for documentation.
    void end_of_line_inner();

private:
    pointer deeper;
    wstring title1;
    wstring title2;
    int line_number;
    int length;
    int width;
    bool is_a_printer;
    int column;
    int page_number;
    time_t page_time;
    bool already_top_diverted;

    void left_and_right(const wstring &lhs, const char *rhs);
    void top_of_page_processing();
    void bottom_of_page_processing();

    /**
      * The default constructor.  Do not use.
      */
    wide_output_header();

    /**
      * The copy constructor.  Do not use.
      */
    wide_output_header(const wide_output_header &);

    /**
      * The assignment operator.  Do not use.
      */
    wide_output_header &operator=(const wide_output_header &);
};

#endif // LIBAEGIS_WIDE_OUTPUT_HEADER_H
