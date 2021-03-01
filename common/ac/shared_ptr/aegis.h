//
//      aegis - project change supervisor
//      Copyright (C) 2007, 2008, 2012 Peter Miller
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

#ifndef COMMON_AC_SHARED_PTR_AEGIS_H
#define COMMON_AC_SHARED_PTR_AEGIS_H

#include <common/ac/assert.h>

/**
  * The aegis_shared_ptr class is used to represent a smart pointer with
  * reference counted copy semantics.  The object pointed to is deleted
  * when the last aegis_shared_ptr pointing to it is destroyed or reset.
  *
  * This class is only used of std::tr1::shared_ptr and
  * boost::shared_ptr are both unavailable.  It only implements as much
  * of a shared_ptr as the Aegis code actually uses.
  */

template<class T>
class aegis_shared_ptr
{
private:
    /**
      * The reference_count instance variable is used to remember the
      * location of the reference count.  By having the reference count
      * separate from the subject, we can cope with compatible pointers,
      * not just exact pointers.
      *
      * This is not ideal because it allocates huge numbers of small
      * objects.  Some heap implementations go slowly when faced with
      * many small allocations.  Some heap implementations waste a lot
      * of memory when faced with many small allocations.
      */
    long *reference_count;

    /**
      * The subject instance variable is used to remember the location
      * of the object being reference counted.  By having it separate
      * from the reference count, we can skip one indirection.
      */
    T *subject;

public:
    /**
      * The valid method is used to establish the correctness of the
      * internal state.  Think of it as the contract.
      */
    bool
    valid()
        const
    {
        return
            (
                (reference_count != 0)
            ?
                (*reference_count > 0 && subject != 0)
            :
                (subject == 0)
            );
    }

    /**
      * The destructor.
      *
      * @note
      *     This destructor is not virtual.  Thou shalt not derive from
      *     this class.
      */
    ~aegis_shared_ptr()
    {
        assert(valid());
        if (reference_count)
        {
            --*reference_count;
            if (*reference_count <= 0)
            {
                delete subject;
                subject = 0;
                delete reference_count;
                reference_count = 0;
            }
        }
    }

    /**
      * Swap the contents of two aegis_shared_ptr<T> objects.
      * This method swaps the internal pointers to T.  This can be done
      * safely without involving a reference / unreference cycle and is
      * therefore highly efficient.
      */
    void
    swap(aegis_shared_ptr &rhs)
    {
        assert(valid());
        assert(rhs.valid());
        long *temp_reference_count = reference_count;
        T *temp_subject = subject;
        reference_count = rhs.reference_count;
        subject = rhs.subject;
        rhs.reference_count = temp_reference_count;
        rhs.subject = temp_subject;
    }

    /**
      * The reset method is used to drop the reference, if there is one.
      */
    void
    reset()
    {
        // swap with a NULL pointer
        aegis_shared_ptr tmp;
        this->swap(tmp);
    }

    /**
      * The default constructor.
      */
    aegis_shared_ptr() :
        reference_count(0),
        subject(0)
    {
        assert(valid());
    }

    /**
      * The constructor.  It permits initialization from any compatable
      * pointer.  Y must be a complete type.
      *
      * @param rhs
      *     The object pointed to, the object to be managed.
      */
    template<class Y> explicit
    aegis_shared_ptr(Y *rhs) :
        reference_count(0),
        subject(0)
    {
        assert(valid());
        if (rhs)
        {
            reference_count = new long(1);
            subject = rhs;
        }
        assert(valid());
    }

    /**
      * The copy constructor.
      *
      * @param rhs
      *     The smart pointer to be copied.
      */
    aegis_shared_ptr(const aegis_shared_ptr &rhs) :
        reference_count(0),
        subject(0)
    {
        assert(rhs.valid());
        if (rhs.subject)
        {
            assert(rhs.reference_count);
            reference_count = rhs.reference_count;
            subject = rhs.subject;
            ++*reference_count;
        }
        assert(valid());
    }

    template <class Y> friend class aegis_shared_ptr;

    /**
      * The compatible copy constructor.
      *
      * @param rhs
      *     The compatable smart pointer to be copied.
      */
    template<class Y>
    aegis_shared_ptr(const aegis_shared_ptr<Y> &rhs) :
        reference_count(0),
        subject(0)
    {
        assert(rhs.valid());
        if (rhs.subject)
        {
            assert(rhs.reference_count);
            reference_count = rhs.reference_count;
            subject = rhs.subject;
            ++*reference_count;
        }
        assert(valid());
    }

    /**
      * The assignment operator.
      *
      * @param rhs
      *     The right hand side of the assigment, the pointer to copy.
      */
    aegis_shared_ptr &
    operator=(const aegis_shared_ptr &rhs)
    {
        assert(valid());
        assert(rhs.valid());
        if (this != &rhs)
        {
            // In case you haven't seen the swap() technique to
            // implement copy assignment before, here's what it does:
            //
            // 1) Create a temporary aegis_shared_ptr<> instance via the
            //    copy constructor, thereby increasing the reference
            //    count of the source object.
            //
            // 2) Swap the internal object pointers of *this and the
            //    temporary aegis_shared_ptr<>.  After this step, *this
            //    already contains the new pointer, and the old pointer
            //    is now managed by temp.
            //
            // 3) The destructor of temp is executed, thereby
            //    unreferencing the old object pointer.
            //
            // This technique is described in Herb Sutter's "Exceptional
            // C++", and has a number of advantages over conventional
            // approaches:
            //
            // - Code reuse by calling the copy constructor.
            // - Strong exception safety for free.
            // - Self assignment is handled implicitly.
            // - Simplicity.
            // - It just works and is hard to get wrong; i.e. you can
            //   use it without even thinking about it to implement
            //   copy assignment where ever the object data is managed
            //   indirectly via a pointer, which is very common.
            //
            aegis_shared_ptr temp(rhs);
            this->swap(temp);
            assert(valid());
            assert(rhs.valid());
        }
        return *this;
    }

    /**
      * The assignment operator.
      *
      * @param rhs
      *     The right hand side of the assigment, the pointer to copy.
      */
    template<class Y>
    aegis_shared_ptr &
    operator=(const aegis_shared_ptr<Y> &rhs)
    {
        assert(valid());
        assert(rhs.valid());
        if (this != (aegis_shared_ptr *)&rhs)
        {
            aegis_shared_ptr tmp(rhs);
            this->swap(tmp);
            assert(valid());
            assert(rhs.valid());
        }
        return *this;
    }

#if 0
    /**
      * The constructor
      *
      * @param rhs
      *     The pointer we are to manage.
      */
    template<class Y> explicit
    aegis_shared_ptr(std::auto_ptr<Y> &rhs) :
        reference_count(0),
        subject(0)
    {
        Y *tmp = r.get();
        if (tmp)
        {
            reference_count = new long(1);
            subject = tmp;
        }
        assert(valid());
    }

    /**
      * the assignment operator.
      *
      * @param rhs
      *     The pointer we are to manage.
      */
    template<class Y>
    aegis_shared_ptr &
    operator=(std::auto_ptr<Y> &rhs)
    {
        assert(valid());
        reset();
        if (tmp)
        {
            reference_count = new long(1);
            subject = tmp;
        }
        assert(valid());
        return *this;
    }

#endif

    /**
      * The dereference operator.  Return a reference to the object, not
      * to the smart pointer.
      *
      * This is part of why it's called a "smart pointer" when it's
      * actually neither - it *acts* like a pointer.
      */
    T &
    operator*()
        const
    {
        assert(subject != 0);
        return *subject;
    }

    /**
      * The "pointing at" operator.  Return a pointer to the object, not
      * to the smart pointer.
      *
      * This is part of why it's called a "smart pointer" when it's
      * actually neither - it *acts* like a pointer.
      */
    T *
    operator->()
        const
    {
        assert(subject != 0);
        return subject;
    }

    /**
      * The get method is used to obtain the pointer to the object.
      */
    T *
    get()
        const
    {
        return subject;
    }

    /**
      * The bool operator, for implicit conversion to bool.
      * Test of the pointers not NULL.
      */
    operator bool()
        const
    {
        return (subject != 0);
    }

    /**
      * The logical not operator.
      * Test if the pointer is NULL.
      */
    bool
    operator!()
        const
    {
        return (subject == 0);
    }

    inline bool
    operator==(const aegis_shared_ptr &rhs)
        const
    {
        return (subject == rhs.get());
    }

    template<class U>
    inline bool
    operator==(const aegis_shared_ptr<U> &rhs)
        const
    {
        return (subject == rhs.get());
    }

    inline bool
    operator!=(aegis_shared_ptr &rhs)
        const
    {
        return (subject != rhs.get());
    }

    template<class U>
    inline bool
    operator!=(aegis_shared_ptr<U> &rhs)
        const
    {
        return (subject != rhs.get());
    }

    inline bool
    operator<(aegis_shared_ptr &rhs)
        const
    {
        return (subject < rhs.get());
    }

    template<class U>
    inline bool
    operator<(aegis_shared_ptr<U> &rhs)
        const
    {
        return (subject < rhs.get());
    }
};

#endif // COMMON_AC_SHARED_PTR_AEGIS_H
// vim: set ts=8 sw=4 et :
