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
    _targetInterface(targetInterface)
{
}

void SelfListener::run()
{
    boost::asio::mutable_buffer b;

    bool running = true;
    while (running)
    {
        int bytes = _targetInterface->TgtRead(b);
        if (bytes > 0)
        {
            const char* data = boost::asio::buffer_cast<const char*>(b);
            emit recvData(data, bytes);
            _targetInterface->TgtReturnReadBuffer(b);
#ifdef OUT_TO_DEBUG_FILE
            _debugFile.write(data, bytes);
#endif
        }
    }
}

