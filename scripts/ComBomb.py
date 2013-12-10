#!/usr/bin/env python
import os, sys, platform
from subprocess import Popen

def runComBomb():
    execname = ""
    cwd = os.path.dirname(os.path.realpath(__file__)) + "/"
    if (platform.system() == "Windows"):
        execname = "win/ComBombGui.exe"
    elif (platform.system() == "Linux"):
        execname = "lin/ComBombGui"
        os.environ['LD_LIBRARY_PATH'] = cwd + "lin"
        os.environ['LIBGL_ALWAYS_INDIRECT'] = "1"
    if (len(execname) > 0):
        p = Popen(cwd + execname)
    else:
        print("Unsupported platform")

if __name__ == "__main__":
    runComBomb()
