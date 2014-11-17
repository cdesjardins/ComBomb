/*
    ComBomb - Terminal emulator
    Copyright (C) 2014  Chris Desjardins
    http://blog.chrisd.info cjd@chrisd.info

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

#include <boost/lexical_cast.hpp>
#include "versioning.h"
#include "v.h"

const char* getVersion()
{
    return CB_GIT_VER_STR;
}

int32_t parseVersionStr(std::string verStr)
{
    int32_t ret = -1;
    if (verStr[0] == 'v')
    {
        std::string text = verStr.substr(1);
        size_t dashIndex = text.find('-');
        if (dashIndex != std::string::npos)
        {
            text = text.substr(0, dashIndex);
        }
        size_t dotIndex = text.find('.');
        if (dotIndex != std::string::npos)
        {
            try
            {
                int32_t major = boost::lexical_cast<int32_t>(text.substr(0, dotIndex));
                int32_t minor = boost::lexical_cast<int32_t>(text.substr(dotIndex + 1));
                ret = ((major << 16) | minor);
            }
            catch (const boost::bad_lexical_cast &)
            {
            }
        }
    }
    return ret;
}

