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
#ifndef CB_TGT_FILE_CONNECTION_H
#define CB_TGT_FILE_CONNECTION_H

#include "QTerminal/TgtIntf.h"
#include <fstream>

class TgtFileIntf : public TgtIntf
{
public:
    struct TgtConnectionConfig : public TgtIntf::TgtConnectionConfigBase
    {
        TgtConnectionConfig(std::string fileName)
            : _fileName(fileName)
        {
        }

        TgtConnectionConfig()
        {
        }

        std::string _fileName;
    };

    static boost::shared_ptr<TgtFileIntf> createFileConnection(const boost::shared_ptr<const TgtConnectionConfig> &config);
    virtual ~TgtFileIntf(void);

    virtual int tgtBreakConnection();
    virtual int tgtRead(boost::intrusive_ptr<RefCntBuffer> &b);
    virtual bool tgtConnected();
    virtual void tgtGetTitle(std::string* szTitle);

protected:
    TgtFileIntf(const boost::shared_ptr<const TgtConnectionConfig> &config);
    virtual void tgtMakeConnection();

    std::ifstream _inputFile;
};

#endif
