//
// aegis - project change supervisor
// Copyright (C) 2001, 2002, 2004-2008, 2012 Peter Miller
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

#include <common/quit.h>
#include <common/sizeof.h>
#include <common/stracc.h>
#include <common/str_list.h>
#include <common/symtab.h>
#include <common/trace.h>
#include <libaegis/input/file.h>
#include <libaegis/sub.h>

#include <aeimport/format/rcs/gram.yacc.h> // must be after <common/str_list.h>
#include <aeimport/format/rcs/lex.h>

static input ip;
static int      keyword_expected;
static symtab_ty *stp;
static int      error_count;

struct table_ty
{
    const char      *name;
    int             value;
};

static table_ty table[] =
{
    { "access", ACCESS, },
    { "author", AUTHOR, },
    { "branch", BRANCH, },
    { "branches", BRANCH, },
    { "comment", COMMENT, },
    { "date", DATE, },
    { "desc", DESC, },
    { "expand", EXPAND, },
    { "head", HEAD, },
    { "locks", LOCKS, },
    { "log", LOG, },
    { "next", NEXT, },
    { "state", STATE, },
    { "strict", STRICT, },
    { "suffix", SUFFIX, },
    { "symbols", SYMBOLS, },
    { "text", TEXT, },
};


void
rcs_lex_open(string_ty *fn)
{
    if (!stp)
    {
        table_ty        *tp;

        stp = new symtab_ty(SIZEOF(table));
        for (tp = table; tp < ENDOF(table); ++tp)
        {
            string_ty       *name;

            name = str_from_c(tp->name);
            stp->assign(name, tp);
            str_free(name);
        }
    }

    //
    // Open it as a binary file.  The lexer will treat random CR
    // characters as white space.  This means that we can cope with
    // binary files when they have been checked into RCS.
    //
    // Note that of there are errors, the error messages won't have
    // line numbers, because binary files don't do line numbers.
    //
    ip = input_file_open(fn);
    keyword_expected = 1;
}


void
rcs_lex_close(void)
{
    if (error_count)
        quit(1);
    ip.close();
}


static void
rcs_lex_error(sub_context_ty *scp, const char *s)
{
    string_ty       *msg;

    msg = subst_intl(scp, s);

    // re-use substitution context
    sub_var_set_string(scp, "MeSsaGe", msg);
    sub_var_set_string(scp, "File_Name", ip->name());
    error_intl(scp, i18n("$filename: $message"));
    str_free(msg);

    if (++error_count >= 20)
    {
        // re-use substitution context
        sub_var_set_string(scp, "File_Name", ip->name());
        fatal_intl(scp, i18n("$filename: too many errors"));
    }
}


void
format_rcs_gram_error(const char *s)
{
    sub_context_ty  *scp;

    scp = sub_context_new();
    rcs_lex_error(scp, s);
    sub_context_delete(scp);
}


//
// map of character types
// ISO 8859/1 (Latin-1)
//
enum ctab_ty
{
    ctab_is_junk,
    ctab_is_space,
    ctab_is_string,
    ctab_is_digit,
    ctab_is_colon,
    ctab_is_semi,
    ctab_is_alpha
};

static ctab_ty ctab[] =
{
    ctab_is_junk,   // 000
    ctab_is_junk,   // 001
    ctab_is_junk,   // 002
    ctab_is_junk,   // 003
    ctab_is_junk,   // 004
    ctab_is_junk,   // 005
    ctab_is_junk,   // 006
    ctab_is_junk,   // 007
    ctab_is_space,  // 010
    ctab_is_space,  // 011
    ctab_is_space,  // 012
    ctab_is_space,  // 013
    ctab_is_space,  // 014
    ctab_is_space,  // 015
    ctab_is_junk,   // 016
    ctab_is_junk,   // 017
    ctab_is_junk,   // 020
    ctab_is_junk,   // 021
    ctab_is_junk,   // 022
    ctab_is_junk,   // 023
    ctab_is_junk,   // 024
    ctab_is_junk,   // 025
    ctab_is_junk,   // 026
    ctab_is_junk,   // 027
    ctab_is_junk,   // 030
    ctab_is_junk,   // 031
    ctab_is_junk,   // 032
    ctab_is_junk,   // 033
    ctab_is_junk,   // 034
    ctab_is_junk,   // 035
    ctab_is_junk,   // 036
    ctab_is_junk,   // 037
    ctab_is_space,  // ' '
    ctab_is_alpha,  // '!'
    ctab_is_alpha,  // '"'
    ctab_is_alpha,  // '#'
    ctab_is_alpha,  // '$'
    ctab_is_alpha,  // '%'
    ctab_is_alpha,  // '&'
    ctab_is_alpha,  // '''
    ctab_is_alpha,  // '('
    ctab_is_alpha,  // ')'
    ctab_is_alpha,  // '*'
    ctab_is_alpha,  // '+'
    ctab_is_alpha,  // ','
    ctab_is_alpha,  // '-'
    ctab_is_digit,  // '.'
    ctab_is_alpha,  // '/'
    ctab_is_digit,  // '0'
    ctab_is_digit,  // '1'
    ctab_is_digit,  // '2'
    ctab_is_digit,  // '3'
    ctab_is_digit,  // '4'
    ctab_is_digit,  // '5'
    ctab_is_digit,  // '6'
    ctab_is_digit,  // '7'
    ctab_is_digit,  // '8'
    ctab_is_digit,  // '9'
    ctab_is_colon,  // ':'
    ctab_is_semi,   // ';'
    ctab_is_alpha,  // '<'
    ctab_is_alpha,  // '='
    ctab_is_alpha,  // '>'
    ctab_is_alpha,  // '?'
    ctab_is_string, // '@'
    ctab_is_alpha,  // 'A'
    ctab_is_alpha,  // 'B'
    ctab_is_alpha,  // 'C'
    ctab_is_alpha,  // 'D'
    ctab_is_alpha,  // 'E'
    ctab_is_alpha,  // 'F'
    ctab_is_alpha,  // 'G'
    ctab_is_alpha,  // 'H'
    ctab_is_alpha,  // 'I'
    ctab_is_alpha,  // 'J'
    ctab_is_alpha,  // 'K'
    ctab_is_alpha,  // 'L'
    ctab_is_alpha,  // 'M'
    ctab_is_alpha,  // 'N'
    ctab_is_alpha,  // 'O'
    ctab_is_alpha,  // 'P'
    ctab_is_alpha,  // 'Q'
    ctab_is_alpha,  // 'R'
    ctab_is_alpha,  // 'S'
    ctab_is_alpha,  // 'T'
    ctab_is_alpha,  // 'U'
    ctab_is_alpha,  // 'V'
    ctab_is_alpha,  // 'W'
    ctab_is_alpha,  // 'X'
    ctab_is_alpha,  // 'Y'
    ctab_is_alpha,  // 'Z'
    ctab_is_alpha,  // '['
    ctab_is_alpha,  // '\'
    ctab_is_alpha,  // ']'
    ctab_is_alpha,  // '^'
    ctab_is_alpha,  // '_'
    ctab_is_alpha,  // '`'
    ctab_is_alpha,  // 'a'
    ctab_is_alpha,  // 'b'
    ctab_is_alpha,  // 'c'
    ctab_is_alpha,  // 'd'
    ctab_is_alpha,  // 'e'
    ctab_is_alpha,  // 'f'
    ctab_is_alpha,  // 'g'
    ctab_is_alpha,  // 'h'
    ctab_is_alpha,  // 'i'
    ctab_is_alpha,  // 'j'
    ctab_is_alpha,  // 'k'
    ctab_is_alpha,  // 'l'
    ctab_is_alpha,  // 'm'
    ctab_is_alpha,  // 'n'
    ctab_is_alpha,  // 'o'
    ctab_is_alpha,  // 'p'
    ctab_is_alpha,  // 'q'
    ctab_is_alpha,  // 'r'
    ctab_is_alpha,  // 's'
    ctab_is_alpha,  // 't'
    ctab_is_alpha,  // 'u'
    ctab_is_alpha,  // 'v'
    ctab_is_alpha,  // 'w'
    ctab_is_alpha,  // 'x'
    ctab_is_alpha,  // 'y'
    ctab_is_alpha,  // 'z'
    ctab_is_alpha,  // '{'
    ctab_is_alpha,  // '|'
    ctab_is_alpha,  // '}'
    ctab_is_alpha,  // '~'
    ctab_is_junk,   // 177
    ctab_is_junk,   // 200
    ctab_is_junk,   // 201
    ctab_is_junk,   // 202
    ctab_is_junk,   // 203
    ctab_is_junk,   // 204
    ctab_is_junk,   // 205
    ctab_is_junk,   // 206
    ctab_is_junk,   // 207
    ctab_is_junk,   // 210
    ctab_is_junk,   // 211
    ctab_is_junk,   // 212
    ctab_is_junk,   // 213
    ctab_is_junk,   // 214
    ctab_is_junk,   // 215
    ctab_is_junk,   // 216
    ctab_is_junk,   // 217
    ctab_is_junk,   // 220
    ctab_is_junk,   // 221
    ctab_is_junk,   // 222
    ctab_is_junk,   // 223
    ctab_is_junk,   // 224
    ctab_is_junk,   // 225
    ctab_is_junk,   // 226
    ctab_is_junk,   // 227
    ctab_is_junk,   // 230
    ctab_is_junk,   // 231
    ctab_is_junk,   // 232
    ctab_is_junk,   // 233
    ctab_is_junk,   // 234
    ctab_is_junk,   // 235
    ctab_is_junk,   // 236
    ctab_is_junk,   // 237
    ctab_is_alpha,  // 240
    ctab_is_alpha,  // 241
    ctab_is_alpha,  // 242
    ctab_is_alpha,  // 243
    ctab_is_alpha,  // 244
    ctab_is_alpha,  // 245
    ctab_is_alpha,  // 246
    ctab_is_alpha,  // 247
    ctab_is_alpha,  // 250
    ctab_is_alpha,  // 251
    ctab_is_alpha,  // 252
    ctab_is_alpha,  // 253
    ctab_is_alpha,  // 254
    ctab_is_alpha,  // 255
    ctab_is_alpha,  // 256
    ctab_is_alpha,  // 257
    ctab_is_alpha,  // 260
    ctab_is_alpha,  // 261
    ctab_is_alpha,  // 262
    ctab_is_alpha,  // 263
    ctab_is_alpha,  // 264
    ctab_is_alpha,  // 265
    ctab_is_alpha,  // 266
    ctab_is_alpha,  // 267
    ctab_is_alpha,  // 270
    ctab_is_alpha,  // 271
    ctab_is_alpha,  // 272
    ctab_is_alpha,  // 273
    ctab_is_alpha,  // 274
    ctab_is_alpha,  // 275
    ctab_is_alpha,  // 276
    ctab_is_alpha,  // 277
    ctab_is_alpha,  // 300
    ctab_is_alpha,  // 301
    ctab_is_alpha,  // 302
    ctab_is_alpha,  // 303
    ctab_is_alpha,  // 304
    ctab_is_alpha,  // 305
    ctab_is_alpha,  // 306
    ctab_is_alpha,  // 307
    ctab_is_alpha,  // 310
    ctab_is_alpha,  // 311
    ctab_is_alpha,  // 312
    ctab_is_alpha,  // 313
    ctab_is_alpha,  // 314
    ctab_is_alpha,  // 315
    ctab_is_alpha,  // 316
    ctab_is_alpha,  // 317
    ctab_is_alpha,  // 320
    ctab_is_alpha,  // 321
    ctab_is_alpha,  // 322
    ctab_is_alpha,  // 323
    ctab_is_alpha,  // 324
    ctab_is_alpha,  // 325
    ctab_is_alpha,  // 326
    ctab_is_alpha,  // 327
    ctab_is_alpha,  // 330
    ctab_is_alpha,  // 331
    ctab_is_alpha,  // 332
    ctab_is_alpha,  // 333
    ctab_is_alpha,  // 334
    ctab_is_alpha,  // 335
    ctab_is_alpha,  // 336
    ctab_is_alpha,  // 337
    ctab_is_alpha,  // 340
    ctab_is_alpha,  // 341
    ctab_is_alpha,  // 342
    ctab_is_alpha,  // 343
    ctab_is_alpha,  // 344
    ctab_is_alpha,  // 345
    ctab_is_alpha,  // 346
    ctab_is_alpha,  // 347
    ctab_is_alpha,  // 350
    ctab_is_alpha,  // 351
    ctab_is_alpha,  // 352
    ctab_is_alpha,  // 353
    ctab_is_alpha,  // 354
    ctab_is_alpha,  // 355
    ctab_is_alpha,  // 356
    ctab_is_alpha,  // 357
    ctab_is_alpha,  // 360
    ctab_is_alpha,  // 361
    ctab_is_alpha,  // 362
    ctab_is_alpha,  // 363
    ctab_is_alpha,  // 364
    ctab_is_alpha,  // 365
    ctab_is_alpha,  // 366
    ctab_is_alpha,  // 367
    ctab_is_alpha,  // 370
    ctab_is_alpha,  // 371
    ctab_is_alpha,  // 372
    ctab_is_alpha,  // 373
    ctab_is_alpha,  // 374
    ctab_is_alpha,  // 375
    ctab_is_alpha,  // 376
    ctab_is_alpha   // 377
};


int
format_rcs_gram_lex(void)
{
    static stracc_t buffer;
    int             c;
    ctab_ty         d;

    for (;;)
    {
        c = ip->getch();
        if (c < 0)
        {
            trace(("EOF\n"));
            return 0;
        }
        d = ctab[c];
        switch (d)
        {
        default:
            trace(("JUNK\n"));
            return JUNK;

        case ctab_is_space:
            break;

        case ctab_is_alpha:
            buffer.clear();
            for (;;)
            {
                buffer.push_back(c);
                c = ip->getch();
                if (c < 0)
                    break;
                d = ctab[c];
                switch (d)
                {
                case ctab_is_alpha:
                case ctab_is_digit:
                    continue;

                default:
                    if (c >= 0)
                        ip->ungetc(c);
                    break;
                }
                break;
            }
            format_rcs_gram_lval.lv_string = buffer.mkstr();
            if (keyword_expected)
            {
                table_ty *tp =
                    (table_ty *)
                    stp->query(format_rcs_gram_lval.lv_string);
                if (tp)
                {
                    str_free(format_rcs_gram_lval.lv_string);
                    format_rcs_gram_lval.lv_string = 0;
                    keyword_expected = 0;
                    trace(("%s\n", tp->name));
                    return tp->value;
                }
            }
            trace(("IDENTIFIER \"%s\"\n",
                format_rcs_gram_lval.lv_string->str_text));
            return IDENTIFIER;

        case ctab_is_digit:
            buffer.clear();
            for (;;)
            {
                buffer.push_back(c);
                c = ip->getch();
                if (c < 0)
                    break;
                d = ctab[c];
                switch (d)
                {
                case ctab_is_digit:
                    continue;

                default:
                    if (c >= 0)
                        ip->ungetc(c);
                    break;
                }
                break;
            }
            format_rcs_gram_lval.lv_string = buffer.mkstr();
            trace(("NUMBER \"%s\"\n",
                format_rcs_gram_lval.lv_string->str_text));
            return NUMBER;

        case ctab_is_string:
            buffer.clear();
            for (;;)
            {
                c = ip->getch();
                if (c < 0)
                        break;
                d = ctab[c];
                if (d == ctab_is_string)
                {
                    c = ip->getch();
                    if (c < 0)
                        break;
                    d = ctab[c];
                    if (d != ctab_is_string)
                    {
                        ip->ungetc(c);
                        break;
                    }
                }
                buffer.push_back(c);
            }
            format_rcs_gram_lval.lv_string = buffer.mkstr();
            trace(("STRING\n"));
            return STRING;

        case ctab_is_colon:
            trace(("COLON\n"));
            return COLON;

        case ctab_is_semi:
            keyword_expected = 1;
            trace(("SEMI\n"));
            return SEMI;
        }
    }
}


void
rcs_lex_keyword_expected(void)
{
    keyword_expected = 1;
}


// vim: set ts=8 sw=4 et :
