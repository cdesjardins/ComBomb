#!/usr/bin/env python
import sys, os
from subprocess import Popen, PIPE

class CreateVer:
    def __init__(self, argv):
        pass
        
    def run(self):
        print os.getcwd()
        process = Popen(["git", "describe", "--dirty", "--always"], stdout=PIPE, cwd="../../ComBomb/")
        gitVerStr = process.communicate()[0].strip()
        f = open("../../ComBomb/ComBombGui/versioning.h", "wb")
        f.write("/* This is a generated file, do not change, see vergen.py   */\n")
        f.write("/* Do not include this file directly, use include/version.h */\n")
        f.write("#define CB_GIT_VER_STR \"" + gitVerStr + "\"\n")
        f.close()


if __name__ == "__main__":
    try:
        CreateVer = CreateVer(sys.argv[1:])
        CreateVer.run();
    except Exception as e:
        print e
        print "Check to make sure git is in the path"
