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
#ifndef Q_MOC_RUN
#include "boost/smart_ptr.hpp"
#endif

class TgtIntf;

class QTerminalImpl : public QTerminalInterface
{
    Q_OBJECT
public:
    QTerminalImpl(const boost::shared_ptr<TgtIntf> &targetInterface, QWidget* parent = 0);
    virtual ~QTerminalImpl();

    void setTerminalFont(const QFont &font);
    void setSize(int h, int v);
    void sendText(const QString& text);

    void setCursorType(CursorType type, bool blinking);

public slots:
    void copyClipboard();
    void pasteClipboard();

protected:
    void focusInEvent(QFocusEvent* focusEvent);
    void showEvent(QShowEvent*);
    virtual void resizeEvent(QResizeEvent*);
    QSize sizeHint() const;
private:
    void initialize(const boost::shared_ptr<TgtIntf> &targetInterface);

    TerminalView* m_terminalView;
    TerminalModel* m_terminalModel;
};

#endif // Q_UNIXTERMINALIMPL