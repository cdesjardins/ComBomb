/*
    ComBomb - Terminal emulator
    Copyright (C) 2026  Chris Desjardins
    https://github.com/cdesjardins/ComBomb cjd@chrisd.info

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "../CBErrorMessage.h"
#include <boost/system/system_error.hpp>
#include <string>

QString cbErrorMessage(const std::exception& e)
{
    std::string msg(e.what());
    const boost::system::system_error* sysErr = dynamic_cast<const boost::system::system_error*>(&e);
    if (sysErr != nullptr)
    {
        // A boost::system::system_error what() is an optional prefix followed
        // by error_code::what(), which is the message plus a bracketed
        // "category:value at file:line:col in function '...'". Swap that tail
        // for the bare message and the prefix (such as "open") is kept.
        const std::string decorated(sysErr->code().what());
        const std::string::size_type pos = msg.rfind(decorated);
        if (pos != std::string::npos)
        {
            msg.replace(pos, decorated.size(), sysErr->code().message());
        }
    }
    return QString::fromStdString(msg);
}
