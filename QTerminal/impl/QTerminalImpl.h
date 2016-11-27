/*  Copyright (C) 2008 e_k (e_k@users.sourceforge.net)
    Copyright (C) 2012 Jacob Dawid <jacob.dawid@googlemail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef Q_UNIXTERMINALIMPL
#define Q_UNIXTERMINALIMPL

#include <QtGui>
#include "TerminalModel.h"
#include "TerminalView.h"
#include "../QTerminalInterface.h"
#include "../QTerminalConfig.h"

class TgtIntf;

class QTerminalImpl : public QTerminalInterface
{
    Q_OBJECT
public:
    QTerminalImpl(const QTerminalConfig& terminalConfig, const std::shared_ptr<TgtIntf>& targetInterface, int width,
                  int height, QWidget* parent = 0);
    virtual ~QTerminalImpl();

    void setTerminalFont(const QFont& font);
    void sendText(const QString& text);
    void sendText(const QByteArray& text);
    void recvText(const QByteArray& data);
    void findText(const QString& searchStr, const bool caseSensitive, const bool searchUp, const bool cont);
    QString findTextHighlighted(const bool caseSensitive);
    void connectToRecvText(QObject* who);

    void setCursorType(CursorType type, bool blinking);
    void applyTerminalConfig(const QTerminalConfig& terminalConfig);
    bool eventFilter(QObject*, QEvent* event);
    void suppressOutput(bool suppress);
    void setTrackOutput(bool trackOutput);
    bool startCapture(const QString& captureFilename, const bool append);
    void stopCapture();
public slots:
    void copyClipboard();
    void pasteClipboard();
    void newlineToggle();
    bool newlines();
    void selectAll();
    void clearScrollback();

protected:
    void focusInEvent(QFocusEvent* focusEvent);
    void showEvent(QShowEvent*);
    virtual void resizeEvent(QResizeEvent*);
    QSize sizeHint() const;
private:
    void initialize(const QTerminalConfig& terminalConfig, const std::shared_ptr<TgtIntf>& targetInterface, int width,
                    int height);

    std::shared_ptr<TerminalView> _terminalView;
    std::unique_ptr<TerminalModel> _terminalModel;
};

#endif // Q_UNIXTERMINALIMPL
