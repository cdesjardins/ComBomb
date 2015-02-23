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

#include "../TgtIntf.h"
#include <QDebug>
#include <QApplication>
#include "QTerminalImpl.h"
#include "BackTabEvent.h"
#include "KeyboardTranslator.h"

QTerminalImpl::QTerminalImpl(const QTerminalConfig& terminalConfig, const std::shared_ptr<TgtIntf>& targetInterface, int width, int height, QWidget* parent)
    : QTerminalInterface(parent)
{
    setMinimumSize(600, 400);
    initialize(terminalConfig, targetInterface, width, height);
}

void QTerminalImpl::initialize(const QTerminalConfig& terminalConfig, const std::shared_ptr<TgtIntf>& targetInterface, int width, int height)
{
    _terminalView.reset(new TerminalView(this));
    _terminalView->setKeyboardCursorShape(TerminalView::IBeamCursor);
    _terminalView->setBlinkingCursor(true);
    _terminalView->setBellMode(TerminalView::NotifyBell);
    _terminalView->setTerminalSizeHint(true);
    _terminalView->setContextMenuPolicy(Qt::CustomContextMenu);
    _terminalView->setTripleClickMode(TerminalView::SelectWholeLine);
    _terminalView->setTerminalSizeStartup(true);
    _terminalView->setScrollBarPosition(TerminalView::ScrollBarRight);

    connect(_terminalView.get(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(handleCustomContextMenuRequested(QPoint)));

    setFocusProxy(_terminalView.get());

    _terminalModel.reset(new TerminalModel(targetInterface));
    _terminalModel->setCodec(QTextCodec::codecForName("UTF-8"));
    _terminalModel->setDarkBackground(true);
    _terminalModel->setKeyBindings("");
    _terminalModel->run();
    _terminalModel->addView(_terminalView);

    // Set the screen size after font and everything else is setup
    _terminalView->setSize(width, height);
    applyTerminalConfig(terminalConfig);
    _terminalView->installEventFilter(this);
}

bool QTerminalImpl::eventFilter(QObject*, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Backtab)
        {
            // Absorb backtab and post a new backtab event.
            QApplication::postEvent(_terminalView.get(), new SendBackTabEvent());
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
    _terminalModel.reset();
    _terminalView.reset();
}

void QTerminalImpl::connectToRecvText(QObject* who)
{
    _terminalModel->connectToRecvText(who);
}

void QTerminalImpl::applyTerminalConfig(const QTerminalConfig& terminalConfig)
{
    _terminalView->setWordCharacters(terminalConfig._wordSelectionDelimiters);
    setTerminalFont(terminalConfig._font);
}

void QTerminalImpl::setTerminalFont(const QFont& font)
{
    if (!_terminalView)
    {
        return;
    }
    _terminalView->setVTFont(font);
}

void QTerminalImpl::setSize(int h, int v)
{
    if (!_terminalView)
    {
        return;
    }
    _terminalView->setSize(h, v);
}

void QTerminalImpl::sendText(const QString& text)
{
    _terminalModel->sendText(text);
}

void QTerminalImpl::sendText(const QByteArray& text)
{
    _terminalModel->sendText(text);
}

void QTerminalImpl::recvText(const QByteArray& data)
{
    _terminalModel->receiveBlock(data);
}

void QTerminalImpl::findText(const QString& searchStr, const bool caseSensitive, const bool searchUp, const bool cont)
{
    _terminalView->findText(searchStr, caseSensitive, searchUp, cont);
}

QString QTerminalImpl::findTextHighlighted(const bool caseSensitive)
{
    return _terminalView->findTextHighlighted(caseSensitive);
}

QSize QTerminalImpl::sizeHint() const
{
    return _terminalView->sizeHint();
}

void QTerminalImpl::suppressOutput(bool suppress)
{
    _terminalModel->suppressOutput(suppress);
}

void QTerminalImpl::setCursorType(CursorType type, bool blinking)
{
    switch (type)
    {
        case UnderlineCursor:
            _terminalView->setKeyboardCursorShape(TerminalView::UnderlineCursor);
            break;

        case BlockCursor:
            _terminalView->setKeyboardCursorShape(TerminalView::BlockCursor);
            break;

        case IBeamCursor:
            _terminalView->setKeyboardCursorShape(TerminalView::IBeamCursor);
            break;
    }
    _terminalView->setBlinkingCursor(blinking);
}

void QTerminalImpl::focusInEvent(QFocusEvent* focusEvent)
{
    Q_UNUSED(focusEvent);
    _terminalView->updateImage();
    _terminalView->repaint();
    _terminalView->update();
}

void QTerminalImpl::showEvent(QShowEvent*)
{
    _terminalView->updateImage();
    _terminalView->repaint();
    _terminalView->update();
}

void QTerminalImpl::resizeEvent(QResizeEvent*)
{
    _terminalView->resize(this->size());
    _terminalView->updateImage();
    _terminalView->repaint();
    _terminalView->update();
}

void QTerminalImpl::copyClipboard()
{
    _terminalView->copyClipboard();
}

void QTerminalImpl::pasteClipboard()
{
    _terminalView->pasteClipboard();
}

void QTerminalImpl::newlineToggle()
{
    _terminalModel->newlineToggle();
}

bool QTerminalImpl::newlines()
{
    return _terminalModel->newlines();
}

void QTerminalImpl::selectAll()
{
    _terminalView->selectAll();
}

void QTerminalImpl::clearScrollback()
{
    _terminalModel->clearScreen();
    setTrackOutput(true);
}

void QTerminalImpl::setTrackOutput(bool trackOutput)
{
    _terminalView->setTrackOutput(trackOutput);
}

