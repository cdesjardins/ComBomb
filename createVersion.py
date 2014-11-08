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

#!/usr/bin/env python
import sys, os
from subprocess import Popen, PIPE

def touch(fname, times=None):
    with file(fname, 'a'):
        os.utime(fname, times)
        
class CreateVer:
    def __init__(self, argv):
        pass
        
    def run(self):
        process = Popen(["git", "describe", "--dirty", "--always"], stdout=PIPE, cwd="../../ComBomb/")
        gitVerStr = process.communicate()[0].strip()
        f = open("../../ComBomb/ComBombGui/v.h", "wb")
        f.write("/*\n\
    ComBomb - Terminal emulator\n\
    Copyright (C) 2014  Chris Desjardins\n\
    http://blog.chrisd.info cjd@chrisd.info\n\
\n\
    This program is free software: you can redistribute it and/or modify\n\
    it under the terms of the GNU General Public License as published by\n\
    the Free Software Foundation, either version 3 of the License, or\n\
    (at your option) any later version.\n\
\n\
    This program is distributed in the hope that it will be useful,\n\
    but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
    GNU General Public License for more details.\n\
\n\
    You should have received a copy of the GNU General Public License\n\
    along with this program.  If not, see <http://www.gnu.org/licenses/>.\n\
*/\n");
        f.write("/* This is a generated file, do not change, see " + os.path.basename(__file__) + "   */\n")
        f.write("#define CB_GIT_VER_STR \"" + gitVerStr + "\"\n")
        f.close()
        #touch("../../ComBomb/ComBombGui/versioning.h")
        return gitVerStr


if __name__ == "__main__":
    try:
        CreateVer = CreateVer(sys.argv[1:])
        CreateVer.run();
    except Exception as e:
        print e
        print "Check to make sure git is in the path"
