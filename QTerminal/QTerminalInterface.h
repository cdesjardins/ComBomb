/*

Copyright (C) 2012 Michael Goffioul.
Copyright (C) 2012 Jacob Dawid.

This file is part of QTerminal.

Foobar is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QTerminal is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef QTERMINALINTERFACE_H
#define QTERMINALINTERFACE_H

#include <QWidget>
#include <QMenu>
#include <QFontDatabase>

#define REPCHAR   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefgjijklmnopqrstuvwxyz0123456789./+@"

class QTerminalInterface : public QWidget
{
    Q_OBJECT
public:
    QTerminalInterface(QWidget* parent = 0) : QWidget(parent)
    {
        connect(this, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(handleCustomContextMenuRequested(QPoint)));

        setContextMenuPolicy(Qt::CustomContextMenu);

        _contextMenu = new QMenu(this);
        QAction* copyAction  = _contextMenu->addAction("Copy");
        QAction* pasteAction = _contextMenu->addAction("Paste");
        QAction* selectAllAction = _contextMenu->addAction("Select All");
        QAction* clearScrollbackAction = _contextMenu->addAction("Clear scrollback");
        _newlineAction = _contextMenu->addAction("Toggle CR/LF");
        QAction* runProcessAction = _contextMenu->addAction("Run Process");

        copyAction->setIcon(QIcon(":/images/page_copy.png"));
        pasteAction->setIcon(QIcon(":/images/page_paste.png"));
        selectAllAction->setIcon(QIcon(":/images/check_box.png"));
        clearScrollbackAction->setIcon(QIcon(":/images/page_refresh.png"));
        runProcessAction->setIcon(QIcon(":/images/script_gear.png"));

        _newlineAction->setCheckable(true);
        connect(copyAction, SIGNAL(triggered()), this, SLOT(copyClipboard()));
        connect(pasteAction, SIGNAL(triggered()), this, SLOT(pasteClipboard()));
        connect(selectAllAction, SIGNAL(triggered()), this, SLOT(selectAll()));
        connect(clearScrollbackAction, SIGNAL(triggered()), this, SLOT(clearScrollback()));
        connect(_newlineAction, SIGNAL(triggered()), this, SLOT(newlineToggle()));
        connect(runProcessAction, SIGNAL(triggered()), this, SLOT(runProcess()));

        connect(this, SIGNAL(triggerCopy()), this, SLOT(copyClipboard()));
        connect(this, SIGNAL(triggerPaste()), this, SLOT(pasteClipboard()));
    }

    virtual ~QTerminalInterface()
    {
    }

    virtual void setTerminalFont(const QFont& font) = 0;
    virtual void setSize(int h, int v) = 0;
    virtual void sendText(const QString& text) = 0;
    virtual void sendText(const QByteArray& text) = 0;

    enum CursorType
    {
        UnderlineCursor,
        BlockCursor,
        IBeamCursor
    };

    virtual void setCursorType(CursorType type, bool blinking)
    {
        // Provide empty default impl in order to avoid conflicts with the win impl.
        Q_UNUSED(type);
        Q_UNUSED(blinking);
    }

    static bool isFontFixed(const QFont& font)
    {
        QFontDatabase database;
        bool ret = database.isFixedPitch(font.family());
        if (ret == true)
        {
            QFontMetrics fm(font);
            int fw = fm.width(REPCHAR[0]);
            for (unsigned int i = 1; i < strlen(REPCHAR); i++)
            {
                if (fw != fm.width(REPCHAR[i]))
                {
                    ret = false;
                    break;
                }
            }
        }
        return ret;
    }

    static bool findAcceptableFontSizes(const QFont& font, QList<int>* sizes)
    {
        bool ret = false;

        if (isFontFixed(font) == true)
        {
            QFont f(font);
            for (int size = 6; size < 20; size++)
            {
                f.setPointSize(size);
                QFontMetrics fm = QFontMetrics(f);
                double dwidth = fontWidth(fm);
                int iwidth = fm.width(REPCHAR) / strlen(REPCHAR);
                if (dwidth == iwidth)
                {
                    ret = true;
                    // If sizes is null, then the caller doesn't care about the actual sizes
                    // just wants to know if it is an acceptable font.
                    if (sizes != NULL)
                    {
                        sizes->push_back(f.pointSize());
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
        return ret;
    }

    static double fontWidth(const QFontMetrics& fontMetrics)
    {
        return (double)fontMetrics.width(REPCHAR) / (double)strlen(REPCHAR);
    }

public slots:
    virtual void copyClipboard() = 0;
    virtual void pasteClipboard() = 0;
    virtual void newlineToggle() = 0;
    virtual bool newlines() = 0;
    virtual void runProcess() = 0;
    virtual void selectAll() = 0;
    virtual void clearScrollback() = 0;
    virtual void handleCustomContextMenuRequested(QPoint at)
    {
        _newlineAction->setChecked(newlines());
        _contextMenu->move(mapToGlobal(at));
        _contextMenu->show();
    }

signals:
    void triggerCopy();
    void triggerPaste();

private:
    QMenu* _contextMenu;
    QAction* _newlineAction;
};

#endif // QTERMINALINTERFACE_H
