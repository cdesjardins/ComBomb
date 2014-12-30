/*
    This file is part of Konsole, an X terminal.

    Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>
    Copyright (C) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>, Copyright (C)2008
    Copyright (C) 2012 Jacob Dawid <jacob.dawid@googlemail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

#ifndef TERMINALMODEL_H
#define TERMINALMODEL_H

// Qt
#include <QtCore/QStringList>
#include <QtCore>
#include <QWidget>

#include "SelfListener.h"
#include "QTerminal/TgtIntf.h"

// Konsole
#include "History.h"

class Emulation;
class TerminalView;

/**
 * Represents a terminal session consisting of a pseudo-teletype and a terminal emulation.
 * The pseudo-teletype (or PTY) handles I/O between the terminal process and Konsole.
 * The terminal emulation ( Emulation and subclasses ) processes the output stream from the
 * PTY and produces a character image which is then shown on views connected to the session.
 *
 * Each Session can be connected to one or more views by using the addView() method.
 * The attached views can then display output from the program running in the terminal
 * or send input to the program in the terminal in the form of keypresses and mouse
 * activity.
 */
class TerminalModel : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(QString keyBindings READ keyBindings WRITE setKeyBindings)
    Q_PROPERTY(QSize size READ size WRITE setSize)

    /**
     * Constructs a new session.
     *
     * To start the terminal process, call the run() method,
     * after specifying the program and arguments
     * using setProgram() and setArguments()
     *
     * If no program or arguments are specified explicitly, the Session
     * falls back to using the program specified in the SHELL environment
     * variable.
     */
    TerminalModel(const boost::shared_ptr<TgtIntf> &targetInterface);
    ~TerminalModel();

    /**
     * Sets the profile associated with this session.
     *
     * @param profileKey A key which can be used to obtain the current
     * profile settings from the SessionManager
     */
    void setProfileKey(const QString& profileKey);
    /**
     * Returns the profile key associated with this session.
     * This can be passed to the SessionManager to obtain the current
     * profile settings.
     */
    QString profileKey() const;

    /**
     * Adds a new view for this session.
     *
     * The viewing widget will display the output from the terminal and
     * input from the viewing widget (key presses, mouse activity etc.)
     * will be sent to the terminal.
     *
     * Views can be removed using removeView().  The session is automatically
     * closed when the last view is removed.
     */
    void addView(const boost::shared_ptr<TerminalView> &widget);
    /**
     * Clears the history store used by this session.
     */
    void clearHistory();
    void clearScreen();
    void suppressOutput(bool suppress);
    /**
     * Enables monitoring for activity in the session.
     * This will cause notifySessionState() to be emitted
     * with the NOTIFYACTIVITY state flag when output is
     * received from the terminal.
     */
    void setMonitorActivity(bool);
    /** Returns true if monitoring for activity is enabled. */
    bool isMonitorActivity() const;

    /**
     * Enables monitoring for silence in the session.
     * This will cause notifySessionState() to be emitted
     * with the NOTIFYSILENCE state flag when output is not
     * received from the terminal for a certain period of
     * time, specified with setMonitorSilenceSeconds()
     */
    void setMonitorSilence(bool);
    /**
     * Returns true if monitoring for inactivity (silence)
     * in the session is enabled.
     */
    bool isMonitorSilence()  const;
    /** See setMonitorSilence() */
    void setMonitorSilenceSeconds(int seconds);

    /**
     * Sets the key bindings used by this session.  The bindings
     * specify how input key sequences are translated into
     * the character stream which is sent to the terminal.
     *
     * @param id The name of the key bindings to use.  The
     * names of available key bindings can be determined using the
     * KeyboardTranslatorManager class.
     */
    void setKeyBindings(const QString& id);
    /** Returns the name of the key bindings used by this session. */
    QString keyBindings() const;

    /** Specifies whether a utmp entry should be created for the pty used by this session. */
    void setAddToUtmp(bool);

    /**
     * Sends @p text to the current foreground terminal program.
     */
    void sendText(const QString& text) const;
    void sendText(const QByteArray& text) const;
    void receiveBlock(const QByteArray& data);
    /** Returns the terminal session's window size in lines and columns. */
    QSize size();
    /**
     * Emits a request to resize the session to accommodate
     * the specified window size.
     *
     * @param size The size in lines and columns to request.
     */
    void setSize(const QSize& size);

    /** Sets the text codec used by this session's terminal emulation. */
    void setCodec(QTextCodec* codec);

    /**
     * Sets whether the session has a dark background or not.  The session
     * uses this information to set the COLORFGBG variable in the process's
     * environment, which allows the programs running in the terminal to determine
     * whether the background is light or dark and use appropriate colors by default.
     *
     * This has no effect once the session is running.
     */
    void setDarkBackground(bool darkBackground);
    /**
     * Returns true if the session has a dark background.
     * See setDarkBackground()
     */
    bool hasDarkBackground() const;

    /**
     * Attempts to get the shell program to redraw the current display area.
     * This can be used after clearing the screen, for example, to get the
     * shell to redraw the prompt line.
     */
    void refresh();
    void newlineToggle();
    bool newlines();
    void connectToRecvText(QObject* who);
    int lineCount();
protected:
    virtual void closeEvent(QCloseEvent* event);
public slots:

    /**
     * Starts the terminal session.
     *
     * This creates the terminal process and connects the teletype to it.
     */
    void run();

signals:

    /** Emitted when the terminal process starts. */
    void started();

    /**
     * Emitted when the terminal process exits.
     */
    void finished();

    /** Emitted when the session's title has changed. */
    void titleChanged();

    /** Emitted when the session's profile has changed. */
    void profileChanged(const QString& profile);

    /**
     * Emitted when the activity state of this session changes.
     *
     * @param state The new state of the session.  This may be one
     * of NOTIFYNORMAL, NOTIFYSILENCE or NOTIFYACTIVITY
     */
    void stateChanged(int state);

    /** Emitted when a bell event occurs in the session. */
    void bellRequest(const QString& message);

    /**
     * Requests that the color the text for any tabs associated with
     * this session should be changed;
     *
     * TODO: Document what the parameter does
     */
    void changeTabTextColorRequest(int);

    /**
     * Requests that the background color of views on this session
     * should be changed.
     */
    void changeBackgroundColorRequest(const QColor&);

    /** TODO: Document me. */
    void openUrlRequest(const QString& url);

    /**
     * Emitted when the terminal process requests a change
     * in the size of the terminal window.
     *
     * @param size The requested window size in terms of lines and columns.
     */
    void resizeRequest(const QSize& size);

    /**
     * Emitted when a profile change command is received from the terminal.
     *
     * @param text The text of the command.  This is a string of the form
     * "PropertyName=Value;PropertyName=Value ..."
     */
    void profileChangeCommandReceived(const QString& text);

private slots:
    void done(int);

    void onReceiveBlock(boost::intrusive_ptr<RefCntBuffer> incoming);
    void monitorTimerDone();

    void onViewSizeChange(int height, int width);
    void onEmulationSizeChange(int lines, int columns);

    void activityStateSet(int);

    void sendData(const char* buf, int len);

private:

    void updateTerminalSize();
    WId windowId() const;

    int _uniqueIdentifier;

    boost::scoped_ptr<Emulation> _emulation;

    QList<boost::shared_ptr<TerminalView> > _views;

    bool _monitorActivity;
    bool _monitorSilence;
    bool _notifiedActivity;
    bool _masterMode;
    boost::scoped_ptr<QTimer> _monitorTimer;

    int _silenceSeconds;

    bool _addToUtmp;
    bool _fullScripting;

    int _masterFd;
    int _slaveFd;

    boost::scoped_ptr<SelfListener> _selfListener;

    QColor _modifiedBackground; // as set by: echo -en '\033]11;Color\007

    QString _profileKey;

    bool _hasDarkBackground;
    boost::shared_ptr<TgtIntf> _targetInterface;
    bool _closed;
    bool _suppressOutput;
};

#endif // TERMINALMODEL_H
