//
// aegis - project change supervisor
// Copyright (C) 2003-2006, 2008, 2012 Peter Miller
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
#include <common/ac/ctype.h>

#include <common/sizeof.h>
#include <common/stracc.h>
#include <common/symtab.h>

#include <aemeasure/end_with.h>
#include <aemeasure/getc_special.h>
#include <aemeasure/lang/c.h>


static FILE     *fp;
static lang_data_t *dp;
static unsigned char level1pushback[4];
static size_t   level1pushback_length;
static unsigned char level2pushback[4];
static size_t   level2pushback_length;


static void
lex_getc1_undo(int c)
{
    if (c != EOF)
    {
        assert(level1pushback_length < SIZEOF(level1pushback));
        level1pushback[level1pushback_length++] = c;
    }
}


static int
lex_getc1(void)
{
    if (level1pushback_length)
        return level1pushback[--level1pushback_length];
    return getc_special(fp, dp);
}


static void
lex_getc2_undo(int c)
{
    if (c != EOF)
    {
        assert(level2pushback_length < SIZEOF(level2pushback));
        level2pushback[level2pushback_length++] = c;
    }
}


static int
lex_getc2(void)
{
    if (level2pushback_length)
        return level2pushback[--level2pushback_length];
    for (;;)
    {
        int             c;

        c = lex_getc1();
        if (c != '\\')
            return c;
        c = lex_getc1();
        if (c != '\n')
        {
            lex_getc1_undo(c);
            return '\\';
        }
    }
}


static int
is_a_keyword(string_ty *s)
{
    static symtab_ty *stp;

    if (!stp)
    {
        static const char *table[] =
        {
            "__attribute__",
            "auto",
            "break",
            "char",
            "__const",
            "const",
            "continue",
            "do",
            "double",
            "else",
            "enum",
            "extern",
            "float",
            "for",
            "goto",
            "if",
            "__inline",
            "inline",
            "int",
            "long",
            "register",
            "return",
            "short",
            "signed",
            "static",
            "struct",
            "switch",
            "typedef",
            "union",
            "unsigned",
            "void",
            "volatile",
            "while",
        };
        const char **tp;

        stp = new symtab_ty(SIZEOF(table));
        for (tp = table; tp < ENDOF(table); ++tp)
        {
            string_ty       *key;

            key = str_from_c(*tp);
            stp->assign(key, key);
        }
    }
    return (0 != stp->query(s));

}


enum token_t
{
    token_operator,
    token_operand,
    token_end_of_file
};


static string_ty *token_value;


static token_t
tokenize(void)
{
    static stracc_t buffer;

    for (;;)
    {
        int             c;
        int             c2;

        if (token_value)
            str_free(token_value);
        token_value = 0;
        buffer.clear();
        c = lex_getc2();
        buffer.push_back(c);
        switch (c)
        {
        case EOF:
            return token_end_of_file;

        case ' ':
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '\v':
            continue;

        case '/':
            c = lex_getc2();
            switch (c)
            {
            case EOF:
                goto is_an_operator;

            case '/':
                // C++ comment
                for (;;)
                {
                    dp->line_has_comment = 1;
                    c = lex_getc2();
                    if (c == EOF || c == '\n')
                        break;
                }
                continue;

            case '*':
                // C comment
                for (;;)
                {
                    for (;;)
                    {
                        dp->line_has_comment = 1;
                        c = lex_getc2();
                        if (c == EOF || c == '*')
                            break;
                    }
                    for (;;)
                    {
                        dp->line_has_comment = 1;
                        c = lex_getc2();
                        if (c != '*')
                            break;
                    }
                    if (c == EOF || c == '/')
                        break;
                }
                continue;

            case '=':
                buffer.push_back(c);
                goto is_an_operator;

            default:
                lex_getc2_undo(c);
                is_an_operator:
                token_value = buffer.mkstr();
                return token_operator;
            }

        case '"':
            for (;;)
            {
                dp->line_has_code = 1;
                c = lex_getc2();
                buffer.push_back(c);
                if (c == EOF || c == '"' || c == '\n')
                    break;
                if (c == '\\')
                {
                    c = lex_getc2();
                    buffer.push_back(c);
                }
            }
            token_value = buffer.mkstr();
            return token_operand;

        case '\'':
            for (;;)
            {
                dp->line_has_code = 1;
                c = lex_getc2();
                buffer.push_back(c);
                if (c == EOF || c == '\'' || c == '\n')
                    break;
                if (c == '\\')
                {
                    c = lex_getc2();
                    buffer.push_back(c);
                }
            }
            token_value = buffer.mkstr();
            return token_operand;

        case '0':
            dp->line_has_code = 1;
            c = lex_getc2();
            if (c == 'x' || c == 'X')
            {
                buffer.push_back(c);
                for (;;)
                {
                    c = lex_getc2();
                    switch (c)
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
                    case 'A':
                    case 'B':
                    case 'C':
                    case 'D':
                    case 'E':
                    case 'F':
                    case 'a':
                    case 'b':
                    case 'c':
                    case 'd':
                    case 'e':
                    case 'f':
                        buffer.push_back(c);
                        continue;

                    default:
                        break;
                    }
                    break;
                }
                lex_getc2_undo(c);
                goto integer_return;
            }
            if (c == '.')
            {
                buffer.push_back(c);
                goto fraction;
            }
            if (c == 'e' || c == 'E')
                goto exponent;
            for (;;)
            {
                switch (c)
                {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    buffer.push_back(c);
                    c = lex_getc2();
                    continue;

                default:
                    break;
                }
                break;
            }
            lex_getc2_undo(c);
            goto integer_return;

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            dp->line_has_code = 1;
            for (;;)
            {
                buffer.push_back(c);
                c = lex_getc2();
                if (c < 0)
                    break;
                if (!isdigit((unsigned char)c))
                    break;
            }
            if (c == '.')
            {
                buffer.push_back(c);
                goto fraction;
            }
            if (c == 'e' || c == 'E')
                goto exponent;
            lex_getc2_undo(c);
            integer_return:
            token_value = buffer.mkstr();
            return token_operand;

        case '.':
            dp->line_has_code = 1;
            c = lex_getc2();
            if (c == EOF || !isdigit((unsigned char)c))
            {
                lex_getc2_undo(c);
                token_value = buffer.mkstr();
                return token_operator;
            }
            buffer.push_back(c);
            fraction:
            for (;;)
            {
                c = lex_getc2();
                if (c == EOF || !isdigit((unsigned char)c))
                    break;
                buffer.push_back(c);
            }
            if (c == 'e' || c == 'E')
            {
                exponent:
                buffer.push_back(c);
                c = lex_getc2();
                if (c == '+' || c == '-')
                {
                    buffer.push_back(c);
                    c = lex_getc2();
                }
                for (;;)
                {
                    c = lex_getc2();
                    if (c == EOF || !isdigit((unsigned char)c))
                        break;
                    buffer.push_back(c);
                }
            }
            lex_getc2_undo(c);
            token_value = buffer.mkstr();
            return token_operand;

        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
        case '_':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
            for (;;)
            {
                c = lex_getc2();
                switch (c)
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
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F':
                case 'G':
                case 'H':
                case 'I':
                case 'J':
                case 'K':
                case 'L':
                case 'M':
                case 'N':
                case 'O':
                case 'P':
                case 'Q':
                case 'R':
                case 'S':
                case 'T':
                case 'U':
                case 'V':
                case 'W':
                case 'X':
                case 'Y':
                case 'Z':
                case '_':
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                case 'g':
                case 'h':
                case 'i':
                case 'j':
                case 'k':
                case 'l':
                case 'm':
                case 'n':
                case 'o':
                case 'p':
                case 'q':
                case 'r':
                case 's':
                case 't':
                case 'u':
                case 'v':
                case 'w':
                case 'x':
                case 'y':
                case 'z':
                    dp->line_has_code = 1;
                    buffer.push_back(c);
                    continue;

                default:
                    lex_getc2_undo(c);
                    break;
                }
                break;
            }
            token_value = buffer.mkstr();
            if (is_a_keyword(token_value))
                return token_operator;
            return token_operand;

        case '!':
        case '%':
        case '*':
        case '=':
        case '^':
            dp->line_has_code = 1;
            c2 = lex_getc2();
            if (c2 == '=')
                buffer.push_back(c2);
            else
                lex_getc2_undo(c2);
            token_value = buffer.mkstr();
            return token_operator;

        case '&':
        case '+':
        case '-':
        case '<':
        case '>':
        case '|':
            dp->line_has_code = 1;
            c2 = lex_getc2();
            if (c2 == '=' || c2 == c)
                buffer.push_back(c2);
            else
                lex_getc2_undo(c2);
            token_value = buffer.mkstr();
            return token_operator;

        default:
            dp->line_has_code = 1;
            token_value = buffer.mkstr();
            return token_operand;
        }
        break;
    }
    // NOTREACHED
    return token_end_of_file;
}


static void
C_count(FILE *afp, lang_data_t *adp)
{
    symtab_ty       *ops;
    symtab_ty       *args;
    static int      place_holder;

    fp = afp;
    dp = adp;
    ops = new symtab_ty(256);
    args = new symtab_ty(256);

    for (;;)
    {
        token_t         tok;

        tok = tokenize();
        switch (tok)
        {
        case token_end_of_file:
            delete ops;
            delete args;
            return;

        case token_operator:
            //
            // n1 = the number of distinct operators
            // N1 = the total number of operators
            //
            if (!ops->query(token_value))
            {
                adp->halstead_n1++;
                ops->assign(token_value, &place_holder);
            }
            adp->halstead_N1++;
            break;

        case token_operand:
            //
            // n2 = the number of distinct operands
            // N2 = the total number of operands
            //
            if (!args->query(token_value))
            {
                adp->halstead_n2++;
                args->assign(token_value, &place_holder);
            }
            adp->halstead_N2++;
            break;
        }
    }
}


static int
C_test(const char *filename)
{
    static const char *list[] =
    {
        ".c", ".h", ".cook", ".def", ".y", ".rpt",
        0
    };
    return ends_with_one_of(filename, list);
}


lang_t lang_c =
{
    "c",
    C_count,
    C_test,
};


// vim: set ts=8 sw=4 et :
