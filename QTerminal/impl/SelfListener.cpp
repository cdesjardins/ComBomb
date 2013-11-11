/* qterminal - a terminal widget for Qt
 * Copyright (C) 2011 Jacob Dawid (jacob.dawid@googlemail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "SelfListener.h"

SelfListener::SelfListener(const boost::shared_ptr<TgtIntf> &targetInterface, QObject* parent) :
    QThread(parent),
    _targetInterface(targetInterface),
    _running(true)
{
}
/*
 * Move this to TgtIntf
 * Make TgtRead give a shared_ptr<buffer>
 * When a buffer is returned check the use count
 * and either garbage collect it right away, or
 * defer collection until the use count goes to 1
 */
SelfListener::~SelfListener()
{
    join();
}

void SelfListener::join()
{
    _running = false;
    wait();
    _targetInterface.reset();
}

void SelfListener::run()
{
    boost::asio::mutable_buffer b;

    while (_running)
    {
        int bytes = _targetInterface->tgtRead(b);
        if (bytes > 0)
        {
            const char* data = boost::asio::buffer_cast<const char*>(b);
            emit recvData(data, bytes);
            _targetInterface->tgtReturnReadBuffer(b);
#ifdef OUT_TO_DEBUG_FILE
            _debugFile.write(data, bytes);
#endif
        }
    }
}

