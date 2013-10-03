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

SelfListener::SelfListener(int a, QObject *parent) :
    QThread(parent) {
    _a = a;
}
#include <fstream>

void SelfListener::run()
{
#if 0
    char buf[4096];
    int len;
    bool running = true;
    while(running) {
         while((len = ::read(_a, buf, 4096)) > 0) {
            buf[len] = 0; // Just in case.
            emit recvData(buf, len);
            msleep(30);
         }
         if(len < 0)
           running = false;
    }
#else
    char buf[4096];
    int len;
    bool running = true;
    static int x = 0;
    len = sprintf(buf, "start");
    emit recvData(buf, len);
    std::ifstream _inputFile;
    _inputFile.open("C:/Users/ChrisD/software_devel/ComBomb-build-Desktop_Qt_5_0_1_MSVC2010_32bit-Debug/test.cbd", std::ifstream::in | std::ifstream::binary);
    while(running)
    {
        _inputFile.read(buf, sizeof(buf));
        len = (size_t)_inputFile.gcount();
        emit recvData(buf, len);
        if (x++ > 100000)
        {
            running = false;
        }
    }

#endif
}
