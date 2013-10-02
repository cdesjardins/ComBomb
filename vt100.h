/*
 * vt100.h	Header file for the vt100 emulator.
 *
 *		$Id: vt100.h,v 1.4 2007-10-10 20:18:20 al-guest Exp $
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
 */

/* 05/09/2010 shoved head first into htpterm and flushed the handle-
 * sigflup
 *
 * 08/17/2012 Made more portable, also now we operate on a character buffer
 * sigflup
 */

#ifndef CJD_VT100_H
#define CJD_VT100_H

#include <deque>
#include "ThreadSafeQueue.h"
#ifndef Q_MOC_RUN
#include <boost/shared_array.hpp>
#include <boost/thread/mutex.hpp>
#endif
/* Keypad and cursor key modes. */
#define NORMAL          1
#define APPL            2

#define VT100           1
#define ANSI            3
#define RAW             4

struct state_t
{
    int vt_echo;
    int vt_nl_delay;    /* Delay after CR key */
    int vt_type; /* Terminal type. */
    int vt_wrap; /* Line wrap on/off */
    int vt_addlf; /* Add linefeed on/off */
    int vt_fg;  /* Standard foreground color. */
    int vt_bg;  /* Standard background color. */
    int vt_keypad;  /* Keypad mode. */
    int vt_cursor;  /* cursor key mode. */
    int vt_asis;    /* 8bit clean mode. */
    int vt_bs;  /* Code that backspace key sends. */
    int vt_insert; /* Insert mode */
    int vt_crlf;    /* Return sends CR/LF */
    int vt_om;  /* Origin mode. */
    int enable_iconv;
    int escparms[8];    /* Cumulated escape sequence. */
    int ptr;    /* Index into escparms array. */
    long vt_tabs[5];    /* Tab stops for max. 32*5 = 160 columns. */
    short newy1;
    short newy2;
    short savex, savey, saveattr, savecol;
};

/*
 * Possible attributes.
 */
#define XA_NORMAL       0
#define XA_BLINK        1
#define XA_BOLD         2
#define XA_REVERSE      4
#define XA_STANDOUT     8
#define XA_UNDERLINE    16
#define XA_ALTCHARSET   32
#define XA_BLANK        64

/*
 * Possible colors
 */
#define BLACK       0
#define RED         1
#define GREEN       2
#define YELLOW      3
#define BLUE        4
#define MAGENTA     5
#define CYAN        6
#define WHITE       7

#define COLATTR(fg, bg) (((fg) << 4) + (bg))
#define COLFG(ca)       ((ca) >> 4)
#define COLBG(ca)       ((ca) % 16)

/*
 * Possible borders.
 */
#define BNONE       0
#define BSINGLE     1
#define BDOUBLE     2

/*
 * Scrolling directions.
 */
#define S_UP        1
#define S_DOWN      2

/*
 * Cursor types.
 */
#define CNONE       0
#define CNORMAL     1

/*
 * Title Positions
 */
#define TLEFT       0
#define TMID        1
#define TRIGHT      2

/*
 * Allright, now the macro's for our keyboard routines.
 */

#define K_BS        8
#define K_CTRLC     3
#define K_CTRLZ     26
#define K_ESC       27
#define K_STOP      256
#define K_F1        257
#define K_F2        258
#define K_F3        259
#define K_F4        260
#define K_F5        261
#define K_F6        262
#define K_F7        263
#define K_F8        264
#define K_F9        265
#define K_F10       266
#define K_F11       277
#define K_F12       278

#define K_HOME      267
#define K_PGUP      268
#define K_UP        269
#define K_LT        270
#define K_RT        271
#define K_DN        272
#define K_END       273
#define K_PGDN      274
#define K_INS       275
#define K_DEL       276

#define NUM_KEYS    23
#define KEY_OFFS    256

/* Here's where the meta keycode start. (512 + keycode). */
#define K_META      512

#ifndef EOF
#  define EOF       ((int) -1)
#endif
#define K_ERA       '\b'
#define K_KILL      ((int) -2)


#define LEFT        0
#define RIGHT       1
#define UP          2
#define DOWN        3

#define BELL        7
#define KEY_BACKSPACE   8
#define QUIT        3

enum
{
    DEC_1,
    DEC_6,
    DEC_7,
    DEC_25,
    DEC_47,
    DEC_67,
    DEC_SIZE
};

struct char_t
{
    unsigned char text, col, attrib;
};

class row_t
{
public:
    row_t(int width)
        : _rowData(new char_t[width]),
        _rowDirty(true),
        _width(width)
    {
        clearChars();
    }
    void clearChars(int x = 0, int direction = 1);
    char_t getChar(int x)
    {
        return _rowData[x];
    }
    void setChar(int x, const char_t & ch)
    {
        _rowData[x] = ch;
    }
    void setChar(int x, const char text, const char col, const char attrib)
    {
        _rowData[x].text = text;
        _rowData[x].col = col;
        _rowData[x].attrib = attrib;
    }
    void setRowDirty(bool d)
    {
        _rowDirty = d;
    }
    bool isRowDirty()
    {
        return _rowDirty;
    }

protected:
    boost::shared_array<char_t> _rowData;
    bool _rowDirty;
    int _width;
};

struct winsize_t
{
  unsigned short ws_row;	/* rows, in characters */
  unsigned short ws_col;	/* columns, in characters */
  unsigned short ws_xpixel;	/* horizontal size, pixels */
  unsigned short ws_ypixel;	/* vertical size, pixels */
};

struct term_t
{
    int cursor_on;
    int dec_mode[DEC_SIZE];
    int color, attr;
    int cursor_x, cursor_y;
    int wrap, doscroll;
    int sy1, sy2;
    state_t state;
    winsize_t ws_conf;
    bool _winDirty;
};

class Terminal
{
public:
    void vt_out(unsigned int);
    void vt_send(unsigned int ch);
    Terminal(int w, int h);
    ~Terminal();

    char_t getChar(int x, int y);
    boost::shared_ptr<row_t> getRow(int y);
    unsigned short getWinSizeRow();
    unsigned short getWinSizeCol();
    bool isWinDirty();
    bool isRowDirty(int y);
    void resize_term(int w, int h);
    ThreadSafeQueue<boost::shared_ptr<row_t> > _scrollOffRows;
protected:
    virtual void char_out(const char c) = 0;
    virtual void str_out(const char *s) = 0;

    void setDirty(int y);
    void term_wscroll(int dir);
    void term_wlocate(int x, int y);
    void term_wputs(const char *s);
    void term_wclrch(int n);
    void term_wclreol();
    void term_wclrbol();
    void term_wclrel();
    void term_wclreos();
    void term_wclrbos();
    void term_winclr();
    void term_winsline();
    void term_wdelline();
    void term_wdelchar();
    void term_winschar(unsigned char c);
    void term_wcursor(int type);
    void term_wputc(unsigned char c);
    void term_wmove(int dir);
    void term_wredraw();
    void clearChars(int x, int y, int direction);
    /* Prototypes from vt100.c */

    void vt_init(int, int, int, int, int);
    void vt_pinit(int, int);
    void vt_set(int, int, int, int, int, int);

    void ansi_mode(int on_off);
    void dec_mode(int on_off);

    size_t one_mbtowc(char *pwc, char *s, size_t n);
    void v_termout(const char *s);

    void state1(int c);
    void state2(int c);
    void state3(int c);
    void state4(int c);
    void state5(int c);
    void state6(int c);
    void state7(int c);
    void state8(int c);
    void state9(int c);

    void term_wflush();

    term_t *win;
    std::deque<boost::shared_ptr<row_t> > _charRows;
    boost::mutex _charRowsMutex;
};

#endif
