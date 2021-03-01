//
//      aegis - project change supervisor
//      Copyright (C) 2003, 2005, 2006, 2008, 2012 Peter Miller
//      Copyright (C) 2008, 2009 Walter Franzini
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

#ifndef LIBAEGIS_INPUT_CURL_H
#define LIBAEGIS_INPUT_CURL_H

#include <common/ac/curl/curl.h>

#include <libaegis/input.h>

/**
  * The input_curl class is used to represent an input obtained via
  * libcurl.
  */
class input_curl:
    public input_ty
{
public:
    /**
      * The destructor.
      */
    virtual ~input_curl();

    /**
      * The constructor.
      */
    input_curl(const nstring &url);

    /**
      * The looks_likely class method is used to examine a filename and
      * see if it starts with a prototype name (e.g. http: or ftp:)
      * making it a likely candidate for the input_curl class.
      */
    static bool looks_likely(const nstring &fn);

    // This is only public so that the CURL progress callback can access it.
    void progress_callback(double down_total, double down_current);

    // This is only public so that the CURL write callback can access it.
    size_t write_callback(char *data, size_t nbytes);

    // This is only public so that the CURL write callback can access it.
    void read_error();

    // See base class for documentation.
    nstring name();

    // See base class for documentation.
    off_t length();

    // See base class for documentation.
    ssize_t read_inner(void *data, size_t nbytes);

    // See base class for documentation.
    off_t ftell_inner();

    // See base class for documentation.
    bool is_remote() const;

    bool verify_handle(CURL *x) const { return (x == handle); }

    void eof_notify() { eof = true; }

private:
    CURL *handle;
    nstring fn;
    off_t pos;

    // buffer to store cached data
    char *curl_buffer;

    // currently allocated buffers length
    size_t curl_buffer_maximum;

    // start of data in buffer
    size_t curl_buffer_position;

    // end of data in buffer
    size_t curl_buffer_length;

    // end of file has been reached
    bool eof;

    char errbuf[CURL_ERROR_SIZE];

    time_t progress_start;
    char *progress_buffer;
    int progress_buflen;
    int progress_cleanup;
#if (LIBCURL_VERSION_NUM < 0x070b01)
    nstring proxy;
    nstring userpass;
#endif

    long read_data(void *data, size_t len);

    /**
      * The default constructor.  Do not use.
      */
    input_curl();

    /**
      * The copy constructor.  Do not use.
      */
    input_curl(const input_curl &arg);

    /**
      * The assignment operator.  Do not use.
      */
    input_curl &operator=(const input_curl &arg);
};

/**
  * The input_curl_open function is used to open an input stream which
  * reads from a Uniform Resource Locator (URL), typically an HTTP or
  * FTP site somewhere on the Internet.
  */
inline input_ty::pointer
input_curl_open(struct string_ty *fn)
{
    return new input_curl(nstring(fn));
}

/**
  * The input_curl_looks_likely function is used to examine a filename
  * and see if it starts with a prototype name (e.g. http: or ftp:)
  * making it a likely bcandidate for the input_curl_open function.
  */
inline bool
input_curl_looks_likely(struct string_ty *fn)
{
    return input_curl::looks_likely(nstring(fn));
}

#endif // LIBAEGIS_INPUT_CURL_H
// vim: set ts=8 sw=4 et :
