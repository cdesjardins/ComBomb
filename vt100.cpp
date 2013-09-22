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
#endif

#define W ws_conf.ws_col
#define H ws_conf.ws_row

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
    int code;
    const char *vt100_st;
    const char *vt100_app;
    const char *ansi;
};

/* Escape sequences for different terminal types. */
static struct escseq vt_keys[] = {
    { K_F1,  "OP",   "OP",   "OP" },
    { K_F2,  "OQ",   "OQ",   "OQ" },
    { K_F3,  "OR",   "OR",   "OR" },
    { K_F4,  "OS",   "OS",   "OS" },
    { K_F5,  "[16~", "[16~", "OT" },
    { K_F6,  "[17~", "[17~", "OU" },
    { K_F7,  "[18~", "[18~", "OV" },
    { K_F8,  "[19~", "[19~", "OW" },
    { K_F9,  "[20~", "[20~", "OX" },
    { K_F10, "[21~", "[21~", "OY" },
    { K_F11, "[23~", "[23~", "OY" },
    { K_F12, "[24~", "[24~", "OY" },
    { K_HOME,    "[1~",  "[1~",  "[H" },
    { K_PGUP,    "[5~",  "[5~",  "[V" },
    { K_UP,  "[A",   "OA",   "[A" },
    { K_LT,  "[D",   "OD",   "[D" },
    { K_RT,  "[C",   "OC",   "[C" },
    { K_DN,  "[B",   "OB",   "[B" },
    { K_END, "[4~",  "[4~",  "[Y" },
    { K_PGDN,    "[6~",  "[6~",  "[U" },
    { K_INS, "[2~",  "[2~",  "[@" },
    { K_DEL, "[3~",  "[3~",  "\177" },
    { 0,     NULL,   NULL,   NULL }
};

char answerback[ANSWERBACK_LEN] = { 0 };
char convcap[CONVCAP_LEN] = { 0 };

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
    win->state.newy2 = win->H - 1;
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
    win->state.newy2 = win->H - 1;
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
    term_wputs((const unsigned char*)s);
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
    case 'D': /* Cursor down */
    case 'M': /* Cursor up */
        x = win->cursor_x;
        if (c == 'D') /* Down. */
        {
            y = win->cursor_y + 1;
            if (y == win->state.newy2 + 1)
            {
                term_wscroll(S_UP);
            }
            else
            if (win->cursor_y < win->H)
            {
                term_wlocate(x, y);
            }
        }
        if (c == 'M')  /* Up. */
        {
            y = win->cursor_y - 1;
            if (y == win->state.newy1 - 1)
            {
                term_wscroll(S_DOWN);
            }
            else
            if (y >= 0)
            {
                term_wlocate(x, y);
            }
        }
        break;
    case 'E': /* CR + NL */
        term_wputs((unsigned char *)"\r\n");
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
        if (x >= win->W)
        {
            x = win->W - 1;
        }
        if (c == 'B') /* Down. */
        {
            y += f;
            if (y >= win->H)
            {
                y = win->H - 1;
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
            win->state.newy2 = win->H;
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
        if (win->state.newy1 >= win->H)
        {
            win->state.newy1 = win->H - 1;
        }
        if (win->state.newy2 >= win->H)
        {
            win->state.newy2 = win->H - 1;
        }
        if (win->state.newy1 >= win->state.newy2)
        {
            win->state.newy1 = 0;
            win->state.newy2 = win->H - 1;
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
        for (y = 0; y < win->H; y++)
        {
            term_wlocate(0, y);
            for (x = 0; x < win->W; x++)
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
//      printf("tab pants\n");
        /* Find next tab stop. */
        for (f = win->cursor_x + 1; f < 160; f++)
        {
            if (win->state.vt_tabs[f / 32] & (1 << f % 32))
            {
                break;
            }
        }
        if (f >= win->W)
        {
            f = win->W - 1;
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
        term_wputc(c);
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
    v_termout("\33");
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
    int i;

#ifdef DEBUG
    printf("term_winschar\n");
#endif

    for (i = win->ws_conf.ws_col - 1; i != win->cursor_x; i--)
    {
        win->chars[i + (win->cursor_y * win->ws_conf.ws_col) + 1].text = win->chars[i + (win->cursor_y * win->ws_conf.ws_col)].text;
        win->chars[i + (win->cursor_y * win->ws_conf.ws_col) + 1].col = win->chars[i + (win->cursor_y * win->ws_conf.ws_col)].col;
        win->chars[i + (win->cursor_y * win->ws_conf.ws_col) + 1].attrib = win->chars[i + (win->cursor_y * win->ws_conf.ws_col)].attrib;
    }
    win->chars[win->cursor_x + (win->cursor_y * win->ws_conf.ws_col)].text = c;
    win->chars[win->cursor_x + (win->cursor_y * win->ws_conf.ws_col)].col = win->color;
    win->chars[win->cursor_x + (win->cursor_y * win->ws_conf.ws_col)].attrib = win->attr;

    term_wmove(RIGHT);
}

/* print a character */
void Terminal::term_wputc(unsigned char c)
{
#ifdef DEBUG
    printf("term_wputc  %x\n", c);
#endif

    switch (c)
    {
    /* XXX:vt_out you */
    case '\r':
        win->cursor_x = 0;
        break;
    case '\n':
//   win->cursor_x = 0;
        term_wmove(DOWN);
        break;
    case BELL:
        /* XXX add bell things */
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
        win->chars[win->cursor_x + (win->cursor_y * win->ws_conf.ws_col)].text = c;
        win->chars[win->cursor_x + (win->cursor_y * win->ws_conf.ws_col)].col = win->color;
        win->chars[win->cursor_x + (win->cursor_y * win->ws_conf.ws_col)].attrib = win->attr;
        term_wmove(RIGHT);
        break;
    }
    win->dirty = true;
}

void Terminal::term_wmove(int dir)
{
#ifdef DEBUG
    printf("term_wmove\n");
#endif

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
    win->dirty = true;
}

/* locate the cursor */
void Terminal::term_wlocate(int x, int y)
{
#ifdef DEBUG
    printf("term_wlocate\n");
#endif

    if ((x >= 0 || x <= win->ws_conf.ws_col) &&
        (y >= 0 || y <= win->ws_conf.ws_row))
    {
        win->cursor_x = x;
        win->cursor_y = y;
    }
    win->dirty = true;
}

/* redraw the term */
void Terminal::term_wredraw()
{
    printf("wredraw\n");
    win->dirty = true;
}

/* flush output */
void Terminal::term_wflush()
{
#ifdef DEBUG
    printf("term_wflush\n");
#endif
}

/* clear the window */
void Terminal::term_winclr()
{
    int q;

#ifdef DEBUG
    printf("term_winclr\n");
#endif

    for (q = 0; q < (win->ws_conf.ws_row * win->ws_conf.ws_col); q++)
    {
        win->chars[q].text = win->chars[q].col = win->chars[q].attrib = 0;
    }
}

/* scroll window */
void Terminal::term_wscroll(int dir)
{
    int x, y;
    int store;

#ifdef DEBUG
    printf("term_wscroll\n");
#endif
    printf("%d %d %d %d\n", win->cursor_x, win->cursor_y, win->sy1, win->sy2);

    if (win->sy1 > win->sy2)
    {
        store = win->sy1;
        win->sy1 = win->sy2;
        win->sy2 = store;
    }
    if (dir == S_DOWN)
    {
        for (y = win->sy1; y < win->sy2; y++)
        {
            for (x = 0; x < win->ws_conf.ws_col; x++)
            {
                win->chars[x + (y * win->ws_conf.ws_col)].text = win->chars[x + ((y + 1) * win->ws_conf.ws_col)].text;
                win->chars[x + (y * win->ws_conf.ws_col)].col =  win->chars[x + ((y + 1) * win->ws_conf.ws_col)].col;
                win->chars[x + (y * win->ws_conf.ws_col)].attrib = win->chars[x + ((y + 1) * win->ws_conf.ws_col)].attrib;
            }
        }
        for (x = 0; x < win->ws_conf.ws_col; x++)
        {
            win->chars[x + (win->ws_conf.ws_col * win->sy2)].text =
                win->chars[x + (win->ws_conf.ws_col * win->sy2)].col =
                    win->chars[x + (win->ws_conf.ws_col * win->sy2)].attrib = 0;
        }
    }
    else
    {
        /* scroll UP */
        for (y = win->sy2; y > win->sy1; y--)
        {
            for (x = 0; x < win->ws_conf.ws_col; x++)
            {
                win->chars[x + (y * win->ws_conf.ws_col)].text = win->chars[x + ((y - 1) * win->ws_conf.ws_col)].text;
                win->chars[x + (y * win->ws_conf.ws_col)].col = win->chars[x + ((y - 1) * win->ws_conf.ws_col)].col;
                win->chars[x + (y * win->ws_conf.ws_col)].attrib =  win->chars[x + ((y - 1) * win->ws_conf.ws_col)].attrib;
            }
        }
        for (x = 0; x < win->ws_conf.ws_col; x++)
        {
            win->chars[x + (win->ws_conf.ws_col * win->sy1)].text =
                win->chars[x + (win->ws_conf.ws_col * win->sy1)].col =
                    win->chars[x + (win->ws_conf.ws_col * win->sy1)].attrib = 0;
        }
    }
    win->dirty = true;
}

/* clear to end of line */
void Terminal::term_wclreol()
{
    int x;

#ifdef DEBUG
    printf("term_wclreol\n");
#endif

    for (x = win->cursor_x; x < win->ws_conf.ws_col; x++)
    {
        win->chars[x + (win->cursor_y * win->ws_conf.ws_col)].text =
            win->chars[x + (win->cursor_y * win->ws_conf.ws_col)].col =
                win->chars[x + (win->cursor_y * win->ws_conf.ws_col)].attrib = 0;
    }
}

/* clear to beginning of line */
/* XXX:clrbol */
void Terminal::term_wclrbol()
{
    int x;

#ifdef DEBUG
    printf("term_wclrbol\n");
#endif

    for (x = win->ws_conf.ws_col; x > 0; x--)
    {
        win->chars[x].text =
            win->chars[x].col =
                win->chars[x].attrib = 0;
    }
}

/* clear to end of screen */
void Terminal::term_wclreos()
{
    int x;

#ifdef DEBUG
    printf("term_wclreos\n");
#endif

    for (x = win->cursor_x + (win->cursor_y * win->ws_conf.ws_col);
         x < (win->ws_conf.ws_row * win->ws_conf.ws_col); x++)
    {
        win->chars[x].text =
            win->chars[x].col =
                win->chars[x].attrib = 0;
    }
}

/* clear to beginning of screen */
void Terminal::term_wclrbos()
{
    int x;

#ifdef DEBUG
    printf("term_wclrbos\n");
#endif

    for (x = win->cursor_x + (win->cursor_y * win->ws_conf.ws_col); x > 0; x--)
    {
        win->chars[x].text =
            win->chars[x].col =
                win->chars[x].attrib = 0;
    }
}

/* clear entire line */
void Terminal::term_wclrel()
{
    int x;

#ifdef DEBUG
    printf("term_wclrel\n");
#endif

    for (x = 0; x < win->cursor_x; x++)
    {
        win->chars[x].text =
            win->chars[x].col =
                win->chars[x].attrib = 0;
    }
}

/* insert? */
void Terminal::term_winsline()
{
    int store;

#ifdef DEBUG
    printf("term_winsline\n");
#endif

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
#ifdef DEBUG
    printf("term_wdelline\n");
#endif

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
#ifdef DEBUG
    printf("term_wdelchar\n");
#endif

    for (i = win->cursor_x; i < win->ws_conf.ws_col - 1; i++)
    {
        win->chars[i + (win->cursor_y * win->ws_conf.ws_col)].text = win->chars[i + (win->cursor_y * win->ws_conf.ws_col) + 1].text;
    }
    win->chars[i + (win->cursor_y * win->ws_conf.ws_col)].col = win->chars[i + (win->cursor_y * win->ws_conf.ws_col) + 1].col;
    win->chars[i + (win->cursor_y * win->ws_conf.ws_col)].attrib = win->chars[i + (win->cursor_y * win->ws_conf.ws_col) + 1].attrib;

    win->chars[win->ws_conf.ws_col + (win->cursor_y * win->ws_conf.ws_col) - 1].text = 0;
    win->chars[win->ws_conf.ws_col + (win->cursor_y * win->ws_conf.ws_col) - 1].col = 0;
    win->chars[win->ws_conf.ws_col + (win->cursor_y * win->ws_conf.ws_row) - 1].attrib = 0;
}

/* clear characters */
void Terminal::term_wclrch(int n)
{
    printf("wclrch\n");
    n = n;
}

/* set cursor type */
void Terminal::term_wcursor(int type)
{
#ifdef DEBUG
    printf("term_wcursor\n");
#endif

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
void Terminal::term_wputs(const unsigned char *s)
{
#ifdef DEBUG
    printf("term_wputs %s\n", s);
#endif
    unsigned char d;
    for (;; )
    {
        if ((d = *s++) == 0)
        {
            break;
        }
        char_out((char )d);
    }
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
    pwin = win;
    pwin->ws_conf.ws_col = w;
    pwin->ws_conf.ws_row = h;
    pwin->chars = (char_t *)malloc(w * h * sizeof(char_t));
    memset(pwin->chars, 0, w * h * sizeof(char_t));
    pwin->cursor_x = 0;
    pwin->cursor_y = 0;
    pwin->dirty = true;
    pwin->sy1 = 0;
    pwin->sy2 = h - 1;

    pwin->color = 0;
    pwin->attr = 0;

    vt_init(ANSI, 0, 0, 1, 1);
}

void Terminal::resize_term(int w, int h)
{
    term_t *pwin = (term_t *)win;
    int x, y;
    char_t *old_chars;
    unsigned char text, col, attrib;

    old_chars = pwin->chars;
    pwin->chars = (char_t *)malloc(w * h * sizeof(char_t));

    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            if ((x < pwin->ws_conf.ws_col) &&
                (y < pwin->ws_conf.ws_row))
            {
                text =   old_chars[x + (y * pwin->ws_conf.ws_col)].text;
                col =    old_chars[x + (y * pwin->ws_conf.ws_col)].col;
                attrib = old_chars[x + (y * pwin->ws_conf.ws_col)].attrib;
            }
            else
            {
                text = 0;
                col = 0;
                attrib = 0;
            }
            pwin->chars[x + (y * w)].text = text;
            pwin->chars[x +  (y * w)].col = col;
            pwin->chars[x + (y * w)].attrib = attrib;
        }
    }

    pwin->ws_conf.ws_col = w;
    pwin->ws_conf.ws_row = h;
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
}

