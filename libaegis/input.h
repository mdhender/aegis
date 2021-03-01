//
//	aegis - project change supervisor
//	Copyright (C) 1999, 2002, 2004-2006, 2008 Peter Miller
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

#ifndef LIBAEGIS_INPUT_H
#define LIBAEGIS_INPUT_H

#include <common/nstring.h>
#include <common/main.h>

class input; // forward

/**
  * The input_ty abstract class represent a generic input source.
  */
class input_ty
{
public:
    /**
      * The destructor is used to close the given input, and delete all
      * resources associated with it.  Once this returns, the given
      * input is no longer available for *any* use.  (Think of this
      * method as "close" if it helps.)
      */
    virtual ~input_ty();

    /**
      * The default constructor.
      */
    input_ty();

    /**
      * The read method is used to read buffered data from the given
      * input stream.  At most \a size bytes will be read into
      * \a buffer.  The number of bytes actually read will be returned.
      * At end-of-file, a value <=0 will be returned, and \a buffer will
      * be unchanged.  All file read errors or format errors are fatal,
      * and will cause the method to not return.
      *
      * @param data
      *     Where to put the results of the read.
      * @param nbytes
      *     The maximum number of bytes to read.
      * @returns
      *     The actual number of bytes read, or zero for end-of-file.
      */
    long read(void *data, size_t nbytes);

    /**
      * The read_strictest method is used to read data from the given
      * input stream.  Exactly \a size bytes will be read into
      * \a buffer.  If there are less than \a size bytes available, a
      * fatal error will result.
      *
      * @param data
      *     Where to put the results of the read.
      * @param size
      *     The number of bytes to read.
      */
    void read_strictest(void *data, size_t size);

    /**
      * The read_strict method is used to read data from the given input
      * stream.  Exactly \a size bytes will be read into \a buffer, or
      * zero bytes at end-of-file.  If there are less than \a size bytes
      * available, a fatal error will result.
      *
      * @param data
      *     Where to put the results of the read.
      * @param size
      *     The number of bytes to read.
      * @returns
      *     bool; true if data was read, false if end-of-file
      */
    bool read_strict(void *data, size_t size);

    /**
      * The skip method is used to read data from the given input
      * stream and discard it.  Exactly \a size bytes will be read.  If
      * there are less than \a size bytes available, a fatal error will
      * result.
      *
      * @param size
      *     The number of bytes to discard.
      */
    void skip(size_t size);

    /**
      * The fatal_error method is used to report a fatal error on
      * an input stream.  This method does not return.
      */
    void fatal_error(const char *msg);

    /**
      * The one_line method is used to read one line from the input
      * (up to the next newline character or end of input).
      *
      * @param result
      *    Where the text is stored.  The newline is not included in the
      *    returned string.
      * @returns
      *    bool; true if any text was read, false if end-of-file is reached.
      */
    bool one_line(nstring &result);

    /**
      * The ftell method is used to determine the current buffered
      * data position within the input.
      */
    long ftell();

    /**
      * The name method is used to determine the name of the input.
      */
    virtual nstring name() = 0;

    /**
      * The length method is used to determine the length of the input.
      * May return -1 if the length is unknown.
      */
    virtual long length() = 0;

    /**
      * The getch method is used to get the next character from
      * the input.  Returns a value<=0 at end-of-file.
      */
    int
    getch()
    {
	if (buffer_position < buffer_end)
	    return *buffer_position++;
	return getc_complicated();
    }

    /**
      * The ungetc method is used to push back a character of input.
      * Think of it as undoing the effects of the getch method.
      */
    void
    ungetc(int c)
    {
	if (c >= 0)
	{
	    if (buffer_position > buffer)
		*--buffer_position = c;
	    else
		ungetc_complicated(c);
	}
    }

    /**
      * The peek method is used to obtain the value of the next input
      * character, without advancing the read position.
      *
      * @returns
      *     the next character, or -1 if the end of file has been reached.
      */
    int
    peek()
    {
	int c = getch();
	ungetc(c);
	return c;
    }

    /**
      * The keepalive method is used to set the SO_KEEPALIVE socket
      * option, if the file is a socket.  Does nothing otherwise.
      */
    virtual void keepalive();

    /**
      * The pushback_transfer method is used by input filter classes'
      * destructors to return unused buffeered input.
      */
    void pushback_transfer(input &from);

    /**
      * The pullback_transfer method is used by input filter classes'
      * destructors to return unused buffeered input.
      */
    void pullback_transfer(input_ty *to);

    /**
      * The pullback_transfer method is used by input filter classes'
      * destructors to return unused buffeered input.
      */
    void pullback_transfer(input &to);

    /**
      * The unread method may be used to reverse the effects of the read
      * method.  The data is pushed into the buffer.  Think of it as a
      * whole bunhs of ungetc calls (backwards).
      *
      * @param data
      *     The base of the array of bytes to be returned.
      * @param nbytes
      *     The number of bytes to be returned.
      */
    void unread(const void *data, size_t nbytes);

    /**
      * The at_end method is used to determine whether or not
      * this input stream is at the end of input.
      */
    bool at_end();

    /**
      * The is_remote method is used to determine whether or not an
      * input stream is from a local file or a remote source.  This is
      * only intended to be a generally informative thing, to provide
      * information to the user, it isn't (and can't be) utterly
      * precise.
      */
    virtual bool is_remote() const;

    void reference_count_up();
    void reference_count_down();
    bool reference_count_valid() const { return (reference_count >= 1); }

protected:
    /**
      * The read_inner method is used to read unbuffered data from the
      * given input stream.  At most \a nbytes bytes will be read into
      * \a data.  The number of bytes actually read will be returned.
      * At end-of-file, a value <= 0 will be returned, and \a data will
      * be unchanged.  All file read errors or format errors are fatal,
      * and will cause the method to not return.
      *
      * @param data
      *     Where to put the results of the read.
      * @param nbytes
      *     The maximum number of bytes to read.
      * @returns
      *     The actual number of bytes read, or zero for end-of-file.
      */
    virtual long read_inner(void *data, size_t nbytes) = 0;

    /**
      * The ftell_inner method is used to determine the unbuffered
      * current position within the input.
      */
    virtual long ftell_inner() = 0;

    /**
      * The getc_complicated method is used to get a character from the
      * input.  Usually users do not call this method directly, but
      * use the getch method instead.
      */
    int getc_complicated();

    /**
      * The ungetc_complicated method is used to push a character back
      * onto an input.  Usually users do not call this method directly,
      * but use the input_ungetc macro instead.
      */
    void ungetc_complicated(int c);

private:
    long reference_count;

    /**
      * The buffer instance variable is used to remember the base of
      * a dynamically allocated array used to buffer the data for
      * buffered input.
      */
    unsigned char *buffer;

    /**
      * The buffer_size instance variable is used to remember the
      * allocation size of the dynamically allocated buffer.
      */
    size_t buffer_size;

    /**
      * The buffer_position instance variable is used to remember the
      * read position of the data within the buffer.
      */
    unsigned char *buffer_position;

    /**
      * The buffer_end instance variable is used to remember the
      * highwater mark of data stored in the buffer.
      */
    unsigned char *buffer_end;
};


inline DEPRECATED string_ty *
input_name(input_ty *ip)
{
    return ip->name().get_ref();
}


inline DEPRECATED void
input_delete(input_ty *ip)
{
    ip->reference_count_down();
}


inline DEPRECATED int
input_getc(input_ty *ip)
{
    return ip->getch();
}


inline DEPRECATED void
input_ungetc(input_ty *ip, int c)
{
    ip->ungetc(c);
}


inline DEPRECATED long
input_length(input_ty *ip)
{
    return ip->length();
}


inline DEPRECATED long
input_read(input_ty *ip, void *data, size_t nbytes)
{
    return ip->read(data, nbytes);
}


inline DEPRECATED void
input_fatal_error(input_ty *ip, const char *msg)
{
    ip->fatal_error(msg);
}


/**
  * The input class is a so-called smart pointer, which automatically
  * keeps track of input usage, and deletes the instance once the last
  * consumer has let it go.
  */
class input
{
    friend class input_ty;

public:
    /**
      * The destructor.
      *
      * @note
      *     This destructor is not virtual.
      *     DO NOT derive from this class.
      */
    ~input();

    /**
      * The default constructor.
      *
      * This is dangerous.  The reference will be NULL, so if you try to
      * dereference it the code will segfault.
      */
    input();

    /**
      * The constructor.
      *
      * @param arg
      *     The input stream to be managed.
      *     Its reference count will NOT be incrimented, it is assumed
      *     you are giving the "dumb" pointer to this "smart pointer" to
      *     manage.
      */
    input(input_ty *arg);

    /**
      * The copy constructor.
      */
    input(const input &arg);

    /**
      * The assignment operator.
      */
    input &operator=(const input &arg);

#if 0
    /**
      * The assignment operator.
      *
      * @param arg
      *     The input stream to be managed.  Its reference count will
      *     NOT be incrimented, it is assumed you are giving the "dumb"
      *     pointer to this "smart pointer" to manage.
      */
    input &operator=(input *arg);
#endif

    /**
      * The member operator.
      *
      * This is why it's called a "smart pointer" when it is actually
      * neither.  Because we return a pointer to the referenced input_ty
      * object, this class presents what appears to be the same
      * interface as input_ty to the interface user.
      */
    input_ty *operator->() { return ref; }

    /**
      * The member operator.
      *
      * This is why it's called a "smart pointer" when it is actually
      * neither.  Because we return a pointer to the referenced input_ty
      * object, this class presents what appears to be the same
      * interface as input_ty to the interface user.
      */
    const input_ty *operator->() const { return ref; }

    /**
      * The close method may be used to delete (actually, decriment
      * the reference count and conditionally delete) the managed
      * input_ty object.  Acess to this smart pointer will hereafter get
      * segfaults until another input is assigned to it.
      */
    void close();

    /**
      * The is_open method may be used to determine if this "smart
      * pointer" is actuallyt pointing at something.  This is an input
      * source so the open/closed metaphore works for us.
      */
    bool is_open() const { return (ref != 0); }

    /**
      * The valid method is used (when debugging) to determine if this
      * object is in a valid state.
      */
    bool valid() const;

private:
    /**
      * The ref instance variable is used to remember the location of
      * the dynamically allocated input object.
      */
    input_ty *ref;
};


#endif // LIBAEGIS_INPUT_H
