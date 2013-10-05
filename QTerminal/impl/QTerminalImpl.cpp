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

#include "QTerminal/TgtIntf.h"
#include <QDebug>

#include "QTerminalImpl.h"

QTerminalImpl::QTerminalImpl(const boost::shared_ptr<TgtIntf> &targetInterface, QWidget* parent)
    : QTerminalInterface(parent)
{
    setMinimumSize(600, 400);
    initialize(targetInterface);
}

void QTerminalImpl::initialize(const boost::shared_ptr<TgtIntf> &targetInterface)
{
    m_terminalView = new TerminalView(this);
    m_terminalView->setKeyboardCursorShape(TerminalView::UnderlineCursor);
    m_terminalView->setBlinkingCursor(true);
    m_terminalView->setBellMode(TerminalView::NotifyBell);
    m_terminalView->setTerminalSizeHint(true);
    m_terminalView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_terminalView->setTripleClickMode(TerminalView::SelectWholeLine);
    m_terminalView->setTerminalSizeStartup(true);
    m_terminalView->setScrollBarPosition(TerminalView::ScrollBarRight);

    connect(m_terminalView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(handleCustomContextMenuRequested(QPoint)));

#ifdef Q_OS_MAC
    QFont font = QFont("Monaco");
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(11);
#else
    QFont font = QFont("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(10);
#endif
    setTerminalFont(font);
    setFocusProxy(m_terminalView);
    setFocus(Qt::OtherFocusReason);

    m_terminalModel = new TerminalModel(targetInterface);
    m_terminalModel->setAutoClose(true);
    m_terminalModel->setCodec(QTextCodec::codecForName("UTF-8"));
    m_terminalModel->setHistoryType(HistoryTypeBuffer(100000));
    m_terminalModel->setDarkBackground(true);
    m_terminalModel->setKeyBindings("");
    m_terminalModel->run();
    m_terminalView->setSize(80, 25);
    m_terminalModel->addView(m_terminalView);
}

QTerminalImpl::~QTerminalImpl()
{
    emit destroyed();
}

void QTerminalImpl::setTerminalFont(const QFont &font)
{
    if (!m_terminalView)
    {
        return;
    }
    m_terminalView->setVTFont(font);
}

void QTerminalImpl::setSize(int h, int v)
{
    if (!m_terminalView)
    {
        return;
    }
    m_terminalView->setSize(h, v);
}

void QTerminalImpl::sendText(const QString& text)
{
    m_terminalModel->sendText(text);
}

QSize QTerminalImpl::sizeHint() const
{
    return m_terminalView->sizeHint();
}

void QTerminalImpl::setCursorType(CursorType type, bool blinking)
{
    switch (type)
    {
        case UnderlineCursor: m_terminalView->setKeyboardCursorShape(TerminalView::UnderlineCursor); break;
        case BlockCursor: m_terminalView->setKeyboardCursorShape(TerminalView::BlockCursor); break;
        case IBeamCursor: m_terminalView->setKeyboardCursorShape(TerminalView::IBeamCursor); break;
    }
    m_terminalView->setBlinkingCursor(blinking);
}

void QTerminalImpl::focusInEvent(QFocusEvent* focusEvent)
{
    Q_UNUSED(focusEvent);
    m_terminalView->updateImage();
    m_terminalView->repaint();
    m_terminalView->update();
}

void QTerminalImpl::showEvent(QShowEvent*)
{
    m_terminalView->updateImage();
    m_terminalView->repaint();
    m_terminalView->update();
}

void QTerminalImpl::resizeEvent(QResizeEvent*)
{
    m_terminalView->resize(this->size());
    m_terminalView->updateImage();
    m_terminalView->repaint();
    m_terminalView->update();
}

void QTerminalImpl::copyClipboard()
{
    m_terminalView->copyClipboard();
}

void QTerminalImpl::pasteClipboard()
{
    m_terminalView->pasteClipboard();
}

