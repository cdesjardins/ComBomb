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
#include <QApplication>
#include "QTerminalImpl.h"
#include "BackTabEvent.h"

QTerminalImpl::QTerminalImpl(const QTerminalConfig &terminalConfig, const boost::shared_ptr<TgtIntf> &targetInterface, int width, int height, QWidget* parent)
    : QTerminalInterface(parent)
{
    setMinimumSize(600, 400);
    initialize(terminalConfig, targetInterface, width, height);
}

void QTerminalImpl::initialize(const QTerminalConfig &terminalConfig, const boost::shared_ptr<TgtIntf> &targetInterface, int width, int height)
{
    m_terminalView.reset(new TerminalView(this));
    m_terminalView->setKeyboardCursorShape(TerminalView::IBeamCursor);
    m_terminalView->setBlinkingCursor(true);
    m_terminalView->setBellMode(TerminalView::NotifyBell);
    m_terminalView->setTerminalSizeHint(true);
    m_terminalView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_terminalView->setTripleClickMode(TerminalView::SelectWholeLine);
    m_terminalView->setTerminalSizeStartup(true);
    m_terminalView->setScrollBarPosition(TerminalView::ScrollBarRight);

    connect(m_terminalView.get(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(handleCustomContextMenuRequested(QPoint)));

    setFocusProxy(m_terminalView.get());

    m_terminalModel.reset(new TerminalModel(targetInterface));
    m_terminalModel->setCodec(QTextCodec::codecForName("UTF-8"));
    m_terminalModel->setHistoryType(HistoryTypeBuffer(100000));
    m_terminalModel->setDarkBackground(true);
    m_terminalModel->setKeyBindings("");
    m_terminalModel->run();
    m_terminalModel->addView(m_terminalView);

    // Set the screen size after font and everything else is setup
    m_terminalView->setSize(width, height);
    applyTerminalConfig(terminalConfig);
    m_terminalView->installEventFilter(this);
}

bool QTerminalImpl::eventFilter(QObject *, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Backtab)
        {
            // Absorb backtab and post a new backtab event.
            QApplication::postEvent(m_terminalView.get(), new SendBackTabEvent());
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

QTerminalImpl::~QTerminalImpl()
{
    emit destroyed();
    m_terminalModel.reset();
    m_terminalView.reset();
}

void QTerminalImpl::connectToRecvText(QObject *who)
{
    m_terminalModel->connectToRecvText(who);
}

void QTerminalImpl::applyTerminalConfig(const QTerminalConfig &terminalConfig)
{
    m_terminalView->setWordCharacters(terminalConfig._wordSelectionDelimiters);
    setTerminalFont(terminalConfig._font);
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

void QTerminalImpl::newlineToggle()
{
    m_terminalModel->newlineToggle();
}

void QTerminalImpl::clearScrollback()
{
    m_terminalModel->clearScreen();
}

