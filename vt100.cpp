/*
 * vt100.c	ANSI/VT102 emulator code.
 *		This code was integrated to the Minicom communications
 *		package, but has been reworked to allow usage as a separate
 *		module.
 *
 *		It's not a "real" vt102 emulator - it's more than that:
 *		somewhere between vt220 and vt420 in commands.
 *
 *		This file is part of the minicom communications package,
 *		Copyright 1991-1995 Miquel van Smoorenburg.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * // jl 04.09.97 character map conversions in and out
 *    jl 06.07.98 use conversion tables with the capture file
 */

/* 05/09/2010 shoved head first into htpterm and flushed the handle-
 * sigflup
 *
 * 08/17/2012 Made more portable, also now we operate on a character buffer
 * sigflup
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vt100.h"
#ifndef Q_MOC_RUN
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/asio.hpp>
#endif

#define term_wsetfgcol(w, fg) ((w)->color = ((w)->color & 15) + ((fg) << 4))
#define term_wsetbgcol(w, bg) ((w)->color = ((w)->color & 240) + (bg))
#define term_wsetattr(w, a) ((w)->attr = (a))
#define term_wgetattr(w) ((w)->attr)
#define term_wsetregion(w, z1, z2) (((w)->sy1 = z1), ((w)->sy2 = z2))
#define term_wresetregion(w) ((w)->sy1 = 0, (w)->sy2 = ((w)->ws_conf.ws_row - 1))

/* BUGS!!!!
 * Why is this above 0xff? See XXX:mappers
 * back-tab \33[Z see XXX:bt
 * character sets see XXX:char sets
 * */

/*
 * The global variable esc_s holds the escape sequence status:
 * 0 - normal
 * 1 - ESC
 * 2 - ESC [
 * 3 - ESC [ ?
 * 4 - ESC (
 * 5 - ESC )
 * 6 - ESC #
 * 7 - ESC P
 */
static int esc_s = 0;

#define ESC 27

/* Structure to hold escape sequences. */
struct escseq
{
    unsigned int code;
    const char *vt100_st;
    const char *vt100_app;
    const char *ansi;
};

/* Escape sequences for different terminal types. */
static struct escseq vt_keys[] = {
    { K_F1,  "\033OP",   "\33OP",   "\33OP" },
    { K_F2,  "\033OQ",   "\33OQ",   "\33OQ" },
    { K_F3,  "\033OR",   "\33OR",   "\33OR" },
    { K_F4,  "\033OS",   "\33OS",   "\33OS" },
    { K_F5,  "\033[16~", "\33[16~", "\33OT" },
    { K_F6,  "\033[17~", "\33[17~", "\33OU" },
    { K_F7,  "\033[18~", "\33[18~", "\33OV" },
    { K_F8,  "\033[19~", "\33[19~", "\33OW" },
    { K_F9,  "\033[20~", "\33[20~", "\33OX" },
    { K_F10, "\033[21~", "\33[21~", "\33OY" },
    { K_F11, "\033[23~", "\33[23~", "\33OY" },
    { K_F12, "\033[24~", "\33[24~", "\33OY" },
    { K_HOME,"\033[1~",  "\33[1~",  "\33[H" },
    { K_PGUP,"\033[5~",  "\33[5~",  "\33[V" },
    { K_UP,  "\033[A",   "\33OA",   "\33[A" },
    { K_LT,  "\033[D",   "\33OD",   "\33[D" },
    { K_RT,  "\033[C",   "\33OC",   "\33[C" },
    { K_DN,  "\033[B",   "\33OB",   "\33[B" },
    { K_END, "\033[4~",  "\33[4~",  "\33[Y" },
    { K_PGDN,"\033[6~",  "\33[6~",  "\33[U" },
    { K_INS, "\033[2~",  "\33[2~",  "\33[@" },
    { K_DEL, "\033[3~",  "\33[3~",  "\33\177" },
    { 0,     NULL,   NULL,   NULL }
};

size_t Terminal::one_mbtowc(char *pwc, char *s, size_t n)
{
    int len;
    len = mbtowc((wchar_t *)pwc, s, n);
    if (len == -1)
    {
        *pwc = *s;
    }
    if (len <= 0)
    {
        len = 1;
    }
    return len;
}

/*
 * Initialize the emulator once.
 */

/* Partial init (after screen resize) */
void Terminal::vt_pinit(int fg, int bg)
{
    win->state.newy1 = 0;
    win->state.newy2 = win->ws_conf.ws_row - 1;
    term_wresetregion(win);
    if (fg > 0)
    {
        win->state.vt_fg = fg;
    }
    if (bg > 0)
    {
        win->state.vt_bg = bg;
    }
    term_wsetfgcol(win, win->state.vt_fg);
    term_wsetbgcol(win, win->state.vt_bg);
}

/* Set characteristics of emulator. */
void Terminal::vt_init(int type, int fg, int bg, int wrap, int add)
{
    win->wrap = 0;
    win->state.vt_wrap = -1;
    win->state.vt_addlf = 0;
    win->state.vt_asis = 0;
    win->state.vt_bs = 8;
    win->state.vt_insert = 0;
    win->state.vt_crlf = 0;
    win->state.enable_iconv = 0;
    win->state.ptr = 0;
    win->state.newy1 = 0;
    win->state.newy2 = win->ws_conf.ws_col;

    win->state.savex = 0;
    win->state.savey = 0;
    win->state.saveattr = XA_NORMAL;
    win->state.savecol = 112;

    win->state.vt_type = type;
    if (win->state.vt_type == ANSI)
    {
        win->state.vt_fg = WHITE;
        win->state.vt_bg = BLACK;
    }
    else
    {
        win->state.vt_fg = fg;
        win->state.vt_bg = bg;
    }
    if (wrap >= 0)
    {
        win->wrap = win->state.vt_wrap = wrap;
    }
    win->state.vt_addlf = add;
    win->state.vt_insert = 0;
    win->state.vt_crlf = 0;
    win->state.vt_om = 0;

    win->state.newy1 = 0;
    win->state.newy2 = win->ws_conf.ws_row - 1;
    term_wresetregion(win);
    win->state.vt_keypad = NORMAL;
    win->state.vt_cursor = NORMAL;
    win->state.vt_echo = 0;
    win->state.vt_tabs[0] = 0x01010100;
    win->state.vt_tabs[1] =
        win->state.vt_tabs[2] =
            win->state.vt_tabs[3] =
                win->state.vt_tabs[4] = 0x01010101;
    win->state.ptr = 0;
    memset(win->state.escparms, 0, sizeof(win->state.escparms));
    esc_s = 0;

    term_wsetfgcol(win, win->state.vt_fg);
    term_wsetbgcol(win, win->state.vt_bg);
}

/* Change some things on the fly. */
void Terminal::vt_set(int addlf, int wrap, int bscode,
            int echo, int cursor, int asis)
{
    if (addlf >= 0)
    {
        win->state.vt_addlf = addlf;
    }
    if (wrap >= 0)
    {
        win->wrap = win->state.vt_wrap = wrap;
    }
    if (bscode >= 0)
    {
        win->state.vt_bs = bscode;
    }
    if (echo >= 0)
    {
        win->state.vt_echo = echo;
    }
    if (cursor >= 0)
    {
        win->state.vt_cursor = cursor;
    }
    if (asis >= 0)
    {
        win->state.vt_asis = asis;
    }
}

/* Output a string to the modem. */
void Terminal::v_termout(const char *s)
{
    const char *p;

    if (win->state.vt_echo)
    {
        for (p = s; *p; p++)
        {
            vt_out(*p);
            if (!win->state.vt_addlf && *p == '\r')
            {
                vt_out('\n');
            }
        }
    }
    term_wputs(s);
}

/*
 * Escape code handling.
 */

/*
 * ESC was seen the last time. Process the next character.
 */
void Terminal::state1(int c)
{
    short x, y, f;

    switch (c)
    {
    case '[':  /* ESC [ */
        esc_s = 2;
        return;
    case '(':  /* ESC ( */
        esc_s = 4;
        return;
    case ')':  /* ESC ) */
        esc_s = 5;
        return;
    case '#': /* ESC # */
        esc_s = 6;
        return;
    case 'P': /* ESC P (DCS, Device Control String) */
        esc_s = 7;
        return;
    case ']':
        esc_s = 8;
        return;
    case 'D': /* Cursor down */
    case 'M': /* Cursor up */
        x = win->cursor_x;
        if (c == 'D') /* Down. */
        {
            y = win->cursor_y + 1;
            if (y == win->state.newy2 + 1)
            {
                term_wscroll(S_DOWN);
            }
            else if (win->cursor_y < win->ws_conf.ws_row)
            {
                term_wlocate(x, y);
            }
        }
        if (c == 'M')  /* Up. */
        {
            y = win->cursor_y - 1;
            if (y == win->state.newy1 - 1)
            {
                term_wscroll(S_UP);
            }
            else if (y >= 0)
            {
                term_wlocate(x, y);
            }
        }
        break;
    case 'E': /* CR + NL */
        term_wputs("\r\n");
        break;
    case '7': /* Save attributes and cursor position */
    case 's':
        win->state.savex = win->cursor_x;
        win->state.savey = win->cursor_y;
        win->state.saveattr = win->attr;
        win->state.savecol = win->color;
        break;
    case '8': /* Restore them */
    case 'u':
        win->color = win->state.savecol; /* HACK should use mc_wsetfgcol etc */
        term_wsetattr(win, win->state.saveattr);
        term_wlocate(win->state.savex, win->state.savey);
        break;
    case '=': /* Keypad into applications mode */
        win->state.vt_keypad = APPL;
        break;
    case '>': /* Keypad into numeric mode */
        win->state.vt_keypad = NORMAL;
        break;
    case 'Z': /* Report terminal type */
        if (win->state.vt_type == VT100)
        {
            v_termout("\033[?1;0c");
        }
        else
        {
            v_termout("\033[?c");
        }
        break;
    case 'c': /* Reset to initial state */
        f = XA_NORMAL;
        term_wsetattr(win, f);
        win->wrap = (win->state.vt_type != VT100);
        if (win->state.vt_wrap != -1)
        {
            win->wrap = win->state.vt_wrap;
        }
        win->state.vt_crlf = win->state.vt_insert = 0;
        vt_init(win->state.vt_type, win->state.vt_fg,
                win->state.vt_bg, win->wrap, 0);
        term_wlocate(0, 0);
        break;
    case 'H': /* Set tab in current position */
        x = win->cursor_x;
        if (x > 159)
        {
            x = 159;
        }
        win->state.vt_tabs[x / 32] |= 1 << (x % 32);
        break;
    case 'N': /* G2 character set for next character only*/
    case 'O': /* G3 "				"    */
    case '<': /* Exit vt52 mode */
    default:
        /* ALL IGNORED */
        break;
    }
    esc_s = 0;
}

/* ESC [ ... [hl] seen. */
void Terminal::ansi_mode(int on_off)
{
    int i;

    for (i = 0; i <= win->state.ptr; i++)
    {
        switch (win->state.escparms[i])
        {
        case 4: /* Insert mode  */
            win->state.vt_insert = on_off;
            break;
        case 20: /* Return key mode */
            win->state.vt_crlf = on_off;
            break;
        }
    }
}

/*
 * ESC [ ... was seen the last time. Process next character.
 */
void Terminal::state2(int c)
{
    short x, y, attr, f;

    /* See if a number follows */
    if (c >= '0' && c <= '9')
    {
        win->state.escparms[win->state.ptr] = 10 * win->state.escparms[win->state.ptr] + c - '0';
        return;
    }
    /* Separation between numbers ? */
    if (c == ';')
    {
        if (win->state.ptr < 15)
        {
            win->state.ptr++;
        }
        return;
    }
    /* ESC [ ? sequence */
    if (win->state.escparms[0] == 0 && win->state.ptr == 0 && c == '?')
    {
        esc_s = 3;
        return;
    }

    /* Process functions with zero, one, two or more arguments */
    switch (c)
    {
    case 'A':
    case 'B':
    case 'C':
    case 'D': /* Cursor motion */
        if ((f = win->state.escparms[0]) == 0)
        {
            f = 1;
        }
        x = win->cursor_x;
        y = win->cursor_y;
        x += f * ((c == 'C') - (c == 'D'));
        if (x < 0)
        {
            x = 0;
        }
        if (x >= win->ws_conf.ws_col)
        {
            x = win->ws_conf.ws_col - 1;
        }
        if (c == 'B') /* Down. */
        {
            y += f;
            if (y >= win->ws_conf.ws_row)
            {
                y = win->ws_conf.ws_row - 1;
            }
            if (y >= win->state.newy2 + 1)
            {
                y = win->state.newy2;
            }
        }
        if (c == 'A') /* Up. */
        {
            y -= f;
            if (y < 0)
            {
                y = 0;
            }
            if (y <= win->state.newy1 - 1)
            {
                y = win->state.newy1;
            }
        }
        term_wlocate(x, y);
        break;
    case 'X': /* Character erasing (ECH) */
        if ((f = win->state.escparms[0]) == 0)
        {
            f = 1;
        }
        term_wclrch(f);
        break;
    case 'K': /* Line erasing */
        switch (win->state.escparms[0])
        {
        case 0:
            term_wclreol();
            break;
        case 1:
            term_wclrbol();
            break;
        case 2:
            term_wclrel();
            break;
        }
        break;
    case 'J': /* Screen erasing */
        x = win->color;
        y = win->attr;
        if (win->state.vt_type == ANSI)
        {
            term_wsetattr(win, XA_NORMAL);
            term_wsetfgcol(win, WHITE);
            term_wsetbgcol(win, BLACK);
        }
        switch (win->state.escparms[0])
        {
        case 0:
            term_wclreos();
            break;
        case 1:
            term_wclrbos();
            break;
        case 2:
            term_winclr();
            break;
        }
        if (win->state.vt_type == ANSI)
        {
            win->color = x;
            win->attr = y;
        }
        break;
    case 'n': /* Requests / Reports */
        switch (win->state.escparms[0])
        {
        case 5: /* Status */
            v_termout("\033[0n");
            break;
        case 6: /* Cursor Position */
            boost::format f("\033[%d;%dR");
            v_termout(str(f % (win->cursor_y + 1) % (win->cursor_x + 1)).c_str());
            break;
        }
        break;
    case 'c': /* Identify Terminal Type */
        if (win->state.vt_type == VT100)
        {
            v_termout("\033[?1;2c");
            break;
        }
        v_termout("\033[?c");
        break;
    case 'x': /* Request terminal parameters. */
        /* Always answers 19200-8N1 no options. */
        {
            boost::format f("\033[%c;1;1;120;120;1;0x");
            v_termout(str(f % (win->state.escparms[0] == 1 ? '3' : '2')).c_str());
        }
        break;
    case 's': /* Save attributes and cursor position */
        win->state.savex = win->cursor_x;
        win->state.savey = win->cursor_y;
        win->state.saveattr = win->attr;
        win->state.savecol = win->color;
        break;
    case 'u': /* Restore them */
        term_wsetfgcol(win, win->state.savecol);
        term_wsetattr(win, win->state.saveattr);
        term_wlocate(win->state.savex, win->state.savey);
        break;
    case 'h':
        ansi_mode(1);
        break;
    case 'l':
        ansi_mode(0);
        break;
    case 'H':
    case 'f': /* Set cursor position */
        if ((y = win->state.escparms[0]) == 0)
        {
            y = 1;
        }
        if ((x = win->state.escparms[1]) == 0)
        {
            x = 1;
        }
        if (win->state.vt_om)
        {
            y += win->state.newy1;
        }
        term_wlocate(x - 1, y - 1);
        break;
    case 'g': /* Clear tab stop(s) */
        if (win->state.escparms[0] == 0)
        {
            x = win->cursor_x;
            if (x > 159)
            {
                x = 159;
            }
            win->state.vt_tabs[x / 32] &= ~(1 << x % 32);
        }
        if (win->state.escparms[0] == 3)
        {
            for (x = 0; x < 5; x++)
            {
                win->state.vt_tabs[x] = 0;
            }
        }
        break;
    case 'm': /* Set attributes */
        attr = term_wgetattr((win));
        for (f = 0; f <= win->state.ptr; f++)
        {
            if (win->state.escparms[f] >= 30 && win->state.escparms[f] <= 37)
            {
                term_wsetfgcol(win, win->state.escparms[f] - 30);
            }
            if (win->state.escparms[f] >= 40 && win->state.escparms[f] <= 47)
            {
                term_wsetbgcol(win, win->state.escparms[f] - 40);
            }
            switch (win->state.escparms[f])
            {
            case 0:
                attr = XA_NORMAL;
                term_wsetfgcol(win, win->state.vt_fg);
                term_wsetbgcol(win, win->state.vt_bg);
                break;
            case 4:
                attr |= XA_UNDERLINE;
                break;
            case 7:
                attr |= XA_REVERSE;
                break;
            case 1:
                attr |= XA_BOLD;
                break;
            case 5:
                attr |= XA_BLINK;
                break;
            case 22: /* Bold off */
                attr &= ~XA_BOLD;
                break;
            case 24: /* Not underlined */
                attr &= ~XA_UNDERLINE;
                break;
            case 25: /* Not blinking */
                attr &= ~XA_BLINK;
                break;
            case 27: /* Not reverse */
                attr &= ~XA_REVERSE;
                break;
            case 39: /* Default fg color */
                term_wsetfgcol(win, win->state.vt_fg);
                break;
            case 49: /* Default bg color */
                term_wsetbgcol(win, win->state.vt_bg);
                break;
            }
        }
        term_wsetattr(win, attr);
        break;
    case 'L': /* Insert lines */
        if ((x = win->state.escparms[0]) == 0)
        {
            x = 1;
        }
        for (f = 0; f < x; f++)
        {
            term_winsline();
        }
        break;
    case 'M': /* Delete lines */
        if ((x = win->state.escparms[0]) == 0)
        {
            x = 1;
        }
        for (f = 0; f < x; f++)
        {
            term_wdelline();
        }
        break;
    case 'P': /* Delete Characters */
        if ((x = win->state.escparms[0]) == 0)
        {
            x = 1;
        }
        for (f = 0; f < x; f++)
        {
            term_wdelchar();
        }
        break;
    case '@': /* Insert Characters */
        if ((x = win->state.escparms[0]) == 0)
        {
            x = 1;
        }
        for (f = 0; f < x; f++)
        {
            term_winschar(' ');
        }
        break;
    case 'r': /* Set scroll region */
        if ((win->state.newy1 = win->state.escparms[0]) == 0)
        {
            win->state.newy1 = 1;
        }
        if ((win->state.newy2 = win->state.escparms[1]) == 0)
        {
            win->state.newy2 = win->ws_conf.ws_row;
        }
        win->state.newy1--; win->state.newy2--;
        if (win->state.newy1 < 0)
        {
            win->state.newy1 = 0;
        }
        if (win->state.newy2 < 0)
        {
            win->state.newy2 = 0;
        }
        if (win->state.newy1 >= win->ws_conf.ws_row)
        {
            win->state.newy1 = win->ws_conf.ws_row - 1;
        }
        if (win->state.newy2 >= win->ws_conf.ws_row)
        {
            win->state.newy2 = win->ws_conf.ws_row - 1;
        }
        if (win->state.newy1 >= win->state.newy2)
        {
            win->state.newy1 = 0;
            win->state.newy2 = win->ws_conf.ws_row - 1;
        }
        if (win->state.newy1 > win->state.newy2)
        {
            term_wsetregion(win, win->state.newy2, win->state.newy1);
        }
        else
        {
            term_wsetregion(win, win->state.newy1, win->state.newy2);
        }
        /// XXX xterm goes to 0,0 ?
        term_wlocate(0, win->state.newy1);
        break;
    case 'i': /* Printing */
    case 'y': /* Self test modes */
    case 'Z':
        /* XXX:bt */
        break;
    default:
        /* IGNORED */
        break;
    }
    /* Ok, our escape sequence is all done */
    esc_s = 0;
    win->state.ptr = 0;
    memset(win->state.escparms, 0, sizeof(win->state.escparms));
    return;
}

/* ESC [? ... [hl] seen. */
void Terminal::dec_mode(int on_off)
{
    int i;
    for (i = 0; i <= win->state.ptr; i++)
    {
        switch (win->state.escparms[i])
        {
        case 1: /* Cursor keys in cursor/appl mode */
            win->state.vt_cursor = on_off ? APPL : NORMAL;
            win->dec_mode[DEC_1] = on_off ? 1 : 0;
            break;
        case 6: /* Origin mode. */
            win->state.vt_om = on_off;
            term_wlocate(0, win->state.newy1);
            break;
        case 7: /* Auto wrap */
            win->wrap = on_off;
            break;
        case 25: /* Cursor on/off */
            term_wcursor(on_off ? CNORMAL : CNONE);
            break;
        case 47:
            break;
        case 67: /* Backspace key sends. (FIXME: vt420) */
            /* setbackspace(on_off ? 8 : 127); */
            break;
        default: /* Mostly set up functions */
            /* IGNORED */
            break;
        }
    }
}

/*
 * ESC [ ? ... seen.
 */
void Terminal::state3(int c)
{
    /* See if a number follows */
    if (c >= '0' && c <= '9')
    {
        win->state.escparms[win->state.ptr] = 10 * win->state.escparms[win->state.ptr] + c - '0';
        return;
    }
    switch (c)
    {
    case 'h':
        dec_mode(1);
        break;
    case 'l':
        dec_mode(0);
        break;
    case 'i': /* Printing */
    case 'n': /* Request printer status */
    default:
        /* IGNORED */
        break;
    }
    esc_s = 0;
    win->state.ptr = 0;
    memset(win->state.escparms, 0, sizeof(win->state.escparms));
    return;
}

/*
 * ESC ( Seen.
 */
void Terminal::state4(int c)
{
    /* Switch Character Sets. */
    switch (c)
    {
    case 'A':
    case 'B':
        break;
    case '0':
    case 'O':
        break;
    }
    esc_s = 0;
}

/*
 * ESC ) Seen.
 */
/* XXX:char set */
void Terminal::state5(int c)
{
    /* Switch Character Sets. */
    switch (c)
    {
    case 'A':
    case 'B':
        break;
    case 'O':
    case '0':
        /* default character set */
        break;
    }
    esc_s = 0;
}

/*
 * ESC # Seen.
 */
void Terminal::state6(int c)
{
    int x, y;

    /* Double height, double width and selftests. */
    switch (c)
    {
    case '8':
        /* Selftest: fill screen with E's */
        win->doscroll = 0;
        term_wlocate(0, 0);
        for (y = 0; y < win->ws_conf.ws_row; y++)
        {
            term_wlocate(0, y);
            for (x = 0; x < win->ws_conf.ws_col; x++)
            {
                term_wputc('E');
            }
        }
        term_wlocate(0, 0);
        win->doscroll = 1;
        term_wredraw();
        break;
    default:
        /* IGNORED */
        break;
    }
    esc_s = 0;
}

/*
 * ESC P Seen.
 */
void Terminal::state7(int c)
{
    /*
     * Device dependant control strings. The Minix virtual console package
     * uses these sequences. We can only turn cursor on or off, because
     * that's the only one supported in termcap. The rest is ignored.
     */
    static char buf[17];
    static int pos = 0;
    static int state = 0;

    if (c == ESC)
    {
        state = 1;
        return;
    }
    if (state == 1)
    {
        buf[pos] = 0;
        pos = 0;
        state = 0;
        esc_s = 0;
        if (c != '\\')
        {
            return;
        }
        /* Process string here! */
        if (!strcmp(buf, "cursor.on"))
        {
            term_wcursor(CNORMAL);
        }
        if (!strcmp(buf, "cursor.off"))
        {
            term_wcursor(CNONE);
        }
        if (!strcmp(buf, "linewrap.on"))
        {
            win->state.vt_wrap = -1;
            win->wrap = 1;
        }
        if (!strcmp(buf, "linewrap.off"))
        {
            win->state.vt_wrap = -1;
            win->wrap = 0;
        }
        return;
    }
    if (pos > 15)
    {
        return;
    }
    buf[pos++] = c;
}

void Terminal::state8(int c)
{
    // throw away until the bell
    c = c;
}

void Terminal::vt_out(unsigned int ch)
{
    int f;
    unsigned char c;
    int go_on = 0;
    wchar_t wc;
    if (!ch)
    {
        return;
    }
    boost::unique_lock<boost::mutex> lock(_charRowsMutex);
    c = (unsigned char)ch;

    /* Process <31 chars first, even in an escape sequence. */
    switch (c)
    {
    case '\r': /* Carriage return */
        term_wputc(c);
        if (win->state.vt_addlf)
        {
            term_wputc('\n');
        }
        break;
    case '\t': /* Non - destructive TAB */
        /* Find next tab stop. */
        for (f = win->cursor_x + 1; f < 160; f++)
        {
            if (win->state.vt_tabs[f / 32] & (1 << f % 32))
            {
                break;
            }
        }
        if (f >= win->ws_conf.ws_col)
        {
            f = win->ws_conf.ws_col - 1;
        }
        term_wlocate(f, win->cursor_y);
        break;
    case 013: /* Old Minix: CTRL-K = up */
        term_wlocate(win->cursor_x, win->cursor_y - 1);
        break;
    case '\f': /* Form feed: clear screen. */
        term_winclr();
        term_wlocate(0, 0);
        break;
    case 14:
        break;
    case 15:
        break;
    case 24:
    case 26:  /* Cancel escape sequence. */
        esc_s = 0;
        break;
    case ESC: /* Begin escape sequence */
        esc_s = 1;
        break;
    case 128 + ESC: /* Begin ESC [ sequence. */
        esc_s = 2;
        break;
    case '\n':
    case '\b':
    case 7: /* Bell */
        if (esc_s != 8)
        {
            term_wputc(c);
        }
        else
        {
            esc_s = 0;
        }
        break;
    default:
        go_on = 1;
        break;
    }
    if (!go_on)
    {
        return;
    }

    /* Now see which state we are in. */
    switch (esc_s)
    {
    case 0: /* Normal character */
        /* XXX:mappers */
        c &= 0xff;

        if (!win->state.enable_iconv)
        {
            one_mbtowc ((char *)&wc, (char *)&c, 1);
            if (win->state.vt_insert)
            {
                term_winschar(wc);
            }
            else
            {
                term_wputc(wc);
            }
        }
        else
        {
            term_wputc(c);
        }

        break;
    case 1: /* ESC seen */
        state1(c);
        break;
    case 2: /* ESC [ ... seen */
        state2(c);
        break;
    case 3:
        state3(c);
        break;
    case 4:
        state4(c);
        break;
    case 5:
        state5(c);
        break;
    case 6:
        state6(c);
        break;
    case 7:
        state7(c);
        break;
    case 8:
        state8(c);
        break;
    }
}

/* Translate keycode to escape sequence. */
void Terminal::vt_send(unsigned int c)
{
    char s[3];
    int f;

    /* Special key? */
    if (c < 256)
    {
        /* Translate backspace key? */
        if (c == K_ERA)
        {
            c = win->state.vt_bs;
        }
        s[0] = c;
        s[1] = 0;
        /* CR/LF mode? */
        if (c == '\r' && win->state.vt_crlf)
        {
            s[1] = '\n';
            s[2] = 0;
        }
        v_termout(s);
        if (win->state.vt_nl_delay > 0 && c == '\r')
        {
            boost::this_thread::sleep(boost::posix_time::milliseconds(win->state.vt_nl_delay));
        }
        return;
    }

    /* Look up code in translation table. */
    for (f = 0; vt_keys[f].code; f++)
    {
        if (vt_keys[f].code == c)
        {
            break;
        }
    }
    if (vt_keys[f].code == 0)
    {
        return;
    }

    /* Now send appropriate escape code. */

    if (win->state.vt_type == VT100)
    {
        if (win->state.vt_cursor == NORMAL)
        {
            v_termout(vt_keys[f].vt100_st);
        }
        else
        {
            v_termout(vt_keys[f].vt100_app);
        }
    }
    else
    {
        v_termout(vt_keys[f].ansi);
    }
}

/* insert a character at cursor position */
/* XXX:winschar */
void Terminal::term_winschar(unsigned char c)
{
    int x;
    for (x = win->ws_conf.ws_col - 1; x != win->cursor_x; x--)
    {
        _charRows[win->cursor_y]->_rowData[x + 1] = _charRows[win->cursor_y]->_rowData[x];
    }
    _charRows[win->cursor_y]->_rowData[win->cursor_x].text = c;
    _charRows[win->cursor_y]->_rowData[win->cursor_x].col = win->color;
    _charRows[win->cursor_y]->_rowData[win->cursor_x].attrib = win->attr;
    term_wmove(RIGHT);
}

/* print a character */
void Terminal::term_wputc(unsigned char c)
{
    switch (c)
    {
    /* XXX:vt_out you */
    case '\r':
        win->cursor_x = 0;
        setDirty(win->cursor_y);
        break;
    case '\n':
        term_wmove(DOWN);
        break;
    case BELL:
        break;
    default:
        if (c == KEY_BACKSPACE)
        {
            term_wmove(LEFT);
            term_wdelchar();
            break;
        }
        if (c < 32)
        {
            break;
        }
        _charRows[win->cursor_y]->_rowData[win->cursor_x].text = c;
        _charRows[win->cursor_y]->_rowData[win->cursor_x].col = win->color;
        _charRows[win->cursor_y]->_rowData[win->cursor_x].attrib = win->attr;
        setDirty(win->cursor_y);
        term_wmove(RIGHT);
        break;
    }
}

void Terminal::term_wmove(int dir)
{
    switch (dir)
    {
    case LEFT:
        win->cursor_x--;
        if (win->cursor_x < 0)
        {
            win->cursor_x = 0;
        }
        break;
    case RIGHT:
        win->cursor_x++;
        if (win->cursor_x >= win->ws_conf.ws_col)
        {
            if (win->wrap == 1)
            {
                win->cursor_x = 0;
                term_wmove(DOWN);
            }
            else
            {
                win->cursor_x = win->ws_conf.ws_col - 1;
            }
        }
        break;
    case UP:
        win->cursor_y--;
        if (win->cursor_y < win->sy1)
        {
            win->cursor_y = win->sy1;
        }
        break;
    case DOWN:
        win->cursor_y++;
        if (win->cursor_y > win->sy2)
        {
            term_wscroll(S_DOWN);
            win->cursor_y = win->sy2;
        }
        break;
    }
    setDirty(win->cursor_y);
}

/* locate the cursor */
void Terminal::term_wlocate(int x, int y)
{
    if (x >= 0)
    {
        if (x < win->ws_conf.ws_col)
        {
            win->cursor_x = x;
        }
        else
        {
            win->cursor_x = win->ws_conf.ws_col - 1;
        }
    }
    else
    {
        win->cursor_x = 0;
    }
    if (y >= 0)
    {
        if (y < win->ws_conf.ws_row)
        {
            win->cursor_y = y;
        }
        else
        {
            win->cursor_y = win->ws_conf.ws_row - 1;
        }
    }
    else
    {
        win->cursor_y = 0;
    }
}

/* redraw the term */
void Terminal::term_wredraw()
{
    int y;
    for (y = 0; y < win->ws_conf.ws_row; y++)
    {
        setDirty(y);
    }
}

/* flush output */
void Terminal::term_wflush()
{
}

/* clear the window */
void Terminal::term_winclr()
{
    int y;

    for (y = 0; y < win->ws_conf.ws_row; y++)
    {
        _charRows[y]->clearChars();
        setDirty(y);
    }
}

/* scroll window */
void Terminal::term_wscroll(int dir)
{
    boost::shared_ptr<row_t> rowData(new row_t(win->ws_conf.ws_col));
    int store;
    int start, end;
    if (win->sy1 > win->sy2)
    {
        store = win->sy1;
        win->sy1 = win->sy2;
        win->sy2 = store;
    }
    if (dir == S_DOWN)
    {
        start = win->sy1;
        end = win->sy2;
    }
    else
    {
        start = win->sy2;
        end = win->sy1;
    }

    _charRows.erase(_charRows.begin() + start);
    _charRows.insert(_charRows.begin() + end, rowData);

    term_wredraw();
}

void row_t::clearChars(int x, int direction)
{
    int i = x;
    int end = _width;
    if (direction == -1)
    {
        i -= 1;
        end = -1;
    }
    for (; i != end; i += direction)
    {
        _rowData[i].text = 0;
        _rowData[i].col = 0;
        _rowData[i].attrib = 0;
    }
}

/* clear to end of line */
void Terminal::term_wclreol()
{
    _charRows[win->cursor_y]->clearChars(win->cursor_x);
}

/* clear to beginning of line */
/* XXX:clrbol */
void Terminal::term_wclrbol()
{
    _charRows[win->cursor_y]->clearChars(win->cursor_x, -1);
}

/* clear to end of screen */
void Terminal::term_wclreos()
{
    int x, y;

    x = win->cursor_x;
    for (y = win->cursor_y; y < win->ws_conf.ws_row; y++)
    {
        _charRows[y]->clearChars(x);
        x = 0;
    }
}

/* clear to beginning of screen */
void Terminal::term_wclrbos()
{
    int x, y;

    x = win->cursor_x;
    for (y = win->cursor_y; y >= 0; y--)
    {
        _charRows[y]->clearChars(x, -1);
        x = win->ws_conf.ws_col;
    }
}

/* clear entire line */
void Terminal::term_wclrel()
{
    _charRows[win->cursor_y]->clearChars();
}

/* insert? */
void Terminal::term_winsline()
{
    int store;

    if (win->cursor_y >= win->sy2)
    {
        return;
    }
    store = win->sy1;
    win->sy1 = win->cursor_y;
    term_wscroll(S_UP);
    win->sy1 = store;
}

/* delete line */
void Terminal::term_wdelline()
{
    int store;
    if (win->cursor_y >= win->sy2)
    {
        return;
    }
    store = win->sy1;
    win->sy1 = win->cursor_y;
    term_wscroll(S_DOWN);
    win->sy1 = store;
}

/* delete char under cursor */
void Terminal::term_wdelchar()
{
    int i;

    for (i = win->cursor_x; i < win->ws_conf.ws_col - 1; i++)
    {
        _charRows[win->cursor_y]->_rowData[i] = _charRows[win->cursor_y]->_rowData[i + 1];
    }
    _charRows[win->cursor_y]->_rowData[i].text = 0;
    _charRows[win->cursor_y]->_rowData[i].col = 0;
    _charRows[win->cursor_y]->_rowData[i].attrib = 0;
}

/* clear characters */
void Terminal::term_wclrch(int n)
{
    n = n;
}

/* set cursor type */
void Terminal::term_wcursor(int type)
{

    /* XXX not sure what this does we have
     * CNONE and CNORMAL */
    if (type == CNONE)
    {
        win->dec_mode[DEC_25] = 0;
    }
    else
    {
        win->dec_mode[DEC_25] = 1;
    }
}

/* print a string */
void Terminal::term_wputs(const char *s)
{
    str_out(s);
}

Terminal::~Terminal()
{
    if (win != NULL)
    {
        delete win;
        win = NULL;
    }
}

Terminal::Terminal(int w, int h)
{
    term_t *pwin;
    win = new term_t;
    memset(win, 0, sizeof(term_t));
    pwin = win;
    pwin->ws_conf.ws_col = w;
    pwin->ws_conf.ws_row = h;
    for (int y = 0; y < pwin->ws_conf.ws_row; y++)
    {
        boost::shared_ptr<row_t> rowData(new row_t(pwin->ws_conf.ws_col));
        _charRows.push_back(rowData);
        _charRows[win->cursor_y]->clearChars();
    }
    pwin->cursor_x = 0;
    pwin->cursor_y = 0;
    pwin->sy1 = 0;
    pwin->sy2 = h - 1;

    pwin->color = 0;
    pwin->attr = 0;

    vt_init(ANSI, 0, 0, 1, 0);
}

char_t Terminal::getChar(int x, int y)
{
    boost::unique_lock<boost::mutex> lock(_charRowsMutex);
    return _charRows[y]->_rowData[x];
}

unsigned short Terminal::getWinSizeRow()
{
    return win->ws_conf.ws_row;
}

unsigned short Terminal::getWinSizeCol()
{
    return win->ws_conf.ws_col;
}

void Terminal::setDirty(int y)
{
    win->_winDirty = true;
    _charRows[y]->_rowDirty = true;
}

bool Terminal::isRowDirty(int y)
{
    boost::unique_lock<boost::mutex> lock(_charRowsMutex);
    bool ret = _charRows[y]->_rowDirty;
    _charRows[y]->_rowDirty = false;
    return ret;
}

bool Terminal::isWinDirty()
{
    bool ret = win->_winDirty;
    win->_winDirty = false;
    return ret;
}
#if 0
void Terminal::resize_term(int w, int h)
{
    /*
    term_t *pwin = (term_t *)win;
    int x, y, old_w, old_h;
    char_t *old_chars;
    char_t *chara;
    unsigned char text, col, attrib;

    old_chars = pwin->charMem;
    pwin->charMem = (char_t *)malloc(w * h * sizeof(char_t));
    memset(pwin->charMem, 0, w * h * sizeof(char_t));

    old_w = pwin->ws_conf.ws_col;
    old_h = pwin->ws_conf.ws_row;
    pwin->ws_conf.ws_col = w;
    pwin->ws_conf.ws_row = h;

    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            if ((x < pwin->ws_conf.ws_col) &&
                (y < pwin->ws_conf.ws_row))
            {
                text =   old_chars[x + (y * old_w)].text;
                col =    old_chars[x + (y * old_w)].col;
                attrib = old_chars[x + (y * old_w)].attrib;
            }
            else
            {
                text = 0;
                col = 0;
                attrib = 0;
            }
            chara = getMutableChar(x, y);
            chara->text = text;
            chara->col = col;
            chara->attrib = attrib;
        }
    }

    free(old_chars);
    if (pwin->cursor_x > w)
    {
        pwin->cursor_x = w;
    }
    if (pwin->cursor_y > h)
    {
        pwin->cursor_y = h;
    }

    pwin->sy1 = 0;
    pwin->sy2 = h - 1;
    */
}
#endif
