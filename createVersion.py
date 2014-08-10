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
        f.write("/* This is a generated file, do not change, see vergen.py   */\n")
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
