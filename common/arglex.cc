//
// aegis - project change supervisor
// Copyright (C) 1991-1995, 1997-1999, 2001-2006, 2008, 2012 Peter Miller
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
#include <common/ac/stddef.h>
#include <common/ac/string.h>
#include <common/ac/ctype.h>

#include <common/arglex.h>
#include <common/error.h>
#include <common/mem.h>
#include <common/progname.h>
#include <common/sizeof.h>
#include <common/str.h>
#include <common/trace.h>

static arglex_table_ty table[] =
{
    { "-",        arglex_token_stdio,   },
    { "-Help",    arglex_token_help,    },
    { "-VERSion", arglex_token_version, },
    { "-TRAce",   arglex_token_trace,   },
    { "-Page_Length", arglex_token_page_length, },
    { "-Page_Width", arglex_token_page_width, },
};

typedef const char **argv_t;

static int      argc;
static argv_t   argv;
arglex_value_ty arglex_value;
int             arglex_token;
static arglex_table_ty *utable;
static const char *partial;
static int      strings_only_mode;
static int      incomplete = -1;
static int      is_synthetic;


//
// NAME
//      arglex_init
//
// SYNOPSIS
//      void arglex_init(int ac, char **av, arglex_table-t *tp);
//
// DESCRIPTION
//      The arglex_init function is used to initialize the
//      command line processing.
//
// ARGUMENTS
//      ac      - aergument count, from main
//      av      - argument values, from main
//      tp      - pointer to table of options
//
// CAVEAT
//      Must be called before the arglex() function.
//

void
arglex_init(int ac, char **av, arglex_table_ty *tp)
{
    progname_set(av[0]);
    argc = ac - 1;
    argv = (argv_t)(av + 1); // gcc incorrectly whines about incompatible
        // pointer types in assignment
    utable = tp;
}


//
// NAME
//      is_a_number
//
// SYNOPSIS
//      int is_a_number(char *s);
//
// DESCRIPTION
//      The is_a_number function is used to determine if the
//      argument is a number.
//
//      The value is placed in arglex_value.alv_number as
//      a side effect.
//
//      Negative and positive signs are accepted.
//      The C conventions for decimal, octal and hexadecimal are understood.
//
//      There may be no white space anywhere in the string,
//      and the string must end after the last digit.
//      Trailing garbage will be interpreted to mean it is not a string.
//
// ARGUMENTS
//      s       - string to be tested and evaluated
//
// RETURNS
//      int;    zero if not a number,
//              non-zero if is a number.
//

static int
is_a_number(const char *s)
{
    long            n;
    int             sign;

    n = 0;
    switch (*s)
    {
    case '-':
        ++s;
        sign = -1;
        break;

    case '+':
        ++s;
        sign = 1;
        break;

    default:
        sign = 1;
        break;
    }
    switch (*s)
    {
    case '0':
        if ((s[1] == 'x' || s[1] == 'X') && s[2])
        {
            s += 2;
            for (;;)
            {
                switch (*s)
                {
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
                    n = n * 16 + *s++ - '0';
                    continue;

                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F':
                    n = n * 16 + *s++ - 'A' + 10;
                    continue;

                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                    n = n * 16 + *s++ - 'a' + 10;
                    continue;
                }
                break;
            }
        }
        else
        {
            for (;;)
            {
                switch (*s)
                {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    n = n * 8 + *s++ - '0';
                    continue;
                }
                break;
            }
        }
        break;

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
            switch (*s)
            {
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
                n = n * 10 + *s++ - '0';
                continue;
            }
            break;
        }
        break;

    default:
        return 0;
    }
    if (*s)
        return 0;
    arglex_value.alv_number = n * sign;
    return 1;
}


//
// NAME
//      arglex
//
// SYNOPSIS
//      int arglex(void);
//
// DESCRIPTION
//      The arglex function is used to perfom lexical analysis
//      on the command line arguments.
//
//      Unrecognised options are returned as arglex_token_option
//      for anything starting with a '-', or
//      arglex_token_string otherwise.
//
// RETURNS
//      The next token in the token stream.
//      When the end is reached, arglex_token_eoln is returned forever.
//
// CAVEAT
//      Must call arglex_init befor this function is called.
//

int
arglex(void)
{
    arglex_table_ty *tp;
    int             j;
    arglex_table_ty *hit[20];
    int             nhit;
    static const char *pushback;
    const char      *arg;
    string_ty       *s1;
    string_ty       *s2;
    int             is_inco;

    trace(("arglex()\n{\n"));
    if (pushback)
    {
        //
        // the second half of a "-foo=bar" style argument.
        //
        arg = pushback;
        is_inco = (incomplete == 0);
        pushback = 0;
    }
    else
    {
        get_another:
        if (argc <= 0)
        {
            arglex_token = arglex_token_eoln;
            arg = "";
            goto done;
        }
        arg = argv[0];
        is_inco = (incomplete == 0);
        argc--;
        argv++;
        incomplete--;

        //
        // The "--" option means the rest of the arguments on
        // the command line are only strings.
        //
        if (strings_only_mode)
        {
            arglex_token =
                (
                    is_inco
                ?
                    arglex_token_string_incomplete
                :
                    arglex_token_string
                );
            goto done;
        }
        if (arg[0] == '-' && arg[1] == '-' && !arg[2])
        {
            strings_only_mode = 1;
            goto get_another;
        }

        //
        // See if it looks like a GNU "-foo=bar" option.
        // Split it at the '=' to make it something the
        // rest of the code understands.
        //
        if (arg[0] == '-' && arg[1] != '=')
        {
            const char *eqp = strchr(arg, '=');
            if (eqp)
            {
                pushback = eqp + 1;
                if (is_inco)
                {
                    incomplete = 0;
                    is_inco = 0;
                }

                //
                // Crop the argument, so that we can figure out that the
                // left hand side means.  It is often a memory leak.
                //
                arg = mem_copy_string(arg, eqp - arg);
            }
        }

        //
        // Turn the GNU-style leading "--"
        // into "-" if necessary.
        //
        if (arg[0] == '-' && arg[1] == '-' && arg[2] && !is_a_number(arg + 1))
            ++arg;
    }

    //
    // see if it is a number
    //
    if (is_a_number(arg))
    {
        arglex_token =
            (is_inco ? arglex_token_number_incomplete : arglex_token_number);
        goto done;
    }

    //
    // scan the tables to see what it matches
    //
    nhit = 0;
    partial = 0;
    if (!is_inco)
    {
        for (tp = table; tp < ENDOF(table); tp++)
        {
            if (arglex_compare(tp->t_name, arg, &partial))
                hit[nhit++] = tp;
        }
        if (utable)
        {
            for (tp = utable; tp->t_name; tp++)
            {
                if (arglex_compare(tp->t_name, arg, &partial))
                    hit[nhit++] = tp;
            }
        }
    }

    //
    // deal with unknown or ambiguous options
    //
    switch (nhit)
    {
    case 0:
        //
        // not found in the tables
        //
        if (*arg == '-')
        {
            arglex_token =
                (
                    is_inco
                ?
                    arglex_token_option_incomplete
                :
                    arglex_token_option
                );
        }
        else
        {
            arglex_token =
                (
                    is_inco
                ?
                    arglex_token_string_incomplete
                :
                    arglex_token_string
                );
        }
        break;

    case 1:
        one:
        arglex_token = hit[0]->t_token;
        if (partial)
            arg = partial;      // const-ness hack
        else
            arg = hit[0]->t_name;
        break;

    default:
        //
        // not an error if they are all the same
        // e.g. due to cultural spelling differences
        // with the same abbreviation.
        //
        for (j = 1; j < nhit; ++j)
            if (hit[0]->t_token != hit[j]->t_token)
                break;
        if (j >= nhit)
            goto one;

        if (is_synthetic)
        {
            arglex_token = arglex_token_option;
            goto done;
        }

        //
        // build a list of the names
        // and complain that it is ambiguous
        //
        s1 = str_from_c(hit[0]->t_name);
        for (j = 1; j < nhit; ++j)
        {
            s2 = str_format("%s, %s", s1->str_text, hit[j]->t_name);
            str_free(s1);
            s1 = s2;
        }
        fatal_raw("option \"%s\" ambiguous (%s)", arg, s1->str_text);
    }

    //
    // here for all exits
    //
    done:
    arglex_value.alv_string = arg;
    trace(("return %d; /* %s */\n", arglex_token, arglex_value.alv_string));
    trace(("}\n"));
    return arglex_token;
}


//
// NAME
//      arglex_prefetch
//
// SYNOPSIS
//      int arglex_prefetch(int *list, int list_len);
//
// DESCRIPTION
//      The arglex_prefetch function is used to perfom lexical analysis
//      on the command line arguments, much like the arglex function.
//      However, it is given a list of tokens to look for on the command
//      line, and such arguments are matched and extracted, which may
//      be used to relax command line argument ordering restrictions.
//
// RETURNS
//      One of the tokens in the list, or
//      ARGLEX_PREFETCH_FAIL if none of the are available.
//
// CAVEAT
//      Must call arglex_init before this function is called.
//

int
arglex_prefetch(int *list, int list_len)
{
    int             j;

    trace(("arglex_prefetch()\n{\n"));
    if (strings_only_mode)
        goto fail;

    for (j = 0; j < argc; ++j)
    {
        const char      *actual;
        int             k;

        if (j == incomplete)
            continue;

        //
        // The "--" option means the rest of the arguments on
        // the command line are only strings.
        //
        actual = argv[j];
        if (actual[0] == '-' && actual[1] == '-' && !actual[2])
            goto fail;

        //
        // see if it is a number
        //
        if (is_a_number(actual))
            continue;

        //
        // Turn the GNU-style leading "--"
        // into "-" if necessary.
        //
        if
        (
            actual[0] == '-'
        &&
            actual[1] == '-'
        &&
            actual[2]
        &&
            !is_a_number(actual + 1)
        )
            ++actual;

        for (k = 0; k < list_len; ++k)
        {
            int             token;
            const char      *formal;

            token = list[k];
            formal = arglex_token_name(token);
            if (arglex_compare(formal, actual, &partial))
            {
                int             m;

                //
                // Shuffle everything down to fill in
                // the hole.
                //
                for (m = j + 1; m < argc; ++m)
                    argv[m - 1] = argv[m];
                --argc;
                if (j < incomplete)
                    --incomplete;

                //
                // Fill in the answer as it would be
                // returned form arglex()
                //
                arglex_value.alv_string = actual;
                arglex_token = token;
                trace(("return %d; /* %s */\n", arglex_token,
                        arglex_value.alv_string));
                trace(("}\n"));
                return arglex_token;
            }
        }
    }

  fail:
    trace(("return FAIL;\n"));
    trace(("}\n"));
    return ARGLEX_PREFETCH_FAIL;
}


const char *
arglex_token_name(int n)
{
    arglex_table_ty *tp;

    switch (n)
    {
    case arglex_token_eoln:
        return "end of command line";

    case arglex_token_number:
        return "number";

    case arglex_token_option:
        return "option";

    case arglex_token_stdio:
        return "standard input or output";

    case arglex_token_string:
        return "string";

    default:
        break;
    }
    for (tp = table; tp < ENDOF(table); tp++)
    {
        if (tp->t_token == n)
            return tp->t_name;
    }
    if (utable)
    {
        for (tp = utable; tp->t_name; tp++)
        {
            if (tp->t_token == n)
                return tp->t_name;
        }
    }

    assert(!"unknown command line token");
    return "unknown command line token";
}


arglex_table_ty *
arglex_table_catenate(arglex_table_ty *tp1, arglex_table_ty *tp2)
{
    size_t          len1;
    size_t          len2;
    size_t          len;
    arglex_table_ty *tp;
    static arglex_table_ty zero = ARGLEX_END_MARKER;

    for (len1 = 0; tp1[len1].t_name; ++len1)
        ;
    for (len2 = 0; tp2[len2].t_name; ++len2)
        ;
    len = len1 + len2;
    tp = (arglex_table_ty *)mem_alloc((len + 1) * sizeof(arglex_table_ty));
    memcpy(tp, tp1, len1 * sizeof(arglex_table_ty));
    memcpy(tp + len1, tp2, len2 * sizeof(arglex_table_ty));
    tp[len] = zero;
    return tp;
}


void
arglex_dispatch(const arglex_dispatch_ty *choices, unsigned choices_size,
    void (*otherwise)(void))
{
    trace(("arglex_dispatch()\n{\n"));
    int *tmp = (int *)mem_alloc(choices_size * sizeof(tmp[0]));
    for (int priority = 0;; ++priority)
    {
        unsigned tmp_len = 0;
        for (unsigned j = 0; j < choices_size; ++j)
        {
            const arglex_dispatch_ty *cp = choices + j;
            if (cp->priority == priority)
                tmp[tmp_len++] = cp->token;
        }
        if (tmp_len == 0)
            break;
        int tok = arglex_prefetch(tmp, tmp_len);
        for (unsigned j = 0; j < choices_size; ++j)
        {
            const arglex_dispatch_ty *cp = choices + j;
            if (tok == cp->token)
            {
                mem_free(tmp);
                if (cp->func)
                    cp->func();
                trace(("}\n"));
                return;
            }
        }
    }
    mem_free(tmp);
    if (otherwise)
        otherwise();
    trace(("}\n"));
}


void
arglex_synthetic(int ac, char **av, int inco)
{
    argc = ac - 1;
    argv = (argv_t)(av + 1); // gcc incorrectly whines about incompatible
        // pointer types in assignment
    is_synthetic = 1;
    incomplete = inco - 1;
}


void
arglex_retable(arglex_table_ty *tp)
{
    utable = tp;
}


int
arglex_get_string(void)
{
    int             result;
    int             hold;

    hold = strings_only_mode;
    strings_only_mode = 1;
    result = arglex();
    strings_only_mode = hold;
    return result;
}


// vim: set ts=8 sw=4 et :
