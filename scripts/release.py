#!/usr/bin/env python
import sys, platform, shutil, os.path, tarfile
from subprocess import Popen, PIPE

linuxSrcExeFile = "../build-ComBomb-Desktop_Qt_5_1_1_GCC_64bit-Release/ComBombGui/ComBombGui"
linuxDstExeFile = os.path.expanduser("~/Dropbox/ComBomb/lin/")

def copyExeFile():
    if (platform.system() == "Windows"):
        pass
    elif (platform.system() == "Linux"):
        shutil.copy(linuxSrcExeFile, linuxDstExeFile)
    else:
        print("Unsupported platform")
        exit(1)

def compressFilesWindows():
    pass

def compressFilesLinux(version):
    filename = "ComBomb-" + version + ".tar.gz"
    file = tarfile.open(filename, "w:gz")
    file.add(linuxDstExeFile, arcname="ComBomb")
    return filename
    
def getVersion():
    process = Popen(["git", "describe", "--dirty", "--always"], stdout=PIPE)
    gitVerStr = process.communicate()[0].strip()
    if ("dirty" in gitVerStr):
        print("Code base is dirty")
        exit(1)
    gitVerStr = gitVerStr[0:gitVerStr.rfind("-")]
    return gitVerStr

def compressFiles(version):
    filename = ""
    if (platform.system() == "Windows"):
        filename = compressFilesWindows(version)
    elif (platform.system() == "Linux"):
        filename = compressFilesLinux(version)
    else:
        print("Unsupported platform")
        exit(1)
    return filename

def main(argv):
    copyExeFile()
    version = getVersion()
    compressFiles(version)

if __name__ == "__main__":
    main(sys.argv[1:])

