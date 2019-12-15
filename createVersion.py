#!/usr/bin/env python
#
#   ComBomb - Terminal emulator
#   Copyright (C) 2014  Chris Desjardins
#   http://blog.chrisd.info cjd@chrisd.info
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import sys, os
import traceback
from subprocess import Popen, PIPE

def touch(fname, times=None):
    with file(fname, 'a'):
        os.utime(fname, times)
        
class CreateVer:
    def __init__(self):
        pass

    def getVerStr(self):
        process = Popen(["git", "describe", "--dirty", "--always"], stdout=PIPE)
        return process.communicate()[0].strip()
        
    def run(self):
        gitVerStr = self.getVerStr()
        f = open("v.h", "wb")
        f.write(b"""/*
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
""");
        f.write(b"/* This is a generated file, do not change, see " + str.encode(os.path.basename(__file__)))
        f.write(b"   */\n#define CB_GIT_VER_STR \"" + gitVerStr + b"\"\n")
        f.close()
        return gitVerStr


if __name__ == "__main__":
    try:
        CreateVer = CreateVer()
        CreateVer.run();
    except Exception as e:
        print(e)
        traceback.print_tb(e.__traceback__)
        print("Check to make sure git is in the path")
