#!/usr/bin/env python
import os, sys, platform
from subprocess import Popen

def runComBomb():
    execname = ""
    if (platform.system() == "Windows"):
        execname = "win/ComBombGui.exe"
    elif (platform.system() == "Linux"):
        execname = "lin/ComBombGui"
        cwd = os.getcwd()
        os.environ['LD_LIBRARY_PATH'] = cwd + "/lin"
    if (len(execname) > 0):
        p = Popen(execname)
    else:
        print("Unsupported platform")

if __name__ == "__main__":
    runComBomb()

