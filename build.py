#!/usr/bin/env python

import shutil, sys, os, platform
from subprocess import call

def rmerror(function, path, excinfo):
    exc_type, exc_value, exc_traceback = excinfo
    print exc_value
    os._exit(1)

def delBuildTree(delDir):
    retries = 0
    while (os.path.exists(delDir) == True):
        shutil.rmtree(delDir, False, rmerror)
        retries += 1
        if (retries > 10):
            break

def which(file):
    for path in os.environ["PATH"].split(":"):
        if os.path.exists(path + "/" + file):
                return path + "/" + file
    return None

def main(argv):
    delBuildTree("build")
    os.makedirs("build")
    os.chdir("build")
    print(which("qmake"))
    call(["qmake", ".."])
    if (platform.system() == "Windows"):
        call(["nmake"])
    else:
        call(["make", "-j"])

if __name__ == "__main__":
    main(sys.argv[1:])
