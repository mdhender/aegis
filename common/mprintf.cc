//
// aegis - project change supervisor
// Copyright (C) 1991-1994, 1997, 1999, 2002-2006, 2008, 2012 Peter Miller
// Copyright (C) 2009 Walter Franzini
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

#include <common/ac/assert.h>
#include <common/ac/errno.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/stdint.h>
#include <common/ac/string.h>

#include <common/error.h>
#include <common/mem.h>
#include <common/mprintf.h>
#include <common/str.h>

//
// size to grow memory by
//
#define QUANTUM 200

//
// maximum width for numbers
//
#define MAX_WIDTH (QUANTUM - 1)

//
// the buffer for storing results
//
static size_t   tmplen;
static size_t   length;
static char    *tmp;


//
// NAME
//      bigger - grow dynamic memory buffer
//
// SYNOPSIS
//      int bigger(void);
//
// DESCRIPTION
//      The bigger function is used to grow the dynamic memory buffer
//      used by vmprintf to store the formatting results.
//      The buffer is increased by QUANTUM bytes.
//
// RETURNS
//      int; zero if failed to realloc memory, non-zero if successful.
//
// CAVEATS
//      The existing buffer is still valid after failure.
//

static int
bigger(void)
{
    char            *hold;
    size_t          nbytes;

    nbytes = tmplen + QUANTUM;
    errno = 0;
    hold = (char *)realloc(tmp, nbytes);
    if (!hold)
    {
        if (!errno)
            errno = ENOMEM;
        return 0;
    }
    tmplen = nbytes;
    tmp = hold;
    return 1;
}


/**
 * @brief
 *      construct formatting specifier string
 *
 * The build_fake function is used to construct a format
 * specification string from the arguments presented.  This is
 * used to guarantee exact replication of sprintf behaviour.
 *
 * @param fake
 *     buffer to store results
 * @param fake_len
 *     size of buffer to store results
 * @param flag
 *     the flag specified (zero if not), e.g. '#'
 * @param width
 *     the width specified (zero if not)
 * @param prec
 *     the precision specified (zero if not)
 * @param qual
 *     the qualifier specified (zero if not), e.g. 'l'
 * @param spec
 *     the formatting specifier specified (e.g. 'd')
 */

static void
build_fake(char *fake, size_t fake_len, int flag, int width, int precision,
    int qualifier, int specifier)
{
    char            *fp;

    fp = fake;
    *fp++ = '%';
    if (flag)
        *fp++ = flag;
    if (width > 0)
    {
        snprintf(fp, fake + fake_len - fp - 5, "%d", width);
        fp += strlen(fp);
    }
    *fp++ = '.';
    snprintf(fp, fake + fake_len - fp - 3, "%d", precision);
    fp += strlen(fp);
    switch (qualifier)
    {
    case '\0':
        break;

    case 'H':
        *fp++ = 'h';
        *fp++ = 'h';
        break;

    case 'L':
        *fp++ = 'l';
        *fp++ = 'l';
        break;

    default:
        *fp++ = qualifier;
        break;
    }
    *fp++ = specifier;
    *fp = 0;
}


//
// NAME
//      vmprintf_errok - build a formatted string in dynamic memory
//
// SYNOPSIS
//      char *vmprintf_errok(char *fmt, va_list ap);
//
// DESCRIPTION
//      The vmprintf_errok function is used to build a formatted string
//      in memory.  It understands all of the ANSI standard sprintf
//      formatting directives.  Additionally, "%S" may be used to
//      manipulate (string_ty *) strings.
//
// ARGUMENTS
//      fmt     - string specifying formatting to perform
//      ap      - arguments of types as indicated by the format string
//
// RETURNS
//      char *; pointer to buffer containing formatted string
//              NULL if there is an error (sets errno)
//
// CAVEATS
//      The contents of the buffer pointed to will change between calls
//      to vmprintf_errok.  The buffer itself may move between calls to
//      vmprintf_errok.  DO NOT hand the result of vmprintf_errok to
//      free().
//

char *
vmprintf_errok(const char *fmt, va_list ap)
{
    int             width;
    int             width_set;
    int             prec;
    int             prec_set;
    int             c;
    const char      *s;
    int             qualifier;
    int             flag;
    char            fake[QUANTUM - 1];

    //
    // Build the result string in a temporary buffer.
    // Grow the temporary buffer as necessary.
    //
    // It is important to only make one pass across the variable argument
    // list.  Behaviour is undefined for more than one pass.
    //
    if (!tmplen)
    {
        tmplen = 500;
        errno = 0;
        tmp = (char *)malloc(tmplen);
        if (!tmp)
        {
            if (!errno)
                errno = ENOMEM;
            return 0;
        }
    }

    length = 0;
    s = fmt;
    while (*s)
    {
        c = *s++;
        if (c != '%')
        {
            normal:
            if (length >= tmplen && !bigger())
                return 0;
            tmp[length++] = c;
            continue;
        }
        c = *s++;

        //
        // get optional flag
        //
        switch (c)
        {
        case '+':
        case '-':
        case '#':
        case '0':
        case ' ':
            flag = c;
            c = *s++;
            break;

        default:
            flag = 0;
            break;
        }

        //
        // get optional width
        //
        width = 0;
        width_set = 0;
        switch (c)
        {
        case '*':
            width = va_arg(ap, int);
            if (width < 0)
            {
                flag = '-';
                width = -width;
            }
            c = *s++;
            width_set = 1;
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            for (;;)
            {
                width = width * 10 + c - '0';
                c = *s++;
                switch (c)
                {
                default:
                    break;

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    continue;
                }
                break;
            }
            width_set = 1;
            break;

        default:
            break;
        }

        //
        // get optional precision
        //
        prec = 0;
        prec_set = 0;
        if (c == '.')
        {
            c = *s++;
            switch (c)
            {
            default:
                prec_set = 1;
                break;

            case '*':
                c = *s++;
                prec = va_arg(ap, int);
                if (prec < 0)
                {
                    prec = 0;
                    break;
                }
                prec_set = 1;
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                for (;;)
                {
                    prec = prec * 10 + c - '0';
                    c = *s++;
                    switch (c)
                    {
                    default:
                        break;

                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        continue;
                    }
                    break;
                }
                prec_set = 1;
                break;
            }
        }

        //
        // get the optional qualifier
        //
        switch (c)
        {
        default:
            qualifier = 0;
            break;

        case 'l':
        case 'h':
        case 'L':
        case 'j':
        case 'z':
        case 't':
            qualifier = c;
            c = *s++;
            break;
        }

        //
        // Convert the standard qualifier for the long long type
        // ('ll') into the one used by mprintf ('L', a GNU extension).
        //
        if (qualifier == 'l' && c == 'l')
        {
            qualifier = 'L';
            c = *s++;
        }
        if (qualifier == 'h' && c == 'h')
        {
            qualifier = 'H';
            c = *s++;
        }

        //
        // get conversion specifier
        //
        switch (c)
        {
        default:
            errno = EINVAL;
            return 0;

        case '%':
            goto normal;

        case 'c':
            {
                int             a;
                char            num[MAX_WIDTH + 1];
                size_t          len;

                a = (unsigned char)va_arg(ap, int);
                if (!prec_set)
                    prec = 1;
                if (width > MAX_WIDTH)
                    width = MAX_WIDTH;
                if (prec > MAX_WIDTH)
                    prec = MAX_WIDTH;
                build_fake(fake, sizeof(fake), flag, width, prec, 0, c);
                // g++ -Wformat-nonlitteral will warn here, it is safe to ignore
                snprintf(num, sizeof(num), fake, a);
                len = strlen(num);
                assert(len < QUANTUM);
                if (length + len > tmplen && !bigger())
                    return 0;
                memcpy(tmp + length, num, len);
                length += len;
            }
            break;

        case 'd':
        case 'i':
            {
                intmax_t        a;
                char            num[MAX_WIDTH + 1];
                size_t          len;

                switch (qualifier)
                {
                case 'h':
                    a = (short)va_arg(ap, int);
                    break;

                case 'H':
                    // same a hh, see above
                    a = (signed char)va_arg(ap, int);
                    break;

                case 'j':
                    a = va_arg(ap, intmax_t);
                    break;

                case 'l':
                    a = va_arg(ap, long);
                    break;

                case 'L':
                    // same as ll, see above
                    a = va_arg(ap, long long);
                    break;

                case 't':
                    a = va_arg(ap, ptrdiff_t);
                    break;

                case 'z':
                    a = va_arg(ap, size_t);
                    break;

                default:
                    a = va_arg(ap, int);
                    break;
                }
                if (!prec_set)
                    prec = 1;
                if (width > MAX_WIDTH)
                    width = MAX_WIDTH;
                if (prec > MAX_WIDTH)
                    prec = MAX_WIDTH;
                build_fake(fake, sizeof(fake), flag, width, prec, 'j', c);
                // g++ -Wformat-nonlitteral will warn here, it is safe to ignore
                snprintf(num, sizeof(num), fake, a);
                len = strlen(num);
                assert(len < QUANTUM);
                if (length + len > tmplen && !bigger())
                    return 0;
                memcpy(tmp + length, num, len);
                length += len;
            }
            break;

        case 'e':
        case 'f':
        case 'g':
        case 'E':
        case 'F':
        case 'G':
            {
                double          a;
                char            num[MAX_WIDTH + 1];
                size_t          len;

                //
                // Ignore "long double" for now,
                // traditional implementations no grok.
                //
                a = va_arg(ap, double);
                if (!prec_set)
                    prec = 6;
                if (width > MAX_WIDTH)
                    width = MAX_WIDTH;
                if (prec > MAX_WIDTH)
                    prec = MAX_WIDTH;
                build_fake(fake, sizeof(fake), flag, width, prec, 0, c);
                // g++ -Wformat-nonlitteral will warn here, it is safe to ignore
                snprintf(num, sizeof(num), fake, a);
                len = strlen(num);
                assert(len < QUANTUM);
                if (length + len > tmplen && !bigger())
                    return 0;
                memcpy(tmp + length, num, len);
                length += len;
            }
            break;

        case 'n':
            switch (qualifier)
            {
            case 'h':
                {
                    short *a = va_arg(ap, short *);
                    *a = length;
                }
                break;

            case 'H':
                {
                    char *a = va_arg(ap, char *);
                    *a = length;
                }
                break;

            case 'j':
                {
                    intmax_t *a = va_arg(ap, intmax_t *);
                    *a = length;
                }
                break;

            case 'l':
                {
                    long *a = va_arg(ap, long *);
                    *a = length;
                }
                break;

            case 'L':
                {
                    long long *a = va_arg(ap, long long *);
                    *a = length;
                }
                break;

            case 't':
                {
                    ptrdiff_t *a = va_arg(ap, ptrdiff_t *);
                    *a = length;
                }
                break;

            case 'z':
                {
                    size_t *a = va_arg(ap, size_t *);
                    *a = length;
                }
                break;

            default:
                {
                    int *a = va_arg(ap, int *);
                    *a = length;
                }
                break;
            }
            break;

        case 'u':
        case 'o':
        case 'x':
        case 'X':
            {
                uintmax_t       a;
                char            num[MAX_WIDTH + 1];
                size_t          len;

                switch (qualifier)
                {
                case 'h':
                    a = (unsigned short)va_arg(ap, unsigned int);
                    break;

                case 'H':
                    a = (unsigned char)va_arg(ap, unsigned int);
                    break;

                case 'j':
                    a = va_arg(ap, uintmax_t);
                    break;

                case 'l':
                    a = va_arg(ap, unsigned long);
                    break;

                case 'L':
                    a = va_arg(ap, unsigned long long);
                    break;

                case 't':
                    a = va_arg(ap, ptrdiff_t);
                    break;

                case 'z':
                    a = va_arg(ap, size_t);
                    break;

                default:
                    a = va_arg(ap, unsigned int);
                    break;
                }
                if (!prec_set)
                    prec = 1;
                if (prec > MAX_WIDTH)
                    prec = MAX_WIDTH;
                if (width > MAX_WIDTH)
                    width = MAX_WIDTH;
                build_fake(fake, sizeof(fake), flag, width, prec, 'l', c);
                // g++ -Wformat-nonlitteral will warn here, it is safe to ignore
                snprintf(num, sizeof(num), fake, a);
                len = strlen(num);
                assert(len < QUANTUM);
                if (length + len > tmplen && !bigger())
                    return 0;
                memcpy(tmp + length, num, len);
                length += len;
            }
            break;

        case 'p':
            {
                void *a = va_arg(ap, void *);
                char num[MAX_WIDTH + 1];
                snprintf(num, sizeof(num), "%p", a);
                size_t len = strlen(num);
                assert(len < QUANTUM);
                if (length + len > tmplen && !bigger())
                    return 0;
                memcpy(tmp + length, num, len);
                length += len;
            }
            break;

        case 's':
            {
                char            *a;
                size_t          len;

                a = va_arg(ap, char *);
                if (prec_set)
                {
                    char            *ep;

                    ep = (char *)memchr(a, 0, prec);
                    if (ep)
                        len = ep - a;
                    else
                        len = prec;
                }
                else
                    len = strlen(a);
                if (!prec_set || len < (size_t)prec)
                    prec = len;
                if (!width_set || width < prec)
                    width = prec;
                len = width;
                while (length + len > tmplen)
                {
                    if (!bigger())
                        return 0;
                }
                if (flag != '-')
                {
                    while (width > prec)
                    {
                        tmp[length++] = ' ';
                        width--;
                    }
                }
                memcpy(tmp + length, a, prec);
                length += prec;
                width -= prec;
                if (flag == '-')
                {
                    while (width > 0)
                    {
                        tmp[length++] = ' ';
                        width--;
                    }
                }
            }
            break;

        case 'S':
            {
                string_ty       *a;
                size_t          len;

                a = va_arg(ap, string_ty *);
                len = a->str_length;
                if (!prec_set)
                    prec = len;
                if (len < (size_t)prec)
                    prec = len;
                if (!width_set)
                    width = prec;
                if (width < prec)
                    width = prec;
                len = width;
                while (length + len > tmplen)
                {
                    if (!bigger())
                        return 0;
                }
                if (flag != '-')
                {
                    while (width > prec)
                    {
                        tmp[length++] = ' ';
                        width--;
                    }
                }
                memcpy(tmp + length, a->str_text, prec);
                length += prec;
                width -= prec;
                if (flag == '-')
                {
                    while (width > 0)
                    {
                        tmp[length++] = ' ';
                        width--;
                    }
                }
            }
            break;
        }
    }

    //
    // append a trailing NUL
    //
    if (length >= tmplen && !bigger())
        return 0;
    tmp[length] = 0;

    //
    // return the temporary string
    //
    return tmp;
}


//
// NAME
//      mprintf_errok - build a formatted string in dynamic memory
//
// SYNOPSIS
//      char *mprintf_errok(char *fmt, ...);
//
// DESCRIPTION
//      The mprintf_errok function is used to build a formatted string
//      in memory.  It understands all of the ANSI standard sprintf
//      formatting directives.  Additionally, "%S" may be used to
//      manipulate (string_ty *) strings.
//
// ARGUMENTS
//      fmt     - string spefiifying formatting to perform
//      ...     - arguments of types as indicated by the format string
//
// RETURNS
//      char *; pointer to buffer containing formatted string
//              NULL if there is an error (sets errno)
//
// CAVEATS
//      The contents of the buffer pointed to will change between calls
//      to mprintf_errok.  The buffer itself may move between calls to
//      mprintf_errok.  DO NOT hand the result of mprintf_errok to
//      free().
//

char *
mprintf_errok(const char *fmt, ...)
{
    char            *result;
    va_list         ap;

    va_start(ap, fmt);
    result = vmprintf_errok(fmt, ap);
    va_end(ap);
    return result;
}


//
// NAME
//      vmprintf - build a formatted string in dynamic memory
//
// SYNOPSIS
//      char *vmprintf(char *fmt, va_list ap);
//
// DESCRIPTION
//      The vmprintf function is used to build a formatted string in memory.
//      It understands all of the ANSI standard sprintf formatting directives.
//      Additionally, "%S" may be used to manipulate (string_ty *) strings.
//
// ARGUMENTS
//      fmt     - string spefiifying formatting to perform
//      ap      - arguments of types as indicated by the format string
//
// RETURNS
//      char *; pointer to buffer containing formatted string
//
// CAVEATS
//      On error, prints a fatal error message and exists; does not return.
//
//      The contents of the buffer pointed to will change between calls
//      to vmprintf.  The buffer itself may move between calls to vmprintf.
//      DO NOT hand the result of vmprintf to free().
//

char *
vmprintf(const char *fmt, va_list ap)
{
    char            *result;

    result = vmprintf_errok(fmt, ap);
    if (!result)
        nfatal("mprintf \"%s\"", fmt);
    return result;
}


//
// NAME
//      mprintf - build a formatted string in dynamic memory
//
// SYNOPSIS
//      char *mprintf(char *fmt, ...);
//
// DESCRIPTION
//      The mprintf function is used to build a formatted string in memory.
//      It understands all of the ANSI standard sprintf formatting directives.
//      Additionally, "%S" may be used to manipulate (string_ty *) strings.
//
// ARGUMENTS
//      fmt     - string spefiifying formatting to perform
//      ...     - arguments of types as indicated by the format string
//
// RETURNS
//      char *; pointer to buffer containing formatted string
//
// CAVEATS
//      On error, prints a fatal error message and exists; does not return.
//
//      The contents of the buffer pointed to will change between calls
//      to mprintf.  The buffer itself may move between calls to mprintf.
//      DO NOT hand the result of mprintfe to free().
//

char *
mprintf(const char *fmt, ...)
{
    char            *result;
    va_list         ap;

    va_start(ap, fmt);
    result = vmprintf(fmt, ap);
    va_end(ap);
    return result;
}


//
// NAME
//      vmprintf_str - build a formatted string in dynamic memory
//
// SYNOPSIS
//      char *vmprintf_str(char *fmt, va_list ap);
//
// DESCRIPTION
//      The vmprintf_str function is used to build a formatted string in memory.
//      It understands all of the ANSI standard sprintf formatting directives.
//      Additionally, "%S" may be used to manipulate (string_ty *) strings.
//
// ARGUMENTS
//      fmt     - string spefiifying formatting to perform
//      ap      - arguments of types as indicated by the format string
//
// RETURNS
//      string_ty *; string containing formatted string
//
// CAVEATS
//      On error, prints a fatal error message and exists; does not return.
//
//      It is the resposnsibility of the caller to invoke str_free to release
//      the results when finished with.
//

string_ty *
vmprintf_str(const char *fmt, va_list ap)
{
    if (!vmprintf_errok(fmt, ap))
        nfatal("mprintf \"%s\"", fmt);
    return str_n_from_c(tmp, length);
}


// vim: set ts=8 sw=4 et :
