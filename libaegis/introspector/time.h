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

#ifndef LIBAEGIS_INTROSPECTOR_TIME_H
#define LIBAEGIS_INTROSPECTOR_TIME_H

#include <libaegis/introspector.h>
#include <libaegis/output.h>

/**
  * The introspector_time class is used to represent the manipulations
  * required to manage time_t values by the meta-data parser.
  */
class introspector_time:
    public introspector
{
public:
    /**
      * The destructor.
      */
    virtual ~introspector_time();

    class adapter
    {
    public:
        typedef aegis_shared_ptr<adapter> pointer;
        virtual ~adapter(){}
    protected:
        adapter(){}
    public:
        virtual void set(time_t) = 0;
        virtual bool is_set() const = 0;
    private:
        adapter(const adapter &rhs);
        adapter &operator=(const adapter &rhs);
    };

    template <class T>
    class adapter_by_method:
        public adapter
    {
    public:
        virtual
        ~adapter_by_method()
        {
        }

    private:
        adapter_by_method(T *a_obj, void (T::*a_set)(time_t),
                bool (T::*a_is_set)() const) :
            obj(a_obj),
            set_method(a_set),
            is_set_method(a_is_set)
        {
        }

    public:
        static pointer
        create(T *ao, void (T::*as)(time_t), bool (T::*ais)() const)
        {
            return pointer(new adapter_by_method(ao, as, ais));
        }

    protected:
        void
        set(time_t x)
        {
            (obj->*set_method)(x);
        }

        bool
        is_set()
            const
        {
            return (obj->*is_set_method)();
        }

    private:
        T *obj;
        void (T::*set_method)(time_t);
        bool (T::*is_set_method)() const;

        adapter_by_method();
        adapter_by_method(const adapter_by_method &);
        adapter_by_method &operator=(const adapter_by_method &);
    };

private:
    /**
      * The constructor.
      * It is private on purpose, use the #create class method instead.
      *
      * @param how
      *     The technique to use to manipulate the actual time_t variable.
      */
    introspector_time(const adapter::pointer &how);

public:
    /**
      * The create class method is used to create new dynamically
      * allocated instances of this class.
      *
      * @param how
      *     The technique to use to manipulate the actual time_t variable.
      */
    static pointer create(const adapter::pointer &how);

    /**
      * The write class method is used to write the named field to the
      * given output.
      *
      * @param op
      *     The output to write the value on
      * @param name
      *     The name of the field being written
      * @param value
      *     The value of the field being written
      * @param show_if_default
      *     Whether or not to display zero values, or hide them.
      */
    static void write(const output::pointer &op, const nstring &name,
        time_t value, bool show_if_default = false);

    /**
      * The write_xml class method is used to write the named field to the
      * given output as an XML element.
      *
      * @param op
      *     The output to write the value on
      * @param name
      *     The name of the field being written
      * @param value
      *     The value of the field being written
      * @param show_if_default
      *     Whether or not to display zero values, or hide them.
      */
    static void write_xml(const output::pointer &op, const nstring &name,
        time_t value, bool show_if_default = false);

protected:
    // See base class for documentation.
    void integer(long n);

    // See base class for documentation.
    void real(double n);

    // See base class for documentation.
    nstring get_name() const;

private:
    /**
      * The where instance variable is ued to remember where to put the
      * value, once parsed.
      */
    adapter::pointer how;

    /**
      * The default constructor.  Do not use.
      */
    introspector_time();

    /**
      * The copy constructor.  Do not use.
      */
    introspector_time(const introspector_time &);

    /**
      * The assignment operator.  Do not use.
      */
    introspector_time &operator=(const introspector_time &);
};

#endif // LIBAEGIS_INTROSPECTOR_TIME_H
