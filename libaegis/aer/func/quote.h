//
//	aegis - project change supervisor
//	Copyright (C) 1998, 1999, 2002, 2005-2007 Peter Miller
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
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

#ifndef LIBAEGIS_AER_FUNC_QUOTE_H
#define LIBAEGIS_AER_FUNC_QUOTE_H

#include <libaegis/aer/func.h>


/**
  * The rpt_func_quote_url class is used to represent the quote_url
  * function, callable from within the report generator.
  */
class rpt_func_quote_url:
    public rpt_func
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_func_quote_url();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      */
    rpt_func_quote_url();

public:
    /**
      * The create class method is used to create a new dynamically
      * allocated instance of this class.
      */
    static rpt_func::pointer create();

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    bool optimizable() const;

    // See base class for documentation.
    bool verify(const rpt_expr::pointer &ep) const;

    // See base class for documentation.
    rpt_value::pointer run(const rpt_expr::pointer &ep, size_t argc,
        rpt_value::pointer *argv) const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    rpt_func_quote_url(const rpt_func_quote_url &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_func_quote_url &operator=(const rpt_func_quote_url &);
};


/**
  * The rpt_func_unquote_url class is used to represent the unquote_url
  * function, callable from within the report generator.
  */
class rpt_func_unquote_url:
    public rpt_func
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_func_unquote_url();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      */
    rpt_func_unquote_url();

public:
    /**
      * The create class method is used to create a new dynamically
      * allocated instance of this class.
      */
    static rpt_func::pointer create();

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    bool optimizable() const;

    // See base class for documentation.
    bool verify(const rpt_expr::pointer &ep) const;

    // See base class for documentation.
    rpt_value::pointer run(const rpt_expr::pointer &ep, size_t argc,
        rpt_value::pointer *argv) const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    rpt_func_unquote_url(const rpt_func_unquote_url &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_func_unquote_url &operator=(const rpt_func_unquote_url &);
};


/**
  * The rpt_func_quote_html class is used to represent the quote_html
  * function, callable from within the report generator.
  */
class rpt_func_quote_html:
    public rpt_func
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_func_quote_html();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      */
    rpt_func_quote_html();

public:
    /**
      * The create class method is used to create a new dynamically
      * allocated instance of this class.
      */
    static rpt_func::pointer create();

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    bool optimizable() const;

    // See base class for documentation.
    bool verify(const rpt_expr::pointer &ep) const;

    // See base class for documentation.
    rpt_value::pointer run(const rpt_expr::pointer &ep, size_t argc,
        rpt_value::pointer *argv) const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    rpt_func_quote_html(const rpt_func_quote_html &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_func_quote_html &operator=(const rpt_func_quote_html &);
};


/**
  * The rpt_func_quote_tcl class is used to represent the quote_tcl
  * function, callable from within the report generator.
  */
class rpt_func_quote_tcl:
    public rpt_func
{
public:
    /**
      * The destructor.
      */
    virtual ~rpt_func_quote_tcl();

private:
    /**
      * The constructor.  It is private on purpose, use the "create"
      * class method instead.
      */
    rpt_func_quote_tcl();

public:
    /**
      * The create class method is used to create a new dynamically
      * allocated instance of this class.
      */
    static rpt_func::pointer create();

protected:
    // See base class for documentation.
    const char *name() const;

    // See base class for documentation.
    bool optimizable() const;

    // See base class for documentation.
    bool verify(const rpt_expr::pointer &ep) const;

    // See base class for documentation.
    rpt_value::pointer run(const rpt_expr::pointer &ep, size_t argc,
        rpt_value::pointer *argv) const;

private:
    /**
      * The copy constructor.  Do not use.
      */
    rpt_func_quote_tcl(const rpt_func_quote_tcl &);

    /**
      * The assignment operator.  Do not use.
      */
    rpt_func_quote_tcl &operator=(const rpt_func_quote_tcl &);
};

#endif // LIBAEGIS_AER_FUNC_QUOTE_H
