//
// aegis - project change supervisor
// Copyright (C) 2003-2008, 2012 Peter Miller
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
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
#include <common/ac/ctype.h>
#include <common/ac/stdio.h>
#include <common/ac/stdlib.h>
#include <common/ac/string.h>

#include <common/arglex.h>
#include <common/progname.h>
#include <common/str.h>

#include <aecomplete/shell/private.h>
#include <aecomplete/shell/zsh.h>


struct shell_zsh_ty
{
    shell_ty        inherited;
    string_ty       *command;
    string_ty       *prefix;
};


static char *
copy_of(const char *s, size_t len)
{
    char *result = new char [len + 1];
    memcpy(result, s, len);
    result[len] = 0;
    return result;
}


static void
destructor(shell_ty *sp)
{
    shell_zsh_ty    *this_thing;

    this_thing = (shell_zsh_ty *)sp;
    if (this_thing->command)
        str_free(this_thing->command);
    if (this_thing->prefix)
        str_free(this_thing->prefix);
}


static void
usage(void)
{
    const char      *prog;

    prog = progname_get();
    fprintf(stderr, "Usage: %s <prog-name> <word> <context>\n", prog);
    exit(1);
}


static int
test(shell_ty *sp)
{
    shell_zsh_ty    *this_thing;
    char            *cp;
    char            *end;
    unsigned long   n;
    char            *comp_line;
    unsigned long   comp_point;
    size_t          ac;
    size_t          ac_max;
    char            **av;
    int             inco_ac;

    this_thing = (shell_zsh_ty *)sp;

    //
    // The PREFIX environment variable must be set and valid.
    //
    cp = getenv("PREFIX");
    if (!cp)
        return 0;
    this_thing->prefix = str_from_c(cp);

    //
    // The BUFFER environment variable must be set and valid.
    //
    comp_line = getenv("BUFFER");
    if (!comp_line || !*comp_line)
        return 0;

    //
    // The CURSOR environment variable must be set and valid.
    //
    cp = getenv("CURSOR");
    if (!cp)
        return 0;
    n = strtoul(cp, &end, 10);
    if (end == cp || *end)
        return 0;
    comp_point = n;
    if (comp_point > strlen(comp_line))
        return 0;

    //
    // There should be at least one command line argument,
    // the name of the command being completed
    //
    if (arglex_get_string() != arglex_token_string)
        usage();
    this_thing->command = str_from_c(arglex_value.alv_string);

    //
    // Ignore the rest of the command line arguments.
    //
    while (arglex_get_string() == arglex_token_string)
        ;
    if (arglex_token != arglex_token_eoln)
        usage();

    //
    // Generate the new command line, by splitting the comp_line string
    // into words.
    //
    ac = 0;
    ac_max = 0;
    av = 0;
    inco_ac = -1;
    for (cp = comp_line; ; )
    {
        //
        // If the completion point is in the middle of white space,
        // or at the start of another word, insert an empty argument to
        // serve as the imcomplete argument in need of attention.
        //
        if ((unsigned long)(cp - comp_line) == comp_point)
        {
            //
            // insert the empty string as the incomplete argument.
            //
            if (ac >= ac_max)
            {
                ac_max = ac_max * 2 + 8;
                char **new_av = new char * [ac_max];
                for (size_t k = 0; k < ac; ++k)
                    new_av[k] = av[k];
                delete [] av;
                av = new_av;
            }
            inco_ac = ac;
            av[ac++] = copy_of(cp, 0);
            comp_point = ~0uL;
        }

        //
        // end of the line
        //
        if (!*cp)
        {
            break;
        }

        //
        // Skip white space around words.
        //
        if (isspace((unsigned char)*cp))
        {
            ++cp;
            continue;
        }

        //
        // Collect one word.
        //
        // Note that the completion point also terminates the word,
        // even if there is no white space present.
        //
        end = cp;
        while
        (
            *end
        &&
            !isspace((unsigned char)*end)
        &&
            end != comp_line + comp_point
        )
        {
            ++end;
        }

        //
        // Insert word into the list.
        //
        if (ac >= ac_max)
        {
            ac_max = ac_max * 2 + 8;
            char **new_av = new char * [ac_max];
            for (size_t k = 0; k < ac; ++k)
                new_av[k] = av[k];
            delete [] av;
            av = new_av;
        }
        if ((unsigned long)(cp - comp_line) < comp_point &&
            comp_point <= (unsigned long)(end - comp_line))
        {
            inco_ac = ac;
            comp_point = ~0uL;
        }
        av[ac++] = copy_of(cp, end - cp);

        //
        // Move past the word.
        //
        cp = end;
    }
    assert(inco_ac >= 0);

    //
    // NULL terminate the list of word pointers.
    //
    if (ac >= ac_max)
    {
        ac_max = ac_max * 2 + 8;
        char **new_av = new char * [ac_max];
        for (size_t k = 0; k < ac; ++k)
            new_av[k] = av[k];
        delete [] av;
        av = new_av;
    }
    av[ac] = 0;

    //
    // Insert our fake command line into the command line processor.
    //
    arglex_synthetic(ac, av, inco_ac);

    //
    // Report success.
    //
    return 1;
}


static string_ty *
command_get(shell_ty *sh)
{
    shell_zsh_ty    *this_thing;

    this_thing = (shell_zsh_ty *)sh;
    return this_thing->command;
}


static string_ty *
prefix_get(shell_ty *sh)
{
    shell_zsh_ty    *this_thing;

    this_thing = (shell_zsh_ty *)sh;
    return this_thing->prefix;
}


static void
emit(shell_ty *, string_ty *s)
{
    char            *cp;

    for (cp = s->str_text; *cp; ++cp)
    {
        switch (*cp)
        {
        case '\\':
        case '\n':
            putchar('\\');
            // fall through...

        default:
            putchar(*cp);
        }
    }
    putchar('\n');
}


static shell_vtbl_ty vtbl =
{
    destructor,
    test,
    command_get,
    prefix_get,
    emit,
    sizeof(shell_zsh_ty),
    "zsh",
};


shell_ty *
shell_zsh(void)
{
    shell_ty        *sp;
    shell_zsh_ty    *this_thing;

    sp = shell_new(&vtbl);
    this_thing = (shell_zsh_ty *)sp;
    this_thing->command = 0;
    this_thing->prefix = 0;
    return sp;
}


// vim: set ts=8 sw=4 et :
